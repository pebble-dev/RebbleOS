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
void gpath_fill_app(n_GContext * ctx, n_GPath * path);
void gpath_draw_app(n_GContext * ctx, n_GPath * path);
void gpath_rotate_to_app(n_GPath * path, int32_t angle);
void gpath_move_to_app(n_GPath * path, n_GPoint offset);

// (VoidFunc)graphics_draw_round_rect_app,

// n_GPath * gpath_create_app(n_GPathInfo * path_info);
n_GRect _jimmy_layer_offset(n_GContext *ctx, n_GRect rect);
n_GPoint _jimmy_layer_point_offset(n_GContext *ctx, n_GPoint point);

GBitmap *graphics_capture_frame_buffer(n_GContext *context);
GBitmap *graphics_capture_frame_buffer_format(n_GContext *context, GBitmap format);
void graphics_release_frame_buffer(n_GContext *context, GBitmap *bitmap);
