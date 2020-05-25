/* notification_window.h
 * Renders a scrollable detailed list of notifications.
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "uuid.h"
#include "window.h"
#include "single_notification_layer.h"

typedef struct NotificationWindow {
    Window   window;
    Uuid    *uuids;
    size_t   nuuids;
    
    size_t   curnotif;
    uint16_t curnotif_height;
    int16_t  curnotif_scroll;
    int      curnotif_nudging;
    
    SingleNotificationLayer n1;
    SingleNotificationLayer n2;
} NotificationWindow;

void notification_window_ctor(NotificationWindow *w);
void notification_window_dtor(NotificationWindow *w);
void notification_window_set_notifications(NotificationWindow *w, Uuid *uuids, size_t count, size_t curnotif);
void notification_window_push_to_top(NotificationWindow *w, Uuid *uuid);
Window *notification_window_get_window(NotificationWindow *w);
