/* action_menu.c
 *
 * A tiered menu
 *
 * RebbleOS
 * 
 * Author: Carson Katri <me@carsonkatri.com>.
 * Author: Hermann Noll <hermann.noll@hotmail.com>
 */

#include "rebbleos.h"
#include "action_menu.h"
#include "menu_layer.h"
#include "pebble_defines.h"

#define ACTION_MENU_SIDEBAR_SIZE        PBL_IF_RECT_ELSE(14, 11)
#define ACTION_MENU_CRUMB_PADDING 8
#define ACTION_MENU_CRUMB_SIZE 5

#define ACTION_MENU_CELL_HEIGHT         PBL_IF_RECT_ELSE(40, 32)
#define ACTION_MENU_FOCUSED_CELL_HEIGHT PBL_IF_RECT_ELSE(40, 70)
#define ACTION_MENU_CELL_FONT           FONT_KEY_GOTHIC_18_BOLD
#define ACTION_MENU_FOCUSED_CELL_FONT   PBL_IF_RECT_ELSE(FONT_KEY_GOTHIC_18_BOLD, FONT_KEY_GOTHIC_24_BOLD)

#define ACTION_MENU_LEVEL_INDICATOR "\xC2\xBB" // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK

struct ActionMenuItem
{
    const char *label;
    void *action_data;
    ActionMenuPerformActionCb cb;
};

struct ActionMenuLevel
{
    ActionMenuLevel* parent; // looking at the API only one parent per level is allowed
    ActionMenuLevelDisplayMode display_mode;
    uint16_t count;
    uint16_t capacity;
    ActionMenuItem items[];
};

struct ActionMenu
{
    Window window;
    MenuLayer *menu_layer;
    ActionMenuConfig config;
    const ActionMenuLevel* cur_level;
    uint16_t level_index;
    bool is_frozen;
    Window *result_window;
};

const char *action_menu_item_get_label(const ActionMenuItem *item)
{
    return item->label;
}

void *action_menu_item_get_action_data(const ActionMenuItem *item)
{
    return item->action_data;
}

ActionMenuLevel *action_menu_level_create(uint16_t max_items)
{
    ActionMenuLevel *level = (ActionMenuLevel *)app_calloc(1, sizeof(ActionMenuLevel) + (sizeof(ActionMenuItem) * max_items));
    level->capacity = max_items;
    level->count = 0;
    level->parent = NULL;
    
    return level;
}

void action_menu_level_set_display_mode(ActionMenuLevel *level, ActionMenuLevelDisplayMode display_mode)
{
    level->display_mode = display_mode;
}

ActionMenuItem *action_menu_level_add_action(ActionMenuLevel *level, const char *label, ActionMenuPerformActionCb cb, void *action_data)
{
    if (level->count >= level->capacity) {
        return NULL;
    }
    
    ActionMenuItem *new_item = &level->items[level->count++];
    new_item->label = label;
    new_item->cb = cb;
    new_item->action_data = action_data;
    return new_item;
}

static void _child_level_action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
    action_menu->level_index++;
    action_menu->cur_level = (ActionMenuLevel*)action->action_data;

    MenuIndex zero_index = { .row = 0, .section = 0 };
    menu_layer_set_selected_index(action_menu->menu_layer, zero_index, MenuRowAlignCenter, false);
    menu_layer_reload_data(action_menu->menu_layer);
    // TODO: start animation
}

ActionMenuItem *action_menu_level_add_child(ActionMenuLevel *level, ActionMenuLevel *child, const char *label)
{
    child->parent = level;
    return action_menu_level_add_action(level, label, _child_level_action_performed_callback, child);
}

void action_menu_hierarchy_destroy(ActionMenuLevel *root, ActionMenuEachItemCb each_cb, void *context)
{
    uint16_t i;
    ActionMenuLevel* cur_node = root;
    while (cur_node != NULL) {
        // Traverse all sub-levels
        for (i = 0; i < cur_node->count; i++) {
            if (cur_node->items[i].cb == _child_level_action_performed_callback) {
                cur_node->items[i].cb = NULL;
                cur_node = (ActionMenuLevel *)cur_node->items[i].action_data;
                break;
            }
        }
        if (i < cur_node->count)
            continue;

        // Call each_cb on every child
        if (each_cb != NULL) {
            for (i = 0; i < cur_node->count; i++) {
                each_cb(&cur_node->items[i], context);
            }
        }

        // Back to parent
        ActionMenuLevel* tmp = cur_node->parent;
        app_free(cur_node);
        cur_node = tmp;
    }
}

