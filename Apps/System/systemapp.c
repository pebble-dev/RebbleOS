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
#include "node_list.h"

extern void flash_dump(void);
extern const char git_version[];
extern const char *git_authors[];

static Window *s_main_window;
static Menu *s_menu;

static Window *s_about_window;
static Layer *s_aboutCanvas_layer;
static ScrollLayer *s_about_scroll;
static int about_did_set_size;
static void about_update_proc(Layer *layer, GContext *nGContext);
static GBitmap *logo_bitmap;
static GBitmap *rocket_bitmap;

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

static MenuItems* settings_item_selected(const MenuItem *item)
{
    appmanager_app_start("Settings");
    return NULL;
}

static MenuItems* run_test_item_selected(const MenuItem *item)
{
    appmanager_app_start("TestApp");
    return NULL;
}

static MenuItems* notification_item_selected(const MenuItem *item)
{
    appmanager_app_start("Notification");
    return NULL;
}

static MenuItems* about_item_selected(const MenuItem *item)
{
    window_stack_push(s_about_window, false);
    return NULL;
}

static MenuItems* watch_list_item_selected(const MenuItem *item) {
    MenuItems *items = menu_items_create(16);
    // loop through all apps
    list_head * app_head = app_manager_get_apps_head();
    App * app;
    list_foreach(app, app_head, App, node)
    {
        if ((!strcmp(app->name, "System")) ||
            (!strcmp(app->name, "watchface")) ||
            app->type != APP_TYPE_FACE)
        {
            continue;
        }
        menu_items_add(items, MenuItem(app->name, NULL, RESOURCE_ID_CLOCK, app_item_selected));
    }
    return items;
}

static MenuItems* app_list_item_selected(const MenuItem *item) {
    MenuItems *items = menu_items_create(16);
    // loop through all apps
    list_head * app_head = app_manager_get_apps_head();
    App * app;
    list_foreach(app, app_head, App, node)
    {
        if ((!strcmp(app->name, "System")) ||
            (!strcmp(app->name, "watchface")) ||
            app->type == APP_TYPE_FACE)
        {
            continue;
        }
        menu_items_add(items, MenuItem(app->name, NULL, RESOURCE_ID_CLOCK, app_item_selected));
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

    // For round, let the menu draw behind the statusbar so it is perfectly centered
    s_menu = menu_create(PBL_IF_RECT_ELSE(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16), GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS)));

    menu_set_callbacks(s_menu, s_menu, (MenuCallbacks) {
        .on_menu_exit = exit_to_watchface
    });
    layer_add_child(window_layer, menu_get_layer(s_menu));

    menu_set_click_config_onto_window(s_menu, window);

    MenuItems *items = menu_items_create(6);
    menu_items_add(items, MenuItem("Watchfaces", "All your faces", RESOURCE_ID_CLOCK, watch_list_item_selected));
    menu_items_add(items, MenuItem("Apps", "Get appy", RESOURCE_ID_CLOCK, app_list_item_selected));
    menu_items_add(items, MenuItem("Settings", "Config", RESOURCE_ID_SPANNER, settings_item_selected));
    menu_items_add(items, MenuItem("Tests", NULL, RESOURCE_ID_CLOCK, run_test_item_selected));
    menu_items_add(items, MenuItem("Notifications", NULL, RESOURCE_ID_SPEECH_BUBBLE, notification_item_selected));
    menu_items_add(items, MenuItem("About", "It's-a me!", RESOURCE_ID_SPEECH_BUBBLE, about_item_selected));
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

static void window_exit_handler(ClickRecognizerRef recognizer, void *context)
{
    window_stack_pop(true);  
}

