#pragma once
/* power.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"

void power_init();
void power_off();
uint16_t power_get_battery_level(void);
