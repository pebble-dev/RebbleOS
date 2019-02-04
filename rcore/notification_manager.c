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

static void _notif_timeout_cb(void *data);
static void _notif_init(OverlayWindow *overlay_window);
static void _notification_window_creating(OverlayWindow *overlay_window, Window *window);
static void _notification_quit_click(ClickRecognizerRef _, void *context);
extern bool battery_overlay_visible(void);

uint8_t notification_init(void)
{
    messages_init();
    
    return 0;
}

void notification_show_message(full_msg_t *msg, uint32_t timeout_ms)
{
    message_add(msg);
    
    if (!message_count())
    {
        SYS_LOG("NOTY", APP_LOG_LEVEL_ERROR, "No Messages?");
        return;
    }

    notification_message *nmsg = noty_calloc(1, sizeof(notification_message));
    nmsg->data.create_callback = &notification_message_display;
    nmsg->message = msg;
    nmsg->data.timeout_ms = timeout_ms;
    nmsg->data.timer = 0;
    
    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
    
    SYS_LOG("NOTYM", APP_LOG_LEVEL_INFO, "Done");
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
    notification_battery *nmsg = noty_calloc(1, sizeof(notification_battery));
    nmsg->data.create_callback = &battery_overlay_display;
    nmsg->data.destroy_callback = &battery_overlay_destroy;
    nmsg->data.timeout_ms = timeout_ms;
    
    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_show_small_message(const char *message, GRect frame)
{
    notification_mini_msg *nmsg = noty_calloc(1, sizeof(notification_mini_msg));
    nmsg->data.create_callback = &mini_message_overlay_display;
    nmsg->data.destroy_callback = &mini_message_overlay_destroy;
    nmsg->data.timeout_ms = 5000;
    nmsg->message = (char *)message;
    nmsg->icon = 0;
    nmsg->frame = frame;
    /* get an overlay */
    overlay_window_create_with_context(_notification_window_creating, (void *)nmsg);
}

void notification_show_incoming_call(const char *caller)
{
    
}

void notification_show_alarm(uint8_t alarm_id)
{
    
}

void notification_window_dismiss()
{
}

/* window click config will call into here to apply the settings.
 *
 * XXX: note that this happens on the app thread, not the overlay thread! */
void notification_load_click_config(Window *app_window)
{
    if (!overlay_window_count() || overlay_window_stack_contains_window(app_window))
        return;
    
    /* if there are any windows that have their own click config, then it
     * will always get preference over any notification without a click config */
    Window *ov_wind = overlay_window_get_next_window_with_click_config();
    if (!ov_wind)
    {
        window_single_click_subscribe(BUTTON_ID_BACK, _notification_quit_click);
        Window *top_ov_window = overlay_window_stack_get_top_window();
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
    notification_message *nm = (notification_message *)context;
    
    if (!nm->data.overlay_window) {
        printf("notification window was already dead?\n");
        return;
    }
    
    if (nm->data.destroy_callback)
        nm->data.destroy_callback(nm->data.overlay_window, &nm->data.overlay_window->window);
    overlay_window_destroy(nm->data.overlay_window);
    nm->data.overlay_window = NULL;
    window_set_click_context(BUTTON_ID_BACK, NULL);
    
    /* We don't cancel the timer here (if there was one), because we're
     * running from the app thread, and the app timer lives on the overlay
     * thread.  So if it needs to, we just let it die off on its own, and
     * waste the wakeup later.  Oh, well.
     */
    SYS_LOG("NOTYM", APP_LOG_LEVEL_INFO, "DESTROY _notification_quit_click\n\n");
    window_dirty(true);
}

/* overlay thread */
static void _notification_window_creating(OverlayWindow *overlay_window, Window *window)
{
    SYS_LOG("NOTYM", APP_LOG_LEVEL_INFO, "Creating");
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
    notification_message *nm = (notification_message *)data;
    
    app_timer_cancel(nm->data.timer);
    nm->data.timer = 0;
    
    _notification_quit_click(NULL, data);
}
