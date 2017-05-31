/*\
|*|
|*|   Neographics: a tiny graphics library.
|*|   Copyright (C) 2016 Johannes Neubrand <johannes_n@icloud.com>
|*|
|*|   This program is free software; you can redistribute it and/or
|*|   modify it under the terms of the GNU General Public License
|*|   as published by the Free Software Foundation; either version 2
|*|   of the License, or (at your option) any later version.
|*|
|*|   This program is distributed in the hope that it will be useful,
|*|   but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|   GNU General Public License for more details.
|*|
|*|   You should have received a copy of the GNU General Public License
|*|   along with this program; if not, write to the Free Software
|*|   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
|*|
\*/

#pragma once

#include <pebble.h>
#include "types.h"
#include "macros.h"
#include "context.h"

/*-----------------------------------------------------------------------------.
|                                                                              |
|                          Common Graphics Routines                            |
|                                                                              |
|   The common graphics routines are fast pixel, row and column drawing        |
|   operations. All primitives should use these functions instead of           |
|   accessing the framebuffer.                                                 |
|                                                                              |
`-----------------------------------------------------------------------------*/

void n_graphics_set_pixel(n_GContext * ctx, n_GPoint p, n_GColor color);
void n_graphics_fill_pixel(n_GContext * ctx, n_GPoint p);
void n_graphics_draw_pixel(n_GContext * ctx, n_GPoint p);

void n_graphics_prv_draw_col(uint8_t * fb,
        int16_t x, int16_t top, int16_t bottom,
        int16_t minx, int16_t maxx, int16_t miny, int16_t maxy,
        uint8_t fill);
void n_graphics_prv_draw_row(uint8_t * fb,
    int16_t y, int16_t left, int16_t right,
    int16_t minx, int16_t maxx, int16_t miny, int16_t maxy,
    uint8_t fill);
