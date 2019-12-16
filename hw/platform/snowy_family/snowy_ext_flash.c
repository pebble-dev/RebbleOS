/* snowy_ext_flash.c
 * FMC NOR flash implementation for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_fsmc.h"
#include "platform.h"
#include "stm32_power.h"
#include "log.h"
#include "appmanager.h"
#include "flash.h"

#define FLASH_CMD_RESET              0x00F0
#define FLASH_CMD_AUTOSELECT         0x0090
#define FLASH_CMD_PROGRAM_SETUP      0x00A0
#define FLASH_CMD_WRITE_BUFFER_LOAD  0x0025
#define FLASH_CMD_WRITE_CONFIRM      0x0029
#define FLASH_CMD_READ_STATUS        0x0070
#define FLASH_CMD_ERASE_SETUP        0x0080
#define FLASH_CMD_SECTOR_ERASE       0x0030
#define FLASH_CMD_CHIP_ERASE         0x0010
#define FLASH_CMD_SUSPEND            0x00B0
#define FLASH_CMD_RESUME             0x0030
#define FLASH_CMD_UNLOCK_BYPASS      0x0020
#define FLASH_CMD_CFI_QUERY          0x0098

#define SECTOR_START(address) (address & 0xFFFF0000)
#define SECTOR_ADDRESS(sector) ((((sector) << 17) & 0xFFFF0000))
#define ALIGN(a) (a & 0xFFFFFFFE)
#define ADDR_SHIFT(Address)   (Bank1_NOR_ADDR + (2 * (Address)))

#define NOR_WRITE(Address, Data)   (*(__IO uint16_t *)(Address) = (Data))
#define NOR_WRITE8(Address, Data)   (*(__IO uint8_t *)(Address) = (Data))
#define NOR_READ(address, buf)  (buf = *(__IO uint16_t *)(address));

/* Timeout values for the various operations */
#define _timeout_block_erase    ((uint32_t)0x00A00000)
#define _timeout_chip_erase     ((uint32_t)0x30000000) 
#define _timeout_program        ((uint32_t)0x00001400)


/* Configure Logging */
#define MODULE_NAME "flash"
#define MODULE_TYPE "DRV"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

static void _nor_gpio_config(void);
static int _flash_test(void);

static void _nor_write16(uint32_t address, uint16_t data);
static void _nor_flash_command(uint16_t command);
static void _nor_enter_write_mode(void);

/* Setup the flash device geometry */
#define FLASH_SECTOR_SIZE 128 * 1024 /* 128Kb*/
#define FLASH_SECTOR_COUNT 128
#define FLASH_BANKS 8
/* this device can write 32 16bit words */
#define MAX_BYTES_PER_WRITE 64
#define BUS_WIDTH 2

/* Uncomment to enable SWD/JTAG semihosting */
// #define SEMIHOSTING_SUPPORT

/*
 * Initialise the flash hardware. 
 * it's NOR flash, using a multiplexed io
 */
