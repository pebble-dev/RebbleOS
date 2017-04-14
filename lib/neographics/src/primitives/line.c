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

#include "line.h"

void n_graphics_prv_draw_1px_line_bounded(n_GContext * ctx,
                                             n_GPoint from, n_GPoint to,
                                             int16_t minx, int16_t maxx,
                                             int16_t miny, int16_t maxy) {
    bool iterate_over_y = false;
#ifdef PBL_BW
    uint8_t color = __ARGB_TO_INTERNAL(ctx->stroke_color.argb);
#endif
    int16_t dy = (to.y - from.y), dx = (to.x - from.x);
    if (abs(dy) > abs(dx)) {
        iterate_over_y = true;
    }
    if (    (iterate_over_y && dy < 0) ||
           (!iterate_over_y && dx < 0)) {
        n_GPoint temp = from;
        from = to;
        to = temp;
        dy = -dy;
        dx = -dx;
    }
    if (iterate_over_y) {
        int8_t e = (dx == 0 ? 0 : (dx > 0 ? 1 : -1));
        int16_t begin = __BOUND_NUM(miny, from.y, maxy - 1);
        int16_t end = __BOUND_NUM(miny, to.y, maxy - 1);
        if (e == 0) {
#ifdef PBL_BW
            n_graphics_prv_draw_col(ctx->fbuf, from.x, from.y, to.y,
                                    minx, maxx, miny, maxy, color);
#else
            n_graphics_prv_draw_col(ctx->fbuf, from.x, from.y, to.y,
                                    minx, maxx, miny, maxy, ctx->stroke_color.argb);
#endif
        } else {
            for (int16_t y = begin; y <= end; y++) {
                int16_t x = (dx * (y-from.y) * 2 + e * dy) / (dy * 2) + from.x;
#ifdef PBL_BW
                n_graphics_set_pixel(ctx, n_GPoint(x, y),
                    ((color >> ((x + y) % 2)) & 1) ?
                        (n_GColor) {.argb = 1} :
                        (n_GColor) {.argb = 0});
#else
                n_graphics_set_pixel(ctx, n_GPoint(x, y), ctx->stroke_color);
#endif
            }
        }
    } else {
        int8_t e = (dy == 0 ? 0 : (dy > 0 ? 1 : -1));
        int16_t begin = __BOUND_NUM(minx, from.x, maxx - 1);
        int16_t end = __BOUND_NUM(minx, to.x, maxx - 1);
        if (e == 0) {
#ifdef PBL_BW
            n_graphics_prv_draw_row(ctx->fbuf, from.y, from.x, to.x,
                                    minx, maxx, miny, maxy, color);
#else
            n_graphics_prv_draw_row(ctx->fbuf, from.y, from.x, to.x,
                                    minx, maxx, miny, maxy, ctx->stroke_color.argb);
#endif
        } else {
            for (int16_t x = begin; x <= end; x++) {
                int16_t y = (dy * (x-from.x) * 2 + e * dx) / (dx * 2) + from.y;
#ifdef PBL_BW
                n_graphics_set_pixel(ctx, n_GPoint(x, y),
                    ((color >> ((x + y) % 2)) & 1) ?
                        (n_GColor) {.argb = 1} :
                        (n_GColor) {.argb = 0});
#else
                n_graphics_set_pixel(ctx, n_GPoint(x, y), ctx->stroke_color);
#endif
            }
        }
    }
}

static uint16_t prv_fast_int_sqrt(uint16_t in) {
    uint16_t a = 1, b = in;
    while (abs(a - b) > 1) { // unrolled to 3 runs/iter for speed
        b = in / a;
        a = (a + b) / 2;
        if (!abs(a - b) > 1)
            return a;
        b = in / a;
        a = (a + b) / 2;
        if (!abs(a - b) > 1)
            return a;
        b = in / a;
        a = (a + b) / 2;
    }
    return a;
}

