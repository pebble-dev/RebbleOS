/* simple_menu_layer.h
 * routines for the simple menu layer
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 * Author: Hermann Noll <hermann.noll@hotmail.com>
 */

#include "simple_menu_layer.h"
#include "menu_layer.h"

static uint16_t _get_number_of_sections(struct MenuLayer *menu_layer, void *callback_context)
{
    SimpleMenuLayer* simple_menu = (SimpleMenuLayer*)callback_context;
    return simple_menu->num_sections;
}

static uint16_t _get_number_of_rows_in_section(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
    SimpleMenuLayer* simple_menu = (SimpleMenuLayer*)callback_context;
    return simple_menu->sections[section_index].num_items;
}

static int16_t _get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
    return MENU_CELL_BASIC_CELL_HEIGHT;
}

static int16_t _get_header_height(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
    SimpleMenuLayer* simple_menu = (SimpleMenuLayer*)callback_context;
    return simple_menu->sections[section_index].title == NULL ? 0 : MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void _draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    SimpleMenuLayer* simple_menu = (SimpleMenuLayer*)callback_context;
    const SimpleMenuItem* item = &simple_menu->sections[cell_index->section].items[cell_index->row];
    menu_cell_basic_draw(ctx, cell_layer, item->title, item->subtitle, item->icon);
}

static void _draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context)
{
    SimpleMenuLayer* simple_menu = (SimpleMenuLayer*)callback_context;
    const SimpleMenuSection* section = &simple_menu->sections[section_index];
    if (section->title != NULL) {
        menu_cell_basic_header_draw(ctx, cell_layer, section->title);
    }
}

static void _select(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
    SimpleMenuLayer* simple_menu = (SimpleMenuLayer*)callback_context;
    const SimpleMenuItem* item = &simple_menu->sections[cell_index->section].items[cell_index->row];
    if (item->callback)
        item->callback(cell_index->row, simple_menu->callback_context);
}

SimpleMenuLayer* simple_menu_layer_create(GRect frame, struct Window *window, const SimpleMenuSection *sections,
                                          int32_t num_sections, void *callback_context)
{
    SimpleMenuLayer* simple_menu = app_calloc(1, sizeof(SimpleMenuLayer));
    simple_menu->sections = sections;
    simple_menu->num_sections = num_sections;
    simple_menu->callback_context = callback_context;

    struct MenuLayer* menu_layer = menu_layer_create(frame);
    simple_menu->menu_layer = menu_layer;
    menu_layer_set_click_config_onto_window(menu_layer, window);
    menu_layer_set_callbacks(menu_layer, simple_menu, (MenuLayerCallbacks){
        .get_num_sections = _get_number_of_sections,
        .get_num_rows = _get_number_of_rows_in_section,
        .get_cell_height = _get_cell_height,
        .get_header_height = _get_header_height,
        .draw_row = _draw_row,
        .draw_header = _draw_header,
        .select_click = _select,
        .select_long_click = NULL,
        .selection_changed = NULL,
        .get_separator_height = NULL,
        .selection_will_change = NULL,
        .draw_background = NULL
    });

    return simple_menu;
}

void simple_menu_layer_destroy(SimpleMenuLayer *simple_menu)
{
    menu_layer_destroy(simple_menu->menu_layer);
    app_free(simple_menu);
}

struct Layer* simple_menu_layer_get_layer(const SimpleMenuLayer *simple_menu)
{
    return menu_layer_get_layer(simple_menu->menu_layer);
}

int simple_menu_layer_get_selected_index(const SimpleMenuLayer *simple_menu)
{
    MenuIndex index = menu_layer_get_selected_index(simple_menu->menu_layer);
    return index.row;
}

void simple_menu_layer_set_selected_index(SimpleMenuLayer *simple_menu, int32_t index, bool animated)
{
    MenuIndex menu_index = {
        .section = 0,
        .row = index
    };
    menu_layer_set_selected_index(simple_menu->menu_layer, menu_index, MenuRowAlignCenter, animated);
}

struct MenuLayer* simple_menu_layer_get_menu_layer(SimpleMenuLayer *simple_menu)
{
    return simple_menu->menu_layer;
}
