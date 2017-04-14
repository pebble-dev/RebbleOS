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

#include "circle.h"

void n_graphics_fill_circle_bounded(n_GContext * ctx, n_GPoint p, uint16_t radius,
        int16_t minx, int16_t maxx, int16_t miny, int16_t maxy) {
    int32_t err = 1 - radius,
            err_a = -radius * 2,
            err_b = 0;
    uint16_t a = radius,
             b = 0;
#ifdef PBL_BW
    uint8_t bytefill = __ARGB_TO_INTERNAL(ctx->fill_color.argb);
#else
    uint8_t bytefill = ctx->fill_color.argb;
#endif
    while (b <= a) {
        n_graphics_prv_draw_row(ctx->fbuf, p.y - b, p.x - a, p.x + a, minx, maxx, miny, maxy, bytefill);
        n_graphics_prv_draw_row(ctx->fbuf, p.y + b, p.x - a, p.x + a, minx, maxx, miny, maxy, bytefill);
        if (err >= 0) {
            n_graphics_prv_draw_row(ctx->fbuf, p.y - a, p.x - b, p.x + b, minx, maxx, miny, maxy, bytefill);
            n_graphics_prv_draw_row(ctx->fbuf, p.y + a, p.x - b, p.x + b, minx, maxx, miny, maxy, bytefill);
            b += 1;
            a -= 1;
            err_a += 2;
            err_b += 2;
            err += err_a + err_b;
        } else {
            b += 1;
            err_b += 2;
            err += err_b + 1;
        }
    }
}

void n_graphics_draw_circle_1px_bounded(n_GContext * ctx, n_GPoint p, uint16_t radius, int16_t minx, int16_t maxx, int16_t miny, int16_t maxy) {
    uint16_t a = radius,
             b = 0;
    int16_t err = 1 - a,
             err_a = -a * 2,
             err_b = 1;
    while (b <= a) {
        if (p.x + a >= minx && p.x + a < maxx) {
            if (p.y + b >= miny && p.y + b < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x + a, p.y + b));
            if (p.y - b >= miny && p.y - b < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x + a, p.y - b));
        }
        if (p.x - a >= minx && p.x - a < maxx) {
            if (p.y + b >= miny && p.y + b < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x - a, p.y + b));
            if (p.y - b >= miny && p.y - b < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x - a, p.y - b));
        }
        if (p.x + b >= minx && p.x + b < maxx) {
            if (p.y + a >= miny && p.y + a < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x + b, p.y + a));
            if (p.y - a >= miny && p.y - a < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x + b, p.y - a));
        }
        if (p.x - b >= minx && p.x - b < maxx) {
            if (p.y + a >= miny && p.y + a < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x - b, p.y + a));
            if (p.y - a >= miny && p.y - a < maxy)
                n_graphics_draw_pixel(ctx, n_GPoint(p.x - b, p.y - a));
        }
        if (err >= 0) {
            a -= 1;
            b += 1;
            err_a += 2;
            err_b += 2;
            err += err_a + err_b;
        } else {
            b += 1;
            err_b += 2;
            err += err_b + 1;
        }
    }
}

