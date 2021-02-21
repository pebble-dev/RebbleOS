/* test.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Hermann Noll
 */

#include "librebble.h"
#include "menu_layer.h"
#include "simple_menu_layer.h"
#include "test_defs.h"

static SimpleMenuLayer *s_simple_menu_layer;
static MenuLayer* s_menu_layer;
static SimpleMenuSection s_menu_section;
static SimpleMenuItem s_menu_items[30];
static char s_menu_item_buf[30][10];

static MenuRowAlign s_row_align = MenuRowAlignCenter;
static MenuLayerReloadBehaviour s_reload_beh = MenuLayerReloadBehaviourManual;
static bool s_padding_enabled = true;

static void _menu_select_callback(ClickRecognizerRef recognizer, void *context)
{
    MenuIndex index = menu_layer_get_selected_index(s_menu_layer);
    if (index.row == 0) {
        bool value = !menu_layer_get_center_focused(s_menu_layer);
        s_menu_items[0].subtitle = value ? "true" : "false";
        menu_layer_set_center_focused(s_menu_layer, value);
    } else if (index.row == 1) {
        const char* name;
        switch(s_row_align) {
            default:
            case MenuRowAlignTop: {
                s_row_align = MenuRowAlignCenter;
                name = "center";
            }break;
            case MenuRowAlignCenter: {
                s_row_align = MenuRowAlignBottom;
                name = "bottom";
            }break;
            case MenuRowAlignBottom: {
                s_row_align = MenuRowAlignTop;
                name = "top";
            }break;
        }
        s_menu_items[1].subtitle = name;
        MenuIndex menu_index = menu_layer_get_selected_index(s_menu_layer);
        menu_layer_set_selected_index(s_menu_layer, menu_index, s_row_align, true);
        layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
    } else if (index.row == 2) {
        s_padding_enabled = !s_padding_enabled;
        s_menu_items[2].subtitle = s_padding_enabled ? "true" : "false";
        menu_layer_pad_bottom_enable(s_menu_layer, s_padding_enabled);
        layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
    } else if (index.row == 3) {
        const char* name;
        switch(s_reload_beh) {
            default:
            case MenuLayerReloadBehaviourManual: {
                s_reload_beh = MenuLayerReloadBehaviourOnSelection;
                name = "Selection";
            }break;
            case MenuLayerReloadBehaviourOnSelection: {
                s_reload_beh = MenuLayerReloadBehaviourOnRender;
                name = "Render";
            }break;
            case MenuLayerReloadBehaviourOnRender: {
                s_reload_beh = MenuLayerReloadBehaviourManual;
                name = "Manual";
            }break;
        }
        s_menu_items[3].subtitle = name;
        menu_layer_set_reload_behaviour(s_menu_layer, s_reload_beh);
        layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
    } else if (index.row == 4) {
        layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
    } else if (index.row == 5) {
        menu_layer_reload_data(s_menu_layer);
    } else if (index.row == 6) {
        test_complete(true);
    }
}

static void _menu_next_callback(ClickRecognizerRef recognizer, void *context)
{
    menu_layer_set_selected_next(s_menu_layer, recognizer, false, s_row_align, true);
}

static void _menu_prev_callback(ClickRecognizerRef recognizer, void *context)
{
    menu_layer_set_selected_next(s_menu_layer, recognizer, true, s_row_align, true);
}

static void _menu_exit_callback(ClickRecognizerRef recognizer, void* context)
{
    test_complete(false);
}

static void _click_provider_config(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)_menu_select_callback);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)_menu_next_callback);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)_menu_prev_callback);
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler)_menu_exit_callback);
}

bool menu_simple_test_init(Window *window)
{
    int num_a_items = 0;

    s_menu_items[num_a_items++] = (SimpleMenuItem){
        .title = "Centered",
        .subtitle = "falrue"
    };
    s_menu_items[num_a_items++] = (SimpleMenuItem){
        .title = "Row Align",
        .subtitle = "center"
    };
    s_menu_items[num_a_items++] = (SimpleMenuItem){
        .title = "Bottom Padding",
        .subtitle = "true"
    };
    s_menu_items[num_a_items++] = (SimpleMenuItem){
        .title = "Reload Behaviour",
        .subtitle = "Manual"
    };
    s_menu_items[num_a_items++] = (SimpleMenuItem){
        .title = "Mark as dirty"
    };
    s_menu_items[num_a_items++] = (SimpleMenuItem){
        .title = "Reload data"
    };
    s_menu_items[num_a_items++] = (SimpleMenuItem){
        .title = "Pass test"
    };
    for (; num_a_items < sizeof(s_menu_items) / sizeof(s_menu_items[0]); num_a_items++) {
        snprintf(s_menu_item_buf[num_a_items], sizeof(s_menu_item_buf[0]), "Item %d", num_a_items);
        s_menu_items[num_a_items] = (SimpleMenuItem) {
            .title = s_menu_item_buf[num_a_items]
        };
    }

    s_menu_section = (SimpleMenuSection){
        .title = NULL,
        .items = s_menu_items,
        .num_items = num_a_items
    };

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    s_simple_menu_layer = simple_menu_layer_create(bounds, window, &s_menu_section, 1, NULL);
    s_menu_layer = simple_menu_layer_get_menu_layer(s_simple_menu_layer);
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_menu_layer));
    menu_layer_set_highlight_colors(s_menu_layer, GColorRed, GColorBlack);
    s_menu_items[0].subtitle = menu_layer_get_center_focused(s_menu_layer) ? "true" : "false";

    window_set_click_config_provider(window, _click_provider_config);
    return true;
}

bool menu_simple_test_exec(void)
{
    return true;
}

bool menu_simple_test_deinit(void)
{
    layer_remove_from_parent(simple_menu_layer_get_layer(s_simple_menu_layer));
    simple_menu_layer_destroy(s_simple_menu_layer);
    return true;
}