void *action_menu_get_context(ActionMenu *action_menu)
{
    return action_menu->config.context;
}

const ActionMenuLevel *action_menu_get_root_level(ActionMenu *action_menu)
{
    return action_menu->config.root_level;
}

static void _side_bar_update_proc(Layer *layer, GContext *nGContext)
{
    Window* window = layer_get_window(layer);
    ActionMenu *action_menu = (ActionMenu *)window;
    ActionMenuConfig *config = &action_menu->config;

    //TODO: Reprocess this el hermano!
    
#ifdef PBL_RECT
    graphics_context_set_fill_color(nGContext, config->colors.background);
    graphics_fill_rect(nGContext, GRect(0, 0, layer->frame.size.w, layer->frame.size.h), 0, GCornerNone);
    
    graphics_context_set_fill_color(nGContext, config->colors.foreground);
    for (int i = 0; i <= action_menu->level_index; i++) {
        n_graphics_fill_circle(nGContext, GPoint(layer->frame.size.w / 2, 8 * (i + 1) + 2), 2);
    }
#else
    // On round, just draw a circle around the menu
    graphics_context_set_stroke_color(nGContext, config->colors.background);
    nGContext->stroke_width = 13;
    n_graphics_draw_circle(nGContext, GPoint(DISPLAY_COLS / 2, DISPLAY_ROWS / 2), (DISPLAY_COLS / 2) - 5);
#endif
}

static uint16_t _get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
    ActionMenu *action_menu = (ActionMenu *)context;
    return action_menu->cur_level->count;
}

static void _draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
    GRect frame = cell_layer->frame;
    ActionMenu *action_menu = (ActionMenu *) context;
    
    bool is_highlighted = menu_layer_is_index_selected(action_menu->menu_layer, cell_index);
    ctx->text_color = is_highlighted ? GColorWhite : GColorDarkGray;
    const char *item_label = action_menu->cur_level->items[cell_index->row].label;
    
    GTextAlignment align = MENU_DEFAULT_TEXT_ALIGNMENT;
    GFont item_font = fonts_get_system_font(is_highlighted ? ACTION_MENU_FOCUSED_CELL_FONT : ACTION_MENU_CELL_FONT);
    GRect item_rect = PBL_IF_RECT_ELSE(
        GRect(ACTION_MENU_SIDEBAR_SIZE + 6, frame.size.h / 2 - 16,
              frame.size.w - 2 * ACTION_MENU_SIDEBAR_SIZE, 24), /* rect */
        GRect(0, frame.size.h / 2 - 16, frame.size.w, 24)  /* round */
    );
    graphics_draw_text(ctx, item_label, item_font, item_rect, GTextOverflowModeTrailingEllipsis, align, NULL);

    // draw level indicator
    if (is_highlighted && action_menu->cur_level->items[cell_index->row].cb == _child_level_action_performed_callback) {
        GFont indicator_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect indicator_rect = PBL_IF_RECT_ELSE(
            GRect(frame.size.w - ACTION_MENU_SIDEBAR_SIZE, frame.size.h/2 - 24,
                  ACTION_MENU_SIDEBAR_SIZE, 16), /* rect */
            GRect(0, 28, frame.size.w, 16)       /* round TODO: Fix text size calculation and use text_height + padding */
        );
        graphics_draw_text(ctx, ACTION_MENU_LEVEL_INDICATOR, indicator_font, indicator_rect, 
                               GTextOverflowModeTrailingEllipsis, align, NULL);
    }
}

static int16_t _get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
    MenuIndex focused_index = menu_layer_get_selected_index(menu_layer);
    return cell_index->row == focused_index.row
        ? ACTION_MENU_FOCUSED_CELL_HEIGHT
        : ACTION_MENU_CELL_HEIGHT;
}

static void _action_menu_select_prev(ClickRecognizerRef recognizer, void *context)
{
    ActionMenu *action_menu = (ActionMenu*)context;
    if (!action_menu->is_frozen)
        menu_layer_set_selected_next(action_menu->menu_layer, recognizer, true, MenuRowAlignCenter, true);
}

static void _action_menu_select_next(ClickRecognizerRef recognizer, void *context)
{
    ActionMenu *action_menu = (ActionMenu*)context;
    if (!action_menu->is_frozen)
        menu_layer_set_selected_next(action_menu->menu_layer, recognizer, false, MenuRowAlignCenter, true);
}

