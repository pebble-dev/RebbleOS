/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "snowy_ambient.h"
#include "stm32_power.h"

/*
 * Initialise the hardware. This means all GPIOs and SPI for the display
 */
void hw_ambient_init(void)
{
    // so its the worst kind of config here, but testing yo
    //
    // set the adc to sample fo-evar.
    // we should switch off or something
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    ADC_StructInit(&ADC_InitStructure);
    ADC_CommonStructInit(&ADC_CommonInitStructure);
    
    //RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
 
    // weird quirk after bootloader, so de-init and re-init
    ADC_DeInit();
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = 0;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_Init(ADC1, &ADC_InitStructure);
 
    ADC_TempSensorVrefintCmd(ENABLE);
    
    // Enable ADC conversion
    ADC_Cmd(ADC1, ENABLE);
/*
    // calibrate
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);

    while(ADC_GetCalibrationStatus(ADC1));*/
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_144Cycles);

    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);


    printf("Ambience\n");
}

uint16_t hw_ambient_get(void)
{
    uint16_t val;
    
    // ambient is connected to PA3 so lets pull it up and energise
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    GPIO_SetBits(GPIOA, GPIO_Pin_3);
    delay_us(10);
    
    ADC_SoftwareStartConv(ADC1);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    val = ADC_GetConversionValue(ADC1);
    
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_ADC1);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    printf("Ambient: %d\n", val);
    
    return val;
}
