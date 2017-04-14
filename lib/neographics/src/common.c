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

#include "common.h"

static void n_graphics_prv_setbit(uint8_t * byte, uint8_t pos, bool val) {
    *byte ^= (-val ^ *byte) & (1 << pos);
}

void n_graphics_set_pixel(n_GContext * ctx, n_GPoint p, n_GColor color) {
#ifdef PBL_BW
    n_graphics_prv_setbit(
        &ctx->fbuf[p.y * __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT + p.x / 8],
        p.x % 8, (color.argb & 0b111111));
#else
    ctx->fbuf[p.y * __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT + p.x] = color.argb;
#endif
}

void n_graphics_draw_pixel(n_GContext * ctx, n_GPoint p) {
    n_graphics_set_pixel(ctx, p, ctx->stroke_color);
}

// NB (top < bottom) leads to undefined behavior
void n_graphics_prv_draw_col(uint8_t * fb,
        int16_t x, int16_t top, int16_t bottom,
        int16_t minx, int16_t maxx, int16_t miny, int16_t maxy,
        uint8_t fill) {
    if (x < minx || x >= maxx || top >= maxy || bottom < miny || bottom < top) {
        return;
    }

    uint16_t begin = __BOUND_NUM(miny, top, maxy - 1),
             end   = __BOUND_NUM(miny, bottom, maxy - 1);

    for (uint16_t y = begin; y <= end; y++) {
#ifdef PBL_BW
        n_graphics_prv_setbit(&fb[y * __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT + x / 8],
            x % 8, (fill >> ((y + x) % 8)) & 1);
#else
        fb[y * __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT + x] = fill;
#endif
    }
}

void n_graphics_prv_draw_row(uint8_t * fb,
        int16_t y, int16_t left, int16_t right,
        int16_t minx, int16_t maxx, int16_t miny, int16_t maxy,
        uint8_t fill) {
    uint8_t * row;
    if (y >= miny && y < maxy && right >= minx && left < maxx) {
        row = fb + (y * __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT);
    } else {
        return;
    }

    uint16_t begin = __BOUND_NUM(minx, left, maxx - 1),
             end   = __BOUND_NUM(minx, right, maxx - 1);

#ifdef PBL_BW
    uint16_t begin_byte = begin / 8,
             end_byte   = end / 8;
#else
    uint16_t begin_byte = begin,
             end_byte   = end;
#endif


#ifdef PBL_BW
    if (y & 1)
        fill = fill >> 1 | fill << 7;
    /*\ Brace yourselves.
    |*| Here's what's going on:
    |*| - We're on b/w, which means, 8 pixels horizontally are represented by
    |*|   1 byte of memory.
    |*| - For maximum write speed, we're grouping two rows of equal width
    |*|   (because we're drawing circles/ellipses) and rendering them
    |*|   concurrently.
    |*| - The quickest way to do that is to check:
    |*|   - Does the full row fit within exectly one bit storage space?
    |*|     - If so, iterate over pixels in row and set bits accordingly.
    |*|     - Otherwise, iterate over pixels in first and last byte of the row
    |*|       and memset everything in between.
    \*/
    if (begin_byte == end_byte) {
        for (int16_t i = begin; i <= end; i++) {
            n_graphics_prv_setbit(&row[end_byte], i - end_byte * 8, (fill >> (i%8)) & 1);
        }
    } else {
        for (int16_t i = begin; i <= begin_byte * 8 + 8; i++) {
            n_graphics_prv_setbit(&row[begin_byte], i - begin_byte * 8, (fill >> (i%8)) & 1);
        }
        if (end_byte - begin_byte > 1) {
            memset(row + begin_byte + 1, fill, end_byte - begin_byte - 1);
        }
        for (int16_t i = end; i >= end_byte * 8; i--) {
            n_graphics_prv_setbit(&row[end_byte], i - end_byte * 8, (fill >> (i%8)) & 1);
        }
    }
#else
    memset(row + begin_byte, fill, end_byte - begin_byte + 1);
#endif
}
