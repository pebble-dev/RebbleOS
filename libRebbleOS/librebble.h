#pragma once
/* librebble.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
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

#include "system_font.h"

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
#include "libros_graphics.h"


void rbl_draw(void);
struct tm *rbl_get_tm(void);
