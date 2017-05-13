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

/*-----------------------------------------------------------------------------.
|                                                                              |
|                                  GContext                                    |
|                                                                              |
|   GContexts exist to store information about the graphics environment,       |
|   most importantly a pointer to the framebuffer, information about current   |
|   graphics settings, and information about the currently active graphics     |
|   origin and clipping box.                                                   |
|                                                                              |
`-----------------------------------------------------------------------------*/

typedef struct n_GContext {
    n_GColor stroke_color;
    n_GColor fill_color;
    n_GColor text_color;
    bool antialias;
    bool stroke_caps;
    uint16_t stroke_width;
#ifndef NGFX_IS_CORE 
    GContext * underlying_context; // This is necessary for the time being
                                   // because direct framebuffer access doens't
                                   // exist in the development/testing environment.
                                   // Once we move to prod & a firmware where
                                   // neographics _is_ the underlying context,
                                   // this will no longer be necessary.
#endif
    GBitmap * bitmap;
    uint8_t * fbuf;
    n_GRect offset;
} n_GContext;

void n_graphics_context_set_stroke_color(n_GContext * ctx, n_GColor color);
void n_graphics_context_set_fill_color(n_GContext * ctx, n_GColor color);
void n_graphics_context_set_text_color(n_GContext * ctx, n_GColor color);
// void n_graphics_context_set_compositing_mode(n_GContext * ctx, ...);
void n_graphics_context_set_stroke_caps(n_GContext * ctx, bool caps);
void n_graphics_context_set_stroke_width(n_GContext * ctx, uint16_t width);
void n_graphics_context_set_antialiased(n_GContext * ctx, bool antialias);


void n_graphics_context_begin(n_GContext * ctx);
void n_graphics_context_end(n_GContext * ctx);

GBitmap * n_graphics_capture_frame_buffer(n_GContext * ctx);
GBitmap * n_graphics_capture_frame_buffer_format(n_GContext * ctx, GBitmapFormat format);

bool n_graphics_release_frame_buffer(n_GContext * ctx, GBitmap * bitmap);

n_GContext * n_graphics_context_from_buffer(uint8_t * buf);
// n_GContext * n_graphics_context_from_graphics_context(GContext * ctx);
void n_graphics_context_destroy(n_GContext * ctx);
