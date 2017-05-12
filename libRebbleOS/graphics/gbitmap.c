/* gbitmap.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "upng.h"
#include "png.h"

extern uint8_t *resource_fully_load_id_app(uint16_t, uint16_t);

void _gbitmap_draw(GBitmap *bitmap, GRect clip);

/*
 * Create a bitmap of size frame
 */
GBitmap *gbitmap_create(GRect frame)
{
    //Allocate gbitmap
    GBitmap *gbitmap = app_calloc(1, sizeof(GBitmap));
    gbitmap->bounds = frame;
    gbitmap->free_data_on_destroy = true;
    gbitmap->free_palette_on_destroy = true;
    gbitmap->format = GBitmapFormat4BitPalette;
    
    return gbitmap;
}

/*
 * Kill a Bitmap and all the things it uses
 */
void gbitmap_destroy(GBitmap *bitmap)
{
    if (bitmap->free_palette_on_destroy)
        app_free(bitmap->palette);
    if (bitmap->free_data_on_destroy)
        app_free(bitmap->addr);
        
    app_free(bitmap);
    bitmap = NULL;
}

/*
 * Call a draw out of a bitmap image into the bounds.
 * If the bitmap is larger it will get clipped
 * If it is larger it will be placed at bounds x + bitmap x
 */
void gbitmap_draw(GBitmap *bitmap, GRect bounds)
{
    _gbitmap_draw(bitmap, bounds);
}

/*
 * Mega draw. Draw based on format etc
 */
void _gbitmap_draw(GBitmap *bitmap, GRect clipping_bounds)
{
    //Decode paletized image to raw rgb values
    uint8_t *buffer = (uint8_t*)bitmap->addr;

    uint8_t alpha_offset = (bitmap->palette_size > 0) && (bitmap->palette[0].a == 0);
    uint32_t pal_idx;
    
    uint8_t *buf = display_get_buffer();
    
    // clip to the smallest real size of the image
    uint16_t ctmp = (bitmap->bounds.size.w > bitmap->raw_bitmap_size.w) ? bitmap->raw_bitmap_size.w : bitmap->bounds.size.w;
    uint16_t w = ctmp > clipping_bounds.size.w ? clipping_bounds.size.w : ctmp;

    ctmp = (bitmap->bounds.size.h > bitmap->raw_bitmap_size.h) ? bitmap->raw_bitmap_size.h : bitmap->bounds.size.h;
    uint16_t h = ctmp > clipping_bounds.size.h ? clipping_bounds.size.h : ctmp;
       
    // set x, y start offset for the row based on the clipping mask
    int16_t clip_y = (clipping_bounds.origin.y > bitmap->bounds.origin.y)
            ? clipping_bounds.origin.y - bitmap->bounds.origin.y 
            : 0;
    int16_t clip_x = clipping_bounds.origin.x > bitmap->bounds.origin.x 
            ? clipping_bounds.origin.x - bitmap->bounds.origin.x
            : 0;
            
    uint8_t bpp = 8;
    switch (bitmap->format)
    {
        case GBitmapFormat1Bit: bpp = 1; break;
        case GBitmapFormat8Bit: bpp = 8; break;
        case GBitmapFormat1BitPalette: bpp = 1; break;
        case GBitmapFormat2BitPalette: bpp = 2; break;
        case GBitmapFormat4BitPalette: bpp = 4; break;
    }
    
    clip_x = ((clip_x + ((8 / bpp) - 1)) / (8 / bpp));
    
    uint16_t newx = bitmap->bounds.origin.x;
    uint16_t newy = bitmap->bounds.origin.y + clip_y;

    for(int y = 0; y < h; y++)
    {
        uint32_t bitmap_row_start = (y + clip_y) * bitmap->row_size_bytes;

        GColor argb;
        
        for(int x = clip_x; x < w; x++)
        {            
            if (bitmap->format == GBitmapFormat2BitPalette)
            {
                // shift the bits according to their position mod
                // I'm sure theres a more efficient way, but heres whats going on here
                // we find the start of the bitmap buffer for this x. Becuase we get 4 colour per bytes
                // divide x over 4. Then shift the result by the mod of x. 
                pal_idx = (buffer[(bitmap_row_start + (x/4))]) >> (6 - (((x % 4) * 2)));
                pal_idx &= 0x03;

                // alpha offset 0 means we have an 255 alpha so skip
                if (bitmap->palette[pal_idx].a > 0)
                {
                    argb = bitmap->palette[pal_idx - alpha_offset];
                    // TODO compositing
                }
                else
                {
                    argb.argb = 0;
                }
            }
            else if (bitmap->format == GBitmapFormat4BitPalette)
            {
                // we read hi lo bytes depending on the odd/even bytes in the final buffer
                // we can cheat and save the *2 on row start when we compute 4-8 bit. It doesn't affect odd/evenness

                if (x % 2)
                    pal_idx = buffer[bitmap_row_start + (x/2)] & 0xF;
                else
                    pal_idx = buffer[bitmap_row_start + (x/2)] >> 4;

                // alpha offset 0 means we have an alpha
                if (bitmap->palette[pal_idx].a > 0)
                {
                    argb = bitmap->palette[pal_idx - alpha_offset];
                }
                else
                {
                    argb.argb = 0;
                }
            }
            else if (bitmap->format == GBitmapFormat1BitPalette)
            {
                pal_idx = (buffer[(bitmap_row_start + (x/8))]) >> (8 - (x % 8));
                pal_idx &= 0x01;

                // alpha offset 0 means we have an 255 alpha so skip
                if (bitmap->palette[pal_idx].a > 0)
                {
                    argb = bitmap->palette[pal_idx - alpha_offset];
                }
                else
                {
                    argb.argb = 0;
                }
            }
            else if (bitmap->format == GBitmapFormat8Bit)
            {
                // straight up copy.
                // TODO memcpy this out of the loop. for now, lazy
                argb = (n_GColor)buffer[bitmap_row_start + x];
            }
            else if (bitmap->format == GBitmapFormat1Bit)
            {
                pal_idx = (buffer[(bitmap_row_start + (x/8))]) >> (8 - (x % 8));
                pal_idx &= 0x01;

                if (pal_idx == 1)
                    argb = GColorWhite;
                else
                    argb = GColorBlack;
            }
            
            // set the pixel in the buffer.
            if (argb.argb > 0)
            {
                n_GContext *ctx = neographics_get_global_context();
                n_graphics_set_pixel(ctx, n_GPoint(x + newx, y + newy), argb);
            }
        }
    }
}

