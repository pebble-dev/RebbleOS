/* music.c
 *
 * Music player
 *
 * RebbleOS
 * 
 * Author: Chris Multhaupt <chris.multhaupt@gmail.com>.
 */

#include "rebbleos.h"
#include "music.h"

#define RECORD_CENTER_X 56
#define RECORD_CENTER_Y 60
#define RECORD_CENTER GPoint(RECORD_CENTER_X, RECORD_CENTER_Y)

static Window *s_main_window;
ActionBarLayer *s_action_bar;

GBitmap *s_up_bitmap;
GBitmap *s_down_bitmap;
GBitmap *s_select_bitmap;

int progress_pixels;
char *current_artist;
char *current_track;

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_click_handler);
    
    window_set_click_context(BUTTON_ID_UP, s_action_bar);
    window_set_click_context(BUTTON_ID_DOWN, s_action_bar);
    window_set_click_context(BUTTON_ID_SELECT, s_action_bar);
}

static void main_layer_update_proc(Layer *layer, GContext *ctx) {
    // TODO Draw these as seperate layers, not all in one
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, RECORD_CENTER, 36);
    graphics_context_set_fill_color(ctx, GColorWindsorTan);
    graphics_fill_circle(ctx, RECORD_CENTER, 34);
    //graphics_context_set_stroke_width(ctx, 1);
    //graphics_context_set_stroke_color(ctx, GColorBlack);
    //graphics_draw_circle(ctx, RECORD_CENTER, 36);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, RECORD_CENTER, 13);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, RECORD_CENTER, 11);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, RECORD_CENTER, 1);
    //graphics_draw_arc(ctx, GRect(RECORD_CENTER_X, RECORD_CENTER_Y,30,30), DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(90));
    graphics_fill_rect(ctx, GRect(4, 116, 105, 6), 1, n_GCornersAll);
    graphics_context_set_fill_color(ctx, GColorWindsorTan);
    graphics_fill_rect(ctx, GRect(5,117, progress_pixels, 4), 0, n_GCornersNone); // TODO define GColorsNone    
    graphics_draw_text(ctx, current_artist, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(4, 97, 105, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, current_track, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(4, 117, 110, 50), n_GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, "9:36", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(0, 4, 113, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void music_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    progress_pixels = 25;
    current_artist = "The Beatles";
    current_track = "Maxwell's Silver Hammer";
    
    Layer *s_main_layer = layer_create(bounds);
    layer_add_child(window_layer, s_main_layer);

    s_action_bar = action_bar_layer_create();
    s_up_bitmap = gbitmap_create_with_resource(21);
    s_down_bitmap = gbitmap_create_with_resource(25);
    s_select_bitmap = gbitmap_create_with_resource(22);
    action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_up_bitmap);
    action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_down_bitmap);
    action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_select_bitmap);
    action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
    action_bar_layer_add_to_window(s_action_bar, s_main_window);
    action_bar_layer_set_background_color(s_action_bar, GColorLightGray);
    
    layer_set_update_proc(s_main_layer, main_layer_update_proc);
    layer_mark_dirty(s_main_layer);
}

static void music_window_unload(Window *window) {
    action_bar_layer_destroy(s_action_bar);
    gbitmap_destroy(s_up_bitmap);
    gbitmap_destroy(s_down_bitmap);
    gbitmap_destroy(s_select_bitmap);
}

void music_init(void) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = music_window_load,
        .unload = music_window_unload,
    });
    window_stack_push(s_main_window, true);
}

void music_deinit(void) {
    window_destroy(s_main_window);
}

void music_main(void) {
    music_init();
    app_event_loop();
    music_deinit();
}