void hw_flash_init(void)
{
    FMC_NORSRAMInitTypeDef fmc_nor_init_struct;
    FMC_NORSRAMTimingInitTypeDef p;
    
    LOG_INFO("Init");
    
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOD);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    
    _nor_gpio_config();
   
    // pull reset high while we setup the device
    // We the device in reset while we configure to stop glitching
    GPIO_SetBits(GPIOD, GPIO_Pin_4);

    // settled on these
    p.FMC_AddressSetupTime = 4;
    p.FMC_AddressHoldTime = 3;
    p.FMC_DataSetupTime = 7;
    p.FMC_BusTurnAroundDuration = 1;  // could be 3
    p.FMC_CLKDivision = 1;
    p.FMC_DataLatency = 0;
    p.FMC_AccessMode = FMC_AccessMode_A;
    
    /*p.FMC_AddressSetupTime = 1;
    p.FMC_AddressHoldTime = 1;
    p.FMC_DataSetupTime = 3;
    p.FMC_BusTurnAroundDuration = 1;  // could be 3
    p.FMC_CLKDivision = 15;
    p.FMC_DataLatency = 15;
    p.FMC_AccessMode = FMC_AccessMode_A;*/
    //p.FMC_AccessMode = FMC_AccessMode_B; could be this

    fmc_nor_init_struct.FMC_Bank = FMC_Bank1_NORSRAM1;
    fmc_nor_init_struct.FMC_DataAddressMux = FMC_DataAddressMux_Enable;
    fmc_nor_init_struct.FMC_MemoryType = FMC_MemoryType_NOR;
    fmc_nor_init_struct.FMC_MemoryDataWidth = FMC_NORSRAM_MemoryDataWidth_16b;
    
    fmc_nor_init_struct.FMC_BurstAccessMode = FMC_BurstAccessMode_Disable;
    fmc_nor_init_struct.FMC_AsynchronousWait = FMC_AsynchronousWait_Disable;
    fmc_nor_init_struct.FMC_WaitSignalPolarity = FMC_WaitSignalPolarity_Low;
    fmc_nor_init_struct.FMC_WrapMode = FMC_WrapMode_Disable;
    fmc_nor_init_struct.FMC_WaitSignalActive = FMC_WaitSignalActive_BeforeWaitState;
    
    fmc_nor_init_struct.FMC_WriteOperation = FMC_WriteOperation_Enable; // known good from bl
    fmc_nor_init_struct.FMC_WaitSignal = FMC_WaitSignal_Enable; // known good from bl
    
    fmc_nor_init_struct.FMC_ExtendedMode = FMC_ExtendedMode_Disable;
    fmc_nor_init_struct.FMC_WriteBurst = FMC_WriteBurst_Disable;
    
    fmc_nor_init_struct.FMC_ReadWriteTimingStruct = &p;
    fmc_nor_init_struct.FMC_WriteTimingStruct = &p;
    fmc_nor_init_struct.FMC_WriteOperation = FMC_WriteOperation_Enable;
    FMC_NORSRAMDeInit(FMC_Bank1_NORSRAM1);
    FMC_NORSRAMInit(&fmc_nor_init_struct);
    
    // release the flash chip
    GPIO_ResetBits(GPIOD, GPIO_Pin_4);
    delay_us(10);
    GPIO_SetBits(GPIOD, GPIO_Pin_4);
    delay_us(30);
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);

    FMC_NORSRAMCmd(FMC_Bank1_NORSRAM1, ENABLE); // Start disabled?. We'll turn it on when we need it
    
    //  let the flash initialise from the reset
    if (!_flash_test())
    {
        LOG_ERROR("Flash version check failed");
        // we carry on here, as it seems to work. TODO find unlock?
        //assert(!err);
    }

    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOD);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
}

void hw_flash_deinit(void)
{
}

static void _nor_gpio_config(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    /* We have the following known config on Snowy
     * S29VS128R flash controller
     * Using multiplexing mode which uses 
     * DA[15:0]
     * A[23:16] (might be 25:16)
     * D[15:0]
     * Also using B7 FMC mode
     * Ports D and E are almost entirely for FMC
     */

    // Common config
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd  = GPIO_PuPd_UP; 
    

    // Deal with B7  NADV
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_FMC);
    gpio_init_struct.GPIO_Pin = GPIO_Pin_7;  
    GPIO_Init(GPIOB, &gpio_init_struct);

    // GPIOs on port D
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);   // DA2
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);   // DA3
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_FMC);   // CLK
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FMC);   // NOE
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FMC);   // NWE
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_FMC);   // NWAIT
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FMC);   // NE1
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);   // DA13
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);   // DA14
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);  // DA15
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FMC);  // A16
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FMC);  // A17
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FMC);  // A18
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);  // DA0
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);  // DA1
    
    gpio_init_struct.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_3  | GPIO_Pin_4  | 
                                GPIO_Pin_5  | GPIO_Pin_6  | GPIO_Pin_7  | GPIO_Pin_8  |
                                GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 |
                                GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    
    GPIO_Init(GPIOD, &gpio_init_struct);
    
    // GPIO on port E
    // NBL0/1 are not used for this NOR flash
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_FMC);   // NBL0
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_FMC);   // NBL1
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource2, GPIO_AF_FMC);   // A23
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource3, GPIO_AF_FMC);   // A19
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_FMC);   // A20
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_FMC);   // A21
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_FMC);   // A22
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FMC);   // DA4
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FMC);   // DA5
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FMC);   // DA6
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FMC);  // DA7
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FMC);  // DA8
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FMC);  // DA9
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FMC);  // DA10
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FMC);  // DA11
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FMC);  // DA12
    
    gpio_init_struct.GPIO_Pin = GPIO_Pin_2  | GPIO_Pin_3  | 
                                GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_6  | GPIO_Pin_7  | 
                                GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | 
                                GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

    GPIO_Init(GPIOE, &gpio_init_struct);
}

