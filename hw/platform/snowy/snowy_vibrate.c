/* snowy_vibrate.c
 * GPIO vibrate implementation for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "snowy_vibrate.h"
#include <stm32f4xx_spi.h>


vibrate_t vibrate = {
    .Pin     = GPIO_Pin_4,
    .Port    = GPIOF,
};

void hw_vibrate_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure_Vibr;
    
    // init the vibrator
    GPIO_InitStructure_Vibr.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Vibr.GPIO_Pin = vibrate.Pin;
    GPIO_InitStructure_Vibr.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Vibr.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Vibr.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(vibrate.Port, &GPIO_InitStructure_Vibr);
}


void hw_vibrate_enable(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(vibrate.Port, vibrate.Pin);
    else
        GPIO_ResetBits(vibrate.Port, vibrate.Pin);
}