void n_graphics_prv_draw_quarter_circle_bounded(n_GContext * ctx, n_GPoint p,
        uint16_t radius, uint16_t width, int8_t x_dir, int8_t y_dir,
        int16_t minx, int16_t maxx, int16_t miny, int16_t maxy) {
    uint16_t line_radius = (width - 1) / 2;
    uint16_t a1 = __BOUND_NUM(0, radius - line_radius, radius),
             b1 = 0,
             a2 = radius + line_radius,
             b2 = 0;
    int16_t err1 = 1 - a1,
            err_a1 = -a1 * 2,
            err_b1 = 1,
            // ---
            err2 = 1 - a2,
            err_a2 = -a2 * 2,
            err_b2 = 1;
#ifdef PBL_BW
    uint8_t color = __ARGB_TO_INTERNAL(ctx->stroke_color.argb);
#else
    uint8_t color = ctx->stroke_color.argb;
#endif
    if (y_dir == 1)
        n_graphics_prv_draw_col(ctx->fbuf, p.x + b2 * x_dir, p.y + a1, p.y + a2,
                                minx, maxx, miny, maxy, color);
    else
        n_graphics_prv_draw_col(ctx->fbuf, p.x + b2 * x_dir, p.y - a2, p.y - a1,
                                minx, maxx, miny, maxy, color);
    if (x_dir == 1)
        n_graphics_prv_draw_row(ctx->fbuf, p.y + b2 * y_dir, p.x + a1, p.x + a2,
                                minx, maxx, miny, maxy, color);
    else
        n_graphics_prv_draw_row(ctx->fbuf, p.y + b2 * y_dir, p.x - a2, p.x - a1,
                                minx, maxx, miny, maxy, color);
    while ((b1 <= a1 && a1 != 0) || b2 <= a2) {
        if (b2 <= a2 && a2 != 0) {
            if (err2 >= 0) {
                b2 += 1;
                a2 -= 1;
                err_a2 += 2;
                err_b2 += 2;
                err2 += err_a2 + err_b2;
            } else {
                b2 += 1;
                err_b2 += 2;
                err2 += err_b2 + 1;
            }
        }
        if (b1 <= a1 && a1 != 0) {
            if (err1 >= 0) {
                a1 -= 1;
                b1 += 1;
                err_a1 += 2;
                err_b1 += 2;
                err1 += err_a1 + err_b1;
            } else {
                b1 += 1;
                err_b1 += 2;
                err1 += err_b1 + 1;
            }
        }
        // TODO possible optimization by not rendering so far
        if (y_dir == 1)
            n_graphics_prv_draw_col(ctx->fbuf, p.x + b2 * x_dir, p.y + a1, p.y + a2,
                                    minx, maxx, miny, maxy, color);
        else
            n_graphics_prv_draw_col(ctx->fbuf, p.x + b2 * x_dir, p.y - a2, p.y - a1,
                                    minx, maxx, miny, maxy, color);
        if (x_dir == 1)
            n_graphics_prv_draw_row(ctx->fbuf, p.y + b2 * y_dir, p.x + a1, p.x + a2,
                                    minx, maxx, miny, maxy, color);
        else
            n_graphics_prv_draw_row(ctx->fbuf, p.y + b2 * y_dir, p.x - a2, p.x - a1,
                                    minx, maxx, miny, maxy, color);
    }
}

void n_graphics_prv_fill_quarter_circle_bounded(n_GContext * ctx, n_GPoint p,
        uint16_t radius, int8_t x_dir, int8_t y_dir,
        int16_t minx, int16_t maxx, int16_t miny, int16_t maxy) {
    int32_t err = 1 - radius,
            err_a = -radius * 2,
            err_b = 0;
    uint16_t a = radius,
             b = 0;
#ifdef PBL_BW
    uint8_t bytefill = __ARGB_TO_INTERNAL(ctx->fill_color.argb);
#else
    uint8_t bytefill = ctx->fill_color.argb;
#endif
    while (b <= a) {
        if (x_dir == 1) {
            n_graphics_prv_draw_row(ctx->fbuf, p.y + b * y_dir, p.x, p.x + a, minx, maxx, miny, maxy, bytefill);
        } else {
            n_graphics_prv_draw_row(ctx->fbuf, p.y + b * y_dir, p.x - a, p.x, minx, maxx, miny, maxy, bytefill);
        }

        if (err >= 0) {
            if (x_dir == 1) {
                n_graphics_prv_draw_row(ctx->fbuf, p.y + a * y_dir, p.x, p.x + b, minx, maxx, miny, maxy, bytefill);
            } else {
                n_graphics_prv_draw_row(ctx->fbuf, p.y + a * y_dir, p.x - b, p.x, minx, maxx, miny, maxy, bytefill);
            }
            a -= 1;
            b += 1;
            err_a += 2;
            err_b += 2;
            err += err_a + err_b;
        } else {
            b += 1;
            err_b += 2;
            err += err_b + 1;
        }
    }
}