/*
 * How many bytes per row of a bitmap. For example, 1 bit image has 8 bits per byte. 8 rows would be 1 byte
 */
uint16_t gbitmap_get_bytes_per_row(const GBitmap *bitmap)
{
    return bitmap->row_size_bytes;
}

/*
 * Gets the format of the bitmap
 */
GBitmapFormat gbitmap_get_format(const GBitmap *bitmap)
{
    return bitmap->format;
}

/*
 * Get the data out of that ther bitmap
 */
uint8_t *gbitmap_get_data(const GBitmap *bitmap)
{
    return (uint8_t *)bitmap->addr;
}

/*
 * given a GBitmap, set the format and data of the image data.
 */
void gbitmap_set_data(GBitmap *bitmap, uint8_t *data, GBitmapFormat format, uint16_t row_size_bytes, bool free_on_destroy)
{
    bitmap->addr = data;
    bitmap->format = format;
    bitmap->row_size_bytes = row_size_bytes;
    bitmap->free_data_on_destroy = free_on_destroy;
}

/*
 * Get the boundary of the bitmp
 */
GRect gbitmap_get_bounds(const GBitmap *bitmap)
{
    return bitmap->bounds;
}

/*
 * Set the boundary of the bitmp
 */
void gbitmap_set_bounds(GBitmap *bitmap, GRect bounds)
{
    bitmap->bounds = bounds;
}

/*
 * Get the palette of the bitmp
 */
GColor *gbitmap_get_palette(const GBitmap *bitmap)
{
    return bitmap->palette;
}

/*
 * Set the palette of the bitmp
 */
void gbitmap_set_palette(GBitmap *bitmap, GColor *palette, bool free_on_destroy)
{
    app_free(bitmap->palette);
    bitmap->palette = palette;
    bitmap->free_palette_on_destroy = free_on_destroy;
}

