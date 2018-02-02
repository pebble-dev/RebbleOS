#pragma once
/* window.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 *         Carson Katri <me@carsonkatri.com>
 */

#include <stdbool.h>
#include "point.h"
#include "rect.h"
#include "size.h"
#include "node_list.h"

struct Layer;

// typedef void (*WindowButtonEventHandler)(AppContextRef app_ctx, struct Window *window, PebbleButtonEvent *event);
//
// typedef struct WindowInputHandlers
// {
//     struct
//     {
//         WindowButtonEventHandler up;
//         WindowButtonEventHandler down;
//     } buttons;
// } WindowInputHandlers;

typedef void (*WindowHandler)(struct Window *window);

typedef struct WindowHandlers
{
    WindowHandler load;
    WindowHandler appear;
    WindowHandler disappear;
    WindowHandler unload;
} WindowHandlers;

typedef struct window_node window_node;

typedef struct Window
{
    Layer *root_layer;
    const GBitmap *status_bar_icon;
    //WindowInputHandlers input_handlers;
    WindowHandlers window_handlers;
    ClickConfigProvider click_config_provider; // callback to the provider function
    void *click_config_context;
    void *user_data;
    GColor background_color;
    bool is_render_scheduled;
    //bool on_screen : 1;
    bool is_loaded;
    //bool overrides_back_button : 1;
    //bool is_fullscreen : 1;
    //const char *debug_name;
    list_node node;
} Window;

// Window management
Window *window_create();
void window_destroy(Window *window) ;
void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider);
void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context);
ClickConfigProvider window_get_click_config_provider(const Window *window);
void *window_get_click_config_context(Window *window);
void window_set_window_handlers(Window *window, WindowHandlers handlers);
Layer *window_get_root_layer(Window *window);
void window_set_background_color(Window *window, GColor background_color);
bool window_is_loaded(Window *window);
void window_set_user_data(Window *window, void *data);
void * window_get_user_data(const Window *window);
void window_single_click_subscribe(ButtonId button_id, ClickHandler handler);
void window_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler);
void window_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler);
void window_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler);
void window_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context);
void window_set_click_context(ButtonId button_id, void *context);

void window_stack_push(Window *window, bool animated);
Window * window_stack_pop(bool animated);
void window_stack_pop_all(const bool animated);
bool window_stack_remove(Window *window, bool animated);
bool window_stack_contains_window(Window *window);
Window * window_stack_get_top_window(void);
void window_dirty(bool is_dirty);
void window_draw();
uint16_t window_count(void);