void n_graphics_prv_draw_thick_line_bounded(n_GContext * ctx,
                                               n_GPoint from, n_GPoint to,
                                               uint8_t width,
                                               int16_t minx, int16_t maxx,
                                               int16_t miny, int16_t maxy) {
    uint16_t radius = (width - 1) / 2;
    if (ctx->stroke_caps) {
        n_GColor tmp_fill = ctx->fill_color;
        ctx->fill_color = ctx->stroke_color;
        n_graphics_fill_circle_bounded(ctx, from, radius, minx, maxx, miny, maxy);
        n_graphics_fill_circle_bounded(ctx, to, radius, minx, maxx, miny, maxy);
        ctx->fill_color = tmp_fill;
    }
    // At this point (see what I did there?), we have to calculate the points
    // which allow us to connect the two drawn circles.

    // SO, here's what we do: we use an integer which keeps being upped until
    // we reach the point where the point translated by a part of the dx/dy pair
    // hits the circle's radius roundabout in the right spot.

    int16_t dx = to.x - from.x, dy = to.y - from.y;
    if (dx > 100) {
        dy = dy * 100 / dx;
        dx = 100;
    } else if (dy > 100) {
        dx = dx * 100 / dy;
        dy = 100;
    }

    n_GPoint from_a = from,
             to_a = to,
             from_b = from,
             to_b = to;
    uint16_t multiplier = 1;
    int16_t sep_dx, sep_dy;
    if (dx == 0) {
        sep_dx = 0;
        sep_dy = width / 2;
    } else if (dy == 0) {
        sep_dx = width / 2;
        sep_dy = 0;
    } else {
        // We're doing a search where we step forward in 100s.
        for (multiplier = 1; multiplier < 40000; multiplier += 10) {
            sep_dx = dx * multiplier / 100;
            sep_dy = dy * multiplier / 100;
            if (prv_fast_int_sqrt(sep_dx * sep_dx + sep_dy * sep_dy) > width / 2) {
                break;
            }
        }
        uint16_t rooted = 0;
        // ...and then step back in 1s.
        do {
            multiplier -= 1;
            sep_dx = dx * multiplier / 100;
            sep_dy = dy * multiplier / 100;
            rooted = prv_fast_int_sqrt(sep_dx * sep_dx + sep_dy * sep_dy);
        } while (multiplier > 1 && rooted > width / 2);

        sep_dx = dx * multiplier / 100;
        sep_dy = dy * multiplier / 100;
    }

    from_a.x -= sep_dy;
    from_a.y += sep_dx;
    to_a.x -= sep_dy;
    to_a.y += sep_dx;

    from_b.x += sep_dy;
    from_b.y -= sep_dx;
    to_b.x += sep_dy;
    to_b.y -= sep_dx;

    int8_t xdir = (from_a.x > from_b.x ? -1 : 1);
    int8_t ydir = (from_a.y > from_b.y ? -1 : 1);

    n_graphics_prv_draw_1px_line_bounded(ctx, from_a, to_a, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    n_graphics_prv_draw_1px_line_bounded(ctx, from_b, to_b, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);

    if (!ctx->stroke_caps || true) {
        // TODO this doesn't look good yet:tm: because the translated line
        // isn't always fully contained within the stroke.
        n_graphics_prv_draw_1px_line_bounded(ctx, from_a, from_b, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
        n_graphics_prv_draw_1px_line_bounded(ctx, to_a, to_b, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    }

    bool change_x = false;
    while (from_a.x != from_b.x || from_a.y != from_b.y) {
        if (ctx->stroke_caps) {
            // simple, quick algo
            // used for round caps to save those precious cpu cycles
            change_x = abs(from_b.x - from_a.x) > abs(from_b.y - from_a.y);
        } else {
            // expensive division: only run this when it's
            // actually needed (i.e. when stroke caps are hidden.)
            change_x = (from_a.y == from_b.y ||
                         (from_a.x != from_b.x &&
                          abs(((from_a.x - from_b.x) << 3) / sep_dy) >
                          abs(((from_a.y - from_b.y) << 3) / sep_dx)));
        }
        if (change_x) {
            from_a.x += xdir;
            to_a.x += xdir;
        } else {
            from_a.y += ydir;
            to_a.y += ydir;
        }
        n_graphics_prv_draw_1px_line_bounded(ctx, from_a, to_a, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
        n_graphics_prv_draw_1px_line_bounded(ctx, from_b, to_b, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    }
}

void n_graphics_draw_line(n_GContext * ctx, n_GPoint from, n_GPoint to) {
    // TODO check layer bounds
    if (ctx->stroke_width == 0 || !(ctx->stroke_color.argb & (0b11 << 6)))
        return;
    else if (ctx->stroke_width == 1)
        n_graphics_prv_draw_1px_line_bounded(ctx, from, to,
            0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    else
        n_graphics_prv_draw_thick_line_bounded(ctx, from, to, ctx->stroke_width,
            0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
}
