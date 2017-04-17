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


// This is for neographics so it can set the screen size properly
// TODO cange this to automatic variables
#define PBL_RECT
#include "FreeRTOS.h"
#include "rebble_memory.h"
#include "pebble_defines.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include <inttypes.h>



#include "graphics.h"

#include "gbitmap.h"
#include "graphics_bitmap.h"
#include "graphics_context.h"
#include "graphics_resource.h"
#include "click_config.h"
#include "layer.h"
#include "bitmap_layer.h"
#include "text_layer.h"
#include "scroll_layer.h"
#include "window.h"
#include "display.h"
#include "animation.h"
#include "neographics.h"
#include "buttons.h"
#include "rebble_time.h"
#include "tick_timer_service.h"
#include "appmanager.h"


void rbl_draw(void);
struct tm *rbl_get_tm(void);
