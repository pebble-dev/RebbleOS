#pragma once
/* display.h
 * 
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <stdbool.h>

uint8_t display_init(void);
void display_done_isr(uint8_t cmd);
void display_reset(uint8_t enabled);
void display_draw(void);
uint8_t *display_get_buffer(void);

bool display_buffer_lock_give(void);
bool display_buffer_lock_take(uint32_t timeout);
