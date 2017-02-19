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


// Layer Functions
Layer *layer_create(GRect bounds)
{
    Layer* layer = (Layer*)calloc(1, sizeof(Layer));
    if (layer == NULL)
    {
        printf("NO MEMORY FOR LAYER!\n");
        return NULL;
    }
    layer->bounds = bounds;
    
    return layer;
}

void layer_destroy(Layer *layer)
{
    // remove our node
    layer->parent->sibling = layer->sibling;

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
    
    while(parent_layer->sibling)
        parent_layer = parent_layer->sibling;
    
    parent_layer->sibling = child_layer;
    child_layer->parent = parent_layer;
}

void layer_mark_dirty(Layer *layer)
{
    //layer->window
    window_dirty(true);
}

GRect layer_get_bounds(Layer *layer)
{
    return layer->bounds;
}

void walk_layers(/*const*/ Layer *layer)
{
    if (layer)
    {
        if (layer->update_proc)
        {
            GContext *context = neographics_get_global_context();
            layer->update_proc(layer, context);
        }
        
        // walk this elements sub elements recursively before moving on to the next element
        walk_layers(layer->child);
        walk_layers(layer->sibling);
    }
}


void layer_delete_tree(Layer *layer)
{
    if(layer)
    {
        layer_delete_tree(layer->child);
        layer_delete_tree(layer->sibling);
        free(layer);
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


// resources
ResHandle resource_get_handle(uint16_t resource_id)
{
    ResHandle res;
    return res;
}

size_t resource_size(ResHandle handle)
{
    return 0;
}



GPoint grect_center_point(GRect *rect)
{
    uint16_t x, y;
    x = (rect->size.w - rect->origin.x) / 2;
    y = (rect->size.h - rect->origin.y) / 2;
    return GPoint(x, y);
}