static void _nor_clock_request(void)
{  
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOD);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
}

static void _nor_clock_release(void)
{
    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOD);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);   
}

/*
 * Issue a CFI command to the region we are reading to reset
 * the flash state machine for this region back to default
 */
void _nor_reset_region(uint32_t address)
{
    _nor_write16(address, FLASH_CMD_RESET);
}

/*
 * Issue a CFI command to reset the whole flash, resetting the state machine
 */
void _nor_reset_state(void)
{
    _nor_write16(0, FLASH_CMD_RESET);
}

/* Function to calculate x raised to the power y

    Time Complexity: O(n)
    Space Complexity: O(1)
    Algorithmic Paradigm: Divide and conquer.
*/
int power1(int x, unsigned int y)
{
    if (y == 0)
        return 1;
    else if ((y % 2) == 0)
        return power1 (x, y / 2) * power1 (x, y / 2);
    else
        return x * power1 (x, y / 2) * power1 (x, y / 2);

}

/*
 * Call for a test. Unlocks the CFI ID region and reads the QRY section
 * NOTE: seems wonky on real hardware. works in emu!
 */
static int _flash_test(void)
{
    
    uint16_t nr, nr1, nr2;
    uint8_t result;
    _nor_clock_request();

    _nor_reset_state();
    /* Write CFI command to enter ID region */
    _nor_flash_command(FLASH_CMD_CFI_QUERY);
    /* 0x20-0x24 are the "Query header QRY" */
    nr = hw_flash_read16(0x20);
    nr1 = hw_flash_read16(0x22);
    nr2 = hw_flash_read16(0x24);

    LOG_DEBUG("READR NR %c NR1 %c NR2 %c", nr, nr1, nr2);
    
    if ( nr != 81 || nr1 != 82 )
        result = 0;
    else
        result = (unsigned int)nr2 - 89 <= 0;
    
    nr2 = hw_flash_read16(0x4E);
    LOG_DEBUG("Device Size %d", power1(2, nr2));    
    
    /* Quit CFI ID mode */
    _nor_reset_region(0xAAA);
    
    _nor_enter_write_mode();
    NOR_WRITE(ADDR_SHIFT(0x0555), FLASH_CMD_AUTOSELECT);

    uint16_t mfgr = *(__IO uint16_t *) ADDR_SHIFT(0x0000);
    uint16_t dev1 = *(__IO uint16_t *) ADDR_SHIFT(0x0001);
    uint16_t dev2 = *(__IO uint16_t *) ADDR_SHIFT(0x000E);
    uint16_t dev3 = *(__IO uint16_t *) ADDR_SHIFT(0x000F);
    
    LOG_INFO("MFG %x  D1 %x D2 %x D3 %x", mfgr, dev1, dev2, dev3);
    
    _nor_clock_release();
    return result;
}

/*
 * Issue a CFI region write request and reset the flash state
 * XXX we really should be unlocking the region properly using CFI
 * http://www.cypress.com/file/218866/download Section 8.1
 * This allows us to hard lock pages in flash so they are not writeable. 
 */
static void _nor_enter_write_mode(void)
{
    NOR_WRITE(ADDR_SHIFT(0x555), 0x00AA);
    NOR_WRITE(ADDR_SHIFT(0x2AA), 0x0055);
}

static void _nor_flash_command(uint16_t command)
{
    NOR_WRITE(ADDR_SHIFT(0x555), command);
}
    
