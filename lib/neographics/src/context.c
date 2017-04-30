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

#include "context.h"

// TODO optimization: calculate bytefill when color is set.

void n_graphics_context_set_stroke_color(n_GContext * ctx, n_GColor color) {
    ctx->stroke_color = color;
}

void n_graphics_context_set_fill_color(n_GContext * ctx, n_GColor color) {
    ctx->fill_color = color;
}

void n_graphics_context_set_text_color(n_GContext * ctx, n_GColor color) {
    ctx->text_color = color;
}

// void n_graphics_context_set_compositing_mode(n_GContext * ctx, ...) {
// }

void n_graphics_context_set_antialiased(n_GContext * ctx, bool enable) {
    ctx->antialias = enable;
}

void n_graphics_context_set_stroke_width(n_GContext * ctx, uint16_t width) {
    if (width == 0)
        return;
    ctx->stroke_width = ((width + 2) & ~(1)) - 1;
}

void n_graphics_context_set_stroke_caps(n_GContext * ctx, bool stroke_caps) {
    ctx->stroke_caps = stroke_caps;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void n_graphics_context_begin(n_GContext * ctx) {
#ifndef NGFX_IS_CORE 
    if (ctx->underlying_context) {
        ctx->bitmap = graphics_capture_frame_buffer(ctx->underlying_context);
        ctx->fbuf = gbitmap_get_data(ctx->bitmap);
    }
#endif
}

void n_graphics_context_end(n_GContext * ctx) {
#ifndef NGFX_IS_CORE 
    if (ctx->underlying_context) {
        graphics_release_frame_buffer(ctx->underlying_context, ctx->bitmap);
        ctx->bitmap = NULL;
        ctx->fbuf = NULL;
    }
#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
GBitmap * n_graphics_capture_frame_buffer(n_GContext * ctx) {
#ifndef NGFX_IS_CORE
    return graphics_capture_frame_buffer(ctx->underlying_context);
#endif
}

GBitmap * n_graphics_capture_frame_buffer_format(n_GContext * ctx, GBitmapFormat format) {
#ifndef NGFX_IS_CORE
    return graphics_capture_frame_buffer_format(ctx->underlying_context, format);
#endif
}

bool n_graphics_release_frame_buffer(n_GContext * ctx, GBitmap * bitmap) {
#ifndef NGFX_IS_CORE
    return graphics_release_frame_buffer(ctx->underlying_context, bitmap);
#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static n_GContext * n_graphics_context_create() {
    n_GContext * out = (n_GContext *)malloc(sizeof(n_GContext));
    //out->underlying_context = NULL;
    if (out == NULL)
        printf("NG: NO HEAP FREE\n");
    n_graphics_context_set_stroke_color(out, (n_GColor) {.argb = 0b11000000});
    n_graphics_context_set_fill_color(out, (n_GColor) {.argb = 0b11111111});
    n_graphics_context_set_text_color(out, (n_GColor) {.argb = 0b11000000});
    // n_graphics_context_set_compositing_mode(out, )
    n_graphics_context_set_stroke_caps(out, true);
    n_graphics_context_set_antialiased(out, true);
    n_graphics_context_set_stroke_width(out, 1);
    return out;
}

n_GContext * n_graphics_context_from_buffer(uint8_t * buf) {
    n_GContext * out = n_graphics_context_create();
    out->fbuf = buf;
    return out;
}

#ifndef NGFX_IS_CORE 
n_GContext * n_graphics_context_from_graphics_context(uint8_t * ctx) {
    n_GContext * out = n_graphics_context_create();
    out->underlying_context = ctx;
    return out;
}
#endif

void n_graphics_context_destroy(n_GContext * ctx) {
    free(ctx);
}
