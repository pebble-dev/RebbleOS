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


static void _bitmap_update_proc(Layer *layer, GContext *nGContext);

BitmapLayer *bitmap_layer_create(GRect frame)
{
    BitmapLayer* blayer = (BitmapLayer*)calloc(1, sizeof(BitmapLayer));
    Layer* layer = layer_create(frame);
    // give the layer a reference back to us
    layer->container = blayer;
    blayer->layer = layer;
    blayer->compositing_mode = GCompOpAssign;
    blayer->background = GColorWhite;
    blayer->alignment = GAlignCenter;
    
    layer_set_update_proc(layer, _bitmap_update_proc);
    
    return blayer;
}

void bitmap_layer_destroy(BitmapLayer *bitmap_layer)
{
    gbitmap_destroy(bitmap_layer->bitmap);
    free(bitmap_layer);
    bitmap_layer = NULL;
}

Layer *bitmap_layer_get_layer(BitmapLayer *bitmap_layer)
{
    return bitmap_layer->layer;
}

const GBitmap *bitmap_layer_get_bitmap(BitmapLayer *bitmap_layer)
{
    return bitmap_layer->bitmap;
}

void bitmap_layer_set_bitmap(BitmapLayer *bitmap_layer, const GBitmap *bitmap)
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
    bitmap_layer->bitmap->bounds = GRect(x, y, bw, bh);

    // fill the background
    graphics_context_set_fill_color(nGContext, bitmap_layer->background);
    graphics_fill_rect(nGContext, layer->bounds, 0, GCornerNone);
    
    gbitmap_draw(bitmap_layer->bitmap, layer->bounds);
}
