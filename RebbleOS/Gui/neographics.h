#pragma once
/* neographics.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <stdbool.h>
#include "graphics_common.h"
#include "gui.h"
#include "display.h"
#include "graphics.h"

void neographics_init(uint8_t *fb);
n_GContext *neographics_get_global_context(void);
