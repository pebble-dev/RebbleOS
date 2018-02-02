/* layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "utils.h"

static void _layer_remove_node(Layer *to_be_removed);
static void _layer_insert_node(Layer *layer_to_insert, Layer *sibling_layer, bool below);
static void _layer_delete_tree(Layer *layer);
static Layer *_layer_find_parent(Layer *orig_layer, Layer *layer);
static void _layer_walk(/*const*/ Layer *layer, GContext *context);

// Layer Functions
Layer *layer_create(GRect frame)
{
    Layer* layer = app_calloc(1, sizeof(Layer));
    if (layer == NULL)
    {
        SYS_LOG("layer", APP_LOG_LEVEL_ERROR, "NO MEMORY FOR LAYER!");
        return NULL;
    }
    layer_ctor(layer, frame);
    
    return layer;
}

Layer *layer_create_with_data(GRect frame, size_t data_size)
{
    Layer *layer = layer_create(frame);
    layer->callback_data = app_calloc(1, data_size);
    
    return layer;
}

void layer_ctor(Layer* layer, GRect frame)
{
    layer->bounds = frame;
    layer->frame = frame;
    layer->child = NULL;
    layer->sibling = NULL;
    layer->parent = NULL;
}

void layer_destroy(Layer* layer)
{
    layer_dtor(layer);
}

void layer_dtor(Layer *layer)
{
    // remove our node
    SYS_LOG("layer", APP_LOG_LEVEL_ERROR, "Layer DTOR");
    _layer_remove_node(layer);
    // free the children too...
    _layer_delete_tree(layer);
}

GRect layer_get_unobstructed_bounds(Layer *layer)
{
    return layer->bounds;
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
        SYS_LOG("layer", APP_LOG_LEVEL_ERROR, "LAYER IS ALREADY CHILD");
        return;
    }
    
    // if parent isn't already with child, it will become it
    // this will set the root node for all other elements in this parent
    if (parent_layer->child == NULL)
    {
        parent_layer->child = child_layer;
        child_layer->parent = parent_layer;
        return;
    }
    
    Layer *child = parent_layer->child;
    
    // now find the parents childs siblings
    while(child->sibling)
    {
        if (child == child_layer)
        {
            SYS_LOG("layer", APP_LOG_LEVEL_ERROR, "LAYER IS ALREADY CHILD\n");
            return;
        }
        child = child->sibling;
    }
    
    child->sibling = child_layer;
    child_layer->parent = parent_layer;

    layer_mark_dirty(parent_layer);
}

void layer_mark_dirty(Layer *layer)
{
    //layer->window
    window_dirty(true);
}

void layer_set_bounds(Layer *layer, GRect bounds)
{
    if (!RECT_EQ(layer->bounds, bounds)) {
        layer->bounds = bounds;
        layer_mark_dirty(layer);
    }
}

GRect layer_get_bounds(Layer *layer)
{
    return layer->bounds;
}

GPoint layer_convert_point_to_screen(const Layer *layer, GPoint point)
{
    const Layer *current_layer = layer;
    while (current_layer != NULL)
    {
        point = GPoint(point.x + current_layer->frame.origin.x,
                       point.y + current_layer->frame.origin.y);
        current_layer = layer->parent;
    } 
    return point;
}

GPoint layer_get_bounds_origin(Layer* layer)
{
    return layer->bounds.origin;
}

void layer_set_bounds_origin(Layer* layer, GPoint origin) {
    if (!POINT_EQ(layer->bounds.origin, origin)) {
        layer->bounds.origin = origin;
        layer_mark_dirty(layer);
    }
}

void layer_set_frame(Layer *layer, GRect frame)
{
    if (!RECT_EQ(layer->frame, frame)) {
        layer->frame = frame;
        layer_mark_dirty(layer);
    }
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
    _layer_remove_node(child);
}

void layer_remove_child_layers(Layer *parent)
{
    _layer_delete_tree(parent->child);
}

void layer_insert_below_sibling(Layer *layer_to_insert, Layer *below_sibling_layer)
{
    _layer_insert_node(layer_to_insert, below_sibling_layer, true);
}

