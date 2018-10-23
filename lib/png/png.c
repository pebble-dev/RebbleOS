#include "upng.h"
#include "png.h"


void png_to_gbitmap(GBitmap *bitmap, uint8_t *raw_buffer, size_t png_size)
{
    /* Set up the bitmap, assuming we will fail. */
    bitmap->palette = NULL;
    bitmap->palette_size = 0;
    bitmap->bounds.origin.x = 0;
    bitmap->bounds.origin.y = 0;
    bitmap->bounds.size.w = 0;
    bitmap->bounds.size.h = 0;
    bitmap->raw_bitmap_size.w = 0;
    bitmap->raw_bitmap_size.h = 0;
    bitmap->addr = NULL;
    bitmap->format = GBitmapFormat8Bit;
    bitmap->free_data_on_destroy = true;
    bitmap->free_palette_on_destroy = true;

    upng_t *upng = upng_new_from_bytes(raw_buffer, png_size, &(bitmap->addr));
    
    if (upng == NULL)
    {
        SYS_LOG("png", APP_LOG_LEVEL_ERROR, "UPNG malloc error");
        return;
    }
    if (upng_get_error(upng) != UPNG_EOK)
    {
        SYS_LOG("png", APP_LOG_LEVEL_ERROR, "UPNG Loaded:%d line:%d", 
      upng_get_error(upng), upng_get_error_line(upng));
        if (upng_get_buffer(upng))
            app_free((void *)upng_get_buffer(upng));
        goto freepng;
    }
    if (upng_decode(upng) != UPNG_EOK)
    {
        SYS_LOG("png", APP_LOG_LEVEL_ERROR, "UPNG Decode:%d line:%d", 
      upng_get_error(upng), upng_get_error_line(upng));
        if (upng_get_buffer(upng))
            app_free((void *)upng_get_buffer(upng));
        goto freepng;
    }


    /* XXX: this leaks the buffer if we don't take this codepath */
    if (upng_get_format(upng) >= UPNG_INDEXED1 || upng_get_format(upng) <= UPNG_INDEXED8)
    {
        //Decode paletized image to raw rgb values
        unsigned int width = upng_get_width(upng);
        unsigned int height = upng_get_height(upng);
        unsigned int bpp = upng_get_bpp(upng);
        uint8_t *upng_buffer = (uint8_t*)upng_get_buffer(upng);

        //rgb palette
        rgb *palette = NULL;
        uint16_t plen = upng_get_palette(upng, &palette);
        // get any alpha bits in tRNS if there
        uint8_t *alpha;
        uint16_t alen = upng_get_alpha(upng, &alpha);

        // convert the palettes and alphas from 8 bit (requiring 4 bytes) to 2 bit rgba (1 byte)
        if (plen > 0)
        {
             n_GColor *conv_palettes = app_calloc(1, plen * sizeof(n_GColor));
            for (uint8_t i = 0; i < plen; i++)
            {
                // png spec says there can be less alphas than palette
                // we should assume that it is full opaque
                uint8_t alpha_val = (i >= alen ? 0xFF : alpha[i]);
                uint8_t pal = n_GColorFromRGBA(palette[i].r, palette[i].g, palette[i].b, alpha_val).argb;

                conv_palettes[i].argb = pal;
            }
            free(bitmap->palette);
    
            bitmap->palette = conv_palettes;
            bitmap->palette_size = plen;
        }
        else
        {
            bitmap->palette = NULL;
            bitmap->palette_size = 0;
        }           
        
         
        bitmap->bounds.origin.x = 0;
        bitmap->bounds.origin.y = 0;
        bitmap->bounds.size.w = width;
        bitmap->bounds.size.h = height;
        bitmap->raw_bitmap_size.w = width;
        bitmap->raw_bitmap_size.h = height;
        // calc the row_size_bytes
        // row size bytes is the actual byte count used by the bitmap in the x
        // this can vary in 1, 2 and 4 bit as the bitmap width of 8 only takes one byte
        // use a ceil roundup to get the most number of bytes required to hold
        // the bpp for this image
        // gbitmap will then use this to determine how many bytes to copy
        bitmap->row_size_bytes = ((width + ((8/bpp) - 1)) / (8/bpp));
        bitmap->addr = upng_buffer;

        // if we have alphas but no palette, construct a new palette
        // this is just a shortcut for gbitmap to render it like it was
        // properly palettised. Not sure why the format is the way it is,
        // will ask Pebble folks
        if (bitmap->palette_size == 0 && alen > 0)
        {
            // convert palette
            n_GColor *conv_palettes = app_calloc(1, 4 * sizeof(n_GColor));

            // black with alpha (black)
            conv_palettes[0].argb = n_GColorFromRGBA(0, 0, 0, 255).argb;
            // black no alpha (totally see through)
            conv_palettes[1].argb = n_GColorFromRGBA(0, 0, 0, 0).argb;
            // white with alpha (white)
            conv_palettes[3].argb = n_GColorFromRGBA(255, 255, 255, 255).argb;
            bitmap->palette = conv_palettes;
            bitmap->palette_size = 4;
        }

        // set the resultant format
        if (bpp == 1 && (bitmap->palette_size > 0 || alen > 0))
        {
            bitmap->format = GBitmapFormat1BitPalette;
        }
        else if (bpp == 1)
        {
            // easier to generate the black/white palette than to reverse every byte
            bitmap->format = GBitmapFormat1BitPalette;

            n_GColor* bw_palette = app_calloc(2, sizeof(n_GColor));
            bw_palette[0] = n_GColorBlack;
            bw_palette[1] = n_GColorWhite;
            bitmap->palette = bw_palette;
            bitmap->palette_size = 2;
        }
        else if (bpp == 2)
        {
            bitmap->format = GBitmapFormat2BitPalette;
        }
        else if (bpp == 4)
        {
            bitmap->format = GBitmapFormat4BitPalette;
        }
        else if (bpp == 8)
        {
            bitmap->format = GBitmapFormat8Bit;

            // Convert 8bit palette to just 8bit
            if (plen)
            {
                uint32_t pixel_count = bitmap->row_size_bytes * height;
                uint8_t* pixel = bitmap->addr;
                while (pixel_count--) {
                    *pixel = bitmap->palette[*pixel].argb;
                    pixel++;
                }
                app_free(bitmap->palette);
                bitmap->palette = 0;
                bitmap->palette_size = 0;
            }
        }
    }

    // Free the png, no longer needed
freepng:
    upng_free(upng);
    upng = NULL;
}
