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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include "rebble_memory.h"
#include <stdlib.h>


// This is for neographics so it can set the screen size properly
// TODO cange this to automatic variables
#define PBL_RECT
// tell neographics we are using it as our core
#define NGFX_IS_CORE

#define TRIG_MAX_RATIO 0xffff
#define TRIG_MAX_ANGLE 0x10000

#include "graphics_bitmap.h"
#include "graphics_reshandle.h"
#include "graphics_resource.h"


struct GBitmap;
struct n_GRect;
struct ResHandle;


int32_t sin_lookup(int32_t angle);
int32_t cos_lookup(int32_t angle);
