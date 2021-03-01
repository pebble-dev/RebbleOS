/* notification_manager.c
 * Routines for loading notification in an overlay window
 * Each notification is loaded with its own stack and heap.
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "overlay_manager.h"
#include "platform_res.h"
#include "notification_message.h"
#include "protocol_service.h"
#include "event_service.h"
#include "protocol_call.h"
#include "notification_window.h"

/* Configure Logging */
#define MODULE_NAME "notym"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

static void _notif_timeout_cb(void *data);
static void _notif_init(OverlayWindow *overlay_window);
static void _notification_window_creating(OverlayWindow *overlay_window, Window *window);
static void _notification_quit_click(ClickRecognizerRef _, void *context);
extern bool battery_overlay_visible(void);
extern bool call_window_visible(void);
extern bool progress_window_visible(void);
extern void progress_window_update_arrived(notification_progress *progress);
extern void call_window_message_arrived(rebble_phone_message *call);

/*
 * 
 * 
 * 
 */


uint8_t notification_init(void)
{
    return 0;
}

static bool _notif_window_visible = false;
static NotificationWindow _notif_window;

static void _notif_window_click_config(void *context) {
    Window *window = (Window *)context;
    
}

static void _notif_window_create(OverlayWindow *overlay, Window *window) {
    notification_window_ctor(&_notif_window, window);
    
    notification_message *msg = overlay->context;
    notification_window_set_notifications(&_notif_window, (Uuid *)msg->uuid, 1, 0);
    notification_window_set_click_config(&_notif_window, (ClickConfigProvider)notification_load_click_config, window);
    
    _notif_window_visible = true;
}

static void _notif_window_destroy(OverlayWindow *overlay, Window *window) {
    notification_window_dtor(&_notif_window);
    free(overlay->context);
    _notif_window_visible = false;
}

