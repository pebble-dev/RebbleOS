/* snowy_vibrate.c
 * GPIO vibrate implementation for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stm32_power.h"
#include "stdio.h"
#include "string.h"
#include "snowy_vibrate.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_gpio.h>

extern const vibrate_t hw_vibrate_config;

void hw_vibrate_init(void)
{
    stm32_power_request(STM32_POWER_AHB1, hw_vibrate_config.clock);
    
    GPIO_InitTypeDef GPIO_InitStructure_Vibr;
    
    // init the vibrator
    GPIO_InitStructure_Vibr.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Vibr.GPIO_Pin = hw_vibrate_config.pin;
    GPIO_InitStructure_Vibr.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Vibr.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Vibr.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(hw_vibrate_config.port, &GPIO_InitStructure_Vibr);
    
    stm32_power_release(STM32_POWER_AHB1, hw_vibrate_config.clock);
}


void hw_vibrate_enable(uint8_t enabled)
{
    stm32_power_request(STM32_POWER_AHB1, hw_vibrate_config.clock);
    
    if (enabled)
        GPIO_SetBits(hw_vibrate_config.port, hw_vibrate_config.pin);
    else
        GPIO_ResetBits(hw_vibrate_config.port, hw_vibrate_config.pin);
    
    stm32_power_release(STM32_POWER_AHB1, hw_vibrate_config.clock);
}
