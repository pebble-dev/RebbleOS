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
#define SHF(Address)   ((Address) << 1)
#if 0
#define NOR_WRITE(Address, Data)   do { printf("NOR + %08x <- %08x\n", (Address) - Bank1_NOR_ADDR, (Data)); *(__IO uint16_t *)(Address) = (Data); } while(0)
#else
#define NOR_WRITE(Address, Data)   do { *(__IO uint16_t *)(Address) = (Data); } while(0)
#endif
#define NOR_WRITE8(Address, Data)   (*(__IO uint8_t *)(Address) = (Data))

void _nor_gpio_config(void);
void _nor_enter_read_mode(uint32_t address);
void _nor_reset_region(uint32_t address);
void _nor_reset_state(void);
void _nor_clock_request(void);
void _nor_clock_release(void);
int _flash_test(void);

static void _nor_write16(uint32_t address, uint16_t data);
static void _nor_flash_command(uint16_t command);

/*
 * Initialise the flash hardware. 
 * it's NOR flash, using a multiplexed io
 */
void hw_flash_init(void)
{
    FMC_NORSRAMInitTypeDef fmc_nor_init_struct;
    FMC_NORSRAMTimingInitTypeDef p;
    
    DRV_LOG("Flash", APP_LOG_LEVEL_DEBUG, "Init");
    
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
        DRV_LOG("Flash", APP_LOG_LEVEL_ERROR, "Flash version check failed");
        // we carry on here, as it seems to work. TODO find unlock?
        //assert(!err);
    }
    /*
    hw_flash_erase_sector(32768);
        char buf[11] = "hello";
    hw_flash_write_bytes(0, buf, 5);
    
    hw_flash_write_bytes(2, buf, 5);
    while(1);
*/
    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOD);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
}

void hw_flash_deinit(void)
{
}

void _nor_gpio_config(void)
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

void _nor_clock_request(void)
{  
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOD);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
}

void _nor_clock_release(void)
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
inline void _nor_reset_region(uint32_t address)
{
    _nor_write16(address, FLASH_CMD_RESET);
}

/*
 * Issue a CFI command to reset the whole flash, resetting the state machine
 */
inline void _nor_reset_state(void)
{
    _nor_write16(0, FLASH_CMD_RESET);
}

/*
 * Call for a test. Unlocks the CFI ID region and reads the QRY section
 * NOTE: seems wonky on real hardware. works in emu!
 */
int _flash_test(void)
{
    
    uint16_t nr, nr1, nr2;
    uint8_t result;
    _nor_clock_request();

    _nor_reset_state();
    // Write CFI command to enter ID region
    _nor_flash_command(FLASH_CMD_CFI_QUERY);
    // 0x20-0x24 are the "Query header QRY"
    nr = hw_flash_read16(0x20);
    nr1 = hw_flash_read16(0x22);
    nr2 = hw_flash_read16(0x24);

    DRV_LOG("Flash", APP_LOG_LEVEL_DEBUG, "READR NR %c NR1 %c NR2 %c\n", nr, nr1, nr2);
    
    if ( nr != 81 || nr1 != 82 )
        result = 0;
    else
        result = (unsigned int)nr2 - 89 <= 0;
    
    // Quit CFI ID mode
    _nor_reset_region(0xAAA);
    
    _nor_clock_release();
    return result;
}

/*
 * Issue a CFI region write request and reset the flash state
 * XXX we really should be unlocking the region properly using CFI
 * http://www.cypress.com/file/218866/download Section 8.1
 * This allows us to hard lock pages in flash so they are not writeable. 
 */
void _nor_enter_write_mode(uint32_t address)
{
    // CFI start write unlock
    *(__IO uint16_t *)((Bank1_NOR_ADDR + SHF(0xAAA))) = 0xAAAA;
    *(__IO uint16_t *)((Bank1_NOR_ADDR + SHF(0x554))) = 0x5555;
    // unlock the address
//   _nor_reset_region(address);
}