void n_graphics_draw_thick_circle_bounded(n_GContext * ctx, n_GPoint p, uint16_t radius, uint16_t width, int16_t minx, int16_t maxx, int16_t miny, int16_t maxy) {
    uint16_t line_radius = (width - 1) / 2;
    uint16_t a1 = __BOUND_NUM(0, radius - line_radius, radius),
             b1 = 0,
             a2 = radius + line_radius,
             b2 = 0;
    int16_t err1 = 1 - a1,
            err_a1 = -a1 * 2,
            err_b1 = 1,
            // ---
            err2 = 1 - a2,
            err_a2 = -a2 * 2,
            err_b2 = 1;
#ifdef PBL_BW
    uint8_t color = __ARGB_TO_INTERNAL(ctx->stroke_color.argb);
#else
    uint8_t color = ctx->stroke_color.argb;
#endif

    n_graphics_prv_draw_col(ctx->fbuf, p.x + b2, p.y - a2, p.y - a1,
                            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_col(ctx->fbuf, p.x + b2, p.y + a1, p.y + a2,
                            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_col(ctx->fbuf, p.x - b2, p.y - a2, p.y - a1,
                            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_col(ctx->fbuf, p.x - b2, p.y + a1, p.y + a2,
                            minx, maxx, miny, maxy, color);

    n_graphics_prv_draw_row(ctx->fbuf, p.y + b2, p.x - a2, p.x - a1,
                            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_row(ctx->fbuf, p.y + b2, p.x + a1, p.x + a2,
                            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_row(ctx->fbuf, p.y - b2, p.x - a2, p.x - a1,
                            minx, maxx, miny, maxy, color);
    n_graphics_prv_draw_row(ctx->fbuf, p.y - b2, p.x + a1, p.x + a2,
                            minx, maxx, miny, maxy, color);
    while ((b1 <= a1 && a1 != 0) || b2 <= a2) {
        if (b2 <= a2 && a2 != 0) {
            if (err2 >= 0) {
                b2 += 1;
                a2 -= 1;
                err_a2 += 2;
                err_b2 += 2;
                err2 += err_a2 + err_b2;
            } else {
                b2 += 1;
                err_b2 += 2;
                err2 += err_b2 + 1;
            }
        }
        if (b1 <= a1 && a1 != 0) {
            if (err1 >= 0) {
                a1 -= 1;
                b1 += 1;
                err_a1 += 2;
                err_b1 += 2;
                err1 += err_a1 + err_b1;
            } else {
                b1 += 1;
                err_b1 += 2;
                err1 += err_b1 + 1;
            }
        }
        // TODO possible optimization by not rendering so far
        n_graphics_prv_draw_col(ctx->fbuf, p.x + b2, p.y - a2, p.y - a1,
                                minx, maxx, miny, maxy, color);
        n_graphics_prv_draw_col(ctx->fbuf, p.x + b2, p.y + a1, p.y + a2,
                                minx, maxx, miny, maxy, color);
        n_graphics_prv_draw_col(ctx->fbuf, p.x - b2, p.y - a2, p.y - a1,
                                minx, maxx, miny, maxy, color);
        n_graphics_prv_draw_col(ctx->fbuf, p.x - b2, p.y + a1, p.y + a2,
                                minx, maxx, miny, maxy, color);

        n_graphics_prv_draw_row(ctx->fbuf, p.y + b2, p.x - a2, p.x - a1,
                                minx, maxx, miny, maxy, color);
        n_graphics_prv_draw_row(ctx->fbuf, p.y + b2, p.x + a1, p.x + a2,
                                minx, maxx, miny, maxy, color);
        n_graphics_prv_draw_row(ctx->fbuf, p.y - b2, p.x - a2, p.x - a1,
                                minx, maxx, miny, maxy, color);
        n_graphics_prv_draw_row(ctx->fbuf, p.y - b2, p.x + a1, p.x + a2,
                                minx, maxx, miny, maxy, color);
    }
}

void n_graphics_draw_circle(n_GContext * ctx, n_GPoint p, uint16_t radius) {
    if (radius == 0 || !(ctx->stroke_color.argb & (0b11 << 6)))
        return;
    if (ctx->stroke_width == 1) {
        n_graphics_draw_circle_1px_bounded(ctx, p, radius, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    } else {
        // naive approach; testing for speed
        n_graphics_draw_thick_circle_bounded(ctx, p, radius, ctx->stroke_width, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
    }
}

void n_graphics_fill_circle(n_GContext * ctx, n_GPoint p, uint16_t radius) {
    if (ctx->fill_color.argb & (0b11 << 6))
        n_graphics_fill_circle_bounded(ctx, p, radius, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
}
