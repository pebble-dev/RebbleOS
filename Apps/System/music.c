/* music.c
 * Music player
 * RebbleOS
 * 
 * Music player to view & control the music playing on the connected device.
 * 
 * Special feature is an animated record player.
 * The arm is drawn using GPaths (defined by pathinfo_arm_base, pathinfo_1_base, pathinfo_2_base pathinfo_head_base).
 * The record is drawn simply using circles.
 * 
 * ANIMATION - GENERAL
 * When a track changes, we move the arm out of the way of the record, swap it, and move it back.
 * When quickly swapping, the arm stays at its home position so we can quickly move the records.
 * The logic for deciding what needs to be animated next is done in the record & arm animation teardown functions.
 * 
 * When stopping & continuing the arm animates between its home position and the current progress of the song on the record.
 * 
 * When skipping songs, s_skip_value keeps track of the amount of skipped songs.
 * We do this, so when you quickly skip songs the changed disks match up with amount of skipped songs.
 * When 3, it means that we need to animate the disk skipping forward 3 times.
 * When it's -1, it means that we need to skip it backward once.
 * 
 * What could be a problem in the future:
 * I envision generating the records color based of the title and artist of the song,
 * so when skipping songs quickly we also need to keep track of the color for each disk.
 * At the moment, it's just random.
 * 
 * ANIMATION - ARM
 * For the arms animation there are multiple variables.
 * The arm has defined angles, being home, start and end.
 * The home position is its resting position, the other two are the start and end positions of the head on the record.
 * The arm angle is tracked by two variables, s_arm_angle & s_old_arm_angle.
 * s_old_arm_angle is used to see if the arm's rotation needs to be recalculated to see when it changes.
 * 
 * ANIMATION - RECORD
 * The record too, has multiple variables.
 * RECORD_SWOOSH_DISTANCE defines the skipping animation travel distance.
 * For the animation, we keep track of the two disks color and position.
 * 
 * TIME PROGRESS
 * Keeping track of a tracks progress is done using the tick timer service.
 * This could be switched out with a timer for greater accuracy.
 * 
 * Author: Chris Multhaupt <chris.multhaupt@gmail.com>.
 */

#include "rebbleos.h"
#include "music.h"
#include "property_animation.h"

#define LERP(a, b)  ((a) + ((b) - (a)) * distance_normalized / ANIMATION_NORMALIZED_MAX)
#define RECORD_CENTER_X 56
#define RECORD_CENTER_Y 57
#define RECORD_CENTER GPoint(RECORD_CENTER_X, RECORD_CENTER_Y)
#define RECORD_SWOOSH_DISTANCE 130
#define RECORD_SWOOSH_SPEED 250
#define ARM_OFFSET_X 17
#define ARM_OFFSET_Y 77
#define ARM_OFFSET GPoint(ARM_OFFSET_X, ARM_OFFSET_Y)
#define ARM_OFFSET_ANGLE -31
#define ARM_HOME_ANGLE 30
#define ARM_SKIP_SPEED 200
#define ARM_QUICK_SPEED 100
#define SKIP_DIRECTION_PREV -1
#define SKIP_DIRECTION_NEXT 1
#define PROGRESS_PIXELS_MAX 105

static Window *s_music_main_window;
static Layer *s_music_main_layer;
static ActionBarLayer *s_music_action_bar;

static n_GPath *s_arm_base_path_ptr;
static n_GPath *s_arm_1_path_ptr;
static n_GPath *s_arm_2_path_ptr;
static n_GPath *s_arm_head_path_ptr;

static n_GPathInfo pathinfo_arm_base = {
    .num_points = 2,
    .points = (n_GPoint[]) {{0, -3}, {0, 3}}
};
static n_GPathInfo pathinfo_arm_1 = {
    .num_points = 2,
    .points = (n_GPoint[]) {{0, -3}, {0, -3-19}}
};
static n_GPathInfo pathinfo_arm_2 = {
    .num_points = 2,
    .points = (n_GPoint[]) {{0, -3-19}, {19, -3-19-15}}
};
static n_GPathInfo pathinfo_arm_head = {
    .num_points = 2,
    .points = (n_GPoint[]) {{19, -3-19-14}, {19+9, -3-19-15-6}}
};

static Animation *s_animation_record_ptr;
static Animation *_s_animation_arm_ptr;
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

static void _skip_track(int32_t direction);
static void _setup_music_animation_arm(int32_t angle, uint32_t duration_ms);
static void _setup_music_animation_record();
static void _music_deinit(void);

