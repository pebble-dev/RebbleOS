/* notification.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "rebbleos.h"
#include "notification.h"
#include "librebble.h"
#include "bitmap_layer.h"
#include "action_bar_layer.h"

const char *notif_name = "Notification";

static Window *s_main_window;

static Layer *main;

static GBitmap *icon;

int offset = 0;

ActionBarLayer *action_bar;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} Time;

static Time s_last_time;

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("UP");
    
    if (offset - 40 < 0)
    {
        offset = 0;
    } else {
        offset -= 40;
    }
    
    layer_mark_dirty(main);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("SELECT");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("DOWN");
    offset += 40;
    layer_mark_dirty(main);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("BACK");
    appmanager_app_start("System");
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler) back_click_handler);
    
    window_set_click_context(BUTTON_ID_DOWN, action_bar);
    window_set_click_context(BUTTON_ID_UP, action_bar);
    window_set_click_context(BUTTON_ID_SELECT, action_bar);
}

// Custom drawing
static void main_update_proc(Layer *layer, GContext *ctx)
{
    GRect bounds = layer_get_unobstructed_bounds(layer);
    
    // Draw the background:
    graphics_context_set_fill_color(ctx, COLOR);
    GRect rect_bounds = GRect(0, 0 - offset, bounds.size.w, 50);
    graphics_fill_rect_app(ctx, rect_bounds, 0, GCornerNone);
    
    // Draw the icon:
    if (icon != NULL)
    {
        GSize icon_size = icon->raw_bitmap_size;
        graphics_draw_bitmap_in_rect_app(ctx, icon, GRect(bounds.size.w / 2 - (icon_size.w / 2), 25 - offset - (icon_size.h / 2), icon_size.w, icon_size.h));
        gbitmap_destroy(icon);
    }
    
    // Draw the app:
    ctx->text_color = GColorBlack;
    GRect app_rect = GRect(10, 53 - offset, bounds.size.w - 20, 20);
    GFont app_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    graphics_draw_text_app(ctx, APP, app_font, app_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
    
    // Draw the title:
    GRect title_rect = GRect(10, 67 - offset, bounds.size.w - 20, 30);
    GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    graphics_draw_text_app(ctx, TITLE, title_font, title_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
    
    // Draw the body:
    GRect body_rect = GRect(10, 86 - offset, bounds.size.w - 20, 200);
    GFont body_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
    graphics_draw_text_app(ctx, BODY, body_font, body_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
}

static void notif_window_load(Window *window)
{
    printf("WF load\n");
    
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    
    // Setup the layers:
    main = layer_create(bounds);
    layer_set_update_proc(main, main_update_proc);
    
    // Setup the view and draw:
    layer_add_child(window_layer, main);
    layer_mark_dirty(main);
    
    // Click config provider
    window_set_click_config_provider_with_context(window, (ClickConfigProvider) click_config_provider, main);
}

void present_reply_options() {
    // Initialize the action bar
    action_bar = action_bar_layer_create();
    
    // Set the icons
    GBitmap *icon1 = gbitmap_create_with_resource(21);
    GBitmap *icon2 = gbitmap_create_with_resource(25);
    GBitmap *icon3 = gbitmap_create_with_resource(22);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon1);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon2);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon3);
    
    // Set the click config provider
    action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
    
    // Add it to the window
    action_bar_layer_add_to_window(action_bar, s_main_window);
    
    // Make it red:
    action_bar_layer_set_background_color(action_bar, GColorRed);
}

static void notif_window_unload(Window *window)
{
    action_bar_layer_destroy(action_bar);
}

void notif_init(void)
{
    printf("init\n");
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = notif_window_load,
        .unload = notif_window_unload,
    });
    
    window_stack_push(s_main_window, true);
    
    //icon = gbitmap_create_with_resource(19);
}

void notif_deinit(void)
{
    window_destroy(s_main_window);
}

void notif_main(void)
{
    notif_init();
    app_event_loop();
    notif_deinit();
}

void notif_tick(void)
{
    struct tm *tick_time = rbl_get_tm();
    
    printf("notification\n");
    // Store time
    s_last_time.hours = tick_time->tm_hour;
    s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
    s_last_time.minutes = tick_time->tm_min;
}
