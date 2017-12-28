/* menu.c
 *
 * Generic menu component.
 *
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include "rebbleos.h"
#include "menu.h"

MenuItems* menu_items_create(uint16_t capacity)
{
    MenuItems *result = (MenuItems *) calloc(1, sizeof(MenuItems)); // XXX: use app heap allocator
    result->capacity = capacity;
    result->count = 0;
    if (capacity > 0)
        result->items = (MenuItem *) calloc(capacity, sizeof(MenuItem)); // XXX: use app heap allocator
    return result;
}

void menu_items_destroy(MenuItems *items)
{
    if (items->back)
        menu_items_destroy(items->back);
    if (items->capacity > 0)
        free(items->items);
    free(items);
}

void menu_items_add(MenuItems *items, MenuItem item)
{
    if (items->capacity == items->count)
        return; // TODO: should we grow, or report error here?

    items->items[items->count++] = item;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, Menu *menu)
{
    return menu->items->count;
}

static void select_click_callback(MenuLayer *menu_layer, MenuIndex *index, Menu *menu)
{
    MenuItem item = menu->items->items[index->row];
    if (item.on_select)
    {
        MenuItems *submenu = item.on_select(&item);
        if (submenu)
        {
            submenu->back = menu->items;
            submenu->back_index = *index;
            menu->items = submenu;
            menu_layer_reload_data(menu->layer);
            menu_layer_set_selected_index(menu->layer, MenuIndex(0, 0), MenuRowAlignTop, false);
        }
    }
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *index, Menu *menu)
{
#ifdef PBL_RECT
    MenuItem *item = &menu->items->items[index->row];
    GBitmap *gbitmap = gbitmap_create_with_resource(item->image_res_id);
    menu_cell_basic_draw(ctx, cell_layer, item->text, item->sub_text, gbitmap);
    gbitmap_destroy(gbitmap);
#else
    menu_cell_chalk_draw(ctx, menu_get_layer(menu), menu->items->items[(index->row) - 1].text, &menu->items->items[index->row], menu->items->items[(index->row) + 1].text);
#endif
}


Menu* menu_create(GRect frame)
{
    Menu *menu = (Menu *) calloc(1, sizeof(Menu)); // XXX: use app heap allocator
    menu->items = menu_items_create(0);
    menu->layer = menu_layer_create(frame);
    menu_layer_set_highlight_colors(menu->layer, GColorRed, GColorWhite);
    menu_layer_set_callbacks(menu->layer, menu, (MenuLayerCallbacks) {
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) get_num_rows_callback,
        .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
        .select_click = (MenuLayerSelectCallback) select_click_callback,
    });

    return menu;
}

void menu_destroy(Menu *menu)
{
    if (menu->items)
        menu_items_destroy(menu->items);
    menu_layer_destroy(menu->layer);
    free(menu);
}

Layer* menu_get_layer(Menu *menu)
{
    return menu_layer_get_layer(menu->layer);
}

void menu_set_items(Menu *menu, MenuItems *items)
{
    menu_items_destroy(menu->items);
    menu->items = items;
    menu_layer_reload_data(menu->layer);
    menu_layer_set_selected_index(menu->layer, MenuIndex(0, 0), MenuRowAlignTop, false);
}

void menu_set_callbacks(Menu *menu, void *context, MenuCallbacks callbacks)
{
    menu->callbacks = callbacks;
    menu->context = context;
}

static void back_single_click_handler(ClickRecognizerRef _, Menu *menu)
{
    if (menu->items->back)
    {
        MenuItems *prev = menu->items;
        menu->items = prev->back;
        menu_layer_reload_data(menu->layer);
        menu_layer_set_selected_index(menu->layer, prev->back_index, MenuRowAlignTop, false);
        prev->back = NULL; // so we don't free that
        menu_items_destroy(prev);
    }
    else if (menu->callbacks.on_menu_exit)
        menu->callbacks.on_menu_exit(menu, menu->context);
}

static void menu_click_config_provider(Menu *menu)
{
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler) back_single_click_handler);
    window_set_click_context(BUTTON_ID_BACK, menu);
}

void menu_set_click_config_onto_window(Menu *menu, Window *window)
{
    menu_layer_set_click_config_provider(menu->layer, (ClickConfigProvider) menu_click_config_provider);
    menu_layer_set_click_config_onto_window(menu->layer, window);
}
