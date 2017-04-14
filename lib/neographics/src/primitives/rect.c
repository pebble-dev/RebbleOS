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

#include "rect.h"


static void n_graphics_draw_thin_rect_bounded(
        n_GContext * ctx, n_GRect rect,
        uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy) {
#ifdef PBL_BW
    uint8_t color = __ARGB_TO_INTERNAL(ctx->stroke_color.argb);
#else
    uint8_t color = ctx->stroke_color.argb;
#endif
    n_graphics_prv_draw_col(ctx->fbuf, rect.origin.x,
            rect.origin.y, rect.origin.y + rect.size.h - 1,
            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_row(ctx->fbuf, rect.origin.y,
            rect.origin.x, rect.origin.x + rect.size.w - 1,
            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_col(ctx->fbuf, rect.origin.x + rect.size.w - 1,
            rect.origin.y, rect.origin.y + rect.size.h - 1,
            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_row(ctx->fbuf, rect.origin.y + rect.size.h - 1,
            rect.origin.x, rect.origin.x + rect.size.w - 1,
            minx, maxx, miny, maxy, color);
}

static void n_graphics_draw_rect_bounded(
        n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask,
        uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy) {
    // NB this could be changed in the future to allow for more shapes, for
    // example one with opposite corners rounded & radius equal to width.


    radius = __BOUND_NUM(0, __BOUND_NUM(0, radius, rect.size.h / 2), rect.size.w / 2);

    // NB potential for more optimization; b/w currently runs at half speed of color.

    if (mask & n_GCornerTopLeft)
        n_graphics_prv_draw_quarter_circle_bounded(ctx,
                n_GPoint(rect.origin.x + radius,
                         rect.origin.y + radius),
                radius, ctx->stroke_width, -1, -1, minx, maxx, miny, maxy);
    if (mask & n_GCornerTopRight)
        n_graphics_prv_draw_quarter_circle_bounded(ctx,
                n_GPoint(rect.origin.x + rect.size.w - radius - 1,
                         rect.origin.y + radius),
                radius, ctx->stroke_width, 1, -1, minx, maxx, miny, maxy);
    if (mask & n_GCornerBottomLeft)
        n_graphics_prv_draw_quarter_circle_bounded(ctx,
                n_GPoint(rect.origin.x + radius,
                         rect.origin.y + rect.size.h - radius - 1),
                radius, ctx->stroke_width, -1, 1, minx, maxx, miny, maxy);
    if (mask & n_GCornerBottomRight)
        n_graphics_prv_draw_quarter_circle_bounded(ctx,
                n_GPoint(rect.origin.x + rect.size.w - radius - 1,
                         rect.origin.y + rect.size.h - radius - 1),
                radius, ctx->stroke_width, 1, 1, minx, maxx, miny, maxy);

    // NB potential for optimization by disabling unnecessary stroke caps

    // Connect corners to each other clockwise.
    n_graphics_draw_line(ctx,
        n_GPoint(rect.origin.x + (mask & n_GCornerTopLeft ? radius : 0),
                 rect.origin.y),
        n_GPoint(rect.origin.x + rect.size.w - 1
                               - (mask & n_GCornerTopRight ? radius : 0),
                 rect.origin.y));
    n_graphics_draw_line(ctx,
        n_GPoint(rect.origin.x + rect.size.w - 1,
                 rect.origin.y + (mask & n_GCornerTopRight ? radius : 0)),
        n_GPoint(rect.origin.x + rect.size.w - 1,
                 rect.origin.y + rect.size.h - 1
                               - (mask & n_GCornerBottomRight ? radius : 0)));
    n_graphics_draw_line(ctx,
        n_GPoint(rect.origin.x + rect.size.w - 1
                               - (mask & n_GCornerBottomRight ? radius : 0),
                 rect.origin.y + rect.size.h - 1),
        n_GPoint(rect.origin.x + (mask & n_GCornerBottomLeft ? radius : 0),
                 rect.origin.y + rect.size.h - 1));
    n_graphics_draw_line(ctx,
        n_GPoint(rect.origin.x,
                 rect.origin.y + rect.size.h - 1
                               - (mask & n_GCornerBottomLeft ? radius : 0)),
        n_GPoint(rect.origin.x,
                 rect.origin.y + (mask & n_GCornerTopLeft ? radius : 0)));
}

void n_graphics_draw_thin_rect(n_GContext * ctx, n_GRect rect) {
    n_graphics_draw_thin_rect_bounded(ctx, n_grect_standardize(rect), 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
}

void n_graphics_draw_rect(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask) {
    if (!(ctx->stroke_color.argb & (0b11 << 6)))
        ;
    else if (ctx->stroke_width == 1 && (radius == 0 || mask == 0))
        n_graphics_draw_thin_rect_bounded(ctx, n_grect_standardize(rect), 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    else
        n_graphics_draw_rect_bounded(ctx, n_grect_standardize(rect), radius, mask, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
}

static void n_graphics_fill_rect_bounded(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask,
        uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy) {
    // NB this could be changed in the future to allow for more shapes, for
    // example one with opposite corners rounded & radius equal to width.:
    if (!(mask & 0b1111)) {
        radius = 0;
    } else {
        radius = __BOUND_NUM(0, radius, rect.size.w / 2);
        radius = __BOUND_NUM(0, radius, rect.size.h / 2);
    }

#ifdef PBL_BW
    uint8_t color = __ARGB_TO_INTERNAL(ctx->fill_color.argb);
#else
    uint8_t color = ctx->fill_color.argb;
#endif

    // TODO these should be inlined & full rows should be drawn to maximize speed.

    if (radius) {
        if (mask & n_GCornerTopLeft)
            n_graphics_prv_fill_quarter_circle_bounded(ctx,
                    n_GPoint(rect.origin.x + radius,
                             rect.origin.y + radius),
                    radius, -1, -1, minx, maxx, miny, maxy);
        if (mask & n_GCornerTopRight)
            n_graphics_prv_fill_quarter_circle_bounded(ctx,
                    n_GPoint(rect.origin.x + rect.size.w - radius - 1,
                             rect.origin.y + radius),
                    radius, 1, -1, minx, maxx, miny, maxy);
        if (mask & n_GCornerBottomLeft)
            n_graphics_prv_fill_quarter_circle_bounded(ctx,
                    n_GPoint(rect.origin.x + radius,
                             rect.origin.y + rect.size.h - radius - 1),
                    radius, -1, 1, minx, maxx, miny, maxy);
        if (mask & n_GCornerBottomRight)
            n_graphics_prv_fill_quarter_circle_bounded(ctx,
                    n_GPoint(rect.origin.x + rect.size.w - radius - 1,
                             rect.origin.y + rect.size.h - radius - 1),
                    radius, 1, 1, minx, maxx, miny, maxy);

        int16_t left_indent_top = rect.origin.x + (mask & n_GCornerTopLeft ? radius : 0),
                right_indent_top = rect.origin.x + rect.size.w - (mask & n_GCornerTopRight ? radius : 0),
                left_indent_bottom = rect.origin.x + (mask & n_GCornerBottomLeft ? radius : 0),
                right_indent_bottom = rect.origin.x + rect.size.w - (mask & n_GCornerBottomRight ? radius : 0);

        for (uint16_t r = 0; r <= radius; r++) {
            n_graphics_prv_draw_row(ctx->fbuf, rect.origin.y + r,
                left_indent_top, right_indent_top,
                minx, maxx, miny, maxy, color);
            n_graphics_prv_draw_row(ctx->fbuf, rect.origin.y + rect.size.h - 1 - r,
                left_indent_bottom, right_indent_bottom,
                minx, maxx, miny, maxy, color);
        }
    }

    int16_t right_indent = rect.origin.x + rect.size.w;
    for (int16_t r = rect.origin.y + radius; r <= rect.origin.y + rect.size.h - radius - 1; r++) {
        n_graphics_prv_draw_row(ctx->fbuf, r,
            rect.origin.x, right_indent,
            minx, maxx, miny, maxy, color);
    }
}

static void n_graphics_fill_0rad_rect_bounded(n_GContext * ctx, n_GRect rect,
        uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy) {
#ifdef PBL_BW
    uint8_t color = __ARGB_TO_INTERNAL(ctx->fill_color.argb);
#else
    uint8_t color = ctx->fill_color.argb;
#endif
    int16_t right_indent = rect.origin.x + rect.size.w - 1,
            max_y = rect.origin.y + rect.size.h - 1;
    for (int16_t r = rect.origin.y; r <= max_y; r++) {
        n_graphics_prv_draw_row(ctx->fbuf, r,
            rect.origin.x, right_indent,
            minx, maxx, miny, maxy, color);
    }
}

void n_graphics_fill_rect(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask) {
    if (!(ctx->stroke_color.argb & (0b11 << 6)))
        ;
    else if (radius == 0 || (mask & 0b1111) == 0)
        n_graphics_fill_0rad_rect_bounded(ctx, rect, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    else
        n_graphics_fill_rect_bounded(ctx, rect, radius, mask, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
}
