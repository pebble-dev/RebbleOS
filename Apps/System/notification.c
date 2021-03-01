/* notification.c
 * An app for displaying notification messages
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 *         Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "notification.h"
#include "single_notification_layer.h"
#include "librebble.h"
#include "bitmap_layer.h"
#include "status_bar_layer.h"
#include "action_bar_layer.h"
#include "platform_res.h"
#include "menu.h"
#include "timeline.h"
#include "notification_manager.h"
#include "rdb.h"
#include "notification_window.h"

static Window *_notif_window;
static StatusBarLayer *_notif_window_status;
static MenuLayer *s_menu_layer;
static Window *s_main_window;

static Window _notifdetail_window_obj;
static NotificationWindow _notifdetail_window;

/* We store all the notification keys in a list, and lazy-load each
 * notification to display on screen.  The special case is if there are no
 * notifications at all, in which case we display a single "no
 * notifications" menu list item.
 */
static rdb_select_result_list _notif_list;
static int _notif_count = 0;

static void _notif_window_load(Window *window);
static void _notif_window_unload(Window *window);

static void _notif_get_all() {
    _notif_count = notifications_get_all(&_notif_list);
    menu_layer_reload_data(s_menu_layer);
}

static void _notif_clear_all() {
    notifications_dismiss_all(&_notif_list);
    _notif_count = 0;
    menu_layer_reload_data(s_menu_layer);
}

void notif_init(void)
{
    _notif_window = window_create();
    window_ctor(&_notifdetail_window_obj);
    notification_window_ctor(&_notifdetail_window, &_notifdetail_window_obj);

    window_set_window_handlers(_notif_window, (WindowHandlers) {
        .load = _notif_window_load,
        .unload = _notif_window_unload,
    });

    window_stack_push(_notif_window, true);
}

void notif_deinit(void)
{
    window_destroy(_notif_window);
    notification_window_dtor(&_notifdetail_window);
    window_dtor(&_notifdetail_window_obj);
}

static uint16_t _notif_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
    if (_notif_count == 0)
        return 1;
    else
        /* Add additional row for "Clear All" */
        return _notif_count + 1;
}

static rebble_notification *_noty_for_index(int notif_index) {
    /* Find the noty. */
    int wantidx = _notif_count - notif_index - 1;
    int i = 0;
    struct rdb_select_result *res;
    rdb_select_result_foreach(res, &_notif_list) {
        if (i == wantidx)
            break;
        
        i++;
    }
    assert(i == wantidx);
    void *key = res->result[0];
    
    /* Now that we have the key, go actually fully load the noty itself. */
    return timeline_get_notification((Uuid *)key);
}

static void _notif_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
    if (_notif_count == 0) {
        menu_cell_basic_draw(ctx, cell_layer, "No notifications", "Asleep at the switch?", /* icon, GBitmap */ NULL);
        return;
    }
    
    if (cell_index->row == 0) {
        GSize size = layer_get_frame(cell_layer).size;
        n_GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        uint8_t line_height = n_graphics_font_get_line_height(font);

        graphics_draw_text(
            ctx,
            "Clear All",
            font,
            GRect(0, (size.h - line_height - 2 * MENU_CELL_PADDING) / 2, size.w, line_height),
            n_GTextOverflowModeFill,
            GTextAlignmentCenter,
            NULL);

        return;
    }

    int noty_index = cell_index->row - 1;
    rebble_notification *noty = _noty_for_index(noty_index);

    if (!noty) {
        menu_cell_basic_draw(ctx, cell_layer, "Error", "Failed to load", NULL);
        return;
    }
    
    rebble_attribute *a;
    const char *sender = NULL;
    const char *subject = NULL;
    const char *message = NULL;
    
    list_foreach(a, &noty->attributes, rebble_attribute, node) {
        switch (a->timeline_attribute.attribute_id) {
        case TimelineAttributeType_Sender:  sender  = (const char *) a->data; break;
        case TimelineAttributeType_Subject: subject = (const char *) a->data; break;
        case TimelineAttributeType_Message: message = (const char *) a->data; break;
        default:
            /* we don't care */
            ;
        }
    }
    
    if (sender  && !strlen(sender )) sender  = NULL;
    if (subject && !strlen(subject)) subject = NULL;
    if (message && !strlen(message)) message = NULL;
    
    const char *title = NULL, *subtitle = NULL;
    if (sender) {
        title = sender;
        if (subject) {
            subtitle = subject;
        } else if (message) {
            subtitle = message;
        }
    } else if (subject) {
        title = subject;
        if (message) {
            subtitle = message;
        }
    } else if (message) {
        title = message;
    } else {
        title = "No title";
    }

    /* XXX: Retrieve an icon from flash by-app. */
    menu_cell_basic_draw(ctx, cell_layer, title, subtitle, NULL);
    
    /* And clean up. */
    timeline_destroy(noty);
    
    return;
}

static int16_t _notif_menu_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
    return 44; /* hardcoded for basic_draw: can we get another one somewhere? */
}

static void _notif_menu_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
    if (cell_index->row == 0) {
        _notif_clear_all();
        return;
    }

    int noty_index = cell_index->row - 1;

    /* Build the list for the notification window. */
    Uuid *uuids = malloc(sizeof(Uuid) * _notif_count);
    if (!uuids)
        return;
    
    struct rdb_select_result *res;
    int i = 0;
    rdb_select_result_foreach(res, &_notif_list) {
        uuids[_notif_count - i - 1] = *(Uuid *)(res->result[0]);
        i++;
    }

    notification_window_set_notifications(&_notifdetail_window, uuids, _notif_count, noty_index);
    
    free(uuids);

    window_stack_push(notification_window_get_window(&_notifdetail_window), false);
}

static void _notif_window_load(Window *window)
{
    s_main_window = window;
    Layer *window_layer = window_get_root_layer(window);
    MenuItems *items;
    
    _notif_window_status = status_bar_layer_create();
    status_bar_layer_set_colors(_notif_window_status, GColorBlack, GColorWhite);
    
#ifdef PBL_RECT
    s_menu_layer = menu_layer_create(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16));
#else
    // Let the menu draw behind the statusbar so it is perfectly centered
    s_menu_layer = menu_layer_create(GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS));
#endif

    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    menu_layer_set_highlight_colors(s_menu_layer, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack), GColorWhite);
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_rows = _notif_menu_get_num_rows,
        .draw_row = _notif_menu_draw_row,
        .get_cell_height = _notif_menu_get_cell_height,
        .select_click = _notif_menu_select_click,
        /* XXX: override back button, a la https://gist.github.com/sarfata/10574031 ... or just add a back-door API */
    });
    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
    layer_add_child(window_layer, status_bar_layer_get_layer(_notif_window_status));

    /* Load in the keys for all the notifications on the system. */
    list_init_head(&_notif_list);
    
    _notif_get_all();
}

static void _notif_window_unload(Window *window)
{
    rdb_select_free_all(&_notif_list);
    menu_layer_destroy(s_menu_layer);
    status_bar_layer_destroy(_notif_window_status);
}

void notif_main(void)
{
    notif_init();
    app_event_loop();
    notif_deinit();
}
