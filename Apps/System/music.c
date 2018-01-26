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
#define RECORD_CENTER_Y 57
#define RECORD_CENTER GPoint(RECORD_CENTER_X, RECORD_CENTER_Y)
#define RECORD_SWOOSH_DISTANCE 130
#define RECORD_SWOOSH_SPEED 250
#define ARM_OFFSET_X 17
#define ARM_OFFSET_Y 77
#define ARM_OFFSET GPoint(ARM_OFFSET_X, ARM_OFFSET_Y)
#define ARM_OFFSET_ANGLE -31
#define ARM_HOME_ANGLE 0
#define ARM_START_ANGLE 18
#define ARM_END_ANGLE 46
static Window *s_main_window;
static Layer *s_main_layer;
ActionBarLayer *s_action_bar;

static n_GPath *arm_base_path_ptr;
static n_GPath *arm_1_path_ptr;
static n_GPath *arm_2_path_ptr;
static n_GPath *arm_head_path_ptr;

static const n_GPathInfo ARM_BASE_PATH_INFO = {    
    .num_points = 2,
    .points = (n_GPoint[]) {{0, -3}, {0, 3}}
};
static const n_GPathInfo ARM_1_PATH_INFO = {    
    .num_points = 2,
    .points = (n_GPoint[]) {{0, -3}, {0, -3-19}}
};
static const n_GPathInfo ARM_2_PATH_INFO = {    
    .num_points = 2,
    .points = (n_GPoint[]) {{0, -3-19}, {19, -3-19-15}}
};
static const n_GPathInfo ARM_HEAD_PATH_INFO = {    
    .num_points = 2,
    .points = (n_GPoint[]) {{19, -3-19-14}, {19+9, -3-19-15-6}}
};

GBitmap *s_up_bitmap;
GBitmap *s_down_bitmap;
GBitmap *s_select_bitmap;

int progress_pixels;
char *curr_artist;
char *curr_track;
char *curr_progress;
char *curr_length;
GColor curr_disk_color;
GColor next_disk_color;
GPoint curr_disk_pos;
GPoint next_disk_pos;
bool animating_disk_change;
bool next_track_animation;
Animation *s_animation_record_ptr;
Animation *s_animation_arm_ptr;
uint32_t arm_angle;
uint32_t old_arm_angle;
uint32_t animation_start_arm_angle;
uint32_t animation_end_arm_angle;

static struct tm s_last_time;

static void status_tick(struct tm *tick_time, TimeUnits tick_units ){
    // Store time
    memcpy(&s_last_time, tick_time, sizeof(struct tm));
    layer_mark_dirty(s_main_layer);
}

static void implementation_setup(Animation *animation) {
    animating_disk_change = true;
    // TODO set next_disk_color based on title + artist,
    // to let each track have the same color between sessions
    next_disk_color =  (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000)  };
    next_disk_pos = GPoint(RECORD_CENTER_X + RECORD_SWOOSH_DISTANCE, RECORD_CENTER_Y);
}

static void implementation_update(Animation *animation, 
                                  const AnimationProgress distance_normalized) {

    curr_disk_pos = GPoint(ANIM_LERP(curr_disk_pos.x, RECORD_CENTER_X - RECORD_SWOOSH_DISTANCE, distance_normalized), RECORD_CENTER_Y);
    next_disk_pos = GPoint(ANIM_LERP(next_disk_pos.x, RECORD_CENTER_X, distance_normalized),                          RECORD_CENTER_Y);

    layer_mark_dirty(s_main_layer);
}

static void implementation_teardown(Animation *animation) {
    animating_disk_change = false;
    curr_disk_color = next_disk_color;
    curr_disk_pos = next_disk_pos;
    animation_end_arm_angle = ARM_START_ANGLE;
    animation_arm(100);
}

static void animation_record() {
    animation_destroy(s_animation_record_ptr);
    s_animation_record_ptr = animation_create();
    animation_set_duration(s_animation_record_ptr, RECORD_SWOOSH_SPEED);
    const AnimationImplementation implementation = {
        .setup = implementation_setup,
        .update = implementation_update,
        .teardown = implementation_teardown
    };
    animation_set_implementation(s_animation_record_ptr, &implementation);
    //animation_set_curve(s_animation_record_ptr, AnimationCurveEaseInOut);
    animation_schedule(s_animation_record_ptr);
}

static void implementation_arm_setup(Animation *animation) {
    animation_start_arm_angle = arm_angle;
}

static void implementation_arm_update(Animation *animation, 
                                  const AnimationProgress distance_normalized) {
    if(animation_start_arm_angle > animation_end_arm_angle)
        arm_angle = animation_start_arm_angle - LERP(animation_end_arm_angle, animation_start_arm_angle);
    else
        arm_angle = LERP(animation_start_arm_angle, animation_end_arm_angle);
    layer_mark_dirty(s_main_layer);
}

static void implementation_arm_teardown(Animation *animation) {
    if(next_track_animation) {
        // If we were just back on our trip back home, animate the record change
        next_track_animation = false;
        animation_record();
    }
}

