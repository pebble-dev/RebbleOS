/* layer.c
 * routines for [...]
 * RebbleOS core
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"

Layer *layer_find_parent(Layer *orig_layer, Layer *layer);
void layer_remove_node(Layer *to_be_removed);
void layer_insert_node(Layer *layer_to_insert, Layer *sibling_layer, bool below);

// Layer Functions
Layer *layer_create(GRect frame)
{
    Layer* layer = app_calloc(1, sizeof(Layer));
    if (layer == NULL)
    {
        printf("NO MEMORY FOR LAYER!\n");
        return NULL;
    }
    layer->bounds = frame;
    layer->frame = frame;
    layer->child = NULL;
    layer->sibling = NULL;
    
    return layer;
}

Layer *layer_create_with_data(GRect frame, size_t data_size)
{
    Layer *layer = layer_create(frame);
    layer->callback_data = app_calloc(1, data_size);
    
    return layer;
}

void layer_destroy(Layer *layer)
{
    // remove our node
    layer_remove_node(layer);

    // free the children too...
    layer_delete_tree(layer);
}

GRect layer_get_unobstructed_bounds(Layer *layer)
{
    return GRect(0, 0, 144, 168);
}

void layer_set_update_proc(Layer *layer, void *proc)
{
    layer->update_proc = proc;
}

void layer_add_child(Layer *parent_layer, Layer *child_layer)
{   
    if (parent_layer == NULL || child_layer == NULL)
        return;

    if(parent_layer->child == child_layer)
    {
        printf("LAYER IS ALREADY CHILD\n");
        return;
    }
    
    // if parent isn't already with child, it will become it
    // this will set the root node for all other elements in this parent
    if (parent_layer->child == NULL)
    {
        parent_layer->child = child_layer;
        return;
    }
    
    Layer *child = parent_layer->child;
    
    // now find the parents childs siblings
    while(child->sibling)
    {
        if (child == child_layer)
        {
            printf("LAYER IS ALREADY CHILD\n");
            return;
        }
        child = child->sibling;
    }
    
    child->sibling = child_layer;
    child_layer->parent = parent_layer;
}

void layer_mark_dirty(Layer *layer)
{
    //layer->window
    window_dirty(true);
}

void layer_set_bounds(Layer *layer, GRect bounds)
{
    layer->bounds = bounds;
}

GRect layer_get_bounds(Layer *layer)
{
    return layer->bounds;
}

void layer_set_frame(Layer *layer, GRect frame)
{
    layer->frame = frame;
}

GRect layer_get_frame(const Layer *layer)
{
    return layer->frame;
}

struct Window *layer_get_window(const Layer *layer)
{
    return layer->window;
}

void layer_remove_from_parent(Layer *child)
{
    layer_remove_node(child);
}

void layer_remove_child_layers(Layer *parent)
{
    layer_delete_tree(parent->child);
}

void layer_insert_below_sibling(Layer *layer_to_insert, Layer *below_sibling_layer)
{
    layer_insert_node(layer_to_insert, below_sibling_layer, true);
}

void layer_insert_above_sibling(Layer *layer_to_insert, Layer *above_sibling_layer)
{
    layer_insert_node(layer_to_insert, above_sibling_layer, false);
}

void layer_set_hidden(Layer *layer, bool hidden)
{
    layer->hidden = hidden;
}

bool layer_get_hidden(const Layer *layer)
{
    return layer->hidden;
}


// private?

void layer_insert_node(Layer *layer_to_insert, Layer *sibling_layer, bool below)
{
    if (below)
    {
        // first find the sibling that points to parent
        Layer *parent = layer_find_parent(sibling_layer, sibling_layer);
        layer_to_insert->parent = parent->parent;
        layer_to_insert->sibling = parent->sibling;
        parent->sibling = layer_to_insert;
    }
    else
    {
        // slot the node in after
        layer_to_insert->parent = sibling_layer->parent;
        layer_to_insert->sibling = sibling_layer->sibling;
        sibling_layer->sibling = layer_to_insert;
    }
}

void layer_remove_node(Layer *to_be_removed)
{
    // remove our node by pointing next to parents next, jumping over us
    to_be_removed->parent->sibling = to_be_removed->sibling;
    to_be_removed->sibling = NULL;
}

/*
 * Recurse through the btree.
 * As we are storing layers as a btree where each sibling
 * is the next layer of the same child as layer->parent
 * layer->child is the head of a new list of siblings where layer->child == new parent
 * This will recurse the children, the siblings of children in a layer_delete_tree
 * When exhaused it will walk the siblings of the parent, etc etc until
 * either 1) no more ram 2) completion
 */
void walk_layers(/*const*/ Layer *layer)
{
    if (layer)
    {
        if (layer->hidden == false) // we don't draw hidden layers or their children
        {
            if (layer->update_proc)
            {
                GContext *context = neographics_get_global_context();
                // call the callback
                layer->update_proc(layer, context);
            }
            
            // walk this elements sub elements recursively before moving on to the next element
            walk_layers(layer->child);
        }        
        walk_layers(layer->sibling);
    }
}

Layer *layer_find_parent(Layer *orig_layer, Layer *layer)
{
    if (layer)
    {
        if (layer->sibling == NULL && layer->sibling == NULL)
        
        if (layer->sibling == orig_layer || layer->child == orig_layer)
        {
            return layer;
        }
        
        // walk this elements sub elements recursively before moving on to the next element
        Layer *found_layer = NULL;
        found_layer = layer_find_parent(orig_layer, layer->child);
        if (found_layer)
            return found_layer;
        
        found_layer = layer_find_parent(orig_layer, layer->sibling);
        if (found_layer)
            return found_layer;
    }
    return NULL;
}


void layer_delete_tree(Layer *layer)
{
    if(layer)
    {
        layer_delete_tree(layer->child);
        layer_delete_tree(layer->sibling);
        app_free(layer);
    }
}


// move me
GBitmap *graphics_capture_frame_buffer(GContext *context)
{
    //GBitmap bitmap;
    // rbl_lock_frame_buffer
    return NULL;
}

void graphics_release_frame_buffer(GContext *context, GBitmap *bitmap)
{
    // rbl_unlock_frame_buffer
}



GPoint grect_center_point(GRect *rect)
{
    uint16_t x, y;
    x = (rect->size.w - rect->origin.x) / 2;
    y = (rect->size.h - rect->origin.y) / 2;
    return GPoint(x, y);
}
