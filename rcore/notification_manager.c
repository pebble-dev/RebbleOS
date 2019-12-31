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
#include "blob_db.h"
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

/*
 * 
 * 
 * 
 */


uint8_t notification_init(void)
{

    return 0;
}

void _notification_show_message(void *context)
{
    if (!context)
        return;
   
    if (notification_window_overlay_visible())
    {
        /* Free as much memory as we can for the message conversion */
        notification_layer_message_arrived(notification_window_get_layer(), (Uuid *)context);
        return;
    }

    rebble_notification *rn;

    notification_message *nmsg = app_calloc(1, sizeof(notification_message));
    nmsg->data.create_callback = &notification_message_display;
    nmsg->uuid = (Uuid *)context;
    nmsg->data.timeout_ms = 3000;
    nmsg->data.timer = 0;

    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_arrived(Uuid *uuid)
{
    Uuid *uid = app_calloc(1, sizeof(Uuid));
    if (!uid)
        return;
    memcpy(uid, uuid, sizeof(Uuid));
    overlay_window_post_create_notification(_notification_show_message, (void *)uid);
}

void _notification_show_battery(void *context)
{
    /* construct a BatteryLayer */
    notification_battery *nmsg = app_calloc(1, sizeof(notification_battery));
    nmsg->data.create_callback = &battery_overlay_display;
    nmsg->data.destroy_callback = &battery_overlay_destroy;
    nmsg->data.timeout_ms = *(uint32_t *)context;

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
    overlay_window_post_create_notification(_notification_show_battery, (void *)timeout_ms);
}

void _notification_show_small_message(void *context)
{
    GRect frame = GRect(0, DISPLAY_ROWS - 20, DISPLAY_COLS, 20);
    notification_mini_msg *nmsg = app_calloc(1, sizeof(notification_mini_msg));
    nmsg->data.create_callback = &mini_message_overlay_display;
    nmsg->data.destroy_callback = &mini_message_overlay_destroy;
    nmsg->data.timeout_ms = 10000;
    nmsg->message = (char *)context;
    nmsg->icon = 0;
    nmsg->frame = frame;

    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_show_small_message(const char *message, GRect frame)
{
    overlay_window_post_create_notification(_notification_show_small_message, (void *)message);
}


void _notification_show_call(void *context)
{
    if (!context)
        return;
       
    RebblePacket packet = (RebblePacket)context;
    rebble_phone_message *msg = protocol_phone_create(packet_get_data(packet), packet_get_data_length(packet));
    
    if (call_window_visible())
    {
        call_window_message_arrived((void *)msg);
        packet_destroy(context);
        protocol_phone_destroy(msg);
        return;
    }

    notification_call *nmsg = app_calloc(1, sizeof(notification_call));
    
    nmsg->data.create_callback = &call_window_overlay_display;
    nmsg->data.destroy_callback = &call_window_overlay_destroy;
    nmsg->data.timeout_ms = 30000;
    nmsg->phone_call = msg;
    nmsg->frame = GRect(10, 15, DISPLAY_COLS - 40, 80);

    packet_destroy(context);
    
    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_show_incoming_call(RebblePacket packet)
{
    overlay_window_post_create_notification(_notification_show_call, (void *)packet);
}

void notification_show_alarm(uint8_t alarm_id)
{
    
}

void notification_window_dismiss()
{
}

/* window click config will call into here to apply the settings.
 */
void notification_load_click_config(Window *app_window)
{
    /* if there are any windows that have their own click config, then it
     * will always get preference over any notification without a click config */
    Window *ov_wind = overlay_window_get_next_window_with_click_config();
    if (!ov_wind)
    {
        window_single_click_subscribe(BUTTON_ID_BACK, _notification_quit_click);
        OverlayWindow *top_ov_window = (OverlayWindow *)app_window;
        window_set_click_context(BUTTON_ID_BACK, top_ov_window->context);
        return;
    }
    window_load_click_config(ov_wind);
}

/* XXX: this could happen on *either* the app thread *or* the overlay
 * thread, depending on how we get called.  If we come in from the click
 * recognizer, then we end up on the app thread; if we come in from the
 * timer callback, then we end up on the overlay thread.  Ick, but oh well.
 */
static void _notification_quit_click(ClickRecognizerRef _, void *context)
{
    notification_data *nm = (notification_data*)context;

    if (!nm->overlay_window) {
        LOG_ERROR("notification window was already dead?");
        return;
    }

    app_timer_cancel(nm->timer);
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
