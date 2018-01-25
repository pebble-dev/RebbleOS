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
#include "property_animation.h"

#define RECORD_CENTER_X 56
#define RECORD_CENTER_Y 60
#define RECORD_CENTER GPoint(RECORD_CENTER_X, RECORD_CENTER_Y)
#define RECORD_SWOOSH_DISTANCE 130
#define RECORD_SWOOSH_SPEED 500
static Window *s_main_window;
static Layer *s_main_layer;
ActionBarLayer *s_action_bar;

GBitmap *s_up_bitmap;
GBitmap *s_down_bitmap;
GBitmap *s_select_bitmap;

int progress_pixels;
char *curr_artist;
char *curr_track;
GColor curr_disk_color;
GColor next_disk_color;
GPoint curr_disk_pos;
GPoint next_disk_pos;
bool animating_disk_change;

static void implementation_setup(Animation *animation) {
    animating_disk_change = true;
    // TODO set next_disk_color based on title + artist,
    // to let each track have the same color between sessions
    next_disk_color =  (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000)  };
    next_disk_pos = GPoint(RECORD_CENTER_X + RECORD_SWOOSH_DISTANCE, RECORD_CENTER_Y);
}

static void implementation_update(Animation *animation, 
                                  const AnimationProgress progress) {
    int distance_normalized = (int)progress;

    curr_disk_pos = GPoint(ANIM_LERP(curr_disk_pos.x, RECORD_CENTER_X - RECORD_SWOOSH_DISTANCE, distance_normalized), RECORD_CENTER_Y);
    next_disk_pos = GPoint(ANIM_LERP(next_disk_pos.x, RECORD_CENTER_X, distance_normalized),                          RECORD_CENTER_Y);

    layer_mark_dirty(s_main_layer);
}

static void implementation_teardown(Animation *animation) {
    curr_disk_color = next_disk_color;
    curr_disk_pos = next_disk_pos;
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO

    Animation *animation = animation_create();
    animation_set_duration(animation, RECORD_SWOOSH_SPEED);
    const AnimationImplementation implementation = {
        .setup = implementation_setup,
        .update = implementation_update,
        .teardown = implementation_teardown
    };
    animation_set_implementation(animation, &implementation);
    animation_schedule(animation);
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

void draw_record(GContext *ctx, GPoint record_center_point, GColor record_color) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, record_center_point, 36);
    graphics_context_set_fill_color(ctx, record_color);
    graphics_fill_circle(ctx, record_center_point, 34);
    //graphics_context_set_stroke_width(ctx, 1);
    //graphics_context_set_stroke_color(ctx, GColorBlack);
    //graphics_draw_circle(ctx, record_center_point, 36);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, record_center_point, 13);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, record_center_point, 11);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, record_center_point, 1);
    //graphics_draw_arc(ctx, GRect(RECORD_CENTER_X, RECORD_CENTER_Y,30,30), DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(90));
}

static void main_layer_update_proc(Layer *layer, GContext *ctx) {
    // TODO Draw these as seperate layers, not all in one
    // eg void _draw_deck(); void _draw_progress()

    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_circle(ctx, RECORD_CENTER, 34);

    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_circle(ctx, RECORD_CENTER, 7);

    // Draw pin
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, RECORD_CENTER, 1);
    
    if (animating_disk_change) {
        draw_record(ctx, next_disk_pos, next_disk_color);
    }
    draw_record(ctx, curr_disk_pos, curr_disk_color);

    // Draw bar
    graphics_fill_rect(ctx, GRect(4, 116, 105, 6), 1, n_GCornersAll);
    graphics_context_set_fill_color(ctx, curr_disk_color);
    graphics_fill_rect(ctx, GRect(5,117, progress_pixels, 4), 0, GCornerNone);
    graphics_context_set_text_color(ctx, GColorBlack);

    // Draw text
    graphics_draw_text(ctx, curr_artist, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(4, 97, 105, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, curr_track, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(4, 117, 110, 50), n_GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, "9:36", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(0, 4, 113, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void music_window_load(Window *window) {
    srand(50);
    // srand(time(NULL)); TODO time seed
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    curr_disk_color = GColorWindsorTan;
    curr_disk_pos = RECORD_CENTER;
    progress_pixels = 25;
    curr_artist = "The Beatles";
    curr_track = "Maxwell's Silver Hammer";
    
    s_main_layer = layer_create(bounds);
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
