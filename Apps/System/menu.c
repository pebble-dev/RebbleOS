/* menu.c
 *
 * Generic menu component.
 *
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include "menu.h"

static list_head _res_list_head = LIST_HEAD(_res_list_head);

typedef struct {
    uint16_t res_id;
    GBitmap *gbitmap;
    list_node node;
} _gbitmap_pair;

MenuItems* menu_items_create(uint16_t capacity)
{
    MenuItems *result = (MenuItems *) app_calloc(1, sizeof(MenuItems));
    result->capacity = capacity;
    result->count = 0;
    result->back = NULL;
    if (capacity > 0)
        result->items = (MenuItem *) app_calloc(capacity, sizeof(MenuItem));
    
    return result;
}

void menu_items_destroy(MenuItems *items)
{
    if (!items)
        return;

    if (items->back)
        menu_items_destroy(items->back);

    if (items->capacity > 0)
        app_free(items->items);

    app_free(items);
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

static GBitmap *_cached_resource(uint16_t res_id, uint16_t capacity)
{
    _gbitmap_pair *fb = NULL;
    
    list_foreach(fb, &_res_list_head, _gbitmap_pair, node)
    {
        if (fb->res_id == res_id)
            return fb->gbitmap;
    }
       
    fb = app_calloc(1, sizeof(_gbitmap_pair));
    GBitmap *gbitmap = gbitmap_create_with_resource(res_id);
    fb->res_id = res_id;
    fb->gbitmap = gbitmap;

    list_init_node(&fb->node);
    list_insert_head(&_res_list_head, &fb->node);
        
    return gbitmap;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *index, Menu *menu)
{
    MenuItem *item = &menu->items->items[index->row];
    GBitmap *gbitmap = _cached_resource(item->image_res_id, menu->items->count);
    
#ifdef PBL_RECT
    menu_cell_basic_draw(ctx, cell_layer, item->text, item->sub_text, gbitmap);
#else
    // The main menu on chalk has left text alignment and icons, but has to be shifted to the right
    static const int16_t rightShift = 20;
    GRect frame = layer_get_frame(cell_layer);
    frame.origin.x += rightShift;
    menu_cell_basic_draw_ex(ctx, frame, item->text, item->sub_text, gbitmap, GTextAlignmentLeft);
    frame.origin.x -= rightShift;
#endif
}


Menu* menu_create(GRect frame)
{
    Menu *menu = (Menu *) app_calloc(1, sizeof(Menu));
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
    app_free(menu);
    
    _gbitmap_pair *fb = list_elem(list_get_head(&_res_list_head), _gbitmap_pair, node);
    while(fb)
    {
        list_remove(&_res_list_head, &fb->node);
        app_free(fb->gbitmap);
        app_free(fb);
        fb = list_elem(list_get_head(&_res_list_head), _gbitmap_pair, node);
    }
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
