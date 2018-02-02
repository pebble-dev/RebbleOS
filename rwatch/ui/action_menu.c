/* action_menu.c
 *
 * A tiered menu
 *
 * RebbleOS
 * 
 * Author: Carson Katri <me@carsonkatri.com>.
 */

#include "rebbleos.h"
#include "action_menu.h"

/* STRUCT DEFS */
struct ActionMenuLevel
{
    ActionMenuLevelDisplayMode *display_mode;
    ActionMenuItem *items;
    int count;
    uint16_t *capacity;
    int index;
};

struct ActionMenuItem
{
    ActionMenuLevel *level;
    char *label;
    void *action_data;
    ActionMenuPerformActionCb cb;
};

struct ActionMenu
{
    ActionMenuConfig *config;
    int *level_index;
    Window *result_window;
    MenuLayer *menu_layer;
};

/* END */

char * action_menu_item_get_label(const ActionMenuItem * item)
{
    return item->label;
}

void * action_menu_item_get_action_data(const ActionMenuItem * item)
{
    return item->action_data;
}

ActionMenuLevel * action_menu_level_create(uint16_t max_items)
{
    ActionMenuLevel *level = (ActionMenuLevel *)app_calloc(1, sizeof(ActionMenuLevel) + (sizeof(ActionMenuItem) * max_items));
    level->capacity = max_items;
    level->count = 0;
    level->index = 0;
    
    return level;
}

void action_menu_level_set_display_mode(ActionMenuLevel * level, ActionMenuLevelDisplayMode display_mode)
{
    level->display_mode = (ActionMenuLevelDisplayMode *) display_mode;
}

ActionMenuItem * action_menu_level_add_action(ActionMenuLevel * level, const char *label, ActionMenuPerformActionCb cb, void * action_data)
{
    ActionMenuItem *items = level->items;
    
    ActionMenuItem *new_item = (ActionMenuItem *)app_calloc(1, sizeof(ActionMenuItem));
    new_item->label = label;
    new_item->level = level;
    new_item->cb = cb;
    new_item->action_data = action_data;
    
    if (level->capacity == level->count)
        return NULL;
    
    items[level->count++] = *new_item;
    return new_item;
}

static void child_level_action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
    // Switch levels
    action_menu->level_index += 1;
}

ActionMenuItem * action_menu_level_add_child(ActionMenuLevel * level, ActionMenuLevel * child, const char * label)
{
    return action_menu_level_add_action(level, label, child_level_action_performed_callback, child);
}

void action_menu_hierarchy_destroy(const ActionMenuLevel * root, ActionMenuEachItemCb each_cb, void * context)
{
    app_free(root->display_mode);
    app_free(root->items);
    
    app_free(root);
}

void * action_menu_get_context(ActionMenu * action_menu)
{
    return action_menu->config->context;
}

ActionMenuLevel * action_menu_get_root_level(ActionMenu * action_menu)
{
    return action_menu->config->root_level;
}

