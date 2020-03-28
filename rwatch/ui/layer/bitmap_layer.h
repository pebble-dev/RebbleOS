#pragma once
/* bitmap_layer.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

// neographics includes
#include "types/point.h"
#include "types/rect.h"
#include "types/size.h"

// rwatch includes
#include "graphics/gbitmap.h"
#include "layer/layer.h"

struct Window;

// TODO in neogfx?

// Make sure these are the same. 
typedef struct BitmapLayer
{
    Layer layer;
    GBitmap *bitmap;
    ResHandle resource_handle;
    GAlign alignment;
    GColor background;
    GCompOp compositing_mode;
} BitmapLayer;

void bitmap_layer_dtor(BitmapLayer *blayer);
void bitmap_layer_ctor(BitmapLayer *blayer, GRect frame);

BitmapLayer *bitmap_layer_create(GRect frame);
void bitmap_layer_destroy(BitmapLayer *bitmap_layer);

Layer *bitmap_layer_get_layer(BitmapLayer *bitmap_layer);
const GBitmap *bitmap_layer_get_bitmap(BitmapLayer *bitmap_layer);
void bitmap_layer_set_bitmap(BitmapLayer *bitmap_layer, GBitmap *bitmap);
void bitmap_layer_set_alignment(BitmapLayer *bitmap_layer, GAlign alignment);
void bitmap_layer_set_background_color(BitmapLayer *bitmap_layer, GColor color);
void bitmap_layer_set_compositing_mode(BitmapLayer *bitmap_layer, GCompOp mode);
