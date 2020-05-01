#pragma once
/* graphics.h
 * routines for manipulating graphics layers when drawing
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

void graphics_fill_rect_app(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask);
void graphics_fill_circle_app(n_GContext * ctx, n_GPoint p, uint16_t radius);
void graphics_draw_circle_app(n_GContext * ctx, n_GPoint p, uint16_t radius);
void graphics_draw_line_app(n_GContext * ctx, n_GPoint from, n_GPoint to);
void graphics_draw_text_app(
    n_GContext * ctx, const char * text, n_GFont const font, const n_GRect box,
    const n_GTextOverflowMode overflow_mode, const n_GTextAlignment alignment,
    n_GTextAttributes * text_attributes);
void graphics_draw_pixel_app(n_GContext * ctx, n_GPoint p);
void graphics_draw_rect_app(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask);
void graphics_draw_bitmap_in_rect_app(GContext *ctx, const GBitmap *bitmap, GRect rect);
void gpath_fill_app(n_GContext * ctx, n_GPath * path);
void gpath_draw_app(n_GContext * ctx, n_GPath * path);
void gpath_rotate_to_app(n_GPath * path, int32_t angle);
void gpath_move_to_app(n_GPath * path, n_GPoint offset);

// (VoidFunc)graphics_draw_round_rect_app,

// n_GPath * gpath_create_app(n_GPathInfo * path_info);

GBitmap *graphics_capture_frame_buffer(n_GContext *context);
GBitmap *graphics_capture_frame_buffer_format(n_GContext *context, GBitmap format);
void graphics_release_frame_buffer(n_GContext *context, GBitmap *bitmap);
bool graphics_frame_buffer_is_captured(GContext * ctx);

void grect_align(GRect *rect, const GRect *inside_rect, const GAlign alignment, const bool clip);
void grect_standardize(GRect *rect);
GColor graphics_gcolor_from_2bit(int color_2bit);
void graphics_context_set_fill_color_2bit(GContext * ctx, int color);
void graphics_context_set_stroke_color_2bit(GContext * ctx, int color);
void gpath_fill_app_legacy(n_GContext * ctx, n_GPath * path);
void graphics_context_set_text_color_2bit(GContext * ctx, int color);