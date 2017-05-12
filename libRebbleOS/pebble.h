#pragma once
/* pebble.h
 * Pebble emulation
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include "rebble_memory.h"
#include "log.h"
#include <stdlib.h>


// This is for neographics so it can set the screen size properly
// TODO cange this to automatic variables
#define PBL_RECT
// tell neographics we are using it as our core
#define NGFX_IS_CORE

#define TRIG_MAX_RATIO 0xffff
#define TRIG_MAX_ANGLE 0x10000

struct GBitmap;
struct n_GRect;
struct ResHandle;

#include "graphics_reshandle.h"
#include "graphics_resource.h"

#include "gbitmap.h"

int32_t sin_lookup(int32_t angle);
int32_t cos_lookup(int32_t angle);