static void animation_arm(uint32_t duration_ms) {
    s_animation_arm_ptr = animation_create();
    animation_set_duration(s_animation_arm_ptr, duration_ms);
    const AnimationImplementation implementation = {
        .setup = implementation_arm_setup,
        .update = implementation_arm_update,
        .teardown = implementation_arm_teardown
    };
    animation_set_implementation(s_animation_arm_ptr, &implementation);
    //animation_set_curve(s_animation_arm_ptr, AnimationCurveEaseInOut);
    animation_schedule(s_animation_arm_ptr);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO
    arm_angle-=1;
    printf("Arm Angle %d\n", arm_angle);
    layer_mark_dirty(s_main_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO
    //arm_angle-=5;
    //layer_mark_dirty(s_main_layer);
    animation_end_arm_angle = ARM_HOME_ANGLE;
    next_track_animation = true;
    animation_arm(200);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO

    animation_record();
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

void draw_arm(GContext *ctx, uint32_t angle) {
    if(angle != old_arm_angle) {
        n_gpath_destroy(arm_base_path_ptr);
        n_gpath_destroy(arm_1_path_ptr);
        n_gpath_destroy(arm_2_path_ptr);
        n_gpath_destroy(arm_head_path_ptr);
        arm_base_path_ptr = n_gpath_create(&ARM_BASE_PATH_INFO);
        arm_1_path_ptr    = n_gpath_create(&ARM_1_PATH_INFO);
        arm_2_path_ptr    = n_gpath_create(&ARM_2_PATH_INFO);
        arm_head_path_ptr = n_gpath_create(&ARM_HEAD_PATH_INFO);
        n_gpath_move_to(arm_base_path_ptr, ARM_OFFSET);
        n_gpath_move_to(arm_1_path_ptr,    ARM_OFFSET);
        n_gpath_move_to(arm_2_path_ptr,    ARM_OFFSET);
        n_gpath_move_to(arm_head_path_ptr, ARM_OFFSET);
        n_gpath_rotate_to(arm_base_path_ptr, TRIG_MAX_ANGLE / 360 * (angle + ARM_OFFSET_ANGLE));
        n_gpath_rotate_to(arm_1_path_ptr,    TRIG_MAX_ANGLE / 360 * (angle + ARM_OFFSET_ANGLE));
        n_gpath_rotate_to(arm_2_path_ptr,    TRIG_MAX_ANGLE / 360 * (angle + ARM_OFFSET_ANGLE));
        n_gpath_rotate_to(arm_head_path_ptr, TRIG_MAX_ANGLE / 360 * (angle + ARM_OFFSET_ANGLE));
        old_arm_angle = arm_angle;
    }
    n_graphics_context_set_stroke_caps(ctx, false);
    n_graphics_context_set_stroke_width(ctx, 5);
    n_gpath_draw(ctx, arm_base_path_ptr);
    n_graphics_context_set_stroke_width(ctx, 1);
    n_gpath_draw(ctx, arm_1_path_ptr);
    n_gpath_draw(ctx, arm_2_path_ptr);
    n_graphics_context_set_stroke_width(ctx, 4);    
    n_gpath_draw(ctx, arm_head_path_ptr);
}

static void main_layer_update_proc(Layer *layer, GContext *ctx) {
    // TODO Draw these as seperate layers, not all in one
    // eg void _draw_deck(); void _draw_progress()

    // Draw base plate
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_circle(ctx, RECORD_CENTER, 34);
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_circle(ctx, RECORD_CENTER, 7);

    // Draw pin
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, RECORD_CENTER, 1);
    
    // Draw text
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, curr_artist, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(4, 108, 105, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, curr_track, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(4, 117, 110, 50), n_GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    
    graphics_draw_text(ctx, curr_progress, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect( 4, 84, (144 - 30) / 2 + 4, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, curr_length, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect((144 - 30) / 2, 84, (144 - 30) / 2 - 4, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
    
    char time_string[8] = "";
    rcore_strftime(time_string, 8, "%R", &s_last_time);

    graphics_draw_text(ctx, time_string, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0, 0, 113, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    if (animating_disk_change) {
        draw_arm(ctx, arm_angle);
        draw_record(ctx, next_disk_pos, next_disk_color);
    }
    draw_record(ctx, curr_disk_pos, curr_disk_color);

    if(!animating_disk_change) {
        draw_arm(ctx, arm_angle);
    }
    // Draw bar
    #define BAR_HEIGHT 102
    graphics_fill_rect(ctx, GRect(4, BAR_HEIGHT, 105, 6), 1, n_GCornersAll);
    graphics_context_set_fill_color(ctx, curr_disk_color);
    graphics_fill_rect(ctx, GRect(5,BAR_HEIGHT + 1, progress_pixels, 4), 0, GCornerNone);
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
    curr_length = "3:27";
    curr_progress = "0:44";
    arm_angle = ARM_END_ANGLE;
    old_arm_angle = -1;
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
    tick_timer_service_subscribe(MINUTE_UNIT, status_tick);
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
