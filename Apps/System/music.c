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
#define ARM_SKIP_SPEED 200
#define ARM_QUICK_SPEED 100
#define SKIP_DIRECTION_PREV -1
#define SKIP_DIRECTION_NEXT 1
#define PROGRESS_PIXELS_MAX 105

static Window *s_main_window;
static Layer *s_main_layer;
static ActionBarLayer *s_action_bar;

static n_GPath *s_arm_base_path_ptr;
static n_GPath *s_arm_1_path_ptr;
static n_GPath *s_arm_2_path_ptr;
static n_GPath *s_arm_head_path_ptr;

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

static Animation *s_animation_record_ptr;
static Animation *s_animation_arm_ptr;
static bool s_animating_disk_change;
static bool s_animating_arm_change;
static bool s_is_paused;
static bool s_was_paused;
static char *s_artist;
static char *s_track;
static GBitmap *s_up_bitmap;
static GBitmap *s_down_bitmap;
static GBitmap *s_select_bitmap;
static GColor s_curr_disk_color;
static GColor s_next_disk_color;
static GPoint s_curr_disk_pos;
static GPoint s_next_disk_pos;
static int32_t s_progress;
static int32_t s_length;
static int32_t s_progress_pixels;
static int32_t s_skip_value;
static int32_t s_arm_angle;
static int32_t s_old_arm_angle;
static int32_t s_animation_start_arm_angle;
static int32_t s_animation_end_arm_angle;
static struct tm s_last_time;
// TODO remove these variables, they are for demoing only
static int32_t s_trackcounter;
static char *artists[] = { "The Beatles", "Deadmau5", "Daniel Ingram" };
static char *tracks[] = { "Maxwell's Silver Hammer", "Strobe", "Smile" };
static int32_t length[] = { 207, 637, 203 }; // { 46, 5, 10 };

static void get_new_track(){
    s_trackcounter++;
    if(s_trackcounter >= 3) {
        s_trackcounter = 0;
    }
    s_artist = artists[s_trackcounter];
    s_track = tracks[s_trackcounter];
    s_length = length[s_trackcounter];
    s_progress = 0;
}
static void get_last_track(){
    s_trackcounter--;
    if(s_trackcounter < 0) {
        s_trackcounter = 2;
    }
    s_artist = artists[s_trackcounter];
    s_track = tracks[s_trackcounter];
    s_length = length[s_trackcounter];
    s_progress = 0;
}

static void music_tick(struct tm *tick_time, TimeUnits tick_units) {
    if((tick_units & SECOND_UNIT) != 0 && !s_is_paused) {
        if(s_progress >= s_length) {
            get_new_track();
            skip_track(SKIP_DIRECTION_NEXT);
            s_progress_pixels = 0;
        } else {
            s_progress++;
            s_progress_pixels = s_progress * (PROGRESS_PIXELS_MAX - 1) / s_length;
            int32_t new_arm_angle = ARM_START_ANGLE + s_progress * (ARM_END_ANGLE - ARM_START_ANGLE) / s_length;
            if(new_arm_angle != s_arm_angle){
                animation_arm(new_arm_angle, ARM_SKIP_SPEED);
            } else {
                layer_mark_dirty(s_main_layer);
            }
        }
    }
    if((tick_units & MINUTE_UNIT) != 0) {
        // Store time for status bar
        memcpy(&s_last_time, tick_time, sizeof(struct tm));
        layer_mark_dirty(s_main_layer);
    }
}

static void implementation_record_setup(Animation *animation) {
    s_animating_disk_change = true;
    // TODO set next_disk_color based on title + artist,
    // to let each track have the same color between sessions
    // Avoid color black and white (Changing the stroke color would be an alternative)
    s_next_disk_color =  (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000)  };

    // Set the initial positions for our two animated records
    if(s_skip_value > 0){
        s_next_disk_pos = GPoint(RECORD_CENTER_X + RECORD_SWOOSH_DISTANCE, RECORD_CENTER_Y);
    } else {
        s_next_disk_pos = GPoint(RECORD_CENTER_X - RECORD_SWOOSH_DISTANCE, RECORD_CENTER_Y);
    }
}

static void implementation_record_update(Animation *animation,
                                  const AnimationProgress distance_normalized) {
    if(s_skip_value > 0) {
        s_curr_disk_pos = GPoint(ANIM_LERP(s_curr_disk_pos.x, RECORD_CENTER_X - RECORD_SWOOSH_DISTANCE, distance_normalized), RECORD_CENTER_Y);
    } else {
        s_curr_disk_pos = GPoint(ANIM_LERP(s_curr_disk_pos.x, RECORD_CENTER_X + RECORD_SWOOSH_DISTANCE, distance_normalized), RECORD_CENTER_Y);
    }
    s_next_disk_pos =     GPoint(ANIM_LERP(s_next_disk_pos.x, RECORD_CENTER_X, distance_normalized),                          RECORD_CENTER_Y);
    layer_mark_dirty(s_main_layer);
}

