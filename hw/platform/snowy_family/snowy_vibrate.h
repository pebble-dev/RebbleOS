#pragma once
/* snowy_vibrate.c
 * GPIO vibrate implementation for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"

typedef struct {
    uint16_t Pin;
    GPIO_TypeDef *Port;
} vibrate_t;


void hw_vibrate_init(void);
void hw_vibrate_enable(uint8_t enabled);
