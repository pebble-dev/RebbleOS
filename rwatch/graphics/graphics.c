/* graphics.c
 * routines for manipulating graphics layers when drawing
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "upng.h"
#include "png.h"


GRect _jimmy_layer_offset(n_GContext *ctx, n_GRect rect)
{
    // jimmy the offsets for the layer before we ask ngfx to draw it
    return (GRect) {
        .origin.x = rect.origin.x + ctx->offset.origin.x,
        .origin.y = rect.origin.y + ctx->offset.origin.y, 
        .size.w = (rect.size.w > ctx->offset.size.w ? ctx->offset.size.w : rect.size.w), 
        .size.h = (rect.size.h > ctx->offset.size.h ? ctx->offset.size.h : rect.size.h), 
    };
}

n_GPoint _jimmy_layer_point_offset(n_GContext *ctx, n_GPoint point)
{
    // jimmy the offsets for the layer before we ask ngfx to draw it
    return (GPoint) {
        .x = point.x + ctx->offset.origin.x,
        .y = point.y + ctx->offset.origin.y,
    };
}


// void n_graphics_fill_rect_app(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask);
void graphics_fill_rect_app(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask)
{
    n_graphics_fill_rect(ctx, _jimmy_layer_offset(ctx, rect), radius, mask);
}

void graphics_fill_circle_app(n_GContext * ctx, n_GPoint p, uint16_t radius)
{
    n_graphics_fill_circle(ctx, _jimmy_layer_point_offset(ctx, p), radius);
}

void graphics_draw_circle_app(n_GContext * ctx, n_GPoint p, uint16_t radius)
{
    n_graphics_draw_circle(ctx, _jimmy_layer_point_offset(ctx, p), radius);
}

void graphics_draw_line_app(n_GContext * ctx, n_GPoint from, n_GPoint to)
{
    n_graphics_draw_line(ctx, 
                         _jimmy_layer_point_offset(ctx, from), 
                         _jimmy_layer_point_offset(ctx, to));
}

void graphics_draw_text_app(
    n_GContext * ctx, const char * text, n_GFont const font, const n_GRect box,
    const n_GTextOverflowMode overflow_mode, const n_GTextAlignment alignment,
    n_GTextAttributes * text_attributes)
{
    n_graphics_draw_text(ctx, text, font, _jimmy_layer_offset(ctx, box),
                            overflow_mode, alignment,
                            text_attributes);
}


void graphics_draw_pixel_app(n_GContext * ctx, n_GPoint p)
{
    n_graphics_draw_pixel(ctx, _jimmy_layer_point_offset(ctx, p));

}

void graphics_draw_rect_app(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask)
{
    n_graphics_draw_rect(ctx, _jimmy_layer_offset(ctx, rect), radius, mask);
}


void gpath_fill_app(n_GContext * ctx, n_GPath * path)
{
    GPoint off = path->offset;
    GPoint r = _jimmy_layer_point_offset(ctx, path->offset);
    path->offset.x = r.x;
    path->offset.y = r.y;
    n_gpath_fill(ctx, path);
    path->offset = off;
}


void gpath_draw_app(n_GContext * ctx, n_GPath * path)
{
    GPoint off = path->offset;
    GPoint r = _jimmy_layer_point_offset(ctx, path->offset);
    path->offset.x = r.x;
    path->offset.y = r.y;
    n_gpath_draw(ctx, path);
    path->offset = off;
}

void gpath_rotate_to_app(n_GPath * path, int32_t angle)
{
//     GPoint off = path->offset;
//     GPoint r = _jimmy_layer_point_offset(ctx, path->offset);
//     path->offset.x = r.x;
//     path->offset.y = r.y;
    n_gpath_rotate_to(path, angle);
//     path->offset = off;
}

void gpath_move_to_app(n_GPath * path, n_GPoint offset)
{
//     GPoint off = path->offset;
//     GPoint r = _jimmy_layer_point_offset(ctx, path->offset);
//     path->offset.x = r.x;
//     path->offset.y = r.y;
    n_gpath_move_to(path, offset);
//     path->offset = off;
}
