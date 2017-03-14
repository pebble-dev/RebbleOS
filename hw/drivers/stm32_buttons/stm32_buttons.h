/* 
 * stm32_buttons.h
 * External-facing API for stm32 buttons (implements platform API for buttons)
 * RebbleOS
 *
 * Barry Carter <barry.carter@gmail.com>
 * Joshua Wise <joshua@joshuawise.com>
 *
 * See stm32_buttons.c for a better description of this file.
 */

#ifndef __STM32_BUTTONS_H
#define __STM32_BUTTONS_H

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