void layer_insert_above_sibling(Layer *layer_to_insert, Layer *above_sibling_layer)
{
    _layer_insert_node(layer_to_insert, above_sibling_layer, false);
}

void layer_set_hidden(Layer *layer, bool hidden)
{
    layer->hidden = hidden;
}

bool layer_get_hidden(const Layer *layer)
{
    return layer->hidden;
}

void layer_draw(const Layer *layer, GContext *context)
{
    _layer_walk(layer, context);
}

void layer_apply_frame_offset(const Layer *layer, GContext *context)
{
    context->offset.origin.x += layer->frame.origin.x;
    context->offset.origin.y += layer->frame.origin.y;
    context->offset.size.w = MAX(0, context->offset.size.w - layer->frame.origin.x);
    context->offset.size.h = MAX(0, context->offset.size.h - layer->frame.origin.y);
}

/* Private functions */

static void _layer_insert_node(Layer *layer_to_insert, Layer *sibling_layer, bool below)
{
    if (below)
    {
        // first find the sibling that points to parent
        Layer *parent = _layer_find_parent(sibling_layer, sibling_layer);
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

static void _layer_remove_node(Layer *to_be_removed)
{
    if (to_be_removed->parent == NULL)
        return;
    
    Layer *parent = _layer_find_parent(to_be_removed, to_be_removed->parent);
    // remove our node by pointing next to parents next, jumping over us
    if (parent == NULL)
    {
        SYS_LOG("window", APP_LOG_LEVEL_INFO, "ROOT layers rem %d %d %d %d", to_be_removed, parent, parent->sibling, parent->child);
        
        /* we are the root node */
        return;
    }
    
    if (parent->sibling == to_be_removed)
    {
        parent->sibling = to_be_removed->sibling;
    }
    else if (parent->child == to_be_removed)
    {
        parent->child = NULL;
    }
    to_be_removed->parent = NULL;   
}

/*
 * Recurse through the btree.
 * As we are storing layers as a btree where each sibling
 * is the next layer of the same child as layer->parent
 * layer->child is the head of a new list of siblings where layer->child == new parent
 * This will recurse the children, the siblings of children in a layer
 * When exhaused it will walk the siblings of the parent, etc etc until
 * either 1) no more ram 2) completion
 */
static void _layer_walk(/*const*/ Layer *layer, GContext *context)
{
    if (layer)
    {
        if (layer->hidden == false) // we don't draw hidden layers or their children
        {
            GRect previous_offset = context->offset;
            layer_apply_frame_offset(layer, context);

            if (layer->update_proc)
                layer->update_proc(layer, context);

            // walk this elements sub elements recursively before moving on to the next element
            _layer_walk(layer->child, context);

            context->offset = previous_offset; // restore offset
        }
        _layer_walk(layer->sibling, context);
    }
}

static Layer *_layer_find_parent(Layer *orig_layer, Layer *layer)
{
    if (layer)
    {       
        if (layer->sibling == orig_layer || layer->child == orig_layer)
        {
            return layer;
        }
        
        // walk this elements sub elements recursively before moving on to the next element
        Layer *found_layer = NULL;
        found_layer = _layer_find_parent(orig_layer, layer->child);
        if (found_layer)
            return found_layer;
        
        found_layer = _layer_find_parent(orig_layer, layer->sibling);
        if (found_layer)
            return found_layer;
    }
    return NULL;
}

int inj = 0;
static void _layer_delete_tree(Layer *layer)
{
    if(layer)
    {
        char sinj[20] = "";
        for(int i = 0; i < inj; i++)
            strncat(sinj, "   ", 3);
       
        SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "DTREE %s |_ CHILD %d", sinj, layer);
        if (layer->child)
        {
            inj++;
            _layer_delete_tree(layer->child);
            inj--;
        }
        if (layer->sibling)
        {
            inj++;
            SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "DTREE %s   = SIB %d", sinj, layer);
            _layer_delete_tree(layer->sibling);
            inj--;
        }
        SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "DTREE %s    DONE %d", sinj, layer);
        app_free(layer);
    }
}