/*
 * Load a resource into the GBitmap by resource id
 */
GBitmap *gbitmap_create_with_resource(uint32_t resource_id)
{
    uint8_t *png_data = resource_fully_load_id(resource_id);
    ResHandle res_handle = resource_get_handle(resource_id);
    size_t png_data_size = resource_size(res_handle);
        
    return gbitmap_create_from_png_data(png_data, png_data_size);
}

GBitmap *gbitmap_create_with_resource_app(uint32_t resource_id, uint16_t slot_id)
{
    uint8_t *png_data = (uint8_t*)resource_fully_load_id_app(resource_id, slot_id);
    ResHandle res_handle = resource_get_handle_app(resource_id, slot_id);
    size_t png_data_size = resource_size(res_handle);
        
    return gbitmap_create_from_png_data(png_data, png_data_size);
}

/*
 * Create a new bitmap with the given data
 */
GBitmap *gbitmap_create_with_data(uint8_t *data)
{
    GRect r;
    // allocate a gbitmap
    GBitmap *bitmap = gbitmap_create(r);

    bitmap->addr = data;
    
    return bitmap;
}

/*
 * Get a sub bitmap from a larger bitmap
 */
GBitmap *gbitmap_create_as_sub_bitmap(const GBitmap *base_bitmap, GRect sub_rect)
{
    GBitmap *bitmap = gbitmap_create(sub_rect);
    bitmap->addr = base_bitmap->addr;
    bitmap->raw_bitmap_size = base_bitmap->raw_bitmap_size;
    bitmap->palette = base_bitmap->palette;
    bitmap->palette_size = base_bitmap->palette_size;
    bitmap->row_size_bytes = base_bitmap->row_size_bytes;
    bitmap->free_palette_on_destroy = base_bitmap->free_palette_on_destroy;
    bitmap->free_data_on_destroy = base_bitmap->free_data_on_destroy;
    bitmap->format = base_bitmap->format;
    bitmap->bounds = sub_rect;
 
    // TODO bounds checking and stuff
    
    return bitmap;
}

/*
 * Given loaded png image, create a new GBitmap
 */
GBitmap *gbitmap_create_from_png_data(uint8_t *png_data, size_t png_data_size)
{   
    GRect fr;
    //Allocate gbitmap
    GBitmap *bitmap = gbitmap_create(fr);

    png_to_gbitmap(bitmap, png_data, png_data_size);
    
    return bitmap;
}

/*
 * Create and initialise a GBitmap
 */
GBitmap *gbitmap_create_blank(GSize size, GBitmapFormat format)
{
    GRect gr = { .size = size, .origin.x = 0, .origin.y = 0 };
    GBitmap *bitmap = gbitmap_create(gr);
    bitmap->format = format;
    bitmap->addr = app_calloc(1, size.w * size.h);
    
    if (bitmap->addr == NULL)
    {
        SYS_LOG("gbitmap", APP_LOG_LEVEL_ERROR, "gbitmap_create_blank Malloc failed");
        return NULL;
    }
    return bitmap;
}

/*
 * Create and initialise a GBitmap with a palette
 */
GBitmap *gbitmap_create_blank_with_palette(GSize size, GBitmapFormat format, GColor *palette, bool free_on_destroy)
{
    GBitmap *bitmap = gbitmap_create_blank(size, format);
    bitmap->palette = palette;
    bitmap->palette_size = 4; // TODO ????
    
    return bitmap;
}

/*
 * TODO
 */
GBitmap *gbitmap_create_palettized_from_1bit(GBitmap *src_bitmap)
{
    return NULL;
}

/*
 * Set the position vars of a bitmap
 */
void _gbitmap_set_size_pos(GBitmap *bitmap, GRect size)
{
    bitmap->bounds = size;
}

// TODO actually does not belong here
/*
 * Draw a bitmap into a graphics context
 * 
 * Bitmaps will be clipped to rect
 * If rect > bitmap, bitmap will be tiled
 */
void graphics_draw_bitmap_in_rect(GContext *ctx, GBitmap *bitmap, GRect rect)
{
    _gbitmap_set_size_pos(bitmap, rect);
    
    gbitmap_draw(bitmap, rect);
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
