#pragma once
/**
 * @file notification_manager_c.h
 * @author Barry Carter
 * @date 12 Feb 2018
 * @brief Notification Windows. Allows you to pop a structured notification over content
 *
 * A notification window will create an \ref OverlayWindow and 
 * inject the relevant subcontent. 
 */
#include "overlay_manager.h"
#include "protocol_notification.h"
#include "protocol_call.h"
#include "event_service.h"
#include "notification_message.h"
#include "timeline.h"

/**
 * @brief When we are in a notification and a button is pressed, 
 * how long to rearm the timer for. it has to go away eventually 
 */
#define REARM_TIMEOUT_MS 30000

/* internal init */
uint8_t notification_init(void);

/**
 * @brief Show a fullscreen message. Or if already visible, show the new message
 * 
 */
void notification_arrived(EventServiceCommand command, void *data, void *context);

/**
 * @brief Show a battery overlay. Shows current battery status
 * 
 * @param timeout_ms Time in ms before we auto close the window. 0 is disabled
 */
void notification_show_battery(uint32_t timeout_ms);
void notification_show_incoming_call(EventServiceCommand command, void *data, void *context);
void notification_show_small_message(EventServiceCommand command, void *data, void *context);
void notification_show_alarm(uint8_t alarm_id);
void notification_show_progress(EventServiceCommand command, void *data, void *context);
void notification_window_dismiss();

/**
 * @brief Apply the click configuraion to the notifcation window.
 * 
 * @param app_window the running application \ref Window that we want to override
 */
void notification_load_click_config(Window *app_window);

/* Internal implementation */
void notification_message_display(OverlayWindow *overlay, Window *window);
void notification_message_destroy(OverlayWindow *overlay, Window *window);
void battery_overlay_display(OverlayWindow *overlay, Window *window);
void battery_overlay_destroy(OverlayWindow *overlay, Window *window);
void mini_message_overlay_display(OverlayWindow *overlay, Window *window);
void mini_message_overlay_destroy(OverlayWindow *overlay, Window *window);
void notification_reschedule_timer(Window *window, uint32_t timeout_ms);
void call_window_overlay_display(OverlayWindow *overlay, Window *window);
void call_window_overlay_destroy(OverlayWindow *overlay, Window *window);
void progress_window_overlay_display(OverlayWindow *overlay, Window *window);
void progress_window_overlay_destroy(OverlayWindow *overlay, Window *window);


typedef struct notification_data_t {
    OverlayCreateCallback create_callback;
    OverlayCreateCallback destroy_callback;
    OverlayWindow *overlay_window;
    AppTimerHandle timer;
    uint32_t timeout_ms;
} notification_data;

typedef struct notification_message_t {
    notification_data data;
    Uuid *uuid;
} notification_message;

typedef struct notification_battery_t {
    notification_data data;
} notification_battery;

typedef struct notification_mini_msg_t {
    notification_data data;
    char *message;
    uint16_t icon;
    GRect frame;
} notification_mini_msg;

typedef struct notification_call_t {
    notification_data data;
    rebble_phone_message *phone_call;
    GRect frame;
} notification_call;

typedef struct notification_progress_t {
    notification_data data;
    uint32_t progress_bytes;
    uint32_t total_bytes;
} notification_progress;
