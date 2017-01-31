#ifndef __SNOWY_DISPLAY_H
#define __SNOWY_DISPLAY_H
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
#include "stm32f4xx.h"

#define MAX_FRAMEBUFFER_SIZE 24192

// display command types
#define DISPLAY_CTYPE_NULL        0x00
#define DISPLAY_CTYPE_PARAM       0x01
#define DISPLAY_CTYPE_DISPLAY_OFF 0x02
#define DISPLAY_CTYPE_DISPLAY_ON  0x03
#define DISPLAY_CTYPE_SCENE       0x04
// in full fat mode
#define DISPLAY_CTYPE_FRAME       0x05


void hw_display_init(void);
void hw_display_reset(void);
void hw_display_start(void);
void snowy_display_init_intn(void);
void hw_backlight_init(void);
void hw_backlight_set(uint16_t val);
void snowy_display_init_timer(uint16_t pwmValue);
void snowy_display_init_SPI6(void);
void snowy_display_cs(uint8_t enabled);
uint8_t snowy_display_SPI6_getver(uint8_t data);
uint8_t snowy_display_SPI6_send(uint8_t data);
uint8_t snowy_display_FPGA_reset(uint8_t mode);
void snowy_display_reset(uint8_t enabled);
void snowy_display_SPI_start(void);
void snowy_display_SPI_end(void);
void snowy_display_drawscene(uint8_t scene);
void hw_display_on();
void hw_display_start_frame(void);
void hw_display_send_frame(void);
void snowy_display_start_frame(void);
void snowy_display_send_frame();
uint8_t snowy_display_wait_FPGA_ready(void);
void snowy_display_splash(uint8_t scene);
void snowy_display_full_init(void);
void snowy_display_program_FPGA(void);

void delay_us(uint16_t us);
void delay_ns(uint16_t ns);
#endif