static void implementation_record_teardown(Animation *animation) {
    // Now that the animation is done, our next disk becomes the current one
    s_curr_disk_color = s_next_disk_color;
    s_curr_disk_pos = s_next_disk_pos;
    // We successfully skipped, reduce the skip value
    if (s_skip_value > 0) {
        s_skip_value--;
    } else if (s_skip_value < 0) {
        s_skip_value++;
    }
    if(s_skip_value != 0) {
        // There's more skipping to be done, just leave the arm and continue swapping
        animation_record();
    } else {   
        // We are done, move the arm to the start position
        // TODO actually dont when paused
        s_animating_disk_change = false;
        if(!s_is_paused || !s_was_paused) {
            if(!s_was_paused){
                s_is_paused = false;
            }
            s_animating_arm_change = true;
            animation_arm(ARM_START_ANGLE, ARM_QUICK_SPEED);
        }
    }
}

static const AnimationImplementation implementation_record = {
    .setup = implementation_record_setup,
    .update = implementation_record_update,
    .teardown = implementation_record_teardown
};

static void animation_record() {
    animation_schedule(s_animation_record_ptr);
}

static void implementation_arm_setup(Animation *animation) {
    s_animation_start_arm_angle = s_arm_angle;
    s_animating_arm_change = true;
}

static void implementation_arm_update(Animation *animation,
                                      const AnimationProgress distance_normalized) {
    s_arm_angle = ANIM_LERP(s_animation_start_arm_angle, s_animation_end_arm_angle, distance_normalized);
    layer_mark_dirty(s_main_layer);
}

static void implementation_arm_teardown(Animation *animation) {
    s_animating_arm_change = false;
    if(s_skip_value != 0) {
        if(s_arm_angle != ARM_HOME_ANGLE) {
            // Skip value is not 0, so it means we are still skipping,
            // we are not at home, so quickly move the arm to make room
            animation_arm(ARM_HOME_ANGLE, ARM_QUICK_SPEED);
        } else {
            // If we were just back on our trip back home, animate the record change
            animation_record();
        }
    }
}

static const AnimationImplementation implementation_arm = {
    .setup = implementation_arm_setup,
    .update = implementation_arm_update,
    .teardown = implementation_arm_teardown
};

static void animation_arm(int32_t angle, uint32_t duration_ms) {
    // TODO Instead of taking a fixed duration set a duration based on the distance
    s_animation_end_arm_angle = angle;
    animation_set_duration(s_animation_arm_ptr, duration_ms);
    animation_schedule(s_animation_arm_ptr);
}

