/* bitmap_layer.c
 * routines for Loading a bitmap into a layer
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "upng.h"
#include "png.h"


static void _bitmap_update_proc(Layer *layer, GContext *nGContext);

void bitmap_layer_ctor(BitmapLayer *blayer, GRect frame)
{
    layer_ctor(&blayer->layer, frame);
    // give the layer a reference back to us
    blayer->layer.container = blayer;
    blayer->compositing_mode = GCompOpAssign;
    blayer->background = GColorWhite;
    blayer->alignment = GAlignCenter;
    
    layer_set_update_proc(&blayer->layer, _bitmap_update_proc);
}

void bitmap_layer_dtor(BitmapLayer *bitmap_layer)
{
    layer_dtor(&bitmap_layer->layer);
    
    if (bitmap_layer->bitmap)
        gbitmap_destroy(bitmap_layer->bitmap);
}

BitmapLayer *bitmap_layer_create(GRect frame)
{
    BitmapLayer* blayer = app_calloc(1, sizeof(BitmapLayer));
    bitmap_layer_ctor(blayer, frame);
    
    return blayer;
}

void bitmap_layer_destroy(BitmapLayer *bitmap_layer)
{
    bitmap_layer_dtor(bitmap_layer);
    app_free(bitmap_layer);
}

Layer *bitmap_layer_get_layer(BitmapLayer *bitmap_layer)
{
    return &bitmap_layer->layer;
}

const GBitmap *bitmap_layer_get_bitmap(BitmapLayer *bitmap_layer)
{
    return bitmap_layer->bitmap;
}

void bitmap_layer_set_bitmap(BitmapLayer *bitmap_layer, GBitmap *bitmap)
{
    bitmap_layer->bitmap = bitmap;
}

void bitmap_layer_set_alignment(BitmapLayer *bitmap_layer, GAlign alignment)
{
    bitmap_layer->alignment = alignment;
}

void bitmap_layer_set_background_color(BitmapLayer *bitmap_layer, GColor color)
{
    bitmap_layer->background = color;
}

void bitmap_layer_set_compositing_mode(BitmapLayer *bitmap_layer, GCompOp mode)
{
    bitmap_layer->compositing_mode = mode;
}

static void _bitmap_update_proc(Layer *layer, GContext *nGContext)
{
    BitmapLayer *bitmap_layer = (BitmapLayer *)layer->container;
    uint16_t x, y;
    uint16_t bw = bitmap_layer->bitmap->bounds.size.w;
    uint16_t bh = bitmap_layer->bitmap->bounds.size.h;
    uint16_t lx = layer->bounds.origin.x;
    uint16_t ly = layer->bounds.origin.y;
    uint16_t lw = layer->bounds.size.w;
    uint16_t lh = layer->bounds.size.h;
    
//     printf("LX %d LY %d LW %d LH %d BW %d BH %d\n", lx, ly, lw, lh, bw, bh);
        
    switch (bitmap_layer->alignment)
    {
        case GAlignCenter:
            x = lx + (lw / 2) - (bw / 2);
            y = ly + (lh / 2) - (bh / 2);
            break;
        case GAlignTopLeft:
            x = lx;
            y = ly;
            break;
        case GAlignTopRight:
            x = lx + lw - bw;
            y = ly;
            break;
        case GAlignTop:
            x = lx + (lw / 2) - (bw / 2);
            y = ly;
            break;
        case GAlignLeft:
            x = lx;
            y = ly + (lh / 2) - (bh / 2);
            break;
        case GAlignBottom:
            x = lx + (lw / 2) - (bw / 2);
            y = ly + lh - bh;
            break;
        case GAlignRight:
            x = lx + lw - bw;
            y = ly + (lh / 2) - (bh / 2);
            break;
        case GAlignBottomRight:
            x = lx + lw - bw;
            y = ly + lh - bh;
            break;
        case GAlignBottomLeft:
            x = lx;
            y = ly + lh - bh;
            break;
        default:
            x = 0;
            y = 0;
            break;
    }
    GRect target = GRect(x, y, bw, bh);

    // fill the background
    graphics_context_set_fill_color(nGContext, bitmap_layer->background);
    graphics_fill_rect(nGContext, layer->bounds, 0, GCornerNone);
    n_graphics_draw_bitmap_in_rect(nGContext, bitmap_layer->bitmap, target);
}
