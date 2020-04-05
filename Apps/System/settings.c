/* settings.c
 * GUI implementation for settings
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "rebbleos.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "platform_res.h"

static Window *_main_window;
static Menu *_menu;
static StatusBarLayer *status_bar;

static void _reset_menu_items(void);

static void exit_to_watchface(struct Menu *menu, void *context)
{
    /* Exit to watchface */
    hw_bluetooth_advertising_visible(0);
    window_stack_pop(true);
}

static struct MenuItems *_really_wipe_fs(const struct MenuItem *ctx) {
    fs_format();
    panic("ok, goodbye");
}

static struct MenuItems *_wipe_fs(const struct MenuItem *ctx) {
    MenuItems *items = menu_items_create(2);
    menu_items_add(items, MenuItem("No", "j/k, lol", RESOURCE_ID_SPANNER, NULL));
    menu_items_add(items, MenuItem("Yes", "I really mean it", RESOURCE_ID_SPANNER, _really_wipe_fs));
    return items;
}

static void _reset_menu_items(void)
{
    MenuItems *back = _menu->items->back;
    MenuIndex *index = &_menu->items->back_index;

    MenuItems *items = menu_items_create(2);
    
    menu_items_add(items, MenuItem("Discoverable", hw_bluetooth_name(), RESOURCE_ID_SPANNER, NULL));
    menu_items_add(items, MenuItem("Format filesystem", "Time to die, sucker!", RESOURCE_ID_SPANNER, _wipe_fs));
    menu_set_items(_menu, items);

    items->back = back;
    items->back_index = *index;
}

static void _settings_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);

#ifdef PBL_RECT
    _menu = menu_create(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16));
#else
    // Let the menu draw behind the statusbar so it is perfectly centered
    _menu = menu_create(GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS));
#endif
    menu_set_callbacks(_menu, _menu, (MenuCallbacks) {
        .on_menu_exit = exit_to_watchface
    });
    layer_add_child(window_layer, menu_get_layer(_menu));
    menu_set_click_config_onto_window(_menu, window);

    _reset_menu_items();

    // Status Bar
    status_bar = status_bar_layer_create();
    layer_add_child(menu_get_layer(_menu), status_bar_layer_get_layer(status_bar));

    hw_bluetooth_advertising_visible(1);
}

static void _settings_window_unload(Window *window)
{
    menu_destroy(_menu);
    status_bar_layer_destroy(status_bar);
    hw_bluetooth_advertising_visible(0);
}

void settings_enter(void) {
    window_stack_push(_main_window, true);
}

void settings_init(void)
{
    _main_window = window_create();

    window_set_window_handlers(_main_window, (WindowHandlers) {
        .load = _settings_window_load,
        .unload = _settings_window_unload,
    });
}

void settings_deinit(void)
{
    window_destroy(_main_window);
}
