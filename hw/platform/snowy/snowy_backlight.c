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
#include "display.h"
#include "snowy_display.h"
#include "backlight.h"
#include "snowy_backlight.h"
#include "stm32_power.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_tim.h>

extern display_t display;

/*
 * Initialise the backlight. This is set as a timer (TIM12)
 * tied to GPIO B14 on PT Snowy
 */
void hw_backlight_init(void)
{
    hw_backlight_set(4999);
}

/* We turn on the clocks when pwmValue > 0, and turn them off when we're not
 * running the backlight.  */
static uint8_t _backlight_clocks_on = 0;

/*
 * Set the PWM value (brightness) of the backlight
 * This is *currently* 9999 as the top value, with 5000 being 50%
 */
void hw_backlight_set(uint16_t pwmValue)
{
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    TIM_OCInitTypeDef TIM_OCStruct;

    GPIO_InitTypeDef GPIO_InitStruct;
    
    // Pebble Time has backlight control driven by TIM12
    // It is set to PWM mode 2 and will count up to n
    
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);
    
    /* Set pins */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    
    TIM_BaseStruct.TIM_Prescaler = 0;
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseStruct.TIM_Period = 9999; // 10khz calculated by clock dividers
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM12, &TIM_BaseStruct);

    // now the OC timer
    if (!_backlight_clocks_on)
        stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_TIM12);
    
    // This shouldn't be here, but for some reason in QEMU, setting
    // the TIM clocks in RCC turns off UART8. weird.
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);

    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;  // set on compare
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    
    TIM_OCStruct.TIM_Pulse = pwmValue;
    TIM_OC1Init(TIM12, &TIM_OCStruct);
    TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);
    
    TIM_Cmd(TIM12, ENABLE);
    TIM_CtrlPWMOutputs(TIM12, ENABLE);
    
    _backlight_clocks_on = pwmValue > 0;
    if (!_backlight_clocks_on)
        stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_TIM12);
    
    printf("Backlight: Set\n");
}
