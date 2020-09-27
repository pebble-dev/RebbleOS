/* settings_tz.c
 * GUI implementation for time zone settings
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "rebbleos.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "action_bar_layer.h"
#include "platform_res.h"

static Window *_tzdir_window;
static MenuLayer *_tzdir_menu;
static StatusBarLayer *status_bar;
static struct fd tzfd;
static int tzfdpos = 0xFFFF;

static uint16_t _tzdir_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
    int ntzs = 0;
    char tzbuf[64];
    
    tz_db_open(&tzfd);
    while (tz_db_nextdir(&tzfd, tzbuf, sizeof(tzbuf)) >= 0)
        ntzs++;
    tzfdpos = ntzs;
    
    return ntzs;
}

static void _tzdir_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
    int tzn = cell_index->row;
    char tzbuf[64];
    
    if (tzn < tzfdpos) {
        tz_db_open(&tzfd);
        tzfdpos = 0;
    }
    
    for (; tzfdpos != tzn; tzfdpos++)
        (void) tz_db_nextdir(&tzfd, tzbuf, sizeof(tzbuf));
    
    tz_db_nextdir(&tzfd, tzbuf, sizeof(tzbuf));
    tzfdpos++;
    
    menu_cell_basic_draw(ctx, cell_layer, tzbuf, NULL, NULL);
}

static int16_t _tzdir_menu_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
    return 44; /* hardcoded for basic_draw: can we get another one somewhere? */
}

static void _tzdir_menu_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
}

static void _tzdir_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);

#ifdef PBL_RECT
    _tzdir_menu = menu_layer_create(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16));
#else
#error round pebble not supported here
#endif
    menu_layer_set_callbacks(_tzdir_menu, NULL, (MenuLayerCallbacks) {
        .get_num_rows = _tzdir_menu_get_num_rows,
        .draw_row = _tzdir_menu_draw_row,
        .get_cell_height = _tzdir_menu_get_cell_height,
        .select_click = _tzdir_menu_select_click,
    });
    layer_add_child(window_layer, menu_layer_get_layer(_tzdir_menu));
    menu_layer_set_click_config_onto_window(_tzdir_menu, window);

    // Status Bar
    status_bar = status_bar_layer_create();
    layer_add_child(window_layer, status_bar_layer_get_layer(status_bar));
}

static void _tzdir_window_unload(Window *window)
{
    menu_layer_destroy(_tzdir_menu);
    status_bar_layer_destroy(status_bar);
}

void settings_tz_invoke() {
    window_stack_push(_tzdir_window, false);
}

void settings_tz_init() {
    _tzdir_window = window_create();
    window_set_window_handlers(_tzdir_window, (WindowHandlers) {
        .load = _tzdir_window_load,
        .unload = _tzdir_window_unload,
    });
}

void settings_tz_deinit() {
    window_destroy(_tzdir_window);
}