uint32_t one_at_a_time_hash(const uint8_t* key, size_t length) {
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

n_GColor8 songColor(unsigned char *str)
{
	uint32_t hash = one_at_a_time_hash(str,strlen(str));
	    APP_LOG("music", APP_LOG_LEVEL_DEBUG, "song hash:%lu",hash);

	return GColorFromRGB(hash, hash >> 8, hash >> 16);
}

static void _get_new_track() {
    s_trackcounter++;
    if (s_trackcounter >= 3) {
        s_trackcounter = 0;
    }
    s_artist = artists[s_trackcounter];
    s_track = tracks[s_trackcounter];
    s_length = length[s_trackcounter];
    s_progress = 0;
}
static void _get_last_track() {
    s_trackcounter--;
    if (s_trackcounter < 0) {
        s_trackcounter = 2;
    }
    s_artist = artists[s_trackcounter];
    s_track = tracks[s_trackcounter];
    s_length = length[s_trackcounter];
    s_progress = 0;
}

static void _music_tick(struct tm *tick_time, TimeUnits tick_units) {
    if ((tick_units & SECOND_UNIT) && !s_is_paused) {
        if (s_progress >= s_length) {
            _get_new_track();
            _skip_track(SKIP_DIRECTION_NEXT);
            s_progress_pixels = 0;
        } else {
            s_progress++;
            s_progress_pixels = s_progress * (PROGRESS_PIXELS_MAX - 1) / s_length;

            if (s_arm_angle != ARM_HOME_ANGLE) {
                _setup_music_animation_arm(ARM_HOME_ANGLE, ARM_SKIP_SPEED);
            } else {
                layer_mark_dirty(s_music_main_layer);
            }
        }
    }
    if ((tick_units & MINUTE_UNIT) != 0) {
        // Store time for status bar
        memcpy(&s_last_time, tick_time, sizeof(struct tm));
        layer_mark_dirty(s_music_main_layer);
    }
}

/*******************
 * Record animation: swaps disks from left and right.
 * Triggered by _implementation_arm_teardown and itself,
 * when there are still tracks to skip.
 */

static void _implementation_record_setup(Animation *animation) {
    s_animating_disk_change = true;
    // TODO set next_disk_color based on title + artist,
    // to let each track have the same color between sessions
    // Avoid color black and white (Changing the stroke color would be an alternative)
    s_next_disk_color =  songColor(s_track) ;

    // Set the initial positions for our two animated records
    if (s_skip_value > 0) {
        s_next_disk_pos = GPoint(RECORD_CENTER_X + RECORD_SWOOSH_DISTANCE, RECORD_CENTER_Y);
    } else {
        s_next_disk_pos = GPoint(RECORD_CENTER_X - RECORD_SWOOSH_DISTANCE, RECORD_CENTER_Y);
    }
}

static void _implementation_record_update(Animation *animation,
                                  const AnimationProgress distance_normalized) {
    if (s_skip_value > 0) {
        s_curr_disk_pos = GPoint(ANIM_LERP(s_curr_disk_pos.x, RECORD_CENTER_X - RECORD_SWOOSH_DISTANCE, distance_normalized), RECORD_CENTER_Y);
    } else {
        s_curr_disk_pos = GPoint(ANIM_LERP(s_curr_disk_pos.x, RECORD_CENTER_X + RECORD_SWOOSH_DISTANCE, distance_normalized), RECORD_CENTER_Y);
    }
    s_next_disk_pos =     GPoint(ANIM_LERP(s_next_disk_pos.x, RECORD_CENTER_X, distance_normalized),                          RECORD_CENTER_Y);
    layer_mark_dirty(s_music_main_layer);
}

static void _implementation_record_teardown(Animation *animation) {
    // Now that the animation is done, our next disk becomes the current one
    s_curr_disk_color = s_next_disk_color;
    s_curr_disk_pos = s_next_disk_pos;
    // We successfully skipped, reduce the skip value
    if (s_skip_value > 0) {
        s_skip_value--;
    } else if (s_skip_value < 0) {
        s_skip_value++;
    }
    if (s_skip_value != 0) {
        // There's more skipping to be done, just leave the arm and continue swapping
        _setup_music_animation_record();
    } else {   
        // We are done, move the arm to the start position
        s_animating_disk_change = false;
        if (!s_was_paused) {
            s_is_paused = false;
            s_animating_arm_change = true;
            _setup_music_animation_arm(ARM_HOME_ANGLE, ARM_QUICK_SPEED);
        }
    }
}

static const AnimationImplementation implementation_record = {
    .setup = _implementation_record_setup,
    .update = _implementation_record_update,
    .teardown = _implementation_record_teardown
};

static void _setup_music_animation_record() {
    animation_schedule(s_animation_record_ptr);
}

/*******************
 * Arm animation: moves in and out.
 * Triggered by skipping songs, play & pause. 
 * Has two speeds ARM_SKIP_SPEED and ARM_QUICK_SPEED.
 */

static void _implementation_arm_setup(Animation *animation) {
    s_animation_start_arm_angle = s_arm_angle;
    s_animating_arm_change = true;
}

static void _implementation_arm_update(Animation *animation,
                                      const AnimationProgress distance_normalized) {
    s_arm_angle = ANIM_LERP(s_animation_start_arm_angle, s_animation_end_arm_angle, distance_normalized);
    layer_mark_dirty(s_music_main_layer);
}

static void _implementation_arm_teardown(Animation *animation) {
    s_animating_arm_change = false;
    if (!s_skip_value)
        return;
    if (s_arm_angle != ARM_HOME_ANGLE) {
        // Skip value is not 0, so it means we are still skipping,
        // we are not at home, so quickly move the arm to make room
        _setup_music_animation_arm(ARM_HOME_ANGLE, ARM_QUICK_SPEED);
    } else {
        // If we were just back on our trip back home, animate the record change
        _setup_music_animation_record();
    }
}

static const AnimationImplementation implementation_arm = {
    .setup = _implementation_arm_setup,
    .update = _implementation_arm_update,
    .teardown = _implementation_arm_teardown
};

static void _setup_music_animation_arm(int32_t angle, uint32_t duration_ms) {
    // TODO Instead of taking a fixed duration set a duration based on the distance
    s_animation_end_arm_angle = angle;
    animation_set_duration(_s_animation_arm_ptr, duration_ms);
    animation_schedule(_s_animation_arm_ptr);
}

static void _skip_track(int32_t direction) {
    // Only start the animation when we are not already animating
    if (!s_animating_arm_change && !s_animating_disk_change)
        s_was_paused = s_is_paused;
    s_is_paused = true;
    s_progress_pixels = 0;
    if (s_skip_value == 0 && !s_animating_disk_change && !s_animating_arm_change) {

      _setup_music_animation_arm(ARM_HOME_ANGLE, ARM_SKIP_SPEED);
    }
    s_skip_value += direction;
    // TODO REMOVE DEMO
    if (direction == SKIP_DIRECTION_NEXT)
        _get_new_track();
    else
        _get_last_track();
}

/*******************
 * Click handles: handle play controls, like skipping and volume.
 * Also has a pop handler for exiting the app
 */

static void _pop_notification_click_handler(ClickRecognizerRef recognizer, void *context)
{
    _music_deinit();
    window_stack_pop(true);
    appmanager_app_start("System");
}

static void _up_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO Switch between volume and skipping
    _skip_track(SKIP_DIRECTION_PREV);
}

