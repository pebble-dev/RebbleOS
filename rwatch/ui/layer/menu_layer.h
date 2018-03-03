#pragma once
/* menu_layer.h
 *
 * MenuLayer component.
 * RebbleOS
 */


void menu_cell_basic_draw(GContext *ctx, const Layer *layer, const char *title,
                          const char *subtitle, GBitmap *icon);
// to support chalk main menu with a single function
void menu_cell_basic_draw_ex(GContext *ctx, GRect frame, const char *title,
                          const char *subtitle, GBitmap *icon, GTextAlignment align);
void menu_cell_title_draw(GContext *ctx, const Layer *layer, const char *title);
void menu_cell_basic_header_draw(GContext *ctx, const Layer *layer, const char *title);

//! Value to describe the reload behaviour for \ref MenuLayer.
//! @see \ref menu_layer_set_reload_behaviour
typedef enum {
    MenuLayerReloadBehaviourManual = 0, //!< The data is reloaded only on manual API calls (e.g. \ref menu_layer_set_callbacks).
    MenuLayerReloadBehaviourOnSelection, //!< The data is reloaded on every selection change (and manual API calls).
    MenuLayerReloadBehaviourOnRender //!< The data is reloaded before every rendering.
} MenuLayerReloadBehaviour;

typedef struct MenuIndex
{
  uint16_t section;
  uint16_t row;
} MenuIndex;

#define MenuIndex(section, row) ((MenuIndex){ (section), (row) })

int16_t menu_index_compare(const MenuIndex *a, const MenuIndex *b);

typedef struct MenuCellSpan
{
  int16_t x;
  int16_t y;
  int16_t h;
  int16_t sep;
  bool header;
  MenuIndex index;
} MenuCellSpan;

struct MenuLayer;

typedef uint16_t (*MenuLayerGetNumberOfSectionsCallback)(struct MenuLayer *menu_layer, void *context);

typedef uint16_t (*MenuLayerGetNumberOfRowsInSectionsCallback)(struct MenuLayer *menu_layer, uint16_t section_index,
                                                               void *context);

typedef int16_t (*MenuLayerGetCellHeightCallback)(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context);

typedef int16_t (*MenuLayerGetHeaderHeightCallback)(struct MenuLayer *menu_layer, uint16_t section_index,
                                                    void *context);

typedef int16_t (*MenuLayerGetSeparatorHeightCallback)(struct MenuLayer *menu_layer, MenuIndex *cell_index,
                                                       void *context);

typedef void (*MenuLayerDrawRowCallback)(GContext *ctx, const Layer *layer, MenuIndex *cell_index, void *context);

typedef void (*MenuLayerDrawHeaderCallback)(GContext *ctx, const Layer *layer, uint16_t section_index, void *context);

typedef void (*MenuLayerDrawBackgroundCallback)(GContext *ctx, const Layer *layer, bool highlight, void *context);

typedef void (*MenuLayerDrawSeparatorCallback)(GContext *ctx, const Layer *layer, MenuIndex *cell_index, void *context);

typedef void (*MenuLayerSelectCallback)(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context);

typedef void (*MenuLayerSelectionChangedCallback)(struct MenuLayer *menu_layer, MenuIndex *new_index,
                                                  MenuIndex *old_index, void *context);

typedef void (*MenuLayerSelectionWillChangeCallback)(struct MenuLayer *menu_layer, MenuIndex *new_index,
                                                     MenuIndex *old_index, void *context);

typedef struct MenuLayerCallbacks
{
  MenuLayerGetNumberOfSectionsCallback get_num_sections;
  MenuLayerGetNumberOfRowsInSectionsCallback get_num_rows;
  MenuLayerGetCellHeightCallback get_cell_height;
  MenuLayerGetHeaderHeightCallback get_header_height;
  MenuLayerDrawRowCallback draw_row;
  MenuLayerDrawHeaderCallback draw_header;
  MenuLayerSelectCallback select_click;
  MenuLayerSelectCallback select_long_click;
  MenuLayerSelectionChangedCallback selection_changed;
  MenuLayerGetSeparatorHeightCallback get_separator_height;
  MenuLayerDrawSeparatorCallback draw_separator;
  MenuLayerSelectionWillChangeCallback selection_will_change;
  MenuLayerDrawBackgroundCallback draw_background;
} MenuLayerCallbacks;

typedef struct MenuSection
{
  int16_t num_rows;
  int16_t cell_height;
  int16_t header_height;
  int16_t separator_height;
} MenuSection;

