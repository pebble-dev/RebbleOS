/* 
 * nrf52_buttons_platform.h
 * External-facing API for nRF52 buttons (implements platform API for buttons)
 * RebbleOS
 *
 * Joshua Wise <joshua@joshuawise.com>
 *
 * See nrf52_buttons.c for a better description of this file.
 */

#ifndef __NRF32_BUTTONS_PLATFORM_H
#define __NRF32_BUTTONS_PLATFORM_H

#include "nrf52_buttons.h"
#include "nrfx_gpiote.h"

extern nrfx_gpiote_pin_t nrf52_buttons_pindefs[HW_BUTTON_MAX];

#endif
