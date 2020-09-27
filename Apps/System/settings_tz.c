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
static StatusBarLayer *_tzdir_status_bar;

static Window *_tzsel_window;
static MenuLayer *_tzsel_menu;
static StatusBarLayer *_tzsel_status_bar;

static struct fd tzfd;
static int tzfddirpos = 0xFFFF;
static int tzfdselpos = 0xFFFF;

static int tzdirsel = 0;

#define NAMSIZ 32
static char *tzdirnam, *tznam;

/* Timezone (the timezone itself) selector logic. */

static uint16_t _tzsel_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
    int ntzs = 0;
    int dirpos = 0;
    char tzbuf[NAMSIZ];
    union tzrec tzrec;
    
    tz_db_open(&tzfd);
    for (dirpos = 0; dirpos <= tzdirsel; dirpos++)
        (void) tz_db_nextdir(&tzfd, tzbuf, sizeof(tzbuf));
    
    tzfdselpos = 0;
    while (tz_db_nexttz(&tzfd, tzbuf, sizeof(tzbuf), &tzrec) >= 0)
        tzfdselpos++;
    return tzfdselpos;
}

static void _tzsel_seek(int tzn, char *buf, size_t sz, union tzrec *tzrec) {
    if (tzn < tzfdselpos) {
        int dirpos;
        
        tz_db_open(&tzfd);
        for (int dirpos = 0; dirpos <= tzdirsel; dirpos++)
            (void) tz_db_nextdir(&tzfd, buf, sz);
        tzfdselpos = 0;
    }
    
    for (; tzfdselpos <= tzn; tzfdselpos++)
        (void) tz_db_nexttz(&tzfd, buf, sz, tzrec);
}

static void _tzsel_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
    int tzn = cell_index->row;
    char tzbuf[NAMSIZ];
    union tzrec tzrec;
    
    _tzsel_seek(tzn, tzbuf, sizeof(tzbuf), &tzrec);
    
    menu_cell_basic_draw(ctx, cell_layer, tzbuf, NULL, NULL);
}

static int16_t _tzsel_menu_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
    return 44; /* hardcoded for basic_draw: can we get another one somewhere? */
}

static void _tzsel_menu_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
    union tzrec tzrec;
    
    _tzsel_seek(cell_index->row, tznam, NAMSIZ, &tzrec);
    tz_load(tzdirnam, tznam);
    
    window_stack_pop(false);
    window_stack_pop(false);
}

static void _tzsel_window_load(Window *window)
{
    /* XXX: Some day this "menu plus a status bar" pattern should be
     * factored out somewhere.  */
    Layer *window_layer = window_get_root_layer(window);

#ifdef PBL_RECT
    _tzsel_menu = menu_layer_create(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16));
#else
#error round pebble not supported here
#endif
    menu_layer_set_callbacks(_tzsel_menu, NULL, (MenuLayerCallbacks) {
        .get_num_rows = _tzsel_menu_get_num_rows,
        .draw_row = _tzsel_menu_draw_row,
        .get_cell_height = _tzsel_menu_get_cell_height,
        .select_click = _tzsel_menu_select_click,
    });
    layer_add_child(window_layer, menu_layer_get_layer(_tzsel_menu));
    menu_layer_set_click_config_onto_window(_tzsel_menu, window);

    // Status Bar
    _tzsel_status_bar = status_bar_layer_create();
    layer_add_child(window_layer, status_bar_layer_get_layer(_tzsel_status_bar));
}

static void _tzsel_window_unload(Window *window)
{
    menu_layer_destroy(_tzsel_menu);
    status_bar_layer_destroy(_tzsel_status_bar);
}

/* Timezone directory selector logic. */

static void _tzdir_seek(int tzn, char *buf, size_t sz) {
    if (tzn <= tzfddirpos) {
        tz_db_open(&tzfd);
        tzfddirpos = 0;
    }
    
    for (; tzfddirpos <= tzn; tzfddirpos++)
        (void) tz_db_nextdir(&tzfd, buf, sz);
}

static uint16_t _tzdir_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
    int ntzs = 0;
    char tzbuf[NAMSIZ];
    
    tz_db_open(&tzfd);
    while (tz_db_nextdir(&tzfd, tzbuf, sizeof(tzbuf)) >= 0)
        ntzs++;
    tzfddirpos = ntzs;
    
    return ntzs;
}

static void _tzdir_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
    int tzn = cell_index->row;
    char tzbuf[NAMSIZ];
    
    _tzdir_seek(tzn, tzbuf, sizeof(tzbuf));
    
    menu_cell_basic_draw(ctx, cell_layer, tzbuf, NULL, NULL);
}

static int16_t _tzdir_menu_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
    return 44; /* hardcoded for basic_draw: can we get another one somewhere? */
}

static void _tzdir_menu_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
    window_stack_push(_tzsel_window, false);
    
    tzdirsel = cell_index->row;
    _tzdir_seek(tzdirsel, tzdirnam, NAMSIZ);
    menu_layer_reload_data(_tzsel_menu);
    menu_layer_set_selected_index(_tzsel_menu, (MenuIndex) { .section = 0, .row = 0 }, MenuRowAlignTop, false);
}

static void _tzdir_window_load(Window *window)
{
    /* XXX: Some day this "menu plus a status bar" pattern should be
     * factored out somewhere.  */
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
    _tzdir_status_bar = status_bar_layer_create();
    layer_add_child(window_layer, status_bar_layer_get_layer(_tzdir_status_bar));
}

static void _tzdir_window_unload(Window *window)
{
    menu_layer_destroy(_tzdir_menu);
    status_bar_layer_destroy(_tzdir_status_bar);
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

    _tzsel_window = window_create();
    window_set_window_handlers(_tzsel_window, (WindowHandlers) {
        .load = _tzsel_window_load,
        .unload = _tzsel_window_unload,
    });

    tzdirnam = malloc(NAMSIZ);
    tznam = malloc(NAMSIZ);
}

void settings_tz_deinit() {
    window_destroy(_tzdir_window);
    window_destroy(_tzsel_window);
    
    free(tzdirnam);
    free(tznam);
}
