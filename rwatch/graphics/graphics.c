/* graphics.c
 * routines for manipulating graphics layers when drawing
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "upng.h"
#include "png.h"
#include "graphics_wrapper.h"

/* Configure Logging */
#define MODULE_NAME "grphcs"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_INFO //RBL_LOG_LEVEL_ERROR
static GBitmap _fb_gbitmap;

static GRect _jimmy_layer_offset(n_GContext *ctx, n_GRect rect)
{
    // jimmy the offsets for the layer before we ask ngfx to draw it
    return (GRect) {
        .origin.x = rect.origin.x + ctx->offset.origin.x,
        .origin.y = rect.origin.y + ctx->offset.origin.y, 
        /* Seems like clipping to the parent frame 
         *(likely the screen size in most cases) isn't the right thing */
        .size.w = rect.size.w, //(rect.size.w > ctx->offset.size.w ? ctx->offset.size.w : rect.size.w), 
        .size.h = rect.size.h, // > ctx->offset.size.h ? ctx->offset.size.h : rect.size.h), 
    };
}

static n_GPoint _jimmy_layer_point_offset(n_GContext *ctx, n_GPoint point)
{
    // jimmy the offsets for the layer before we ask ngfx to draw it
    return (GPoint) {
        .x = point.x + ctx->offset.origin.x,
        .y = point.y + ctx->offset.origin.y,
    };
}


// void n_graphics_fill_rect_app(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask);
void graphics_fill_rect(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask)
{
    n_graphics_fill_rect(ctx, _jimmy_layer_offset(ctx, rect), radius, mask);
}

void graphics_fill_circle(n_GContext * ctx, n_GPoint p, uint16_t radius)
{
    n_graphics_fill_circle(ctx, _jimmy_layer_point_offset(ctx, p), radius);
}

void graphics_draw_circle(n_GContext * ctx, n_GPoint p, uint16_t radius)
{
    n_graphics_draw_circle(ctx, _jimmy_layer_point_offset(ctx, p), radius);
}

void graphics_draw_line(n_GContext * ctx, n_GPoint from, n_GPoint to)
{
    n_graphics_draw_line(ctx, 
                         _jimmy_layer_point_offset(ctx, from), 
                         _jimmy_layer_point_offset(ctx, to));
}

void graphics_draw_text(
    n_GContext * ctx, const char * text, n_GFont const font, const n_GRect box,
    const n_GTextOverflowMode overflow_mode, const n_GTextAlignment alignment,
    n_GTextAttributes * text_attributes)
{
    LOG_DEBUG("text");
    n_graphics_draw_text(ctx, text, font, _jimmy_layer_offset(ctx, box),
                            overflow_mode, alignment,
                            text_attributes);
}

void graphics_draw_bitmap_in_rect(GContext *ctx, const GBitmap *bitmap, GRect rect)
{
    LOG_DEBUG("gbir");
    GRect offsetted = _jimmy_layer_offset(ctx, rect);
    n_graphics_draw_bitmap_in_rect(ctx, bitmap, offsetted);
}


void graphics_draw_pixel(n_GContext * ctx, n_GPoint p)
{
    LOG_DEBUG("dip");
    n_graphics_draw_pixel(ctx, _jimmy_layer_point_offset(ctx, p));

}

void graphics_draw_rect(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask)
{
    LOG_DEBUG("rect");
    n_graphics_draw_rect(ctx, _jimmy_layer_offset(ctx, rect), radius, mask);
}


GBitmap *graphics_capture_frame_buffer(n_GContext *context)
{
    // rbl_lock_frame_buffer
    if (!_fb_gbitmap.addr)
    {
        _fb_gbitmap.addr = display_get_buffer();
        _fb_gbitmap.raw_bitmap_size.w = DISPLAY_COLS;
        _fb_gbitmap.raw_bitmap_size.h = DISPLAY_ROWS;
        _fb_gbitmap.row_size_bytes = DISPLAY_COLS;
        _fb_gbitmap.bounds = GRect(0, DISPLAY_COLS, 0, DISPLAY_ROWS);
        _fb_gbitmap.format = n_GBitmapFormat8Bit;
    }
    return &_fb_gbitmap;
}

GBitmap *graphics_capture_frame_buffer_format(n_GContext *context, GBitmap format)
{
    // rbl_lock_frame_buffer
    LOG_DEBUG("fb lock");
    return (GBitmap *)display_get_buffer();
}

void graphics_release_frame_buffer(n_GContext *context, GBitmap *bitmap)
{
    // rbl_unlock_frame_buffer
    LOG_DEBUG("fb unlock");
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
