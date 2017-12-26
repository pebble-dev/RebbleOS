/* layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "utils.h"
#include "action_bar_layer.h"
#include "bitmap_layer.h"

ActionBarLayer *action_bar_layer_create()
{
    ActionBarLayer *action_bar = (ActionBarLayer*)calloc(1, sizeof(ActionBarLayer));
    
    GRect frame = GRect(144-ACTION_BAR_WIDTH, 0, ACTION_BAR_WIDTH, 168);
    
    Layer* layer = layer_create(frame);
    // give the layer a reference back to us
    layer->container = action_bar;
    action_bar->layer = layer;
    action_bar->context = action_bar;
    
    layer_set_update_proc(layer, draw);
    
    return action_bar;
}

void action_bar_layer_destroy(ActionBarLayer *action_bar)
{
    layer_destroy(action_bar);
    
    free(action_bar);
}

Layer *action_bar_layer_get_layer(ActionBarLayer *action_bar)
{
    return action_bar->layer;
}

void action_bar_layer_set_context(ActionBarLayer *action_bar, void *context)
{
    action_bar->context = context;
}

void action_bar_layer_set_click_config_provider(ActionBarLayer *action_bar, ClickConfigProvider click_config_provider)
{
    action_bar->click_config_provider = click_config_provider;
}

void action_bar_layer_set_icon(ActionBarLayer *action_bar, ButtonId button_id, const GBitmap *icon)
{
    action_bar->icons[button_id] = icon;
}

void action_bar_layer_set_icon_animated(ActionBarLayer *action_bar, ButtonId button_id, const GBitmap *icon, bool animated)
{
    action_bar_layer_set_icon(action_bar, button_id, icon);
}

void action_bar_layer_clear_icon(ActionBarLayer *action_bar, ButtonId button_id)
{
    action_bar_layer_set_icon(action_bar, button_id, NULL);
}

void action_bar_layer_add_to_window(ActionBarLayer *action_bar, struct Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    layer_add_child(window_layer, action_bar->layer);
    window_set_click_config_provider_with_context(window, (ClickConfigProvider) action_bar->click_config_provider,
                                                  action_bar);
    
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    action_bar->layer->frame = GRect(bounds.size.w - ACTION_BAR_WIDTH, 0, ACTION_BAR_WIDTH, bounds.size.h);
}

void action_bar_layer_remove_from_window(ActionBarLayer *action_bar)
{
    layer_remove_from_parent(action_bar->layer);
}

void action_bar_layer_set_background_color(ActionBarLayer *action_bar, GColor background_color)
{
    action_bar->background_color = background_color;
}

void action_bar_layer_set_icon_press_animation(ActionBarLayer *action_bar, ButtonId button_id, ActionBarLayerIconPressAnimation animation)
{
    
}

static void draw(Layer *layer, GContext *context)
{
    ActionBarLayer *action_bar = (ActionBarLayer *) layer->container;
    
    GRect full_bounds = layer_get_bounds(layer);
    
    // Draw the background
    GColor background_color = (GColor) action_bar->background_color;
    graphics_context_set_fill_color(context, background_color);
    graphics_fill_rect(context, full_bounds, 0, GCornerNone);
    
    // Draw the icons
    int y = 0;
    int increment = 168 / NUM_ACTION_BAR_ITEMS;
    y = increment;
    
    for (int i = 1; i <= NUM_ACTION_BAR_ITEMS; i+=1)
    {
        if (action_bar->icons[i] != NULL) {
            // Draw the icon:
            GSize icon_size = action_bar->icons[i]->raw_bitmap_size;
            printf(action_bar->icons[i]);
            graphics_draw_bitmap_in_rect_app(context, action_bar->icons[i], GRect(-full_bounds.size.w - 3, y - (increment / 2) - (icon_size.h / 2), icon_size.w, icon_size.h));
        }
        
        y += increment;
    }
}
