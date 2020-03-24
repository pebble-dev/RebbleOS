/* checkbox_window.c
 *
 * Checkbox menu component.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "rebbleos.h"
#include "checkbox_window.h"
#include "platform_res.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;

GColor menu_background, menu_foreground;

static GBitmap *s_tick_black_bitmap, *s_tick_white_bitmap;

char checkbox_selection_labels[10][14];
int checkbox_selection_labels_num = 0;
static bool s_selections[checkbox_selection_labels_num];

void add_checkbox_selection(char *selection_label) {
  strcpy(checkbox_selection_labels[checkbox_selection_labels_num], selection_label);

  checkbox_selection_labels_num++;
}

void set_checkbox_selection_colors(GColor background, GColor foreground) {
  menu_background = background;
  menu_foreground = foreground;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return checkbox_selection_labels_num + 1;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if(cell_index->row == checkbox_selection_labels_num) {
    // Submit item
    menu_cell_basic_draw(ctx, cell_layer, "Submit", NULL, NULL);
  } else {
    // Choice item
    static char s_buff[16];
    snprintf(s_buff, sizeof(s_buff), checkbox_selection_labels[(int)cell_index->row]);
    menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);

    // Selected?
    GBitmap *ptr = s_tick_black_bitmap;
    if(menu_cell_layer_is_highlighted(cell_layer)) {
      graphics_context_set_stroke_color(ctx, GColorWhite);
      ptr = s_tick_white_bitmap;
    }

    GRect bounds = layer_get_bounds(cell_layer);
    GRect bitmap_bounds = gbitmap_get_bounds(ptr);

    // Draw checkbox
    GRect r = GRect(
      bounds.size.w - (2 * CHECKBOX_WINDOW_BOX_SIZE),
      (bounds.size.h / 2) - (CHECKBOX_WINDOW_BOX_SIZE / 2),
      CHECKBOX_WINDOW_BOX_SIZE, CHECKBOX_WINDOW_BOX_SIZE);
    graphics_draw_rect(ctx, r, 1, GCornerNone);
    if(s_selections[cell_index->row]) {
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_draw_bitmap_in_rect(ctx, ptr, GRect(r.origin.x, r.origin.y - 68, bitmap_bounds.size.w, bitmap_bounds.size.h));
    }
  }

}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    CHECKBOX_WINDOW_CELL_HEIGHT);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if(cell_index->row == checkbox_selection_labels_num) {
    // Do something with choices made
    for(int i = 0; i < checkbox_selection_labels_num; i++) {
      //APP_LOG(APP_LOG_LEVEL_INFO, "Option %d was %s", i, (s_selections[i] ? "selected" : "not selected"));
      APP_LOG("test",APP_LOG_LEVEL_DEBUG,"Option %d was %s", i, (s_selections[i] ? "selected" : "not selected"));
    }
    window_stack_pop(true);
  } else {
    // Check/uncheck
    int row = cell_index->row;
    s_selections[row] = !s_selections[row];
    menu_layer_reload_data(menu_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_tick_black_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ALARM_BELL_RINGING);
  s_tick_white_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ALARM_BELL);

  s_menu_layer = menu_layer_create(bounds);

  menu_layer_set_highlight_colors(s_menu_layer, menu_background, menu_foreground);

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
  });

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);

  gbitmap_destroy(s_tick_black_bitmap);
  gbitmap_destroy(s_tick_white_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

void checkbox_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}