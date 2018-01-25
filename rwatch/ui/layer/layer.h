#pragma once
/* layer.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "point.h"
#include "rect.h"
#include "size.h"

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
GPoint layer_convert_point_to_screen(const Layer *layer, GPoint point);
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


void walk_layers(/*const*/ Layer *layer, GContext *context);

// updates context offset based on layer frame, used to properly adjust layer drawing calls
void layer_apply_frame_offset(const Layer *layer, GContext *context);

void layer_delete_tree(Layer *layer);

