#pragma once
/* simple_menu_layer.h
 * routines for the simple menu layer
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 * Author: Hermann Noll <hermann.noll@hotmail.com>
 */

#include "rect.h"
#include "menu_layer.h"

struct Window;
struct MenuLayer;
struct Layer;

typedef void(* SimpleMenuLayerSelectCallback)(int index, void *context);

typedef struct SimpleMenuItem
{
    const char *title;
    const char *subtitle;
    GBitmap *icon;
    SimpleMenuLayerSelectCallback callback;
} SimpleMenuItem;

typedef struct SimpleMenuSection
{
    const char *title;
    const SimpleMenuItem *items;
    uint32_t num_items;
} SimpleMenuSection;

typedef struct SimpleMenuLayer
{
    MenuLayer menu_layer;
    const SimpleMenuSection *sections;
    int32_t num_sections;
    void *callback_context;
} SimpleMenuLayer;

SimpleMenuLayer* simple_menu_layer_create(GRect frame, struct Window *window, const SimpleMenuSection *sections,
                                          int32_t num_sections, void *callback_context);
void simple_menu_layer_destroy(SimpleMenuLayer *simple_menu);
struct Layer* simple_menu_layer_get_layer(const SimpleMenuLayer *simple_menu);
int simple_menu_layer_get_selected_index(const SimpleMenuLayer *simple_menu);
void simple_menu_layer_set_selected_index(SimpleMenuLayer *simple_menu, int32_t index, bool animated);
struct MenuLayer* simple_menu_layer_get_menu_layer(SimpleMenuLayer *simple_menu);
