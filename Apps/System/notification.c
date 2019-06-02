/* notification.c
 * An app for displaying notification messages
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 *         Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "notification.h"
#include "notification_layer.h"
#include "librebble.h"
#include "bitmap_layer.h"
#include "action_bar_layer.h"
#include "platform_res.h"
#include "menu.h"
#include "timeline.h"
#include "notification_manager.h"
#include "blob_db.h"

static NotificationLayer* _notif_layer;
static Window* _notif_window;
static Menu *s_menu;
static Window *s_main_window;

static void _notif_window_load(Window *window);
static void _notif_window_unload(Window *window);
static void _exit_to_watchface(struct Menu *menu, void *context);
static void _notif_destroy_layer_cb(ClickRecognizerRef _, void *context);

void notif_init(void)
{
    _notif_window = window_create();

    window_set_window_handlers(_notif_window, (WindowHandlers) {
        .load = _notif_window_load,
        .unload = _notif_window_unload,
    });

    window_stack_push(_notif_window, true);
}

static MenuItems* _msg_list_item_selected(const MenuItem *item)
{
    char *app = "RebbleOS";
    char *title = "Message";
    rebble_notification *msg = (rebble_notification *)item->context;

    rebble_attribute *attr = list_elem(list_get_head(&msg->attributes), rebble_attribute, node);

    Layer *layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(layer);
    _notif_layer = notification_layer_create(bounds);
//     Notification *notification = notification_create(app, title, (const char *)attr->data, gbitmap_create_with_resource(RESOURCE_ID_SPEECH_BUBBLE), GColorRed);
    
//     notification_layer_stack_push_notification(_notif_layer, notification);
    notification_layer_configure_click_config(_notif_layer, s_main_window, _notif_destroy_layer_cb);
    layer_add_child(layer, notification_layer_get_layer(_notif_layer));
        
    layer_mark_dirty(layer);
    window_dirty(true);
    
    return NULL;
}

static void _notif_destroy_layer_cb(ClickRecognizerRef _, void *context)
{
    notification_layer_destroy(_notif_layer);
    menu_set_click_config_onto_window(s_menu, s_main_window);
    window_load_click_config(s_main_window);
}

static void _notif_window_load(Window *window)
{
    s_main_window = window;
    Layer *window_layer = window_get_root_layer(window);
    MenuItems *items;
    
#ifdef PBL_RECT
    s_menu = menu_create(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16));
#else
    // Let the menu draw behind the statusbar so it is perfectly centered
    s_menu = menu_create(GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS));
#endif
    menu_set_callbacks(s_menu, s_menu, (MenuCallbacks) {
        .on_menu_exit = _exit_to_watchface
    });
    layer_add_child(window_layer, menu_get_layer(s_menu));

    menu_set_click_config_onto_window(s_menu, window);

    list_head *lh = blobdb_select_items_all(BlobDatabaseID_Notification, 
                            offsetof(timeline_item, uuid), FIELD_SIZEOF(timeline_item, uuid),
                            0,0);

    uint8_t msg_count = 0;
    blobdb_result_set *rs;

    list_foreach(rs, lh, blobdb_result_set, node)
        msg_count++;

    if (!msg_count)
    {
        items = menu_items_create(1);
        menu_items_add(items, MenuItem("No Messages", NULL, RESOURCE_ID_SPEECH_BUBBLE, NULL));
        menu_set_items(s_menu, items);
        return;
    }

    items = menu_items_create(msg_count);
//     rebble_notification *msg;
//     cmd_phone_attribute_t *a;
//     
//     
//     list_foreach(msg, message_head, rebble_notification, node)
//     {
//         list_foreach(a, &msg->attributes_list_head, cmd_phone_attribute_t, node)
//         {
//             MenuItem mi = MenuItem((char *)a->data, NULL, RESOURCE_ID_SPEECH_BUBBLE, _msg_list_item_selected);
//             mi.context = msg;
//             menu_items_add(items, mi);
//         }
//     }        
    menu_set_items(s_menu, items);

    
    return;
    
}

static void _notif_window_unload(Window *window)
{
    if (_notif_layer)
    {
        notification_layer_destroy(_notif_layer);
        _notif_layer = NULL;
    }
}

void notif_deinit(void)
{
//     notification_window_destroy(notif_window);
    window_destroy(_notif_window);
}

void notif_main(void)
{
    notif_init();
    app_event_loop();
    notif_deinit();
}

static void _exit_to_watchface(struct Menu *menu, void *context)
{
    // Exit to watchface
    appmanager_app_start("Simple");
}
