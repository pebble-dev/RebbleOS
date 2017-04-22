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
#include "stm32f4xx.h"

#define DISPLAY_ROWS 168
#define DISPLAY_COLS 144

// flash regions
#define REGION_PRF_START    0x200000
#define REGION_PRF_SIZE     0x1000000
// DO NOT WRITE TO THIS REGION
#define REGION_MFG_START    0xE0000
#define REGION_MFG_SIZE     0x20000
// Resource start
#define REGION_RES_START    0x380000
#define REGION_RES_SIZE     0x7D000 // TODO

#define REGION_APP_RES_START    0xB3A000
#define REGION_APP_RES_SIZE     0x7D000

// app slots
#define APP_SLOT_1_START    0xB3E000
#define APP_SLOT_9_START    0xc34000
#define APP_SLOT_17_START    0xc82000
#define APP_HEADER_BIN_OFFSET 0x59
#define APP_SLOT_SIZE       0x48000

#define RES_START           0x200C
