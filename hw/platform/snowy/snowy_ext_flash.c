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


// base region
#define Bank1_NOR_ADDR ((uint32_t)0x60000000)

void _nor_gpio_config(void);
void _nor_enter_read_mode(uint32_t address);
void _nor_reset_region(uint32_t address);
void _nor_reset_state(void);
int _flash_test(void);

static uint16_t _nor_read16(uint32_t address);
static void _nor_write16(uint32_t address, uint16_t data);

hw_driver_ext_flash_t _hw_flash_driver = {
    .common_info.module_name = "NOR Flash",
    .common_info.init = hw_flash_init,
    .common_info.deinit = hw_flash_deinit,
    .common_info.test = _flash_test,
    .read_bytes = hw_flash_read_bytes,
};

static hw_driver_handler_t *_handler;

void *hw_flash_module_init(hw_driver_handler_t *handler)
{
    _handler = handler;
    return &_hw_flash_driver;
}

/*
 * Initialise the flash hardware. 
 * it's NOR flash, using a multiplexed io
 */
void hw_flash_init(void)
{
    FMC_NORSRAMInitTypeDef fmc_nor_init_struct;
    FMC_NORSRAMTimingInitTypeDef p;
    
    
    // The bootloader toggles the reset manually before we init.
    // It didn't do anything below, so I assume it works. Also works without
    // bootloader sets D4 high here
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_OUT;
    // gpio_init_struct.GPIO_Pin =  GPIO_Pin_4;
    // gpio_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_100MHz;
    // gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    // GPIO_Init(GPIOD, &gpio_init_struct);
    // 
    // GPIO_SetBits(GPIOD, GPIO_Pin_4);
    _nor_gpio_config();
    
    //GPIO_ResetBits(GPIOD, GPIO_Pin_4);
    //delay_us(10);
    //GPIO_SetBits(GPIOD, GPIO_Pin_4);
    // let the flash initialise from the reset
    delay_ms(30);    
    
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
   
    // settled on these
    p.FMC_AddressSetupTime = 4;
    p.FMC_AddressHoldTime = 3;
    p.FMC_DataSetupTime = 7;
    p.FMC_BusTurnAroundDuration = 1;
    p.FMC_CLKDivision = 1;
    p.FMC_DataLatency = 0;
    p.FMC_AccessMode = FMC_AccessMode_A;

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

    FMC_NORSRAMDeInit(FMC_Bank1_NORSRAM1);
    FMC_NORSRAMInit(&fmc_nor_init_struct);
    FMC_NORSRAMCmd(FMC_Bank1_NORSRAM1, ENABLE);
        
    if (!_flash_test())
    {
        DRV_LOG("Flash", APP_LOG_LEVEL_ERROR, "Flash version check failed");
        // we carry on here, as it seems to work. TODO find unlock?
        //assert(!err);
    }
    
    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
}

void hw_flash_deinit(void)
{
}

void _nor_gpio_config(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOD);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    
    /* We have the following known config on Snowy
     * S29VS128R flash controller
     * Using multiplexing mode
     * Also using B7 FMC mode
     * Ports D and E are almost entirely for FMC
     */

    // Common config
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_PuPd  = GPIO_PuPd_NOPULL; 
    

    // Deal with B7
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_FMC);
    gpio_init_struct.GPIO_Pin = GPIO_Pin_7;  
    GPIO_Init(GPIOB, &gpio_init_struct);

    // GPIOs on port D
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);
    
    gpio_init_struct.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  |GPIO_Pin_3  | GPIO_Pin_4 | 
                                    GPIO_Pin_5  | GPIO_Pin_6  |GPIO_Pin_7  | GPIO_Pin_8 |
                                    GPIO_Pin_9  | GPIO_Pin_10 |GPIO_Pin_11 |GPIO_Pin_12 |
                                    GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    
    GPIO_Init(GPIOD, &gpio_init_struct);
    
    // GPIO on port E
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource2, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource3, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FMC); 
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FMC);
    
    gpio_init_struct.GPIO_Pin = GPIO_Pin_2  |GPIO_Pin_3  | GPIO_Pin_4 |GPIO_Pin_5  | 
                                    GPIO_Pin_6  |GPIO_Pin_7  | GPIO_Pin_8 |GPIO_Pin_9  | 
                                    GPIO_Pin_10 |GPIO_Pin_11 |GPIO_Pin_12 |GPIO_Pin_13 |
                                    GPIO_Pin_14 | GPIO_Pin_15;
                                    
    GPIO_Init(GPIOE, &gpio_init_struct);

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
    _nor_write16(address, 0xF0);
}

/*
 * Issue a CFI command to reset the whole flash, resetting the state machine
 */
inline void _nor_reset_state(void)
{
    _nor_write16(0, 0xF0);
}

/*
 * Call for a test. Unlocks the CFI ID region and reads the QRY section
 * NOTE: seems wonky on real hardware. works in emu!
 */
int _flash_test(void)
{
    uint16_t nr, nr1, nr2;
    uint8_t result;
    
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);

    _nor_reset_state();
    // Write CFI command to enter ID region
    _nor_write16(0xAAA, 0x98);
    // 0x20-0x24 are the "Qrery header QRY"
    nr = _nor_read16(0x20);
    nr1 = _nor_read16(0x22);
    nr2 = _nor_read16(0x24);

    DRV_LOG("Flash", APP_LOG_LEVEL_DEBUG, "READR NR %d NR1 %d NR2 %d\n", nr, nr1, nr2);
    
    if ( nr != 81 || nr1 != 82 )
        result = 0;
    else
        result = (unsigned int)nr2 - 89 <= 0;
    
    // Quit CFI ID mode
    _nor_reset_region(0xAAA);
    
    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);

    return result;
}

/*
 * Issue a CFI region read request and reset the flash state
 */
void _nor_enter_read_mode(uint32_t address)
{
    // CFI start read unlock
    _nor_write16(0xAAA, 0xAA);
    _nor_write16(0x554, 0x55);
    // unlock the address
    _nor_reset_region(address);
}

static void _nor_write16(uint32_t address, uint16_t data)
{
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);

     (*(__IO uint16_t *)(Bank1_NOR_ADDR + address) = (data));
     
    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
}

static uint16_t _nor_read16(uint32_t address)
{
    uint16_t rv;
    
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);

    _nor_enter_read_mode(address);

    rv = *(__IO uint16_t *)(Bank1_NOR_ADDR + address);
    
    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
    
    return rv;
}

void hw_flash_read_bytes(uint32_t address, uint8_t *buffer, size_t length)
{
    stm32_power_request(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);

    _nor_enter_read_mode(address);
    for(uint32_t i = 0; i < length; i++)
    {
        buffer[i] = *(__IO uint8_t *)((Bank1_NOR_ADDR + address + i));
    }
    _nor_reset_region(0xAAA);

    stm32_power_release(STM32_POWER_AHB3, RCC_AHB3Periph_FMC);
}

