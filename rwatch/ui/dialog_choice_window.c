/* dialog_choice_window.c
 *
 * Dialog choice component. Adapted from Pebble UI Examples.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "dialog_choice_window.h"
#include "action_bar_layer.h"
#include "platform_res.h"

struct DialogchoiceWindow
{
  Window window;
  TextLayer *s_label_layer;
  BitmapLayer *s_icon_layer;
  ActionBarLayer *s_action_bar_layer;

  char choice_window_message[10];
  GBitmap *s_icon_bitmap, *s_tick_bitmap, *s_cross_bitmap;
};

void dialogchoice_window_set_message(DialogchoiceWindow *dial, char dialogchoice_window_message) {
  strcpy(dial->choice_window_message, dialogchoice_window_message);
}


static void dial_window_load(Window *window) {

  DialogchoiceWindow *dial = (DialogchoiceWindow *)window;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  dial->s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SPEECH_BUBBLE);

  const GEdgeInsets icon_insets = {.top = 7, .right = 28, .bottom = 56, .left = 14};
  dial->s_icon_layer = bitmap_layer_create(grect_inset(bounds, icon_insets));
  bitmap_layer_set_bitmap(dial->s_icon_layer, dial->s_icon_bitmap);
  bitmap_layer_set_compositing_mode(dial->s_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(dial->s_icon_layer));

  const GEdgeInsets label_insets = {.top = 112, .right = ACTION_BAR_WIDTH, .left = ACTION_BAR_WIDTH / 2};
  dial->s_label_layer = text_layer_create(grect_inset(bounds, label_insets));
  text_layer_set_text(dial->s_label_layer, dial->choice_window_message);
  text_layer_set_background_color(dial->s_label_layer, GColorClear);
  text_layer_set_text_alignment(dial->s_label_layer, GTextAlignmentCenter);
  text_layer_set_font(dial->s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(dial->s_label_layer));

  dial->s_tick_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MUSIC_PLAY);
  dial->s_cross_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MUSIC_PAUSE);

  dial->s_action_bar_layer = action_bar_layer_create();
  action_bar_layer_set_icon(dial->s_action_bar_layer, BUTTON_ID_UP, dial->s_tick_bitmap);
  action_bar_layer_set_icon(dial->s_action_bar_layer, BUTTON_ID_DOWN, dial->s_cross_bitmap);
  action_bar_layer_add_to_window(dial->s_action_bar_layer, window);
}

static void dial_window_unload(Window *window) {

  DialogchoiceWindow *dial = (DialogchoiceWindow *)window;

  text_layer_destroy(dial->s_label_layer);
  action_bar_layer_destroy(dial->s_action_bar_layer);
  bitmap_layer_destroy(dial->s_icon_layer);

  gbitmap_destroy(dial->s_icon_bitmap);
  gbitmap_destroy(dial->s_tick_bitmap);
  gbitmap_destroy(dial->s_cross_bitmap);

  window_destroy(&dial->window);
  window = NULL;
}

void dialog_choice_window_push(DialogchoiceWindow *dial) {
  window_stack_push(&dial->window, true);
}

DialogchoiceWindow *dialogchoice_window_create() {
  DialogchoiceWindow *dial = (DialogchoiceWindow *)app_calloc(1, sizeof(DialogchoiceWindow));

  window_ctor(&dial->window);
  dial->window.user_data = dial;
    window_set_window_handlers(&dial->window, (WindowHandlers) {
        .load = dial_window_load,
        .unload = dial_window_unload,
    });

  window_set_background_color(&dial->window, PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite));

  return dial;  
}