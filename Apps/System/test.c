/* test.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "rebbleos.h"
#include "test.h"
#include "librebble.h"
#include "bitmap_layer.h"
#include "action_bar_layer.h"
#include "status_bar_layer.h"
#include "platform_res.h"

const char * const test_name = "Test";

static Window *s_main_window;

ActionBarLayer *action_bar;
StatusBarLayer *status_bar;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} Time;

static Time s_last_time;

static void play_click_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("UP\n");
}

static void pause_click_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("DOWN\n");
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) pause_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) play_click_handler);
    
    window_set_click_context(BUTTON_ID_DOWN, action_bar);
    window_set_click_context(BUTTON_ID_UP, action_bar);
    window_set_click_context(BUTTON_ID_SELECT, action_bar);
}

static void test_window_load(Window *window)
{
    printf("WF load\n");
    
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    
    // Setup the view
    Layer *wmain = layer_create(bounds);
    layer_add_child(window_layer, wmain);
    
    // Initialize the action bar
    action_bar = action_bar_layer_create();
    
    // Set the icons
    GBitmap *icon1 = gbitmap_create_with_resource(RESOURCE_ID_MUSIC_PLAY);
    GBitmap *icon2 = gbitmap_create_with_resource(RESOURCE_ID_CLOCK);
    GBitmap *icon3 = gbitmap_create_with_resource(RESOURCE_ID_MUSIC_PAUSE);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon1);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon2);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon3);
    
    // Set the click config provider
    action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
    
    // Add it to the window
    action_bar_layer_add_to_window(action_bar, s_main_window);
    
    // Make it red:
    action_bar_layer_set_background_color(action_bar, GColorRed);
    
    // Status Bar
    status_bar = status_bar_layer_create();
    layer_add_child(wmain, status_bar_layer_get_layer(status_bar));
}

static void test_window_unload(Window *window)
{
    action_bar_layer_destroy(action_bar);
}

void test_init(void)
{
    printf("init\n");
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = test_window_load,
        .unload = test_window_unload,
    });
    
    window_stack_push(s_main_window, true);
}

void test_deinit(void)
{
    window_destroy(s_main_window);
}

void test_main(void)
{
    test_init();
    app_event_loop();
    test_deinit();
}

void test_tick(void)
{
    struct tm *tick_time = rbl_get_tm();
    
    printf("system\n");
    // Store time
    s_last_time.hours = tick_time->tm_hour;
    s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
    s_last_time.minutes = tick_time->tm_min;
}
