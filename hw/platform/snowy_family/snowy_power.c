/* snowy_power.c
 * MAX14690 PMIC management routines for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
/*
 * Battery management us done using the MAX14690 PMIC. It is connected to I2C bus 1
 * It has a status register for the charge mode.
 * It has an interrupt pin (TODO where is this? is it connected?) for status change notifications 
 * this is stubbed out in this code.
 * 
 * Battery level is read over ADC. This is connected to PA2
 * 
 * The PMIC is programmed via register 0x19 to set MON battery output and voltage divider.
 * 
 * TODO: ADC common it. Who does this. ADC Mutex (becuase of ambience etc)
 *  consider rolling all ADC common functions into main.c? or an stm_adc driver?
 * 
 */

#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "snowy_power.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_tim.h>
#include "stm32_i2c.h"
#include "stm32_power.h"
#include "log.h"
#include "power.h"

/* Setup the power pin ADC and port */
#define PWR_BAT_PIN          GPIO_Pin_1
#define PWR_BAT_PORT         GPIOA
#define PWR_BAT_ADC_CHANNEL  1

/* min and max recorded mV for the battery 
 *  (3:1 div, raw adc value given) */
#define BAT_MV_MAX     3500
#define BAT_MV_MIN     2550

static const stm32_i2c_conf_t i2c_conf = {
    .i2c_x                     = I2C1,
    .i2c_clock                 = RCC_APB1Periph_I2C1,
    .gpio_clock                = RCC_AHB1Periph_GPIOB,
    .gpio_af                   = GPIO_AF_I2C1,
    .gpio_port                 = GPIOB,
    .gpio_pin_scl              = GPIO_Pin_6,
    .gpio_pin_sda              = GPIO_Pin_9,
    .gpio_pinsource_scl        = GPIO_PinSource6,
    .gpio_pinsource_sda        = GPIO_PinSource9,
};


static const max14690_t max14690 = {
    .address     = 0x28, /* Address of the device is 0x28, or 0x50|1 (unshifted) */
    .pin_intn    = GPIO_Pin_0, /* unverified */
};

static void _adc_init(void);
static void _int_init(void);
static uint16_t _read_adc(uint8_t channel);

void hw_power_init(void)
{   
    i2c_init(&i2c_conf);
    _int_init();
    _adc_init();
    
    /* PMIC will stay on, pullup enabled */
    max14690_stay_on(1);
}

static void _adc_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_CommonStructInit(&ADC_CommonInitStructure);

    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
    
    /* weird quirk after bootloader, so de-init and re-init */
    ADC_DeInit();
    ADC_CommonStructInit(&ADC_CommonInitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = 0;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* channel 17 is internal and the core vref */
    ADC_TempSensorVrefintCmd(ENABLE);
    
    ADC_RegularChannelConfig(ADC1, PWR_BAT_ADC_CHANNEL, 1, ADC_SampleTime_480Cycles);
   
    /* Enable ADC conversion */
    ADC_Cmd(ADC1, ENABLE);
     
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
}

static uint16_t _read_adc(uint8_t channel)
{
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_480Cycles);
    ADC_SoftwareStartConv(ADC1);
    
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    uint16_t val = ADC_GetConversionValue(ADC1);

    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    return val;
}

/* XXX TODO Interrupt ISR here for posterity. It doesn't work (I think) */
#if 0
char buf[30];
void EXTI0_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        char nbuf[2];
        // this is pretty terrible. dont do in isr. for testing only
        i2c_read_reg(&i2c_conf, max14690.address, 0x05, (uint8_t *)nbuf, 2);
        snprintf(buf, 30, "IA: %x %x %x %x %x %x", nbuf[0], nbuf[1], GPIOE->IDR & (1 << 0), GPIOE->IDR & (1 << 1), GPIOA->IDR & (1 << 0), GPIOF->IDR & (1 << 7));
        send_os_msg(buf, strlen(buf));
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
#endif

static void _int_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_Pin = PWR_BAT_PIN;
    GPIO_Init(PWR_BAT_PORT, &GPIO_InitStruct);
    
    /* XXX TODO Interrupt pin setup here for posterity. It doesn't work (I think) */
#if 0
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    
    /* Set interrupt pin as input */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;// | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    /* interrupt from MAX14690 setup */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOF, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
    
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 7;  // must be > 5
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
#endif
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
}

uint16_t hw_power_get_bat_mv(void)
{
#define SAMPLE_COUNT 4  
    /* Read vref */
    uint16_t vref, mv, vbat, pct;
    
    vref = _read_adc(ADC_Channel_Vrefint);
    mv = (1200 * (double)(4096.0 / (double)vref)); // 1.2v core

    /* Set the PMIC MON pin on, BATT mode. Ratio 3:1 */
    max14690_set_monitor_status(MON_CTRL_BATT, MON_OFF_MODE_HIZ, MON_RATIO_3_1);
    
    /* Read the ADC n times to get the battery */
    for(uint8_t i = 0; i < SAMPLE_COUNT; i++)
    {
        vbat += _read_adc(PWR_BAT_ADC_CHANNEL);
        delay_us(10);
    }
    vbat /= SAMPLE_COUNT;
    
    /* scale back to battery voltage */
    mv = 3 * (3600 * (40 * vbat / 0x5B) / (3 * (40 * BAT_MV_MAX / 0x5B)));

    /* MON pin off. At least we have the illusion of saving power :P */
    max14690_set_monitor_status(0, 0, 0);
    
    return mv;
}

uint8_t hw_power_get_chg_status(void)
{
    uint8_t buf[2];
    /* Read the charge status */
    i2c_read_reg(&i2c_conf, max14690.address, REG_STATUS_A, buf, 2);
    // StatusB UsbOK triggers
    uint8_t ch = buf[1] & (1 << 3);
    uint8_t chm = buf[0] & 7;
    if (!ch)
        chm = 0;
    return chm;
}

/*
    MAX 14690 specific functions 
   Note: when we have more than a few of these, move to another file
*/

void max14690_set_monitor_status(uint8_t control, uint8_t mode, uint8_t ratio)
{
    uint8_t val = ratio << 4;
    val |= mode << 3;
    val |= control;
    i2c_write_reg(&i2c_conf, max14690.address, REG_MON_CFG, val);
    /* it takes 48us to latch */
    delay_us(100);
}

void max14690_stay_on(uint8_t pullup)
{
    if (pullup)
        i2c_write_reg(&i2c_conf, max14690.address, REG_PWR_CFG, 0x81); /* on | pullup */
    i2c_write_reg(&i2c_conf, max14690.address, REG_PWR_CFG, 0x01);
}
