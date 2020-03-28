#pragma once
/* pebble.h
 * Pebble emulation
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include "FreeRTOS.h"
#include "rebble_memory.h"
#include "log.h"
#include "resource.h"
#include "platform_config.h"

// tell neographics we are using it as our core
#define NGFX_PREFERRED_free app_free
#define NGFX_PREFERRED_calloc app_calloc
#define NGFX_PREFERRED_malloc app_malloc
#define NGFX_PREFERRED_resource_load resource_load

#define TRIG_MAX_RATIO 0xffff
#define TRIG_MAX_ANGLE 0x10000

#include "types/rect.h"
#include "graphics_reshandle.h"
#include "graphics_resource.h"
#include "pebble_defines.h"

int32_t sin_lookup(int32_t angle);
int32_t cos_lookup(int32_t angle);

//! Represents insets for four sides. Negative values mean a side extends.
//! @see \ref grect_inset
typedef struct {
  //! The inset at the top of an object.
  int16_t top;
  //! The inset at the right of an object.
  int16_t right;
  //! The inset at the bottom of an object.
  int16_t bottom;
  //! The inset at the left of an object.
  int16_t left;
} GEdgeInsets;

//! helper for \ref GEdgeInsets macro
#define GEdgeInsets4(t, r, b, l) \
  ((GEdgeInsets){.top = t, .right = r, .bottom = b, .left = l})

//! helper for \ref GEdgeInsets macro
#define GEdgeInsets3(t, rl, b) \
  ((GEdgeInsets){.top = t, .right = rl, .bottom = b, .left = rl})

//! helper for \ref GEdgeInsets macro
#define GEdgeInsets2(tb, rl) \
  ((GEdgeInsets){.top = tb, .right = rl, .bottom = tb, .left = rl})

//! helper for \ref GEdgeInsets macro
#define GEdgeInsets1(trbl) \
  ((GEdgeInsets){.top = trbl, .right = trbl, .bottom = trbl, .left = trbl})

//! helper for \ref GEdgeInsets macro
#define GEdgeInsetsN(_1, _2, _3, _4, NAME, ...) NAME

//! Convenience macro to make a GEdgeInsets
//! This macro follows the CSS shorthand notation where you can call it with
//!  - just one value GEdgeInsets(v1) to configure all edges with v1
//!    (GEdgeInsets){.top = v1, .right = v1, .bottom = v1, .left = v1}
//!  - two values v1, v2 to configure a vertical and horizontal inset as
//!    (GEdgeInsets){.top = v1, .right = v2, .bottom = v1, .left = v2}
//!  - three values v1, v2, v3 to configure it with
//!    (GEdgeInsets){.top = v1, .right = v2, .bottom = v3, .left = v2}
//!  - four values v1, v2, v3, v4 to configure it with
//!    (GEdgeInsets){.top = v1, .right = v2, .bottom = v3, .left = v4}
//! @see \ref grect_insets
#define GEdgeInsets(...) \
GEdgeInsetsN(__VA_ARGS__, GEdgeInsets4, GEdgeInsets3, GEdgeInsets2, GEdgeInsets1)(__VA_ARGS__)

GRect grect_inset(GRect rect, GEdgeInsets insets);