static void _nor_flash_command(uint16_t command)
{
    _nor_clock_request();
    *(__IO uint16_t *)(Bank1_NOR_ADDR + SHF(0xAAA)) = command;
    _nor_clock_release();
}
    

static void _nor_write16(uint32_t address, uint16_t data)
{
    _nor_clock_request();
     (*(__IO uint16_t *)(Bank1_NOR_ADDR + ALIGN(address)) = (data));
    _nor_clock_release();
}

uint16_t hw_flash_read16(uint32_t address)
{
    uint16_t rv;
    
    _nor_clock_request();
    rv = *(__IO uint16_t *)(Bank1_NOR_ADDR + ALIGN(address));
    _nor_clock_release();
    
    return rv;
}

void hw_flash_read_bytes(uint32_t address, uint8_t *buffer, size_t length)
{
    _nor_clock_request();
    for(size_t i = 0; i < length; i++)
    {
        buffer[i] = *(__IO uint8_t *)((Bank1_NOR_ADDR + address + i));
    }

    _nor_clock_release();
    flash_operation_complete(0);
}

int hw_flash_write_sync(uint32_t address, uint8_t *buffer, size_t length)
{
    _nor_clock_request();
    uint8_t rv = 0;
    int start_align = address % 2;
    int end_align = (address + length) % 2;
    uint32_t addr_aligned = address & ~1;
    uint32_t len_padded = length + start_align + end_align;
    assert((len_padded & 1) == 0);
    
    uint32_t rem = len_padded;
    size_t bufpos = 0;
    while(rem > 0)
    {
        int pg_left = 64 - (addr_aligned % 64);
        int write_len = rem > 64 ? 64 : rem;
        
        if (pg_left - write_len > 64)
            write_len = pg_left;        
        
        if (write_len > pg_left)
            write_len = pg_left;
    
        NOR_WRITE(Bank1_NOR_ADDR + addr_aligned, FLASH_CMD_WRITE_BUFFER_LOAD);
        NOR_WRITE(Bank1_NOR_ADDR + addr_aligned +  SHF(0x2AA), write_len / 2 - 1);
        
        for(size_t i = 0; i < write_len; i+=2)
        {
            uint8_t v[2];
            v[0] = ((len_padded == (rem - i)) && start_align) ? 0xFF : buffer[bufpos++];
            v[1] = (((rem - i)        == 2) && end_align  ) ? 0xFF : buffer[bufpos++];
            NOR_WRITE(Bank1_NOR_ADDR + SHF((addr_aligned/2) + i/2),  *((uint16_t *)v));
        }       
        
        NOR_WRITE(Bank1_NOR_ADDR + SECTOR_START(addr_aligned) + SHF(0x555), FLASH_CMD_WRITE_CONFIRM);
        
        // We automatically wait for completion using the WAIT signal.
        (void)*(volatile uint16_t *)(Bank1_NOR_ADDR + SECTOR_START(addr_aligned));

        addr_aligned += write_len;
        rem -= write_len;
        _nor_reset_state();
    }
    
    _nor_clock_release();
    //flash_operation_complete(ex);
    return rv;
}

int hw_flash_erase_32k_sync(uint32_t address)
{
    _nor_clock_request();
    printf("erase %x %x\n", address, (address));

    NOR_WRITE(Bank1_NOR_ADDR + (address) + SHF(0x555), FLASH_CMD_ERASE_SETUP);
    NOR_WRITE(Bank1_NOR_ADDR + (address) + SHF(0x2AA), FLASH_CMD_SECTOR_ERASE);
        
    _nor_reset_state();

    _nor_clock_release();
    return 0;
}

int hw_flash_erase_sync(uint32_t addr, uint32_t len) {
    assert((addr & (32*1024 - 1)) == 0);
    assert((len & (32*1024 - 1)) == 0);

    while (len) {
        int rv = hw_flash_erase_32k_sync(addr);
        if (rv)
            return rv;
        addr += 32*1024;
        len -= 32*1024;
    }

    return 0;
}
