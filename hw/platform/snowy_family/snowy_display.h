#pragma once
/* snowy_display.h
 * 
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "platform.h"

#define MAX_FRAMEBUFFER_SIZE DISPLAY_ROWS * DISPLAY_COLS

void hw_display_init(void);
void hw_display_reset(void);
void hw_display_start(void);
uint8_t hw_display_is_ready();
uint8_t *hw_display_get_buffer(void);
uint8_t hw_display_process_isr(void);

void hw_display_on();
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset);

// TODO: move to scanline
void scanline_convert(uint8_t *out_buffer, uint8_t *frame_buffer, uint8_t column_index);

void delay_us(uint16_t us);
void delay_large(uint16_t ms);

