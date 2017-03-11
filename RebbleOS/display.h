#pragma once
/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "FreeRTOS.h"
// #include "ugui.h"

#define DISPLAY_MODE_BOOTLOADER      0
#define DISPLAY_MODE_FULLFAT         1

// todo TYPEDEF ENUM
#define DISPLAY_CMD_DRAW             1
#define DISPLAY_CMD_RESET            2

#define DISPLAY_STATE_BOOTING        0
#define DISPLAY_STATE_FRAME_INIT     1
#define DISPLAY_STATE_FRAME          2
#define DISPLAY_STATE_IDLE           3


#define BLACK 0x0
#define BLUE   0x03
#define GREEN 0x0C
#define RED  0x30


/* XXX this is not portable yet, and really needs to get split into hw/ */
#ifdef STM32F2XX
#include "stm32f2xx.h"
#else
#include "stm32f4xx.h"
#endif


void display_init(void);
void display_done_ISR(uint8_t cmd);
void display_reset(uint8_t enabled);
void display_on();
void display_start_frame(uint8_t offset_x, uint8_t offset_y);
void display_send_frame();
void backlight_set(uint16_t brightness);
void display_logo(uint8_t *frameData);
uint16_t display_checkerboard(char *frameData, uint8_t invert);
void display_cmd(uint8_t cmd, char *data);
void vDisplayISRProcessor(void *pvParameters);
void vDisplayCommandTask(void *pvParameters);
void display_draw(void);
uint8_t *display_get_buffer(void);

int init_gui(void);
