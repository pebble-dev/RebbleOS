#pragma once
/* window.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"

typedef struct NotificationWindow
{
    Window *window;
    GBitmap *icon;
    const char *app_name;
    const char *title;
    const char *body;
    int *offset;
} NotificationWindow;

NotificationWindow* notification_window_create(const char *app_name, const char *title, const char *body, GBitmap *icon);

Window* notification_window_get_window(NotificationWindow *notification_window);

void notification_window_update_proc(Layer *layer, GContext *context);
