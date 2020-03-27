/* radio_button_window.c
 *
 * Radio button menu component.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "rebbleos.h"
#include "radio_button_window.h"

static int s_current_selection = 0;

int radio_selection_labels_num = 4;

struct RadiobuttonWindow
{
  Window window;
  MenuLayer *menu_layer;

  GColor radiobutton_foreground_color;
  GColor radiobutton_background_color;
  char radiobutton_selections[10][14];
  int selection_count;
};

void radiobutton_add_selection(RadiobuttonWindow *radio_star, char *selection_label) {
  strcpy(radio_star->radiobutton_selections[radio_star->selection_count], selection_label);

  radio_star->selection_count++;
}

void set_radiobutton_selection_colors(RadiobuttonWindow *radio_star, GColor background, GColor foreground) {
  radio_star->radiobutton_background_color = background;
  radio_star->radiobutton_foreground_color = foreground;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return radio_selection_labels_num + 1;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  
  RadiobuttonWindow *radio_star = (RadiobuttonWindow *) context;
  
  if(cell_index->row == radio_selection_labels_num) {
    // This is the submit item
    menu_cell_basic_draw(ctx, cell_layer, "Submit", NULL, NULL);
  } else {
    // This is a choice item
    static char s_buff[16];
    snprintf(s_buff, sizeof(s_buff), radio_star->radiobutton_selections[(int)cell_index->row]);
    menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);

    GRect bounds = layer_get_bounds(cell_layer);
    GPoint p = GPoint(bounds.size.w - (3 * RADIO_BUTTON_WINDOW_RADIO_RADIUS), (bounds.size.h / 2));

    // Selected?
    if(menu_cell_layer_is_highlighted(cell_layer)) {
      graphics_context_set_stroke_color(ctx, GColorWhite);
      graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
      graphics_context_set_fill_color(ctx, GColorBlack);
    }

    // Draw radio filled/empty
    graphics_draw_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS);
    if(cell_index->row == s_current_selection) {
      // This is the selection
      graphics_fill_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS - 3);
    }
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ? 
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    44);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if(cell_index->row == radio_selection_labels_num) {
    // Do something with user choice
    //APP_LOG(APP_LOG_LEVEL_INFO, "Submitted choice %d", s_current_selection);
    window_stack_pop(true);
  } else {
    // Change selection
    s_current_selection = cell_index->row;
    menu_layer_reload_data(menu_layer);
  }
}

static void radiobutton_window_load(Window *window) {

  RadiobuttonWindow *radio_star = (RadiobuttonWindow *)window;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  radio_star->menu_layer = menu_layer_create(bounds);

  menu_layer_set_click_config_onto_window(radio_star->menu_layer, window);
  menu_layer_set_callbacks(radio_star->menu_layer, radio_star, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(radio_star->menu_layer));
}

static void radiobutton_window_unload(Window *window) {
  /*menu_layer_destroy(s_menu_layer);

  window_destroy(window);
  s_main_window = NULL;*/
}

void radiobutton_window_push(RadiobuttonWindow *radio_star) {
  window_stack_push(&radio_star->window, true);
}

RadiobuttonWindow *radiobutton_window_create(uint16_t max_items)
{
  RadiobuttonWindow *radio_star = (RadiobuttonWindow *)app_calloc(1, sizeof(RadiobuttonWindow) + (sizeof(char[10]) * max_items));

  window_ctor(&radio_star->window);
  radio_star->window.user_data = radio_star;
    window_set_window_handlers(&radio_star->window, (WindowHandlers) {
        .load = radiobutton_window_load,
        .unload = radiobutton_window_unload,
    });

  radio_star->radiobutton_foreground_color = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack);
  radio_star->radiobutton_background_color = GColorWhite;
  radio_star->selection_count = 0;

  return radio_star;
}