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

