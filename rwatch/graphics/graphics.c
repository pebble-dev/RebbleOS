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

void grect_align(GRect *rect, const GRect *inside_rect, const GAlign alignment, const bool clip)
{
    int16_t x = 0, y = 0;
    /* Align x */
    switch (alignment)
    {
        case GAlignBottomLeft:
        case GAlignLeft:
        case GAlignTopLeft:
            x = 0;
            break;
        case GAlignBottomRight:
        case GAlignRight:
        case GAlignTopRight:
            x = inside_rect->size.w - rect->size.w;
            break;
        case GAlignBottom:
        case GAlignCenter:
        case GAlignTop:
            x = (inside_rect->size.w - rect->size.w) / 2;
            break;
    }

    /* Align y */
    switch (alignment)
    {
        case GAlignBottom:
        case GAlignBottomLeft:
        case GAlignBottomRight:
            y = inside_rect->size.h - rect->size.h;
            break;
        case GAlignLeft:
        case GAlignRight:
        case GAlignCenter:
            y = (inside_rect->size.h - rect->size.h) / 2;
            break;
        case GAlignTopLeft:
        case GAlignTopRight:
        case GAlignTop:
            y = 0;
            break;
    }

    if (clip)
    {
        if (x < 0)
        {
            rect->size.w -= x;
            x = 0;
        }
        if (x + rect->size.w > inside_rect->size.w) {
            rect->size.w -= (x + rect->size.w) - inside_rect->size.w;
        }
        if (y < 0)
        {
            rect->size.h -= y;
            y = 0;
        }
        if (y + rect->size.h > inside_rect->size.h)
        {
            rect->size.h -= (y + rect->size.h) - inside_rect->size.h;
        }
    }

    rect->origin.x = inside_rect->origin.x + x;
    rect->origin.y = inside_rect->origin.y + y;
}


void grect_standardize(GRect *rect)
{
    if (rect->size.w < 0)
    {
        rect->origin.x += rect->size.w;
        rect->size.w = -rect->size.w;
    }

    if (rect->size.h < 0)
    {
        rect->origin.y += rect->size.h;
        rect->size.h = -rect->size.h;
    }
}

//! Returns a rectangle that is shrinked or expanded by the given edge insets.
//! @note The rectangle is standardized and then the inset parameters are applied.
//! If the resulting rectangle would have a negative height or width, a GRectZero is returned.
//! @param rect The rectangle that will be inset
//! @param insets The insets that will be applied
//! @return The resulting rectangle
//! @note Use this function in together with the \ref GEdgeInsets macro
//! \code{.c}
//! GRect r_inset_all_sides = grect_inset(r, GEdgeInsets(10));
//! GRect r_inset_vertical_horizontal = grect_inset(r, GEdgeInsets(10, 20));
//! GRect r_expand_top_right_shrink_bottom_left = grect_inset(r, GEdgeInsets(-10, -10, 10, 10));
//! \endcode
GRect grect_inset(GRect rect, GEdgeInsets insets)
{
    GRect r;
    // top, right, bottom, left
    r.origin.x = insets.left;
    r.origin.x = insets.top;
    r.size.w = rect.size.w - insets.right - insets.left;
    r.size.h = rect.size.h - insets.top - insets.bottom;
    return r;
}

bool graphics_frame_buffer_is_captured(GContext * ctx)
{
    return display_is_buffer_locked();
}
