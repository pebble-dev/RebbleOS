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

/* Configure Logging */
#define MODULE_NAME "notym"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

static void _notif_timeout_cb(void *data);
static void _notif_init(OverlayWindow *overlay_window);
static void _notification_window_creating(OverlayWindow *overlay_window, Window *window);
static void _notification_quit_click(ClickRecognizerRef _, void *context);
extern bool battery_overlay_visible(void);
extern bool notification_window_overlay_visible(void);
extern NotificationLayer *notification_window_get_layer(void);
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

void notification_arrived(EventServiceCommand command, void *data, void *context)
{
    Uuid *uuid = (Uuid *)data;
    if (!uuid)
        return;
   
    if (notification_window_overlay_visible())
    {
        /* Free as much memory as we can for the message conversion */
        notification_layer_message_arrived(notification_window_get_layer(), uuid);
        return;
    }

    rebble_notification *rn;

    notification_message *nmsg = app_calloc(1, sizeof(notification_message));
    nmsg->data.create_callback = &notification_message_display;
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

