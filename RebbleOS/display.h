#pragma once
/* display.h
 * 
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"

#define DISPLAY_MODE_BOOTLOADER      0
#define DISPLAY_MODE_FULLFAT         1

// commands for draw
#define DISPLAY_CMD_DRAW             1
#define DISPLAY_CMD_RESET            2
#define DISPLAY_CMD_DONE             3


/* XXX this is not portable yet, and really needs to get split into hw/ */
#ifdef STM32F2XX
#include "stm32f2xx.h"
#else
#include "stm32f4xx.h"
#endif


void display_init(void);
void display_done_ISR(uint8_t cmd);
void display_reset(uint8_t enabled);
void display_draw(void);
uint8_t *display_get_buffer(void);
