/* vibes_test.c
 * routines for vibration testing
 * libRebbleOS
 *
 * Author: Author: Elliot Hawkins <elliotshawkins@gmail.com>.
 */
#include "display.h"
#include "systemapp.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "test_defs.h"
#include "vibes.h"

static Window *_main_window;
static TextLayer *_output_text_layer;

static const uint32_t vibe_durations[] = {250, 300, 500};
static const VibePattern custom_pattern = {
    .durations = vibe_durations,
    .num_segments = 3
};

static int _current_test = 0;
static int _current_tick = 0;

static void _vibes_test_short_vibe();
static void _vibes_test_long_vibe();
static void _vibes_test_double_vibe();
static void _vibes_test_custom_vibe();
static void _vibes_test_tick(struct tm *tick_time, TimeUnits tick_units);

bool vibes_test_init(Window *window)
{
    APP_LOG("test", APP_LOG_LEVEL_ERROR, "Init: Vibes Test");
    _main_window = window;
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    _output_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
    text_layer_set_text_alignment(_output_text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(_output_text_layer));
    text_layer_set_text(_output_text_layer, "Vibes Test");
    tick_timer_service_subscribe(SECOND_UNIT, _vibes_test_tick);

    return true;
}

bool vibes_test_exec(void)
{
    APP_LOG("test", APP_LOG_LEVEL_ERROR, "Exec: Vibes Test");

    //Reset test variables
    _current_test = 0;
    _current_tick = 0;

    return true;
}

bool vibes_test_deinit(void)
{
    APP_LOG("test", APP_LOG_LEVEL_ERROR, "De-Init: Vibes Test");
    tick_timer_service_unsubscribe();
    text_layer_destroy(_output_text_layer);

    if (_main_window != NULL){
        _main_window = NULL;
    }

    if (_output_text_layer != NULL){
        _output_text_layer = NULL;
    }

    return true;
}

static void _vibes_test_short_vibe(){
    switch (_current_tick){
        case 1:
            APP_LOG("test", APP_LOG_LEVEL_ERROR, "SHORT VIBE");
            text_layer_set_text(_output_text_layer, "Short Vibe");
            window_dirty(_main_window);
            break;
        case 2:
            vibes_short_pulse();
            _current_test++;
            _current_tick = 0;
            break;
    }
}

static void _vibes_test_long_vibe(){
    switch (_current_tick){
        case 1:
            APP_LOG("test", APP_LOG_LEVEL_ERROR, "LONG VIBE");
            text_layer_set_text(_output_text_layer, "Long Vibe");
            window_dirty(_main_window);
            break;
        case 2:
            vibes_long_pulse();
            _current_test++;
            _current_tick = 0;
            break;
    }
}

static void _vibes_test_double_vibe(){
    switch (_current_tick) {
        case 1:
            APP_LOG("test", APP_LOG_LEVEL_ERROR, "DOUBLE VIBE");
            text_layer_set_text(_output_text_layer, "Double Vibe");
            window_dirty(_main_window);
            break;
        case 2:
            vibes_double_pulse();
            _current_test++;
            _current_tick = 0;
            break;
    }
}

static void _vibes_test_custom_vibe(){
    switch (_current_tick) {
        case 1:
            APP_LOG("test", APP_LOG_LEVEL_ERROR, "CUSTOM VIBE");
            text_layer_set_text(_output_text_layer, "Custom Vibe");
            window_dirty(_main_window);
            break;
        case 2:
            vibes_enqueue_custom_pattern(custom_pattern);
            _current_test++;
            _current_tick = 0;
            break;
    }
}

void _vibes_test_tick(struct tm *tick_time, TimeUnits tick_units) {
    _current_tick++;

    switch (_current_test) {
        case 0:
            _vibes_test_short_vibe();
            break;
        case 1:
            _vibes_test_long_vibe();
            break;
        case 2:
            _vibes_test_double_vibe();
            break;
        case 3:
            _vibes_test_custom_vibe();
            break;
        default:
            test_complete(true);
    }
}