static void _down_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO Switch between volume and skipping
    _skip_track(SKIP_DIRECTION_NEXT);
}

static void _select_click_handler(ClickRecognizerRef recognizer, void *context) {
    // TODO Toggle between toggling volume && skipping and play pause
    // When paused, move arm to side
    s_is_paused = !s_is_paused;
    if (s_is_paused) {
        tick_timer_service_subscribe(MINUTE_UNIT, _music_tick);
        _setup_music_animation_arm(ARM_HOME_ANGLE, ARM_SKIP_SPEED);
    } else {
        tick_timer_service_subscribe(SECOND_UNIT, _music_tick);
        _setup_music_animation_arm(ARM_HOME_ANGLE, ARM_SKIP_SPEED);
    }
}

static void _click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) _up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) _down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) _select_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler) _pop_notification_click_handler);

    window_set_click_context(BUTTON_ID_UP, s_music_action_bar);
    window_set_click_context(BUTTON_ID_DOWN, s_music_action_bar);
    window_set_click_context(BUTTON_ID_SELECT, s_music_action_bar);
}

/*******************
 * Draw routines: Draws the record, arm and the track info.
 */

static void _draw_record(GContext *ctx, GPoint record_center_point, GColor record_color) {
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

static void _destroy_paths() {
    n_gpath_destroy(s_arm_base_path_ptr);
    n_gpath_destroy(s_arm_1_path_ptr);
    n_gpath_destroy(s_arm_2_path_ptr);
    n_gpath_destroy(s_arm_head_path_ptr);
}

static void _draw_arm(GContext *ctx, int32_t angle) {
    if (angle != s_old_arm_angle) {
        // Only recalculate the points if the angle has changed
        _destroy_paths();
        s_arm_base_path_ptr = n_gpath_create(&pathinfo_arm_base);
        s_arm_1_path_ptr    = n_gpath_create(&pathinfo_arm_1);
        s_arm_2_path_ptr    = n_gpath_create(&pathinfo_arm_2);
        s_arm_head_path_ptr = n_gpath_create(&pathinfo_arm_head);
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

static void _main_layer_update_proc(Layer *layer, GContext *ctx) {
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
    snprintf(progress_string, 6, "%ld:%02ld", s_progress / 60, s_progress % 60);

    char length_string[6] = "";
    snprintf(length_string, 6, "%ld:%02ld", s_length / 60, s_length % 60);

    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, s_artist,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(4, 108, 105, 10),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, s_track,
                       fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(4, 117, 110, 50),
                       n_GTextOverflowModeWordWrap,//TODO remove 'n_' prefix
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, progress_string,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect( 4, 84, (144 - 30) / 2 + 4, 10),
                       GTextOverflowModeTrailingEllipsis, 
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, length_string,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect((144 - 30) / 2, 84, (144 - 30) / 2 - 4, 10),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentRight, NULL);
    graphics_draw_text(ctx, time_string,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(0, 0, 113, 10),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
    
    if (s_animating_disk_change) {
        _draw_arm(ctx, s_arm_angle);
        _draw_record(ctx, s_next_disk_pos, s_next_disk_color);
    }
    _draw_record(ctx, s_curr_disk_pos, s_curr_disk_color);

    if (!s_animating_disk_change) {
        _draw_arm(ctx, s_arm_angle);
    }
    // Draw bar
    graphics_fill_rect(ctx, GRect(4, 102, PROGRESS_PIXELS_MAX, 6), 1, n_GCornersAll);
    graphics_context_set_fill_color(ctx, s_curr_disk_color);
    graphics_fill_rect(ctx, GRect(5, 103, s_progress_pixels, 4), 0, GCornerNone);
}

/*******************
 * Music app init & deinit
 */

static void _music_window_load(Window *window) {
    srand(50);
    // srand(time(NULL)); TODO time seed
    Layer *window_layer = window_get_root_layer(s_music_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    s_progress_pixels = 25;
    s_trackcounter = 2;
    _get_new_track();
    s_progress = 44;
    s_arm_angle = ARM_HOME_ANGLE;
    s_old_arm_angle = -1;
    s_skip_value = 0;
    s_music_main_layer = layer_create(bounds);
    s_is_paused = false;
    s_curr_disk_color = songColor(s_track);
        s_curr_disk_pos = RECORD_CENTER;
    layer_add_child(window_layer, s_music_main_layer);
    s_animating_arm_change = false;
    s_animating_disk_change = false;

    s_animation_record_ptr = animation_create();
    animation_set_duration(s_animation_record_ptr, RECORD_SWOOSH_SPEED);
    animation_set_implementation(s_animation_record_ptr, &implementation_record);
    //animation_set_curve(s_animation_record_ptr, AnimationCurveEaseInOut);
    _s_animation_arm_ptr = animation_create();
    animation_set_implementation(_s_animation_arm_ptr, &implementation_arm);
    //animation_set_curve(_s_animation_arm_ptr, AnimationCurveEaseInOut);
    
    s_music_action_bar = action_bar_layer_create();
    s_up_bitmap = gbitmap_create_with_resource(21); //XXX
    s_down_bitmap = gbitmap_create_with_resource(25); //XXX
    s_select_bitmap = gbitmap_create_with_resource(22); //XXX
    action_bar_layer_set_icon(s_music_action_bar, BUTTON_ID_UP, s_up_bitmap);
    action_bar_layer_set_icon(s_music_action_bar, BUTTON_ID_SELECT, s_down_bitmap);
    action_bar_layer_set_icon(s_music_action_bar, BUTTON_ID_DOWN, s_select_bitmap);
    action_bar_layer_set_click_config_provider(s_music_action_bar, _click_config_provider);
    action_bar_layer_add_to_window(s_music_action_bar, s_music_main_window);
    action_bar_layer_set_background_color(s_music_action_bar, GColorLightGray);
    
    layer_set_update_proc(s_music_main_layer, _main_layer_update_proc);
    tick_timer_service_subscribe(SECOND_UNIT, _music_tick);
}

static void _music_window_unload(Window *window) {
    action_bar_layer_destroy(s_music_action_bar);
    gbitmap_destroy(s_up_bitmap);
    gbitmap_destroy(s_down_bitmap);
    gbitmap_destroy(s_select_bitmap);
    animation_destroy(s_animation_record_ptr);
    animation_destroy(_s_animation_arm_ptr);
    _destroy_paths();
    tick_timer_service_unsubscribe();
}

static void _music_init(void) {
    s_music_main_window = window_create();
    window_set_window_handlers(s_music_main_window, (WindowHandlers) {
        .load = _music_window_load,
        .unload = _music_window_unload,
    });
    window_stack_push(s_music_main_window, true);
}

static void _music_deinit(void) {
    window_destroy(s_music_main_window);
}

void music_main(void) {
    _music_init();
    app_event_loop();
    _music_deinit();
}
