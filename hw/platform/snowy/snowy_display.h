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
#include "platform.h"

#define MAX_FRAMEBUFFER_SIZE DISPLAY_ROWS * DISPLAY_COLS

// display command types
#define DISPLAY_CTYPE_NULL        0x00
#define DISPLAY_CTYPE_PARAM       0x01
#define DISPLAY_CTYPE_DISPLAY_OFF 0x02
#define DISPLAY_CTYPE_DISPLAY_ON  0x03
#define DISPLAY_CTYPE_SCENE       0x04
// in full fat mode
#define DISPLAY_CTYPE_FRAME       0x05


typedef struct {
//    SPI *spi;  // SPI6
    GPIO_TypeDef *PortDisplay;
    uint16_t PinReset;
    uint16_t PinPower;
    uint16_t PinCs;
    uint16_t PinBacklight;
    GPIO_TypeDef *PortBacklight;
    
    uint16_t PinMiso;
    uint16_t PinMosi;
    uint16_t PinSck;
    
    // inputs
    uint16_t PinResetDone;
    uint16_t PinIntn;
    
    //state
    uint8_t PowerOn;
    uint8_t State; // busy etc
    uint8_t DisplayMode; // bootloader or full
    
    uint8_t DisplayBuffer[DISPLAY_ROWS * DISPLAY_COLS];
    uint8_t BackBuffer[DISPLAY_ROWS * DISPLAY_COLS];
} display_t;


void hw_display_init(void);
void hw_display_reset(void);
void hw_display_start(void);
void hw_backlight_init(void);
void hw_backlight_set(uint16_t val);
uint8_t hw_display_get_state();
uint8_t *hw_display_get_buffer(void);

void hw_display_on();
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset);

#endif
