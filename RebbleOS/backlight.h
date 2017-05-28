#pragma once
/* backlight.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"

#define BACKLIGHT_OFF  0
#define BACKLIGHT_ON   1
#define BACKLIGHT_FADE 2

void rblcore_backlight_init(void);
void rblcore_backlight_on(uint16_t brightness_pct, uint16_t time);
