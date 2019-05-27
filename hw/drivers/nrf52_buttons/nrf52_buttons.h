/* 
 * nrf52_buttons.h
 * External-facing API for nRF52 buttons (implements platform API for buttons)
 * RebbleOS
 *
 * Joshua Wise <joshua@joshuawise.com>
 *
 * See nrf52_buttons.c for a better description of this file.
 */

#ifndef __NRF52_BUTTONS_H
#define __NRF52_BUTTONS_H

typedef enum hw_button {
    HW_BUTTON_BACK = 0,
    HW_BUTTON_UP,
    HW_BUTTON_SELECT,
    HW_BUTTON_DOWN,
    HW_BUTTON_MAX
} hw_button_t;

typedef void (*hw_button_isr_t)(hw_button_t id);

void hw_button_init(void);
int hw_button_pressed(hw_button_t button_id);
void hw_button_set_isr(hw_button_isr_t isr);

#endif
