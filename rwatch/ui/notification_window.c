/* notification_window.c
 * Renders a scrollable detailed list of notifications.
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "notification_window.h"
#include "timeline.h"

static void _notification_window_load(Window *window);
static void _notification_window_unload(Window *window);

void notification_window_ctor(NotificationWindow *w) {
    window_ctor(&w->window);
    GRect frame;
    frame.origin.x = 0;
    frame.origin.y = 0;
    frame.size.w = DISPLAY_COLS;
    frame.size.h = 2048;
    
    w->uuids = NULL;
    w->nuuids = 0;
    
    w->curnotif = 0;
    
    single_notification_layer_ctor(&w->n1, frame);
    single_notification_layer_ctor(&w->n2, frame);
    
    window_set_window_handlers(&w->window, (WindowHandlers) {
        .load = _notification_window_load,
        .unload = _notification_window_unload
    });
}

void notification_window_dtor(NotificationWindow *w) {
    window_dtor(&w->window);
    single_notification_layer_dtor(&w->n1);
    single_notification_layer_dtor(&w->n2);
}

void notification_window_set_notifications(NotificationWindow *w, Uuid *uuids, size_t count, size_t curnotif) {
    void *newuuids = realloc(w->uuids, count * sizeof(Uuid));
    if (!newuuids) {
        free(w->uuids);
        w->nuuids = 0;
        w->uuids = NULL;
        w->curnotif = (size_t) -1;
        return;
    }
    w->uuids = newuuids;
    
    memcpy(w->uuids, uuids, count * sizeof(Uuid));
    w->nuuids = count;
    
    if (curnotif >= count) {
        w->curnotif = (size_t) -1;
        return;
    }

    w->curnotif = curnotif;
    w->curnotif_scroll = 0;
    
    rebble_notification *notif = timeline_get_notification(w->uuids + curnotif);
    if (!notif) {
        w->curnotif = (size_t) -1;
        return;
    }
    single_notification_layer_set_notification(&w->n1, notif);
    timeline_destroy(notif);
    
    w->curnotif_height = single_notification_layer_height(&w->n1);
}

void notification_window_push_to_top(NotificationWindow *w, Uuid *uuid) {
    void *newuuids = realloc(w->uuids, (w->nuuids + 1) * sizeof(Uuid));
    if (!newuuids)
        return;
    w->uuids = newuuids;
    memmove(w->uuids + 1, w->uuids, w->nuuids * sizeof(Uuid));
    *w->uuids = *uuid;
    
    if (w->curnotif || w->curnotif_scroll) {
        w->curnotif++;
        return;
    }
    
    /* We're the top notification and haven't scrolled, so we reset the
     * notification to view the new one.  */
    w->curnotif = w->curnotif_scroll = 0;
    rebble_notification *notif = timeline_get_notification(w->uuids);
    if (!notif) {
        w->curnotif = (size_t) -1;
        return;
    }
    single_notification_layer_set_notification(&w->n1, notif);
    timeline_destroy(notif);

    w->curnotif_height = single_notification_layer_height(&w->n1);
}

Window *notification_window_get_window(NotificationWindow *w) {
    return &w->window;
}

static void _notification_window_load(Window *window) {
    NotificationWindow *w = container_of(window, NotificationWindow, window);
    Layer *window_layer = window_get_root_layer(window);
    
    layer_add_child(window_layer, single_notification_layer_get_layer(&w->n1));
}

static void _notification_window_unload(Window *window) {
}
