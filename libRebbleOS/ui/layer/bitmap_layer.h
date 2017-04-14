#pragma once
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
void bitmap_layer_set_bitmap(BitmapLayer *bitmap_layer, const GBitmap *bitmap);
