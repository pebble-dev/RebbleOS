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

/*
 * Load a resource into the GBitmap by resource id
 */
GBitmap *gbitmap_create_with_resource(uint32_t resource_id)
{
    uint8_t *png_data = resource_fully_load_id_system(resource_id);
    ResHandle res_handle = resource_get_handle_system(resource_id);
    size_t png_data_size = resource_size(res_handle);

    if (!png_data)
        return NULL;

    return gbitmap_create_from_png_data(png_data, png_data_size);
}

GBitmap *gbitmap_create_with_resource_app(uint32_t resource_id, const struct file *file)
{
    size_t png_size;
    uint8_t *png_data = (uint8_t*)resource_fully_load_id_app_file(resource_id, file, &png_size);

    if (!png_data)
        return NULL;

    return gbitmap_create_from_png_data(png_data, png_size);
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
GBitmapDataRowInfo gbitmap_get_data_row_info(const GBitmap * bitmap, uint16_t y);
*/