static void _nor_write16(uint32_t address, uint16_t data)
{
    _nor_clock_request();
     (*(__IO uint16_t *)(Bank1_NOR_ADDR + ALIGN(address)) = (data));
    _nor_clock_release();
}

/* Poll for the status of the operation. We wait for the banks All good (bit 7) to be set */
static uint32_t _nor_flash_get_status(uint32_t address, uint32_t timeout)
{ 
    uint16_t val1 = 0x00;
    uint32_t _timeout = timeout;
#define NOR_STATUS_OK      0
#define NOR_STATUS_TIMEOUT 1
#define NOR_STATUS_BUSY    2
    uint8_t status = NOR_STATUS_BUSY;

    while((timeout != 0x00) && (status != NOR_STATUS_OK))
    {
        timeout--;
        IWDG_ReloadCounter();

        /* Read the status register and see where we are at */
        NOR_WRITE(address + ADDR_SHIFT(0x555), FLASH_CMD_READ_STATUS);

        val1 = *(__IO uint16_t *)(Bank1_NOR_ADDR + ALIGN(address));

        if ((val1 & 0x80) == 0x80)
        {       
            if ((val1 & 0x20) == 0)
                return NOR_STATUS_OK;

            continue;
        }
        else
        {
            delay_us(10);
            continue;
        }
    }

    if (timeout == 0x00)
    {
        status = NOR_STATUS_TIMEOUT;
        LOG_ERROR("TIMEOUT");
    }

    return status;
}

uint8_t hw_flash_write_bytes(uint32_t address, uint8_t *buffer, size_t length)
{
    uint16_t *src = (uint16_t *)buffer;
    uint32_t aaaa = address;
    _nor_clock_request();
    length /= BUS_WIDTH; /* the incoming buffer and length is bytes. convert to shorts */
    
    LOG_DEBUG("write len %d addr %x", length,  (address));
    
    while(length > 0)
    {
        uint32_t start_address = address;
        uint32_t sector_address = SECTOR_START(ALIGN(address));
        
        uint32_t wlength = MAX_BYTES_PER_WRITE / BUS_WIDTH;
        
        /* XXX should probably check for writes crossing sector boundaries. 
           This isn't permitted and should be broken into multiple writes */
        
        if (length < wlength)
            wlength = length;
        
        /*LOG_INFO("chunk len %d clen %d addr %x sec %x", length, wlength, address, sector_address);*/
        
        /* unlock write mode */
        _nor_enter_write_mode();
        
        /* Set the mode to buffer load and set the controllers length to len-1 */
        NOR_WRITE(sector_address + ADDR_SHIFT(0x555), FLASH_CMD_WRITE_BUFFER_LOAD);
        NOR_WRITE(sector_address + ADDR_SHIFT(0x2AA), wlength - 1);
        
        /* Write the bytes out to the destination. This is using mem mapped FMC driver here */

        for(size_t i = 0; i < wlength; i++)
        {
            NOR_WRITE(Bank1_NOR_ADDR + address, *src);
            src++;
            address += BUS_WIDTH;
        }
        
        /* Tell the flash driver to commit the contents from buffer to flash */
        NOR_WRITE(sector_address + ADDR_SHIFT(0x555), FLASH_CMD_WRITE_CONFIRM);
     
        /* Poll for completion */
        _nor_flash_get_status(sector_address, _timeout_program);

#ifdef FLASHVERIFY
        for (int i = 0; i < wlength - 1; i++)
        {
            uint16_t val = 0;
            NOR_READ(Bank1_NOR_ADDR + start_address + (2*i), val);
            uint16_t srcval = *(src - wlength + i);
            if (val != srcval)
            {
                LOG_INFO("W VERIFY FAILED %d", val);
                _nor_clock_release();
                return -1;
            }
        }
#endif    
        length -= wlength;
    }
    
    _nor_reset_state();
    _nor_clock_release();
    //flash_operation_complete(ex);
    return 1;
}


