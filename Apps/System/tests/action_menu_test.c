/* action_menu_test.c
 * RebbleOS
 * 
 * Author: Hermann Noll <hermann.noll@hotmail.com>
 */

#include "librebble.h"
#include "action_menu.h"
#include "test_defs.h"

static TextLayer *s_label;
static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level, *s_sub_level;

static void _action_performed(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
    test_complete(true);
}

static void _select_click(ClickRecognizerRef rec, void *context)
{
    ActionMenuConfig config = (ActionMenuConfig){
        .root_level = s_root_level,
        .colors = {
            .background = GColorChromeYellow,
            .foreground = GColorBlue},
        .align = ActionMenuAlignTop};

    s_action_menu = action_menu_open(&config);
}

static void _back_click(ClickRecognizerRef rec, void *context)
{
    test_complete(false);
}

static void _click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, _select_click);
    window_single_click_subscribe(BUTTON_ID_BACK, _back_click);
}

bool action_menu_test_init(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect frame = layer_get_frame(window_layer);
    window_set_background_color(window, GColorWhite);
    window_set_click_config_provider(window, _click_config_provider);

    s_label = text_layer_create(GRect(frame.origin.x, frame.origin.y + frame.size.h/2 - 24, frame.size.w, frame.size.h));
    text_layer_set_text(s_label, "Press SELECT to open the ActionMenu");
    text_layer_set_font(s_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_color(s_label, GColorBlack);
    layer_add_child(window_layer, text_layer_get_layer(s_label));

    // Create the root level
    s_root_level = action_menu_level_create(4);
    action_menu_level_add_action(s_root_level, "First", _action_performed, NULL);
    action_menu_level_add_action(s_root_level, "Second", _action_performed, NULL);
    action_menu_level_add_action(s_root_level, "Third", _action_performed, NULL);

    // Create and set up the secondary level, adding it as a child to the root one
    s_sub_level = action_menu_level_create(3);
    action_menu_level_add_child(s_root_level, s_sub_level, "Sub level");
    action_menu_level_add_action(s_sub_level, "Sub First", _action_performed, NULL);
    action_menu_level_add_action(s_sub_level, "Sub Second", _action_performed, NULL);
    action_menu_level_add_action(s_sub_level, "Sub Third", _action_performed, NULL);

    return true;
}

bool action_menu_test_exec(void)
{
    return true;
}

bool action_menu_test_deinit(void)
{
    layer_remove_from_parent(text_layer_get_layer(s_label));
    return true;
}
