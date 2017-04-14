/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "librebble.h"
#include "upng.h"
#include "png.h"

void _gbitmap_draw(GBitmap *bitmap, GRect clip);

GBitmap *gbitmap_create(GRect frame)
{
    //Allocate gbitmap
    GBitmap *gbitmap = calloc(1, sizeof(GBitmap));
    gbitmap->bounds = frame;
    gbitmap->free_data_on_destroy = true;
    gbitmap->free_palette_on_destroy = true;
    
    return gbitmap;
}

void gbitmap_destroy(GBitmap *bitmap)
{
    if (bitmap->free_palette_on_destroy)
        free(bitmap->palette);
    if (bitmap->free_data_on_destroy)
        free(bitmap->addr);
        
    free(bitmap);
    bitmap = NULL;
}

void gbitmap_draw(GBitmap *bitmap, GRect bounds)
{
    _gbitmap_draw(bitmap, bounds);
}


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


uint16_t gbitmap_get_bytes_per_row(const GBitmap *bitmap)
{
    return bitmap->row_size_bytes;
}

GBitmapFormat gbitmap_get_format(const GBitmap *bitmap)
{
    return bitmap->format;
}

uint8_t *gbitmap_get_data(const GBitmap *bitmap)
{
    return (uint8_t *)bitmap->addr;
}

void gbitmap_set_data(GBitmap *bitmap, uint8_t *data, GBitmapFormat format, uint16_t row_size_bytes, bool free_on_destroy)
{
    bitmap->addr = data;
    bitmap->format = format;
    bitmap->row_size_bytes = row_size_bytes;
    bitmap->free_data_on_destroy = free_on_destroy;
}

GRect gbitmap_get_bounds(const GBitmap *bitmap)
{
    return bitmap->bounds;
}

void gbitmap_set_bounds(GBitmap *bitmap, GRect bounds)
{
    bitmap->bounds = bounds;
}

GColor *gbitmap_get_palette(const GBitmap *bitmap)
{
    return bitmap->palette;
}

void gbitmap_set_palette(GBitmap *bitmap, GColor *palette, bool free_on_destroy)
{
    free(bitmap->palette);
    bitmap->palette = palette;
    bitmap->free_palette_on_destroy = free_on_destroy;
}

GBitmap *gbitmap_create_with_resource(uint32_t resource_id)
{
    ResHandle res_handle = resource_get_handle(resource_id);
    size_t png_data_size = resource_size(res_handle);
    uint8_t *png_data = calloc(1, png_data_size); //freed by upng impl
    
    if (png_data == NULL)
    {
        printf("PNG Load alloc failed\n");
        return NULL;
    }
    resource_load(res_handle, png_data, png_data_size);
    
    return gbitmap_create_from_png_data(png_data, png_data_size);
}

GBitmap *gbitmap_create_with_data(const uint8_t *data)
{
    GRect r;
    // allocate a gbitmap
    GBitmap *bitmap = gbitmap_create(r);

    bitmap->addr = data;
    
    return bitmap;
}

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
    
    return bitmap;
}

GBitmap *gbitmap_create_from_png_data(const uint8_t *png_data, size_t png_data_size)
{   
    GRect fr;
    //Allocate gbitmap
    GBitmap *bitmap = gbitmap_create(fr);

    png_to_gbitmap(bitmap, png_data, png_data_size);
    
    return bitmap;
}

GBitmap *gbitmap_create_blank(GSize size, GBitmapFormat format)
{
    GRect gr = { .size = size, .origin.x = 0, .origin.y = 0 };
    GBitmap *bitmap = gbitmap_create(gr);
    bitmap->format = format;
    bitmap->addr = calloc(1, size.w * size.h);
    
    if (bitmap->addr == NULL)
    {
        printf("gbitmap_create_blank Malloc failed\n");
        return NULL;
    }
    return bitmap;
}

GBitmap *gbitmap_create_blank_with_palette(GSize size, GBitmapFormat format, GColor *palette, bool free_on_destroy)
{
    GBitmap *bitmap = gbitmap_create_blank(size, format);
    bitmap->palette = palette;
    bitmap->palette_size = 4; // TODO ????
    
    return bitmap;
}

GBitmap *gbitmap_create_palettized_from_1bit(const GBitmap *src_bitmap)
{
    return NULL;
}

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
void graphics_draw_bitmap_in_rect(GContext *ctx, const GBitmap *bitmap, GRect rect)
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
