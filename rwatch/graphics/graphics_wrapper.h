#pragma once
#include "librebble.h"
#include "upng.h"
#include "png.h"
#include "graphics.h"
void graphics_fill_rect(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask);
void graphics_fill_circle(n_GContext * ctx, n_GPoint p, uint16_t radius);
void graphics_draw_circle(n_GContext * ctx, n_GPoint p, uint16_t radius);
void graphics_draw_line(n_GContext * ctx, n_GPoint from, n_GPoint to);
void graphics_draw_text(
    n_GContext * ctx, const char * text, n_GFont const font, const n_GRect box,
    const n_GTextOverflowMode overflow_mode, const n_GTextAlignment alignment,
    n_GTextAttributes * text_attributes);
void graphics_draw_text_ex(
    n_GContext * ctx, const char * text, n_GFont const font, const n_GRect box,
    const n_GTextOverflowMode overflow_mode, const n_GTextAlignment alignment,
    n_GTextAttributes * text_attributes, n_GSize *outsz);
void graphics_draw_bitmap_in_rect(GContext *ctx, const GBitmap *bitmap, GRect rect);
void graphics_draw_pixel(n_GContext * ctx, n_GPoint p);
void graphics_draw_rect(n_GContext * ctx, n_GRect rect, uint16_t radius, n_GCornerMask mask);
GBitmap *graphics_capture_frame_buffer(n_GContext *context);