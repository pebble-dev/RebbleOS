/* menu_multi_column_test
 * RebbleOS
 *
 * Author: Hermann Noll
 */

#include "librebble.h"
#include "graphics_wrapper.h"
#include "menu_layer.h"
#include "test_defs.h"

static MenuLayer* s_menu_layer;

static uint16_t _get_num_sections(MenuLayer* menu_layer, void* context)
{
    return 2;
}

static uint16_t _get_num_rows(MenuLayer* menu_layer, uint16_t section, void* context)
{
    return 9 + (!!section) * 2;
}

static int16_t _get_cell_height(MenuLayer* menu_layer, MenuIndex* cell_index, void* context)
{
    int16_t i = cell_index->section ? 8 - cell_index->row : cell_index->row;
    return 24 + i * 2;
}

static int16_t _get_header_height(MenuLayer* menu_layer, uint16_t section, void* context)
{
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void _draw_row(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* context)
{
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    char text[2] = { (cell_index->section ? 'A' : '0') + cell_index->row , '\0' };
    GRect rect = { .origin = GPoint(0, 0), .size = layer_get_frame(cell_layer).size };
    graphics_draw_text(ctx, text, font, rect, GTextOverflowModeTrailingEllipsis, 
                       GTextAlignmentCenter, NULL);
}

static void _draw_header(GContext* ctx, const Layer* cell_layer, uint16_t section, void* context)
{
    menu_cell_basic_header_draw(ctx, cell_layer, section
        ? "Letters (decr. cell h)"
        : "Digits (incr. cell h)");
}

static void _select_item(ClickRecognizerRef recognizer, void* context)
{
    test_complete(true);
}

static void _exit_callback(ClickRecognizerRef recognizer, void* context)
{
    test_complete(false);
}

static void _click_provider_config(void *context)
{
    window_single_click_subscribe(BUTTON_ID_BACK, _exit_callback);
    window_single_click_subscribe(BUTTON_ID_SELECT, _select_item);
}

bool menu_multi_column_test_init(Window* window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect frame = layer_get_frame(window_layer);

    s_menu_layer = menu_layer_create(frame);
    menu_layer_set_column_count(s_menu_layer, 3);
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = _get_num_sections,
        .get_num_rows = _get_num_rows,
        .get_cell_height = _get_cell_height,
        .get_header_height = _get_header_height,
        .draw_row = _draw_row,
        .draw_header = _draw_header
    });
    menu_layer_set_click_config_provider(s_menu_layer, _click_provider_config);
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    menu_layer_set_highlight_colors(s_menu_layer, GColorRed, GColorBlack);
    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
    return true;
}

bool menu_multi_column_test_exec(void)
{
    return true;
}

bool menu_multi_column_test_deinit(void)
{
    layer_remove_from_parent(menu_layer_get_layer(s_menu_layer));
    menu_layer_destroy(s_menu_layer);
    return true;
}