static void side_bar_update_proc(Layer *layer, GContext *nGContext)
{
    ActionMenu *action_menu = (ActionMenu *) layer->container;
    ActionMenuConfig *config = action_menu->config;
    
#ifdef PBL_RECT
    // Draw the sidebar:
    graphics_context_set_fill_color(nGContext, config->colors.background);
    graphics_fill_rect(nGContext, GRect(0, 0, layer->frame.size.w, layer->frame.size.h), 0, GCornerNone);
    
    // Draw the level:
    int index = action_menu->level_index;
    SYS_LOG("notification_window", APP_LOG_LEVEL_DEBUG, "INDEX: %d", index);
    graphics_context_set_fill_color(nGContext, GColorWhite);
    n_graphics_fill_circle(nGContext, GPoint(layer->frame.size.w / 2, (10 * (index + 1)) + (5 * index)), 4);
#else
    // On round, just draw a circle around the menu
    graphics_context_set_stroke_color(nGContext, config->colors.background);
    nGContext->stroke_width = 13;
    n_graphics_draw_circle(nGContext, GPoint(DISPLAY_COLS / 2, DISPLAY_ROWS / 2), (DISPLAY_COLS / 2) - 5);
#endif
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer,
                                      uint16_t section_index, void *context) {
    ActionMenu *action_menu = (ActionMenu *) context;
    const uint16_t num_rows = 3;
    return num_rows;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer,
                              MenuIndex *cell_index, void *context) {
    GRect frame = cell_layer->frame;
    ActionMenu *action_menu = (ActionMenu *) context;
    
    // Draw this row's index
    ctx->text_color = GColorWhite;
    
    const char *item_text = &action_menu->config->root_level->items[cell_index->row].label;
    
#ifdef PBL_RECT
    GTextAlignment align = GTextAlignmentLeft;
    GFont item_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    GRect item_rect = GRect(30, frame.size.h / 2 - 16, frame.size.w - 35, 24);
    graphics_draw_text(ctx, item_text, item_font, item_rect, GTextOverflowModeTrailingEllipsis, align, 0);
#else
    GTextAlignment align = GTextAlignmentCenter;
    GFont item_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    GRect item_rect = GRect(20, frame.size.h / 2 - 16, frame.size.w - 25, 24);
    graphics_draw_text(ctx, item_text, item_font, item_rect, GTextOverflowModeTrailingEllipsis, align, 0);
#endif
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer,
                                        MenuIndex *cell_index, void *context) {
#ifdef PBL_RECT
    const int16_t cell_height = 44;
#else
    const int16_t cell_height = 180;
#endif
    return cell_height;
}

static void item_selected(struct MenuLayer *menu_layer,
                            MenuIndex *cell_index, void *context) {
    // Do something in response to the button press
}

static void action_menu_window_load(Window *window)
{
    ActionMenu *action_menu = (ActionMenu *) window->user_data;
    ActionMenuConfig *config = action_menu->config;
    
    // Make a menu
    GRect frame = layer_get_unobstructed_bounds(window_get_root_layer(window));
    action_menu->menu_layer = menu_layer_create(frame);
    menu_layer_set_callbacks(action_menu->menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_rows = get_num_rows_callback,
        .draw_row = draw_row_callback,
        .get_cell_height = get_cell_height_callback,
        .select_click = item_selected
    });
    layer_add_child(window_get_root_layer(window), menu_layer_get_layer(action_menu->menu_layer));
    
    action_menu->menu_layer->bg_color = GColorBlack;
    action_menu->menu_layer->bg_hi_color = GColorBlack;
    action_menu->menu_layer->fg_hi_color = GColorWhite;
    action_menu->menu_layer->fg_color = GColorWhite;
    
    //menu_layer_set_click_config_provider(action_menu->menu_layer, (ClickConfigProvider) menu_click_config_provider);
    menu_layer_set_click_config_onto_window(action_menu->menu_layer, window);
    
    action_menu->menu_layer->context = action_menu;
#ifdef PBL_RECT
    Layer *side_bar_layer = layer_create(GRect(0, 0, 20, DISPLAY_ROWS));
#else
    Layer *side_bar_layer = layer_create(GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS));
#endif
    side_bar_layer->container = action_menu;
    layer_set_update_proc(side_bar_layer, side_bar_update_proc);
    
    layer_add_child(window_get_root_layer(window), side_bar_layer);
    
    layer_mark_dirty(menu_layer_get_layer(action_menu->menu_layer));
}

static void action_menu_window_unload(Window *window)
{
    // Unloaded
}

ActionMenu * action_menu_open(ActionMenuConfig * config)
{
    ActionMenu *action_menu = (ActionMenu *)app_calloc(1, sizeof(ActionMenu));
    action_menu->config = config;
    action_menu->level_index = 0;
    
    Window *main = window_create();
    main->user_data = action_menu;
    
    window_set_window_handlers(main, (WindowHandlers) {
        .load = action_menu_window_load,
        .unload = action_menu_window_unload,
    });
    
    window_stack_push(main, true);
    
    app_event_loop();
    
    return action_menu;
}

void action_menu_freeze(ActionMenu * action_menu)
{
    
}

void action_menu_unfreeze(ActionMenu * action_menu)
{
    
}

void action_menu_set_result_window(ActionMenu * action_menu, Window * result_window)
{
    action_menu->result_window = result_window;
}

void action_menu_close(ActionMenu * action_menu, bool animated)
{
    
}