/* Erase a sector at the given address */
int hw_flash_erase_address(uint32_t address)
{
    _nor_clock_request();
    LOG_INFO("erase %x %x sz %d\n", address, SECTOR_START(ALIGN(address)), FLASH_SECTOR_SIZE);

    /* unlock */
    _nor_enter_write_mode();
    
    NOR_WRITE(address + ADDR_SHIFT(0x555), FLASH_CMD_ERASE_SETUP);
    NOR_WRITE(address + ADDR_SHIFT(0x2AA), FLASH_CMD_SECTOR_ERASE);

    /* Takes a mo, so have a nap */
    delay_us(50);
    _nor_flash_get_status(address, _timeout_block_erase);
    
#ifdef FLASHVERIFY
    for (int i = 0; i < FLASH_SECTOR_SIZE/2; i++)
    {
        uint16_t val = 0;
        NOR_READ(Bank1_NOR_ADDR + address + (2*i), val);
        if (val != 0xFFFF)
        {
            LOG_INFO("VERIFY FAILED %d %x", 2*i, val);
            _nor_clock_release();
            while(1);
            return -1;
        }
    }
#endif
    _nor_clock_release();
    return 1;
}

uint16_t hw_flash_read16(uint32_t address)
{
    uint16_t rv;
    
    _nor_clock_request();
    NOR_READ(Bank1_NOR_ADDR + ALIGN(address), rv);
    _nor_clock_release();
    
    return rv;
}

void hw_flash_read_bytes(uint32_t address, uint8_t *buffer, size_t length)
{
    _nor_clock_request();
    for(size_t i = 0; i < length; i++)
    {
        NOR_READ(Bank1_NOR_ADDR + address + i, buffer[i]);
    }

    _nor_clock_release();
    flash_operation_complete(0);
}

#ifdef SEMIHOSTING_SUPPORT

__asm__(
"semihosting_syscall:\n"
"       bkpt #0xAB\n"
"       bx lr\n"
);

extern int semihosting_syscall(int c, const void *p);

#define MODE_RB 1
#define MODE_WB 5
static int _open(const char *s, int mode) {
    uint32_t args[3] = {(uint32_t) s, mode, strlen(s)};
    return semihosting_syscall(0x01, args);
}

static int _read(int fd, void *s, int n) {
    uint32_t args[3] = {fd, (uint32_t) s, n};
    return semihosting_syscall(0x06, args);
}

static int _close(int fd) {
    return semihosting_syscall(0x02, (void *)fd);
}

#define BUFLEN 4096

static uint8_t _readbuf[BUFLEN];

void hw_flash_backdoor_load(const char *fname, uint32_t start, uint32_t size) {
    LOG_INFO("*** Taking over the system in semihosting mode to write data -- hold on tight!\n");
    
    LOG_INFO("Step 0: opening %s...\n", fname);
    int fd = _open(fname, MODE_RB);
    if (fd < 0) {
        LOG_INFO("... resource pack open failed\n");
        return;
    }
    LOG_INFO("... fd %d\n", fd);
    
    uint32_t addr;
    LOG_INFO("Step 1: erasing region...\n");
    for (addr = start; addr < start + size; addr += FLASH_SECTOR_SIZE) {
        hw_flash_erase_address(addr);
        LOG_INFO(".");
    }
    
    LOG_INFO("Step 2: writing to flash...\n");
    
    int len;
    addr = start;
    while ((len = _read(fd, _readbuf, BUFLEN)) != BUFLEN) {
        len = BUFLEN - len; /* The standard returns the number of bytes *not* filled.  Excuse me? */
        if ((addr & 16383) == 0) LOG_INFO("...%ld...\n", addr - start);
        hw_flash_write_bytes(addr, _readbuf, len);
        addr += len;
    }
    
    LOG_INFO("...done; wrote %ld bytes\n", addr - start);
    
    _close(fd);
}

void hw_flash_backdoor_load_respack() {
    hw_flash_backdoor_load("build/snowy/res/snowy_res.pbpack", REGION_RES_START, REGION_RES_SIZE);
}

void hw_flash_backdoor_load_fs() {
    hw_flash_backdoor_load("fs.pbfs", REGION_FS_START, REGION_FS_N_PAGES * REGION_FS_PAGE_SIZE);
}


#endif
