#pragma once
/* gbitmap.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "types.h"
#include "pebble_defines.h"
#include "color.h"
#include "upng.h"

struct file;

typedef enum GAlign
{
    GAlignCenter,
    GAlignTopLeft,
    GAlignTopRight,
    GAlignTop,
    GAlignLeft,
    GAlignBottom,
    GAlignRight,
    GAlignBottomRight,
    GAlignBottomLeft
} GAlign;


typedef enum GCompOp {
    GCompOpAssign = 0,
    GCompOpAssignInverted,
    GCompOpOr,
    GCompOpAnd,
    GCompOpClear,
    GCompOpSet
} GCompOp;

typedef enum GBitmapFormat {
  GBitmapFormat1Bit = 0,
  GBitmapFormat8Bit,
  GBitmapFormat1BitPalette,
  GBitmapFormat2BitPalette,
  GBitmapFormat4BitPalette,
} GBitmapFormat;

typedef struct GBitmap
{
    uint8_t *addr;
    n_GSize raw_bitmap_size;
    n_GColor *palette;
    uint8_t palette_size;
    uint16_t row_size_bytes;
//     uint16_t info_flags;
    bool free_palette_on_destroy; // TODo move me to a bit status register above for size
    bool free_data_on_destroy; // TODo move me to a bit status register above for size
    n_GRect bounds;
    GBitmapFormat format;
} GBitmap;



bool gcolor_equal(GColor8 x, GColor8 y);
GColor8 gcolor_legible_over(GColor8 background_color);
bool gpoint_equal(const GPoint *const point_a, const GPoint *const point_b);
bool gsize_equal(const GSize *size_a, const GSize *size_b);
bool grect_equal(const GRect *const rect_a, const GRect *const rect_b);
bool grect_is_empty(const GRect *const rect);
void grect_standardize(GRect *rect);
void grect_clip(GRect *const rect_to_clip, const GRect *const rect_clipper);
bool grect_contains_point(const GRect *rect, const GPoint *point);
// GPoint n_graphics_center_point_rect(const GRect *rect);
GRect grect_crop(GRect rect, const int32_t crop_size_px);
uint16_t gbitmap_get_bytes_per_row(const GBitmap *bitmap);
GBitmapFormat gbitmap_get_format(const GBitmap *bitmap);
uint8_t *gbitmap_get_data(const GBitmap *bitmap);
void gbitmap_set_data(GBitmap *bitmap, uint8_t *data, GBitmapFormat format, uint16_t row_size_bytes, bool free_on_destroy);
GRect gbitmap_get_bounds(const GBitmap *bitmap);
void gbitmap_set_bounds(GBitmap *bitmap, GRect bounds);
GColor *gbitmap_get_palette(const GBitmap *bitmap);
void gbitmap_set_palette(GBitmap *bitmap, GColor *palette, bool free_on_destroy);
GBitmap *gbitmap_create_with_resource(uint32_t resource_id);
GBitmap *gbitmap_create_with_resource_app(uint32_t resource_id, const struct file *file);
GBitmap *gbitmap_create_with_data(uint8_t *data);
GBitmap *gbitmap_create_as_sub_bitmap(const GBitmap *base_bitmap, GRect sub_rect);
GBitmap *gbitmap_create_from_png_data(uint8_t *png_data, size_t png_data_size);
GBitmap *gbitmap_create_blank(GSize size, GBitmapFormat format);
GBitmap *gbitmap_create_blank_with_palette(GSize size, GBitmapFormat format, GColor *palette, bool free_on_destroy);
GBitmap *gbitmap_create_palettized_from_1bit(GBitmap *src_bitmap);
void gbitmap_destroy(GBitmap *bitmap);

void gbitmap_draw(GBitmap *bitmap, GRect bounds);

/*

GBitmapSequence *gbitmap_sequence_create_with_resource(uint32_t resource_id);
bool gbitmap_sequence_update_bitmap_next_frame(GBitmapSequence *bitmap_sequence, GBitmap *bitmap, uint32_t *delay_ms);
bool gbitmap_sequence_update_bitmap_by_elapsed(GBitmapSequence *bitmap_sequence, GBitmap *bitmap, uint32_t elapsed_ms);
void gbitmap_sequence_destroy(GBitmapSequence *bitmap_sequence);
bool gbitmap_sequence_restart(GBitmapSequence *bitmap_sequence);
int32_t gbitmap_sequence_get_current_frame_idx(GBitmapSequence *bitmap_sequence);
uint32_t gbitmap_sequence_get_total_num_frames(GBitmapSequence *bitmap_sequence);
uint32_t gbitmap_sequence_get_play_count(GBitmapSequence *bitmap_sequence);
void gbitmap_sequence_set_play_count(GBitmapSequence *bitmap_sequence, uint32_t play_count); GSize gbitmap_sequence_get_bitmap_size(GBitmapSequence *bitmap_sequence);
GBitmapDataRowInfo gbitmap_get_data_row_info(const GBitmap *bitmap, uint16_t y);
void grect_align(GRect *rect, const GRect *inside_rect, const GAlign alignment, const bool clip);
GRect grect_inset(GRect rect, GEdgeInsets insets);
*/



/* TODO MOVE ME
 */
