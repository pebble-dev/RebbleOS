#ifndef __SNOWY_H
#define __SNOWY_H
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
#include <sys/types.h>

// How often are we resetting the watchdog timer (ms)
#define WATCHDOG_RESET_MS 500

// main hooks
void debug_init(void);
void debug_write(const unsigned char *p, size_t len);
void platform_init(void);

// internal
void init_USART3(void);
void init_USART8(void);


// implementation
void hw_watchdog_init(void);
void hw_watchdog_reset(void);


#endif