static void _action_menu_activate_item(ClickRecognizerRef recognizer, void *context)
{
    ActionMenu *action_menu = (ActionMenu*)context;
    if (!action_menu->is_frozen) {
        MenuIndex index = menu_layer_get_selected_index(action_menu->menu_layer);
        const ActionMenuItem* item = &action_menu->cur_level->items[index.row];

        if (item->cb) {
            item->cb(action_menu, item, item->action_data);

            if (item->cb != _child_level_action_performed_callback)
                action_menu_close(action_menu, true);
        }
    }
}

static void _action_menu_prev_level(ClickRecognizerRef recognizer, void* context)
{
    ActionMenu *action_menu = (ActionMenu*)context;
    if (!action_menu->is_frozen) {
        if (action_menu->level_index > 0) {
            action_menu->level_index--;
            action_menu->cur_level = action_menu->cur_level->parent;

            MenuIndex zero_index = { .row = 0, .section = 0 };
            menu_layer_set_selected_index(action_menu->menu_layer, zero_index, MenuRowAlignCenter, false);
            menu_layer_reload_data(action_menu->menu_layer);
        } else {
            action_menu_close(action_menu, true);
        }
    }
}

static void _action_menu_click_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, _action_menu_select_prev);
    window_single_click_subscribe(BUTTON_ID_DOWN, _action_menu_select_next);
    window_single_click_subscribe(BUTTON_ID_SELECT, _action_menu_activate_item);
    window_single_click_subscribe(BUTTON_ID_BACK, _action_menu_prev_level);
}

static void _action_menu_window_load(Window *window)
{
    ActionMenu *action_menu = (ActionMenu *)window;
    ActionMenuConfig *config = &action_menu->config;
    Layer* root_layer = window_get_root_layer(window);
    GRect full_frame = layer_get_frame(root_layer);
    window_set_background_color(window, GColorBlack);
    window_set_click_config_provider_with_context(window, _action_menu_click_provider, action_menu);
    
    // Create the menu
    action_menu->menu_layer = menu_layer_create(full_frame);
    menu_layer_set_callbacks(action_menu->menu_layer, action_menu, (MenuLayerCallbacks) {
        .get_num_rows = _get_num_rows_callback,
        .draw_row = _draw_row_callback,
        .get_cell_height = _get_cell_height_callback
    });
    menu_layer_set_normal_colors(action_menu->menu_layer, GColorClear, GColorClear);
    menu_layer_set_highlight_colors(action_menu->menu_layer, GColorClear, GColorClear);
#ifndef PBL_RECT
    menu_layer_set_reload_behaviour(action_menu->menu_layer, MenuLayerReloadBehaviourOnSelection);
#endif
    layer_add_child(root_layer, menu_layer_get_layer(action_menu->menu_layer));

    // Create the sidebar
    GRect side_bar_frame = PBL_IF_RECT_ELSE(
        GRect(0, 0, ACTION_MENU_SIDEBAR_SIZE, full_frame.size.h), /* rect */
        
        GRect(ACTION_MENU_SIDEBAR_SIZE, ACTION_MENU_SIDEBAR_SIZE,
              full_frame.size.w - 2*ACTION_MENU_SIDEBAR_SIZE,
              full_frame.size.h - 2*ACTION_MENU_SIDEBAR_SIZE) /* round */
    );
    Layer *side_bar_layer = layer_create(side_bar_frame);
    layer_set_update_proc(side_bar_layer, _side_bar_update_proc);
    layer_add_child(root_layer, side_bar_layer);
}

static void _action_menu_window_unload(Window *window)
{
    // Unloaded
}

ActionMenu *action_menu_open(ActionMenuConfig *config)
{
    ActionMenu *action_menu = (ActionMenu *)app_calloc(1, sizeof(ActionMenu));
    memcpy(&action_menu->config, config, sizeof(ActionMenuConfig));
    action_menu->level_index = 0;
    action_menu->cur_level = config->root_level;
    action_menu->is_frozen = false;
    
    window_ctor(&action_menu->window);
    action_menu->window.user_data = action_menu;
    window_set_window_handlers(&action_menu->window, (WindowHandlers) {
        .load = _action_menu_window_load,
        .unload = _action_menu_window_unload,
    });
    window_stack_push(&action_menu->window, true);
    
    return action_menu;
}

void action_menu_freeze(ActionMenu *action_menu)
{
    action_menu->is_frozen = true;
}

void action_menu_unfreeze(ActionMenu *action_menu)
{
    action_menu->is_frozen = false;
}

void action_menu_set_result_window(ActionMenu *action_menu, Window *result_window)
{
    action_menu->result_window = result_window;
}

void action_menu_close(ActionMenu *action_menu, bool animated)
{
    window_destroy(&action_menu->window);
}