typedef struct MenuLayer
{
  Layer layer; // content layer - all items are drawn here
  ScrollLayer scroll_layer;
  struct MenuLayerCallbacks callbacks;
  ClickConfigProvider click_config_provider; // additional click provider for clients
  void *context;
  MenuLayerReloadBehaviour reload_behaviour;

  uint16_t column_count;
  size_t cells_count;
  MenuCellSpan *cells;
  MenuIndex selected;
  MenuIndex end_index;

  GColor bg_color;
  GColor bg_hi_color;
  GColor fg_color;
  GColor fg_hi_color;

  bool is_center_focus;
  bool is_bottom_padding_enabled;
  bool is_reload_scheduled;
} MenuLayer;

void menu_layer_ctor(MenuLayer *mlayer, GRect frame);
void menu_layer_dtor(MenuLayer *menu);

MenuLayer *menu_layer_create(GRect frame);

void menu_layer_destroy(MenuLayer *menu_layer);

Layer *menu_layer_get_layer(const MenuLayer *menu_layer);

ScrollLayer *menu_layer_get_scroll_layer(const MenuLayer *menu_layer);

void menu_layer_set_callbacks(MenuLayer *menu_layer, void *context, MenuLayerCallbacks callbacks);

void menu_layer_set_click_config_provider(MenuLayer *menu_layer, ClickConfigProvider provider);

void menu_layer_set_click_config_onto_window(MenuLayer *menu_layer, struct Window *window);

typedef enum
{
  MenuRowAlignNone,
  MenuRowAlignCenter,
  MenuRowAlignTop,
  MenuRowAlignBottom,
} MenuRowAlign;

void menu_layer_set_selected_next(MenuLayer *menu_layer, bool up, MenuRowAlign scroll_align,
                                  bool animated);

void menu_layer_set_selected_index(MenuLayer *menu_layer, MenuIndex index,
                                   MenuRowAlign scroll_align, bool animated);

MenuIndex menu_layer_get_selected_index(const MenuLayer *menu_layer);

void menu_layer_reload_data(MenuLayer *menu_layer);

bool menu_cell_layer_is_highlighted(const Layer *layer);

void menu_layer_set_normal_colors(MenuLayer *menu_layer, GColor background, GColor foreground);

void menu_layer_set_highlight_colors(MenuLayer *menu_layer, GColor background, GColor foreground);

void menu_layer_pad_bottom_enable(MenuLayer *menu_layer, bool enable);

bool menu_layer_get_center_focused(MenuLayer *menu_layer);

void menu_layer_set_center_focused(MenuLayer *menu_layer, bool center_focused);

bool menu_layer_is_index_selected(const MenuLayer *menu_layer, MenuIndex *index);

//! Sets the count of columns for the \ref MenuLayer. A cell then has a width of
//! `frame.size.w / num_columns`. The height of a particular row is determined by the
//! maximum height of the cells (and the separators). If there are less than
//! `num_columns` in a row the additional space is not touched.
//! @param menu_layer Pointer to the \ref MenuLayer for which to set the column count.
//! @param num_columns The count of columns to set for the \ref MenuLayer.
void menu_layer_set_column_count(MenuLayer *menu_layer, uint16_t num_columns);

//! Sets the reload behaviour for the \ref MenuLayer. This mode decides when the menu
//! data is reloaded via the \ref MenuLayerCallbacks.
//! @param menu_layer Pointer to the \ref MenuLayer for which to set the reload behaviour.
//! @param behaviour The new reload behaviour to set for the \ref MenuLayer.
//! @see MenuLayerReloadBehaviour
void menu_layer_set_reload_behaviour(MenuLayer *menu_layer, MenuLayerReloadBehaviour behaviour);

#ifdef PBL_RECT
#define MENU_DEFAULT_TEXT_ALIGNMENT GTextAlignmentLeft
#else
#define MENU_DEFAULT_TEXT_ALIGNMENT GTextAlignmentCenter
#endif

#define MENU_CELL_BASIC_HEADER_HEIGHT ((const int16_t) 16)
#define MENU_CELL_BASIC_CELL_HEIGHT ((const int16_t) DISPLAY_ROWS / 4)

#define MENU_INDEX_NOT_FOUND ((const uint16_t) ~0)
#define MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT ((const int16_t) 68)
#define MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT ((const int16_t) 84)
#define MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT ((const int16_t) 24)
#define MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT ((const int16_t) 32)
#define MENU_BOTTOM_PADDING ((const int16_t) 20)
#define MENU_CELL_PADDING ((const int16_t) 5)
