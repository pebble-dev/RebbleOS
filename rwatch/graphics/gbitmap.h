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
#include "gbitmap/gbitmap.h"

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

void grect_standardize(GRect *rect);
GBitmap *gbitmap_create_with_resource(uint32_t resource_id);
GBitmap *gbitmap_create_with_resource_app(uint32_t resource_id, const struct file *file);
GBitmap *gbitmap_create_with_data(uint8_t *data);
GBitmap *gbitmap_create_from_png_data(uint8_t *png_data, size_t png_data_size);

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