static void skip_track(int32_t direction) {
    // Only start the animation when we are not already animating
    s_was_paused = s_is_paused;
    s_is_paused = true;
    s_progress_pixels = 0;
    if(s_skip_value == 0 && !s_animating_disk_change && !s_animating_arm_change) {
        animation_arm(ARM_HOME_ANGLE, ARM_SKIP_SPEED);
    }
    s_skip_value += direction;
    // TODO REMOVE DEMO
    if(direction == SKIP_DIRECTION_NEXT)
        get_new_track();
    else
        get_last_track();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO Switch between volume and skipping
    skip_track(SKIP_DIRECTION_PREV);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO Switch between volume and skipping
    skip_track(SKIP_DIRECTION_NEXT);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO Toggle between toggling volume && skipping and play pause
    // When paused, move arm to side
    s_is_paused = !s_is_paused;
    if(s_is_paused) {
        tick_timer_service_subscribe(MINUTE_UNIT, music_tick);
        animation_arm(ARM_HOME_ANGLE, ARM_SKIP_SPEED);
    } else {
        tick_timer_service_subscribe(SECOND_UNIT, music_tick);
    }
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

void destroy_paths() {
    n_gpath_destroy(s_arm_base_path_ptr);
    n_gpath_destroy(s_arm_1_path_ptr);
    n_gpath_destroy(s_arm_2_path_ptr);
    n_gpath_destroy(s_arm_head_path_ptr);
}

void draw_arm(GContext *ctx, int32_t angle) {
    if(angle != s_old_arm_angle) {
        // Only recalculate the points if the angle has changed
        destroy_paths();
        s_arm_base_path_ptr = n_gpath_create(&ARM_BASE_PATH_INFO);
        s_arm_1_path_ptr    = n_gpath_create(&ARM_1_PATH_INFO);
        s_arm_2_path_ptr    = n_gpath_create(&ARM_2_PATH_INFO);
        s_arm_head_path_ptr = n_gpath_create(&ARM_HEAD_PATH_INFO);
        n_gpath_move_to(s_arm_base_path_ptr, ARM_OFFSET);
        n_gpath_move_to(s_arm_1_path_ptr,    ARM_OFFSET);
        n_gpath_move_to(s_arm_2_path_ptr,    ARM_OFFSET);
        n_gpath_move_to(s_arm_head_path_ptr, ARM_OFFSET);
        int32_t calculated_angle = TRIG_MAX_ANGLE / 360 * (angle + ARM_OFFSET_ANGLE);
        n_gpath_rotate_to(s_arm_base_path_ptr, calculated_angle);
        n_gpath_rotate_to(s_arm_1_path_ptr,    calculated_angle);
        n_gpath_rotate_to(s_arm_2_path_ptr,    calculated_angle);
        n_gpath_rotate_to(s_arm_head_path_ptr, calculated_angle);
        s_old_arm_angle = s_arm_angle;
    }
    n_graphics_context_set_stroke_caps(ctx, false);
    n_graphics_context_set_stroke_width(ctx, 5);
    n_gpath_draw(ctx, s_arm_base_path_ptr);
    n_graphics_context_set_stroke_width(ctx, 1);
    n_gpath_draw(ctx, s_arm_1_path_ptr);
    n_gpath_draw(ctx, s_arm_2_path_ptr);
    n_graphics_context_set_stroke_width(ctx, 4);    
    n_gpath_draw(ctx, s_arm_head_path_ptr);
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
    char time_string[8] = "";
    strftime(time_string, 8, "%R", &s_last_time);

    char progress_string[6] = "";
    // TODO display tracks over 59:59s long differently
    snprintf(progress_string, 6, "%d:%02d", s_progress / 60, s_progress % 60);

    char length_string[6] = "";
    snprintf(length_string, 6, "%d:%02d", s_length / 60, s_length % 60);

    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, s_artist,    fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(4, 108, 105, 10),                            GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, s_track,     fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(4, 117, 110, 50),                            n_GTextOverflowModeWordWrap,//TODO remove 'n_' prefix
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, progress_string,  fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect( 4, 84, (144 - 30) / 2 + 4, 10),             GTextOverflowModeTrailingEllipsis, 
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, length_string,    fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect((144 - 30) / 2, 84, (144 - 30) / 2 - 4, 10), GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentRight, NULL);
    graphics_draw_text(ctx, time_string, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0, 0, 113, 10),                              GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
    
    if (s_animating_disk_change) {
        draw_arm(ctx, s_arm_angle);
        draw_record(ctx, s_next_disk_pos, s_next_disk_color);
    }
    draw_record(ctx, s_curr_disk_pos, s_curr_disk_color);

    if(!s_animating_disk_change) {
        draw_arm(ctx, s_arm_angle);
    }
    // Draw bar
    graphics_fill_rect(ctx, GRect(4, 102, PROGRESS_PIXELS_MAX, 6), 1, n_GCornersAll);
    graphics_context_set_fill_color(ctx, s_curr_disk_color);
    graphics_fill_rect(ctx, GRect(5, 103, s_progress_pixels, 4), 0, GCornerNone);
}

static void music_window_load(Window *window) {
    srand(50);
    // srand(time(NULL)); TODO time seed
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    s_curr_disk_color = GColorWindsorTan;
    s_curr_disk_pos = RECORD_CENTER;
    s_progress_pixels = 25;
    s_trackcounter = 2;
    get_new_track();
    s_progress = 44;
    s_arm_angle = ARM_HOME_ANGLE;
    s_old_arm_angle = -1;
    s_skip_value = 0;
    s_main_layer = layer_create(bounds);
    s_is_paused = false;
    layer_add_child(window_layer, s_main_layer);

    s_animation_record_ptr = animation_create();
    animation_set_duration(s_animation_record_ptr, RECORD_SWOOSH_SPEED);
    animation_set_implementation(s_animation_record_ptr, &implementation_record);
    //animation_set_curve(s_animation_record_ptr, AnimationCurveEaseInOut);
    s_animation_arm_ptr = animation_create();
    animation_set_implementation(s_animation_arm_ptr, &implementation_arm);
    //animation_set_curve(s_animation_arm_ptr, AnimationCurveEaseInOut);
    
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
    tick_timer_service_subscribe(SECOND_UNIT, music_tick);
}

static void music_window_unload(Window *window) {
    action_bar_layer_destroy(s_action_bar);
    gbitmap_destroy(s_up_bitmap);
    gbitmap_destroy(s_down_bitmap);
    gbitmap_destroy(s_select_bitmap);
    animation_destroy(s_animation_record_ptr);
    animation_destroy(s_animation_arm_ptr);
    destroy_paths();
    tick_timer_service_unsubscribe();
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