void notification_arrived(EventServiceCommand command, void *data, void *context)
{
    Uuid *uuid = (Uuid *)data;
    if (!uuid)
        return;
   
    if (_notif_window_visible)
    {
        assert(appmanager_get_current_thread()->thread_type == AppThreadOverlay);
        notification_window_push_to_top(&_notif_window, uuid);
        
        notification_message *nmsg = (notification_message *) notification_window_get_window(&_notif_window)->context;
        if (nmsg->data.timer)
            app_timer_reschedule(nmsg->data.timer, 15000);
        return;
    }

    rebble_notification *rn;

    notification_message *nmsg = app_calloc(1, sizeof(notification_message));
    nmsg->data.create_callback = &_notif_window_create;
    nmsg->data.destroy_callback = &_notif_window_destroy;
    nmsg->uuid = app_calloc(1, sizeof(Uuid));
    memcpy(nmsg->uuid, uuid, sizeof(Uuid));
    nmsg->data.timeout_ms = 15000;
    nmsg->data.timer = 0;
    rcore_backlight_on(100, 3000);
    
    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_show_battery(uint32_t timeout_ms)
{
    if (battery_overlay_visible())
    {
        /* if we are already visible, just request a redraw */
        window_dirty(true);
        return;
    }
    
    /* construct a BatteryLayer */
    notification_battery *nmsg = app_calloc(1, sizeof(notification_battery));
    nmsg->data.create_callback = &battery_overlay_display;
    nmsg->data.destroy_callback = &battery_overlay_destroy;
    nmsg->data.timeout_ms = timeout_ms;

    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_show_small_message(EventServiceCommand command, void *data, void *context)
{
    GRect frame = GRect(0, DISPLAY_ROWS - 20, DISPLAY_COLS, 20);
    notification_mini_msg *nmsg = app_calloc(1, sizeof(notification_mini_msg));
    nmsg->data.create_callback = &mini_message_overlay_display;
    nmsg->data.destroy_callback = &mini_message_overlay_destroy;
    nmsg->data.timeout_ms = 10000;
    nmsg->message = data;
    nmsg->icon = 0;
    nmsg->frame = frame;

    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_show_incoming_call(EventServiceCommand command, void *data, void *context)
{
    RebblePacket packet = (RebblePacket)data;
    if (!packet)
        return;
       
    rebble_phone_message *msg = protocol_phone_create(packet_get_data(packet), packet_get_data_length(packet));
    
    if (call_window_visible())
    {
        call_window_message_arrived((void *)msg);
        protocol_phone_destroy(msg);
        rcore_backlight_on(100, 1000);
        return;
    }

    notification_call *nmsg = app_calloc(1, sizeof(notification_call));
    
    nmsg->data.create_callback = &call_window_overlay_display;
    nmsg->data.destroy_callback = &call_window_overlay_destroy;
    nmsg->data.timeout_ms = 30000;
    nmsg->phone_call = msg;
    nmsg->frame = GRect(10, 15, DISPLAY_COLS - 40, 80);
    
    rcore_backlight_on(100, 3000);
    
    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}


void notification_show_alarm(uint8_t alarm_id)
{
    
}

void notification_window_dismiss()
{
}

static void _notification_quit_from_button(ClickRecognizerRef ref, void *context)
{
    LOG_ERROR("notification ");
    notification_data *nm = (notification_data*)context;
    app_timer_cancel(nm->timer);
    _notification_quit_click(ref, context);
}

/* window click config will call into here to apply the settings.
 */
void notification_load_click_config(Window *app_window)
{
    window_single_click_subscribe(BUTTON_ID_BACK, _notification_quit_from_button);
    OverlayWindow *top_ov_window = (OverlayWindow *)app_window;
    window_set_click_context(BUTTON_ID_BACK, top_ov_window->context);
}

static void _notification_quit_click(ClickRecognizerRef _, void *context)
{
    notification_data *nm = (notification_data*)context;

    if (!nm->overlay_window) {
        LOG_ERROR("notification window was already dead?");
        return;
    }
    
    if (nm->destroy_callback)
        nm->destroy_callback(nm->overlay_window, &nm->overlay_window->window);

    overlay_window_destroy(nm->overlay_window);
    nm->overlay_window = NULL;

    window_set_click_context(BUTTON_ID_BACK, NULL);
    LOG_DEBUG("DESTROY _notification_quit_click");
    window_dirty(true);
    LOG_DEBUG("FREE: %d", app_heap_bytes_free());
}

void notification_reschedule_timer(Window *window, uint32_t timeout_ms)
{
    LOG_INFO("Reschedule Timer");
    notification_message *nm = (notification_message *)window->context;
    app_timer_reschedule(nm->data.timer, timeout_ms);
}


/* overlay thread */
static void _notification_window_creating(OverlayWindow *overlay_window, Window *window)
{
    LOG_INFO("Creating Window");
    notification_message *nm = (notification_message *)overlay_window->context;
    nm->data.overlay_window = overlay_window;

    /* the overlay context has the message data 
     * lets take it with us */
    window->context = overlay_window->context;
    
    /* call the function that we should execute */
    if (nm->data.create_callback)
        nm->data.create_callback(overlay_window, &overlay_window->window);

    if (nm->data.timer)
        app_timer_cancel(nm->data.timer);
    nm->data.timer = 0;
    if (nm->data.timeout_ms)
        nm->data.timer = app_timer_register(nm->data.timeout_ms, 
                                            (AppTimerCallback)_notif_timeout_cb, nm);

    overlay_window_stack_push(overlay_window, false);
}


static void _notif_timeout_cb(void *data)
{
    LOG_INFO("Notification window timed out");

    _notification_quit_click(NULL, data);
}

void notification_show_progress(EventServiceCommand command, void *data, void *context)
{
    if (progress_window_visible())
    {
        progress_window_update_arrived((void *)data);
        appmanager_post_draw_message(1);
        return;
    }
    notification_progress *np = (notification_progress *)data;
    notification_progress *nmsg = app_calloc(1, sizeof(notification_progress));    
    
    nmsg->data.create_callback = &progress_window_overlay_display;
    nmsg->data.destroy_callback = &progress_window_overlay_destroy;
    nmsg->data.timeout_ms = 30000;
    nmsg->progress_bytes = np->progress_bytes;
    nmsg->total_bytes = np->total_bytes;
    
    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

int notifications_get_all(rdb_select_result_list *notif_list)
{
    struct rdb_database *db = rdb_open(RDB_ID_NOTIFICATION);
    struct rdb_iter it;

    int notif_count = 0;

    if (rdb_iter_start(db, &it)) {
        struct rdb_selector selectors[] = {
            { offsetof(timeline_item, uuid), FIELD_SIZEOF(timeline_item, uuid), RDB_OP_RESULT },
            { }
        };
        notif_count = rdb_select(&it, notif_list, selectors);
    }

    rdb_close(db);

    return notif_count;
}

void notifications_dismiss_all(rdb_select_result_list *notif_list)
{
    struct rdb_database *db = rdb_open(RDB_ID_NOTIFICATION);
    struct rdb_select_result *res;

    rdb_select_result_foreach(res, notif_list) {
        rdb_delete(&res->it);
    }

    rdb_close(db);

    rdb_select_free_all(notif_list);
    list_init_head(notif_list);
}