/*
 * stm32_backlight.h
 * API for backlight control
 * RebbleOS
 *
 * Barry Carter <barry.carter@gmail.com>
 * Thomas England <thomas.c.england@gmail.com>
 */

#ifndef __STM32_BACKLIGHT_H
#define __STM32_BACKLIGHT_H

void hw_backlight_init(void);
void hw_backlight_set(uint16_t val);

#endif