static void about_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(s_about_window);
    GRect bounds = layer_get_bounds(window_layer);

    status_bar = status_bar_layer_create();
    status_bar_layer_set_separator_mode(status_bar, StatusBarLayerSeparatorModeDotted);

    status_bar_layer_set_colors(status_bar, PBL_IF_COLOR_ELSE(GColorRed,GColorBlack), GColorWhite);

    status_bar_layer_set_text(status_bar, "About RebbleOS");

    s_about_scroll = scroll_layer_create(bounds);
    scroll_layer_set_click_config_onto_window(s_about_scroll, window);

    s_aboutCanvas_layer = layer_create(bounds);
    layer_set_update_proc(s_aboutCanvas_layer, about_update_proc);
    scroll_layer_add_child(s_about_scroll, s_aboutCanvas_layer);	
    layer_mark_dirty(s_aboutCanvas_layer);

    layer_add_child(window_layer, scroll_layer_get_layer(s_about_scroll));
    layer_add_child(window_layer, status_bar_layer_get_layer(status_bar));

    //TODO: Get black and white rocket bitmap for Classic
    logo_bitmap = gbitmap_create_with_resource(PBL_IF_BW_ELSE(RESOURCE_ID_REBBLE_LOGO_BW, RESOURCE_ID_REBBLE_LOGO_DARK));
    rocket_bitmap = gbitmap_create_with_resource(PBL_IF_BW_ELSE(RESOURCE_ID_TO_MOON_BW, RESOURCE_ID_TO_MOON));

    about_did_set_size = 0;
    
    /* XXX: Some day it would be cool to automatically scroll the credits after a short delay. */
}

static void about_update_proc(Layer *layer, GContext *nGContext)
{  
    GRect bounds = layer_get_unobstructed_bounds(layer);
    graphics_context_set_text_color(nGContext, GColorBlack);
    graphics_context_set_compositing_mode(nGContext, GCompOpSet);

    window_single_click_subscribe(BUTTON_ID_BACK, window_exit_handler);

    graphics_draw_text(nGContext, "Version:", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect((bounds.size.w/2)-70, (bounds.size.h/2)-10, 140, 20),
                               GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);

    graphics_draw_text(nGContext, git_version, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect((bounds.size.w/2)-70, (bounds.size.h/2)+5, 140, 20),
                               GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);

    graphics_draw_text(nGContext, "Join us!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect((bounds.size.w/2)-70, (bounds.size.h/2)+20, 140, 20),
                               GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);

    graphics_draw_text(nGContext, "https://rebble.io/discord", fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect((bounds.size.w/2)-70, (bounds.size.h/2)+38, 140, 20),
                               GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);

    graphics_draw_bitmap_in_rect(nGContext, logo_bitmap, GRect((bounds.size.w/2)-17, (bounds.size.h/2)-63, 34, 53));
    graphics_draw_bitmap_in_rect(nGContext, rocket_bitmap, GRect((bounds.size.w/2)-8, (bounds.size.h/2)+60, 19, 19));
    
    graphics_draw_text(nGContext, "Credits", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(10, bounds.size.h, bounds.size.w - 20, 20),
                       n_GTextOverflowModeWordWrap, GTextAlignmentCenter, 0);
    
    const char **authorp = git_authors;
    int y = bounds.size.h + 30;
    for (authorp = git_authors; *authorp; authorp++) {
        /* XXX: Do better about authors who have long names. */
        graphics_draw_text(nGContext, *authorp, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, y, bounds.size.w - 20, 16),
                           n_GTextOverflowModeWordWrap, GTextAlignmentCenter, 0);
        y += 16;
    }
    
    /* Set the size on the first render only, lest we interfere with actual scrolling. */
    if (!about_did_set_size) {
        about_did_set_size = 1;
        scroll_layer_set_content_size(s_about_scroll, (GSize) {bounds.size.w, y});
    }
}

static void about_window_unload(Window *window)
{
    scroll_layer_destroy(s_about_scroll);
    gbitmap_destroy(logo_bitmap);
    gbitmap_destroy(rocket_bitmap);
}

void systemapp_init(void)
{
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = systemapp_window_load,
        .unload = systemapp_window_unload,
    });

    s_about_window = window_create();

    window_set_window_handlers(s_about_window, (WindowHandlers) {
        .load = about_window_load,
        .unload = about_window_unload,
    });

    window_stack_push(s_main_window, true);
}

void systemapp_deinit(void)
{
    window_destroy(s_main_window);
    window_destroy(s_about_window);
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
