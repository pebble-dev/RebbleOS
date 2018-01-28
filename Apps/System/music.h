#pragma once
/* music.h
 *
 * Music player
 *
 * RebbleOS
 * 
 * Author: Chris Multhaupt <chris.multhaupt@gmail.com>.
 */

#include "action_bar_layer.h"

#define BTN_SELECT_PRESS 1
#define BTN_BACK_PRESS   2
#define BTN_UP_PRESS     3
#define BTN_DOWN_PRESS   4

static void music_tick(struct tm *tick_time, TimeUnits tick_units);
static void implementation_record_setup(Animation *animation);
static void implementation_record_update(Animation *animation, 
                                  const AnimationProgress distance_normalized);
static void implementation_record_teardown(Animation *animation);
static void animation_record();
static void implementation_arm_setup(Animation *animation);
static void implementation_arm_update(Animation *animation, 
                                  const AnimationProgress distance_normalized);
static void implementation_arm_teardown(Animation *animation);
static void animation_arm(int32_t angle, uint32_t duration_ms);
static void skip_track(int32_t direction);
static void up_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
void draw_record(GContext *ctx, GPoint record_center_point, GColor record_color);
void destroy_paths();
void draw_arm(GContext *ctx, int32_t angle);
static void main_layer_update_proc(Layer *layer, GContext *ctx);
static void music_window_load(Window *window);
static void music_window_unload(Window *window);
void music_init(void);
void music_deinit(void);

void music_main(void);