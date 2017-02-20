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

struct Window;
struct Layer;

// Callback for the layer drawing
// typedef it for cleanness
typedef void (*LayerUpdateProc)(struct Layer *layer, GContext *context);

// Make sure these are the same. 
typedef struct Layer
{
    struct Layer *child;
    struct Layer *sibling;
    struct Layer *parent;
    void *container; // pointer to parent type, if any. i.e. a textlayer
    struct Window  *window; // not connected yet
    GRect bounds;
    GRect frame;
    LayerUpdateProc update_proc;
    void *callback_data;
    bool hidden;
} Layer;


// Layer Functions
Layer *layer_create(GRect bounds);
Layer *layer_create_with_data(GRect frame, size_t data_size);
void layer_destroy(Layer *layer);
void layer_set_frame(Layer *layer, GRect frame);
GRect layer_get_frame(const Layer *layer);
void layer_set_bounds(Layer *layer, GRect bounds);
GRect layer_get_unobstructed_bounds(Layer *layer);
void layer_set_update_proc(Layer *layer, void *proc);
void layer_add_child(Layer *parent_layer, Layer *child_layer);
void layer_mark_dirty(Layer *layer);
GRect layer_get_bounds(Layer *layer);
GPoint layer_convert_point_to_screen(const Layer *layer, GPoint point); //TODO
GRect layer_convert_rect_to_screen(const Layer *layer, GRect rect); //TODO
struct Window *layer_get_window(const Layer *layer);
void layer_remove_from_parent(Layer *child);
void layer_remove_child_layers(Layer *parent);
void layer_insert_below_sibling(Layer *layer_to_insert, Layer *below_sibling_layer);
void layer_insert_above_sibling(Layer *layer_to_insert, Layer *above_sibling_layer);
void layer_set_hidden(Layer *layer, bool hidden);
bool layer_get_hidden(const Layer *layer);
void layer_set_clips(Layer *layer, bool clips);  //TODO
bool layer_get_clips(const Layer *layer); //TODO
void *layer_get_data(const Layer *layer); //TODO


void walk_layers(/*const*/ Layer *layer);
void layer_delete_tree(Layer *layer);

// TODO: should go into neographics
GPoint grect_center_point(GRect *rect);
