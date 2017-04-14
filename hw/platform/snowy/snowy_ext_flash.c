#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_fsmc.h"
#include "platform.h"

// base region
#define Bank1_NOR_ADDR ((uint32_t)0x60000000)


uint8_t _flash_test(void);
void _nor_enter_read_mode(uint32_t address);

void hw_flash_init(void)
{
    FMC_NORSRAMInitTypeDef FMC_NORSRAMInitStructure;
    FMC_NORSRAMTimingInitTypeDef p;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    // init the D4 Output
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // bootloader sets D4 high here
    GPIO_SetBits(GPIOD, GPIO_Pin_4);
    
    // init the B7 Output & AF
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // set B7 FSMC
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_FMC);

    // Init GPIOD
    for (uint8_t i = 0; i < 16; i++)
    {
        // pin2 is an input, so ignore
        if (i == 2)
            continue;
        
        // init the D0-D15 Output & AF
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Pin =  (uint16_t)i;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_Init(GPIOD, &GPIO_InitStructure);
        
        // set B7 FSMC
        GPIO_PinAFConfig(GPIOD, i, GPIO_AF_FMC);
    }
    
    // init D Set all as input/outputs
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin =  0xFFFB;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    
    // Init GPIOE
    for (uint8_t i = 2; i < 16; i++)
    {        
        // init the D0-D15 Output & AF
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Pin =  (uint16_t)i;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_Init(GPIOE, &GPIO_InitStructure);

        // set B7 FSMC
        GPIO_PinAFConfig(GPIOE, i, GPIO_AF_FMC);
    }

    // init E Set all as input/outputs
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin =  0xFFFC;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    GPIO_ResetBits(GPIOD, GPIO_Pin_4);
    delay_us(10);
    GPIO_SetBits(GPIOD, GPIO_Pin_4);
    delay_us(30);
    // ahb3enr
    
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);
        
    // clock config
    p.FMC_AddressSetupTime = 1;
    p.FMC_AddressHoldTime = 1;
    p.FMC_DataSetupTime = 3;
    p.FMC_BusTurnAroundDuration = 1;
    p.FMC_CLKDivision = 15;
    p.FMC_DataLatency = 15;
    p.FMC_AccessMode = FSMC_AccessMode_A;

    FMC_NORSRAMInitStructure.FMC_Bank = FMC_Bank1_NORSRAM1;
    FMC_NORSRAMInitStructure.FMC_DataAddressMux = FMC_DataAddressMux_Enable;
    FMC_NORSRAMInitStructure.FMC_MemoryType = FMC_MemoryType_NOR;
    FMC_NORSRAMInitStructure.FMC_MemoryDataWidth = FMC_NORSRAM_MemoryDataWidth_16b;
//     FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
//     FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
//     FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
//     FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
//     FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FMC_NORSRAMInitStructure.FMC_WriteOperation = FMC_WriteOperation_Enable;
    FMC_NORSRAMInitStructure.FMC_WaitSignal = FMC_WaitSignal_Enable;
//     FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
//     FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FMC_NORSRAMInitStructure.FMC_ReadWriteTimingStruct = &p;
    FMC_NORSRAMInitStructure.FMC_WriteTimingStruct = &p;

    FMC_NORSRAMInit(&FMC_NORSRAMInitStructure);
    FMC_NORSRAMCmd(FMC_Bank1_NORSRAM1, ENABLE);
    
    if (!_flash_test())
    {
        printf("FLASH FAILED!\n");
    }
    
    char buffer[128];
    _nor_enter_read_mode(2048);
    nor_read_bytes(buffer, 2048, 64);
    for (uint8_t i = 0; i < 128; i++)
    {
        printf("%c", buffer[i]);
    }
    printf("\n");
}

uint8_t _flash_test(void)
{
    uint16_t nr, nr1, nr2;
    uint8_t result;

    nor_write(0xAAA, 152);
    nr = nor_read16(0x20);
    nr1 = nor_read16(0x22);
    nr2 = nor_read16(0x24);
    printf("READR NR %d NR1 %d NR2 %d\n", nr, nr1, nr2);
    if ( nr != 81 || nr1 != 82 )
        result = 0;
    else
        result = (unsigned int)nr2 - 89 <= 0;
    
    nor_write(0xAAA, 0xF0);
    return result;
}

void _nor_enter_read_mode(uint32_t address)
{
    // CFI start read unlock
    nor_write(0xAAA, 0xAA);
    nor_write(0x554, 0x55);
    // unlock the address
    nor_write(address, 0xF0);
}

void nor_write(uint32_t address, uint16_t data)
{
     (*(volatile uint16_t *)(Bank1_NOR_ADDR + address) = (data));
}

uint16_t nor_read16(uint32_t address)
{
     return (*(volatile uint16_t *)(address));
}

void nor_read_bytes(uint32_t address, uint8_t *buffer, size_t length)
{
    _nor_enter_read_mode(address);
    for(uint32_t i = 0; i < length; i++)
    {
        buffer[i] = *(volatile uint8_t *)((Bank1_NOR_ADDR + address + i));
    }
}

uint32_t nor_read_word(uint32_t address)
{
    return (*(volatile uint32_t *)((Bank1_NOR_ADDR + address)));
}
