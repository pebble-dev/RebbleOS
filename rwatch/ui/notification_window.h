#pragma once
/* notification_window.h
 *
 * Displays notifications sent to the watch from the phone.
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"

typedef enum NotificationAction
{
    NotificationActionDismiss = 0,
    NotificationActionDismissAll = 1,
    NotificationActionReply = 2,
    NotificationActionCustom = 3
} NotificationAction;

typedef struct NotificationWindow NotificationWindow;

typedef struct Notification Notification;

struct Notification
{
    GBitmap *icon;
    const char *app_name;
    const char *title;
    const char *body;
    char *custom_actions;
    GColor color;
    
    // Doubly linked list
    Notification *next;
    Notification *previous;
};

struct NotificationWindow
{
    Window *window;
    int *offset;
    NotificationAction *actions;
    Notification *active;
};

Notification* notification_window_create(const char *app_name, const char *title, const char *body, GBitmap *icon, GColor color);

Window* notification_window_get_window(NotificationWindow *notification_window);

void notification_window_update_proc(Layer *layer, GContext *context);
