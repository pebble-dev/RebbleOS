#include "pin_window.h"
#include "selection_layer.h"

struct PinWindow{
  Window *window;
  TextLayer *main_text, *sub_text;
  Layer *selection;
  GColor highlight_color;
  StatusBarLayer *status;
  PinWindowCallbacks callbacks;

  int pin_box_amount;
  int pin_max_value;

  PIN pin;
  char field_buffs[PIN_WINDOW_NUM_CELLS][2];
  int8_t field_selection;
};

static char* selection_handle_get_text(int index, void *context) {
  PinWindow *pin_window = (PinWindow*)context;
  snprintf(
    pin_window->field_buffs[index], 
    sizeof(pin_window->field_buffs[0]), "%d",
    (int)pin_window->pin.digits[index]
  );
  return pin_window->field_buffs[index];
}

static void selection_handle_complete(void *context) {
  PinWindow *pin_window = (PinWindow*)context;
  pin_window->callbacks.pin_complete(pin_window->pin, pin_window);
}

static void selection_handle_inc(int index, uint8_t clicks, void *context) {
  PinWindow *pin_window = (PinWindow*)context;
  pin_window->pin.digits[index]++;
  if(pin_window->pin.digits[index] > pin_window->pin_max_value) {
    pin_window->pin.digits[index] = 0;
  }
}

static void selection_handle_dec(int index, uint8_t clicks, void *context) {
  PinWindow *pin_window = (PinWindow*)context;
  pin_window->pin.digits[index]--;
  if(pin_window->pin.digits[index] < 0) {
    pin_window->pin.digits[index] = pin_window->pin_max_value;
  }
}

PinWindow* pin_window_create(PinWindowCallbacks callbacks) {
  PinWindow *pin_window = (PinWindow*)malloc(sizeof(PinWindow));
  if (pin_window) {
    pin_window->window = window_create();
    window_set_background_color(pin_window->window, GColorWhite);
    pin_window->callbacks = callbacks;
    if (pin_window->window) {
      pin_window->field_selection = 0;
      for(int i = 0; i < PIN_WINDOW_NUM_CELLS; i++) {
        pin_window->pin.digits[i] = 0;
      }
      
      // Get window parameters
      Layer *window_layer = window_get_root_layer(pin_window->window);
      GRect bounds = layer_get_bounds(window_layer);
      
      // Main TextLayer
      const GEdgeInsets main_text_insets = {.top = 30};
      pin_window->main_text = text_layer_create(grect_inset(bounds, main_text_insets));
      text_layer_set_text(pin_window->main_text, "PIN Required");
      text_layer_set_font(pin_window->main_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      text_layer_set_text_alignment(pin_window->main_text, GTextAlignmentCenter);
      layer_add_child(window_layer, text_layer_get_layer(pin_window->main_text));
      
      // Sub TextLayer
      const GEdgeInsets sub_text_insets = {.top = 115, .right = 5, .bottom = 10, .left = 5};
      pin_window->sub_text = text_layer_create(grect_inset(bounds, sub_text_insets));
      text_layer_set_text(pin_window->sub_text, "Enter your PIN to continue");
      text_layer_set_text_alignment(pin_window->sub_text, GTextAlignmentCenter);
      text_layer_set_font(pin_window->sub_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      layer_add_child(window_layer, text_layer_get_layer(pin_window->sub_text));
      
      // Create selection layer
      const GEdgeInsets selection_insets = GEdgeInsets(
        //(bounds.size.h - PIN_WINDOW_SIZE.h) / 2, 
        //(bounds.size.w - PIN_WINDOW_SIZE.w) / 2);
        (bounds.size.h - 34) / 2, 
        (bounds.size.w - 128) / 2);
      pin_window->selection = selection_layer_create(grect_inset(bounds, selection_insets), PIN_WINDOW_NUM_CELLS);
      for (int i = 0; i < PIN_WINDOW_NUM_CELLS; i++) {
        selection_layer_set_cell_width(pin_window->selection, i, 40);
      }
      selection_layer_set_cell_padding(pin_window->selection, 4);
      selection_layer_set_active_bg_color(pin_window->selection, GColorRed);
      selection_layer_set_inactive_bg_color(pin_window->selection, GColorDarkGray);
      selection_layer_set_click_config_onto_window(pin_window->selection, pin_window->window);
      selection_layer_set_callbacks(pin_window->selection, pin_window, (SelectionLayerCallbacks) {
        .get_cell_text = selection_handle_get_text,
        .complete = selection_handle_complete,
        .increment = selection_handle_inc,
        .decrement = selection_handle_dec,
      });
      layer_add_child(window_get_root_layer(pin_window->window), pin_window->selection);

      // Create status bar
      pin_window->status = status_bar_layer_create();
      //status_bar_layer_set_colors(pin_window->status, GColorClear, GColorBlack);
      status_bar_layer_set_colors(pin_window->status, GColorWhite, GColorBlack);
      layer_add_child(window_layer, status_bar_layer_get_layer(pin_window->status));
      return pin_window;
    }
  }

  //APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create PinWindow");
  return NULL;
}

void pin_window_destroy(PinWindow *pin_window) {
  if (pin_window) {
    status_bar_layer_destroy(pin_window->status);
    selection_layer_destroy(pin_window->selection);
    text_layer_destroy(pin_window->sub_text);
    text_layer_destroy(pin_window->main_text);
    free(pin_window);
    pin_window = NULL;
    return;
  }
}

void pin_window_push(PinWindow *pin_window, bool animated) {
  window_stack_push(pin_window->window, animated);
}

void pin_window_pop(PinWindow *pin_window, bool animated) {
  window_stack_remove(pin_window->window, animated);
}

bool pin_window_get_topmost_window(PinWindow *pin_window) {
  return window_stack_get_top_window() == pin_window->window;
}

void pin_window_set_highlight_color(PinWindow *pin_window, GColor color) {
  pin_window->highlight_color = color;
  selection_layer_set_active_bg_color(pin_window->selection, color);
}

void pin_window_set_pin_amount(PinWindow *pin_window, int pin_amount){
  pin_window->pin_box_amount = pin_amount;
}

void pin_window_set_pin_max_value(PinWindow *pin_window, int max_value){
  pin_window->pin_max_value = max_value;
}