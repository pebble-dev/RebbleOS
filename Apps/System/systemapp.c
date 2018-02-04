/* systemapp.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "systemapp.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "platform_config.h"
#include "platform_res.h"

extern void flash_dump(void);

const char *systemapp_name = "System";

static Window *s_main_window;
static Menu *s_menu;

StatusBarLayer *status_bar;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} Time;

static Time s_last_time;

static MenuItems* flash_dump_item_selected(const MenuItem *item)
{
    flash_dump();
    return NULL;
}

static MenuItems* watch_list_item_selected(const MenuItem *item);

static MenuItems* app_item_selected(const MenuItem *item)
{
    appmanager_app_start(item->text);
    return NULL;
}

static MenuItems* test_item_selected(const MenuItem *item)
{
    appmanager_app_start("Settings");
    return NULL;
}

static MenuItems* run_test_item_selected(const MenuItem *item)
{
//     appmanager_app_start("TestApp");
    return NULL;
}

static MenuItems* notification_item_selected(const MenuItem *item)
{
    appmanager_app_start("Notification");
    return NULL;
}

static MenuItems* watch_list_item_selected(const MenuItem *item) {
    MenuItems *items = menu_items_create(16);
    // loop through all apps
    App *node = app_manager_get_apps_head();
    while(node)
    {
        if ((!strcmp(node->name, "System")) ||
            //             (!strcmp(node->name, "91 Dub 4.0")) ||
            (!strcmp(node->name, "watchface")))
        {
            node = node->next;
            continue;
        }
        menu_items_add(items, MenuItem(node->name, "", RESOURCE_ID_CLOCK, app_item_selected));

        node = node->next;
    }
    return items;
}

static void exit_to_watchface(struct Menu *menu, void *context)
{
    // Exit to watchface
    appmanager_app_start("Simple");
}

static void systemapp_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(s_main_window);
#ifdef PBL_RECT
    s_menu = menu_create(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16));
#else
    // Let the menu draw behind the statusbar so it is perfectly centered
    s_menu = menu_create(GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS));
#endif
    menu_set_callbacks(s_menu, s_menu, (MenuCallbacks) {
        .on_menu_exit = exit_to_watchface
    });
    layer_add_child(window_layer, menu_get_layer(s_menu));

    menu_set_click_config_onto_window(s_menu, window);

    MenuItems *items = menu_items_create(4);
    menu_items_add(items, MenuItem("Watchfaces", "All your faces", RESOURCE_ID_CLOCK, watch_list_item_selected));
    menu_items_add(items, MenuItem("Settings", "Move Along", RESOURCE_ID_SPEECH_BUBBLE, test_item_selected));
    menu_items_add(items, MenuItem("Tests", NULL, RESOURCE_ID_CLOCK, run_test_item_selected));
    menu_items_add(items, MenuItem("RebbleOS", "... v0.0.0.1", RESOURCE_ID_SPEECH_BUBBLE, NULL));
    menu_set_items(s_menu, items);

#ifdef PBL_RECT
    // Status bar is only on rectangular pebbles
    status_bar = status_bar_layer_create();
    layer_add_child(menu_get_layer(s_menu), status_bar_layer_get_layer(status_bar));
#endif

    //tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void systemapp_window_unload(Window *window)
{
    menu_destroy(s_menu);
}

void systemapp_init(void)
{
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = systemapp_window_load,
        .unload = systemapp_window_unload,
    });
    
    window_stack_push(s_main_window, true);
}

void systemapp_deinit(void)
{
    window_destroy(s_main_window);
}

void systemapp_main(void)
{
    systemapp_init();
    app_event_loop();
    systemapp_deinit();
}

void systemapp_tick(void)
{
    struct tm *tick_time = rbl_get_tm();
    
    // Store time
    s_last_time.hours = tick_time->tm_hour;
    s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
    s_last_time.minutes = tick_time->tm_min;
}
