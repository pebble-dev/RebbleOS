#pragma once
/* menu.h
 *
 * Generic menu component.
 *
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include "librebble.h"
#include "menu_layer.h"

struct Menu;
struct MenuItem;
struct MenuItems;

typedef struct MenuItems* (*MenuItemCallback)(const struct MenuItem *item);

typedef struct MenuItem
{
    char *text;
    char *sub_text;
    uint16_t image_res_id;
    MenuItemCallback on_select;
} MenuItem;

#define MenuItem(text, sub_text, image, on_select) ((MenuItem) { text, sub_text, image, on_select })

typedef struct MenuItems
{
    uint16_t count;
    uint16_t capacity;
    MenuItem *items;

    struct MenuItems *back;
    MenuIndex back_index;
} MenuItems;

MenuItems* menu_items_create(uint16_t capacity);
void menu_items_destroy(MenuItems *items);
void menu_items_add(MenuItems *items, MenuItem item);

// called when back is pressed while in top menu
typedef void (*MenuExitCallback)(struct Menu *menu, void *context);

typedef struct MenuCallbacks
{
    MenuExitCallback on_menu_exit;
} MenuCallbacks;

typedef struct Menu
{
    MenuItems *items;
    MenuLayer *layer;
    MenuCallbacks callbacks;
    void *context;
} Menu;


Menu* menu_create(GRect frame);
void menu_destroy(Menu *menu);
void menu_set_items(Menu *menu, MenuItems *items);
Layer* menu_get_layer(Menu *menu);
void menu_set_callbacks(Menu *menu, void *context, MenuCallbacks callbacks);
void menu_set_click_config_onto_window(Menu *menu, struct Window *window);
