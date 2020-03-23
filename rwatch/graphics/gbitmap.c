/* gbitmap.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "upng.h"
#include "flash.h"
#include "png.h"
#include "ngfxwrap.h"
#include "fs.h"

/*
 * Load a resource into the GBitmap by resource id
 */
GBitmap *gbitmap_create_with_resource(uint32_t resource_id)
{
    struct file file;
    void *png_data;
    size_t png_data_size;
    
    resource_file(&file, resource_get_handle_system(resource_id));
    png_data = resource_fully_load_file(&file, &png_data_size);
    if (!png_data)
        return NULL;

    return gbitmap_create_from_png_data(png_data, png_data_size);
}

GBitmap *gbitmap_create_with_resource_app(uint32_t resource_id, const struct file *ifile)
{
    struct file file;
    void *png_data;
    size_t png_data_size;
    
    resource_file_from_file_handle(&file, ifile, resource_get_handle(resource_id));
    png_data = resource_fully_load_file(&file, &png_data_size);
    if (!png_data)
        return NULL;

    return gbitmap_create_from_png_data(png_data, png_data_size);
}

/*
 * Create a new bitmap with the given data
 */
GBitmap *gbitmap_create_with_data(uint8_t *data)
{
    // TODO: Implement PBI (issue #52)
    return NULL;
}

/*
 * Given loaded png image, create a new GBitmap
 */
GBitmap *gbitmap_create_from_png_data(uint8_t *png_data, size_t png_data_size)
{
    GBitmap *bitmap = (GBitmap*)app_malloc(sizeof(GBitmap));

    png_to_gbitmap(bitmap, png_data, png_data_size);

    return bitmap;
}

GBitmapDataRowInfo gbitmap_get_data_row_info(const GBitmap * bitmap, uint16_t y)
{
    uint8_t *data = gbitmap_get_data(bitmap);
    uint16_t bpr = gbitmap_get_bytes_per_row(bitmap);
    GRect bounds = gbitmap_get_bounds(bitmap);
    GBitmapDataRowInfo row_info;
    row_info.data = data + bpr * y;
    row_info.min_x = bounds.origin.x;
    row_info.max_x = bounds.origin.x + bounds.size.w - 1;
    return row_info;
}

// scary sequence stuff
// read this more to figure out impl
/*
GBitmapSequence * gbitmap_sequence_create_with_resource(uint32_t resource_id);
bool gbitmap_sequence_update_bitmap_next_frame(GBitmapSequence * bitmap_sequence, GBitmap * bitmap, uint32_t * delay_ms);
bool gbitmap_sequence_update_bitmap_by_elapsed(GBitmapSequence * bitmap_sequence, GBitmap * bitmap, uint32_t elapsed_ms);
void gbitmap_sequence_destroy(GBitmapSequence * bitmap_sequence);
bool gbitmap_sequence_restart(GBitmapSequence * bitmap_sequence);
int32_t gbitmap_sequence_get_current_frame_idx(GBitmapSequence * bitmap_sequence);
uint32_t gbitmap_sequence_get_total_num_frames(GBitmapSequence * bitmap_sequence);
uint32_t gbitmap_sequence_get_play_count(GBitmapSequence * bitmap_sequence);
void gbitmap_sequence_set_play_count(GBitmapSequence * bitmap_sequence, uint32_t play_count); GSize gbitmap_sequence_get_bitmap_size(GBitmapSequence * bitmap_sequence);
*/
