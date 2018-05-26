/*
 * stm32_backlight_platform.h
 * External-facing API for backlight control (provides backlight API with platform configuration)
 * RebbleOS
 *
 * Thomas England <thomas.c.england@gmail.com>
 */

#ifndef __STM32_BACKLIGHT_PLATFORM_H
#define __STM32_BACKLIGHT_PLATFORM_H

#include <stdint.h>

typedef struct {
  TIM_TypeDef* tim;
  uint16_t pin;
  uint8_t pin_source;
  uint32_t port;
  uint8_t af;
  uint32_t rcc_tim;
} stm32_backlight_config_t;

extern stm32_backlight_config_t platform_backlight;
#endif
