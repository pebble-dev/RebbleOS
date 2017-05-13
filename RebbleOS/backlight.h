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

void backlight_init(void);
void backlight_set_raw(uint16_t brightness);
void backlight_set(uint16_t brightness_pct);
void backlight_on(uint16_t brightness_pct, uint16_t time);
void backlight_set_from_ambient(void);
