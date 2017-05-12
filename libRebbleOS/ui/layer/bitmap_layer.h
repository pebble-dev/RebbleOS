#pragma once
/* bitmap_layer.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "point.h"
#include "rect.h"
#include "size.h"
#include "neographics.h"
#include "gbitmap.h"

struct Window;
struct Layer;
struct BitmapLayer;


// TODO in neogfx?

// Make sure these are the same. 
typedef struct BitmapLayer
{
    struct Layer *layer;
    GBitmap *bitmap;
    ResHandle resource_handle;
    GAlign alignment;
    GColor background;
    GCompOp compositing_mode;
} BitmapLayer;

BitmapLayer *bitmap_layer_create(GRect frame);
void bitmap_layer_destroy(BitmapLayer *bitmap_layer);
Layer *bitmap_layer_get_layer(BitmapLayer *bitmap_layer);
const GBitmap *bitmap_layer_get_bitmap(BitmapLayer *bitmap_layer);
void bitmap_layer_set_bitmap(BitmapLayer *bitmap_layer, GBitmap *bitmap);
void bitmap_layer_set_alignment(BitmapLayer *bitmap_layer, GAlign alignment);
void bitmap_layer_set_background_color(BitmapLayer *bitmap_layer, GColor color);
void bitmap_layer_set_compositing_mode(BitmapLayer *bitmap_layer, GCompOp mode);
