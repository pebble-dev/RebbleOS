/* settings.c
 * GUI implementation for settings
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "rebbleos.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "action_bar_layer.h"
#include "platform_res.h"

static int _in_pairing = 0;
static Window *_bt_pair_window;
static Layer *_bt_pair_layer;
static ActionBarLayer *_bt_pair_action_bar;
static GBitmap *_bt_pair_accept, *_bt_pair_reject;
static StatusBarLayer *_bt_pair_status;
static char *_bt_pair_name;

static void _bluetooth_pair_request(EventServiceCommand svc, void *data, void *ctx) {
    const char *name = (const char *)data;
    APP_LOG("settings", APP_LOG_LEVEL_INFO, "BT pair request: %s", name);
    _bt_pair_name = strdup(name);
    window_stack_push(_bt_pair_window, false);
}

static void _bt_pair_do_accept(ClickRecognizerRef recognizer, void *context) {
    APP_LOG("settings", APP_LOG_LEVEL_INFO, "BT pair accept!");
    bluetooth_bond_acknowledge(1);
    window_stack_pop(false);
}

static void _bt_pair_do_reject(ClickRecognizerRef recognizer, void *context) {
    APP_LOG("settings", APP_LOG_LEVEL_INFO, "BT pair reject!");
    bluetooth_bond_acknowledge(0);
    window_stack_pop(false);
}

static void _bt_pair_action_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) _bt_pair_do_accept);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) _bt_pair_do_reject);
}

static void _bt_pair_update_proc(Window *window, GContext *ctx) {
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, "Pair?", fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(0, 40, DISPLAY_COLS-ACTION_BAR_WIDTH, 28), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
    graphics_draw_text(ctx, _bt_pair_name, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0, 100, DISPLAY_COLS-ACTION_BAR_WIDTH, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
}

static void _bt_pair_window_load(Window *window) {
    _bt_pair_accept = gbitmap_create_with_resource(RESOURCE_ID_CHECK_BLACK);
    _bt_pair_reject = gbitmap_create_with_resource(RESOURCE_ID_DISMISS_BLACK);
    GRect bmsize = gbitmap_get_bounds(_bt_pair_accept);
    APP_LOG("settings", APP_LOG_LEVEL_INFO, "bt pair window load");
    
    window_set_background_color(window, GColorWhite);
    
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    
    _bt_pair_layer = layer_create(bounds);
    layer_add_child(window_layer, _bt_pair_layer);
    layer_set_update_proc(_bt_pair_layer, _bt_pair_update_proc);
    layer_mark_dirty(_bt_pair_layer);
    
    _bt_pair_action_bar = action_bar_layer_create();
    action_bar_layer_set_click_config_provider(_bt_pair_action_bar, _bt_pair_action_click_config_provider);
    action_bar_layer_set_icon_animated(_bt_pair_action_bar, BUTTON_ID_UP, _bt_pair_accept, true);
    action_bar_layer_set_icon_animated(_bt_pair_action_bar, BUTTON_ID_DOWN, _bt_pair_reject, true);
    action_bar_layer_set_background_color(_bt_pair_action_bar, GColorBlack);
    action_bar_layer_add_to_window(_bt_pair_action_bar, window);
}

static void _bt_pair_window_unload(Window *window) {
    action_bar_layer_destroy(_bt_pair_action_bar);
    gbitmap_destroy(_bt_pair_accept);
    gbitmap_destroy(_bt_pair_reject);
    layer_destroy(_bt_pair_layer);
    status_bar_layer_destroy(_bt_pair_status);
    free(_bt_pair_name);
}

static Window *_main_window;
static Menu *_menu;
static StatusBarLayer *status_bar;

static void _reset_menu_items(void);


static void exit_to_watchface(struct Menu *menu, void *context)
{
    /* Exit to watchface */
    bluetooth_advertising_visible(0);
    window_stack_pop(true);
}

static struct MenuItems *_really_wipe_fs(const struct MenuItem *ctx) {
    fs_format();
    panic("ok, goodbye");
    return NULL;
}

static struct MenuItems *_wipe_fs(const struct MenuItem *ctx) {
    MenuItems *items = menu_items_create(2);
    menu_items_add(items, MenuItem("No", "j/k, lol", RESOURCE_ID_SPANNER, NULL));
    menu_items_add(items, MenuItem("Yes", "I really mean it", RESOURCE_ID_SPANNER, _really_wipe_fs));
    return items;
}

static struct MenuItems *_dummy_bt(const struct MenuItem *ctx) {
    _bluetooth_pair_request(0, "Dummy phone name", NULL);
    return NULL;
}

static struct MenuItems *_do_panic(const struct MenuItem *ctx) {
    panic("Lolol!");
    return NULL;
}

static void _reset_menu_items(void)
{
    MenuItems *back = _menu->items->back;
    MenuIndex *index = &_menu->items->back_index;

    MenuItems *items = menu_items_create(4);
    
    menu_items_add(items, MenuItem("Discoverable", bluetooth_name(), RESOURCE_ID_SPANNER, NULL));
    menu_items_add(items, MenuItem("Format filesystem", "Time to die, sucker!", RESOURCE_ID_SPANNER, _wipe_fs));
    menu_items_add(items, MenuItem("Dummy BT pair", "Be a dummy", RESOURCE_ID_SPANNER, _dummy_bt));
    menu_items_add(items, MenuItem("Instapanic", "Oh no!", RESOURCE_ID_SPANNER, _do_panic));
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

    bluetooth_advertising_visible(1);
    event_service_subscribe(EventServiceCommandBluetoothPairRequest, _bluetooth_pair_request);
}

static void _settings_window_unload(Window *window)
{
    menu_destroy(_menu);
    status_bar_layer_destroy(status_bar);
    bluetooth_advertising_visible(0);
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
    
    _bt_pair_window = window_create();
    window_set_window_handlers(_bt_pair_window, (WindowHandlers) {
        .load = _bt_pair_window_load,
        .unload = _bt_pair_window_unload,
    });
}

void settings_deinit(void)
{
    window_destroy(_main_window);
    window_destroy(_bt_pair_window);
}
