#pragma once
/* action_menu.h
 *
 * A tiered menu.
 *
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>.
 */

#include "librebble.h"
#include "menu_layer.h"

struct ActionMenuLevel;
typedef struct ActionMenuLevel ActionMenuLevel;

struct ActionMenuItem;
typedef struct ActionMenuItem ActionMenuItem;

struct ActionMenu;
typedef struct ActionMenu ActionMenu;

typedef enum ActionMenuLevelDisplayMode
{
    ActionMenuAlignTop = 0,
    ActionMenuAlignCenter = 1
} ActionMenuAlign;

typedef enum
{
    ActionMenuLevelDisplayModeWide = 0, // ROWS
    ActionMenuLevelDisplayModeThin = 1 // GRIDED
} ActionMenuLevelDisplayMode;

typedef void(* ActionMenuDidCloseCb)(ActionMenu *menu, const ActionMenuItem *performed_action, void *context);
typedef void(* ActionMenuPerformActionCb)(ActionMenu *action_menu, const ActionMenuItem *action, void *context);
typedef void(* ActionMenuEachItemCb)(const ActionMenuItem *item, void *context);

typedef struct ActionMenuConfig
{
    const ActionMenuLevel *root_level;
    void *context;
    struct
    {
        GColor background; // Left column
        GColor foreground; // Dots
    } colors;
    ActionMenuDidCloseCb will_close;
    ActionMenuDidCloseCb did_close;
    ActionMenuAlign align;
} ActionMenuConfig;

char * action_menu_item_get_label(const ActionMenuItem * item);

void * action_menu_item_get_action_data(const ActionMenuItem * item);

ActionMenuLevel * action_menu_level_create(uint16_t max_items);

void action_menu_level_set_display_mode(ActionMenuLevel * level, ActionMenuLevelDisplayMode display_mode);

ActionMenuItem * action_menu_level_add_action(ActionMenuLevel * level, const char *label, ActionMenuPerformActionCb cb, void * action_data);

ActionMenuItem * action_menu_level_add_child(ActionMenuLevel * level, ActionMenuLevel * child, const char * label);

void action_menu_hierarchy_destroy(const ActionMenuLevel * root, ActionMenuEachItemCb each_cb, void * context);

void * action_menu_get_context(ActionMenu * action_menu);

ActionMenuLevel * action_menu_get_root_level(ActionMenu * action_menu);

ActionMenu * action_menu_open(ActionMenuConfig * config);

void action_menu_freeze(ActionMenu * action_menu);

void action_menu_unfreeze(ActionMenu * action_menu);

void action_menu_set_result_window(ActionMenu * action_menu, Window * result_window);

void action_menu_close(ActionMenu * action_menu, bool animated);

/*
struct ActionMenuItem;
struct ActionMenuItems;

typedef struct ActionMenuItems* (*ActionMenuItemCallback)(const struct ActionMenuItem *item);

typedef struct ActionMenuItem
{
    char *text;
    char *sub_text;
    MenuItemCallback on_select;
} ActionMenuItem;

#define ActionMenuItem(text, sub_text, image, on_select) ((MenuItem) { text, sub_text, image, on_select })

typedef struct ActionMenuItems
{
    uint16_t count;
    uint16_t capacity;
    MenuItem *items;
    
    struct ActionMenuItems *back;
    ActionMenuIndex back_index;
} ActionMenuItems;

ActionMenuItems* action_menu_items_create(uint16_t capacity);
void action_menu_items_destroy(ActionMenuItems *items);
void action_menu_items_add(ActionMenuItems *items, ActionMenuItem item);

// called when back is pressed while in top menu
typedef void (*ActionMenuExitCallback)(struct ActionMenu *menu, void *context);

typedef struct ActionMenuCallbacks
{
    ActionMenuExitCallback on_action_menu_exit;
} ActionMenuCallbacks;

typedef struct ActionMenu
{
    ActionMenuItems *items;
    MenuLayer *layer;
    ActionMenuCallbacks callbacks;
    void *context;
} ActionMenu;


ActionMenu* action_menu_create(GRect frame);
void menu_destroy(Menu *menu);
void menu_set_items(Menu *menu, MenuItems *items);
Layer* menu_get_layer(Menu *menu);
void menu_set_callbacks(Menu *menu, void *context, MenuCallbacks callbacks);
void menu_set_click_config_onto_window(Menu *menu, struct Window *window);*/
