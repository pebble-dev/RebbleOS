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

#ifndef PBL_RECT
#  error notification_window not implemented on round Pebble
#endif

#define VIEWPORT_HEIGHT DISPLAY_ROWS
#define NUDGE_HEIGHT 40
#define SCROLL_INCR 20

void notification_window_ctor(NotificationWindow *w, Window *win) {
    w->window = win;
    win->user_data = w; /* we use this because the noty / overlay subsystem uses context */

    GRect frame;
    frame.origin.x = 0;
    frame.origin.y = 0;
    frame.size.w = DISPLAY_COLS;
    frame.size.h = 2048;
    
    w->uuids = NULL;
    w->nuuids = 0;
    
    w->curnotif = (size_t) -1;
    w->curnotif_nudging = 0;
    
    single_notification_layer_ctor(&w->n1, frame);
    single_notification_layer_ctor(&w->n2, frame);
    
    window_set_window_handlers(w->window, (WindowHandlers) {
        .load = _notification_window_load,
        .unload = _notification_window_unload
    });
}

void notification_window_dtor(NotificationWindow *w) {
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

    /* Reset everything first. */
    GRect frame = layer_get_frame(single_notification_layer_get_layer(&w->n1));
    frame.origin.y = 0;
    layer_set_frame(single_notification_layer_get_layer(&w->n1), frame);
    w->curnotif_scroll = 0;

    if (w->curnotif_nudging)
        layer_remove_from_parent(single_notification_layer_get_layer(&w->n2));
    w->curnotif_nudging = 0;

    w->curnotif = curnotif;
    
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
#if 0
    void *newuuids = realloc(w->uuids, (w->nuuids + 1) * sizeof(Uuid));
    if (!newuuids)
        return;
    w->uuids = newuuids;
    memmove(w->uuids + 1, w->uuids, w->nuuids * sizeof(Uuid));
#else
    Uuid *newuuids = malloc((w->nuuids + 1) * sizeof(Uuid));
    if (!newuuids)
        return;
    memcpy(newuuids + 1, w->uuids, w->nuuids * sizeof(Uuid));
    free(w->uuids);
    w->uuids = newuuids;
#endif
    *w->uuids = *uuid;
    w->nuuids++;
    
    if (w->curnotif || w->curnotif_scroll) {
        w->curnotif++;
        return;
    }
    
    /* We're the top notification and haven't scrolled, so we reset the
     * notification to view the new one.  */
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
    return w->window;
}

static void _down_single_click_handler(ClickRecognizerRef _, void *_w) {
    NotificationWindow *w = _w;
    
    w->curnotif_scroll += SCROLL_INCR;

    if (w->curnotif == -1)
        return;
    
    int16_t curnotif_maxscroll = w->curnotif_height - VIEWPORT_HEIGHT;
    if (curnotif_maxscroll < 0)
        curnotif_maxscroll = 0;
    int16_t curnotif_nudge     = curnotif_maxscroll + NUDGE_HEIGHT;
    if (w->curnotif_nudging) {
        /* We are nudging and moving still scrolling down -- swap the
         * notifications to the next.  We can only be nudging if there is a
         * next notification, so we're good on that front.  */
        w->curnotif++;
        w->curnotif_scroll = 0;
        w->curnotif_nudging = 0;
        
        rebble_notification *notif = timeline_get_notification(w->uuids + w->curnotif);
        if (!notif) {
            w->curnotif = (size_t) -1;
            return;
        }
        single_notification_layer_set_notification(&w->n1, notif);
        timeline_destroy(notif);

        layer_remove_from_parent(single_notification_layer_get_layer(&w->n2));
        
        GRect frame = layer_get_frame(single_notification_layer_get_layer(&w->n1));
        frame.origin.y = 0;
        layer_set_frame(single_notification_layer_get_layer(&w->n1), frame);
        
        w->curnotif_height = single_notification_layer_height(&w->n1);
        
    } else if ((w->curnotif_scroll > curnotif_maxscroll) && !w->curnotif_nudging && ((w->curnotif + 1) < w->nuuids)) {
        /* We're moving down and about to run out, and there's another
         * notification ready.  Load it into the nudge box at the bottom. 
         */
        w->curnotif_scroll = curnotif_nudge;
        w->curnotif_nudging = 1;
            
        /* Load the next one in place. */
        rebble_notification *notif = timeline_get_notification(w->uuids + w->curnotif + 1);
        if (!notif) {
            w->curnotif = (size_t) -1;
            return;
        }
        single_notification_layer_set_notification(&w->n2, notif);
        timeline_destroy(notif);

        GRect frame = layer_get_frame(single_notification_layer_get_layer(&w->n2));
        frame.origin.y = VIEWPORT_HEIGHT - NUDGE_HEIGHT;
        layer_set_frame(single_notification_layer_get_layer(&w->n2), frame);

        layer_add_child(window_get_root_layer(w->window), single_notification_layer_get_layer(&w->n2));
        
        frame = layer_get_frame(single_notification_layer_get_layer(&w->n1));
        frame.origin.y = -curnotif_nudge;
        layer_set_frame(single_notification_layer_get_layer(&w->n1), frame);

    } else {
        if (w->curnotif_scroll > curnotif_maxscroll)
            w->curnotif_scroll = curnotif_maxscroll;
        
        GRect frame = layer_get_frame(single_notification_layer_get_layer(&w->n1));
        frame.origin.y = -w->curnotif_scroll;
        layer_set_frame(single_notification_layer_get_layer(&w->n1), frame);
    }
}

static void _up_single_click_handler(ClickRecognizerRef _, void *_w) {
    NotificationWindow *w = _w;
    
    int16_t curnotif_maxscroll = w->curnotif_height - VIEWPORT_HEIGHT;
    if (curnotif_maxscroll < 0)
        curnotif_maxscroll = 0;
    
    if (w->curnotif == (size_t) -1)
        return;
    
    w->curnotif_scroll -= SCROLL_INCR;
    
    if (w->curnotif_nudging) {
        /* Un-nudge. */
        w->curnotif_scroll = curnotif_maxscroll;
        GRect frame = layer_get_frame(single_notification_layer_get_layer(&w->n1));
        frame.origin.y = -w->curnotif_scroll;
        layer_set_frame(single_notification_layer_get_layer(&w->n1), frame);
        
        w->curnotif_nudging = 0;
        layer_remove_from_parent(single_notification_layer_get_layer(&w->n2));

    } else if (w->curnotif_scroll == -SCROLL_INCR && w->curnotif > 0) {
        /* Kick ourselves to the bottom of the screen, make the previous one
         * active.  Load ourselves first.  */
        rebble_notification *notif = timeline_get_notification(w->uuids + w->curnotif);
        if (!notif) {
            w->curnotif = (size_t) -1;
            return;
        }
        single_notification_layer_set_notification(&w->n2, notif);
        timeline_destroy(notif);
        
        GRect frame = layer_get_frame(single_notification_layer_get_layer(&w->n2));
        frame.origin.y = VIEWPORT_HEIGHT - NUDGE_HEIGHT;
        layer_set_frame(single_notification_layer_get_layer(&w->n2), frame);
        
        layer_add_child(window_get_root_layer(w->window), single_notification_layer_get_layer(&w->n2));
        w->curnotif_nudging = 1;
        
        /* Now load the previous one. */
        w->curnotif--;
        notif = timeline_get_notification(w->uuids + w->curnotif);
        if (!notif) {
            w->curnotif = (size_t) -1;
            return;
        }
        single_notification_layer_set_notification(&w->n1, notif);
        timeline_destroy(notif);

        w->curnotif_height = single_notification_layer_height(&w->n1);
        curnotif_maxscroll = w->curnotif_height - VIEWPORT_HEIGHT;
        if (curnotif_maxscroll < 0)
            curnotif_maxscroll = 0;
        w->curnotif_scroll = curnotif_maxscroll + NUDGE_HEIGHT;
        
        frame = layer_get_frame(single_notification_layer_get_layer(&w->n1));
        frame.origin.y = -w->curnotif_scroll;
        layer_set_frame(single_notification_layer_get_layer(&w->n1), frame);

    } else {
        /* Normal scroll. */
        if (w->curnotif_scroll < 0)
            w->curnotif_scroll = 0;
        
        GRect frame = layer_get_frame(single_notification_layer_get_layer(&w->n1));
        frame.origin.y = -w->curnotif_scroll;
        layer_set_frame(single_notification_layer_get_layer(&w->n1), frame);
    }
}

static void _notification_window_click_config_provider(NotificationWindow *w) {
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 500, _down_single_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP  , 500, _up_single_click_handler  );
    window_set_click_context(BUTTON_ID_DOWN, w);
    window_set_click_context(BUTTON_ID_UP  , w);
    
    if (w->clickconfig)
        w->clickconfig(w->clickconfigcontext);
}

void notification_window_set_click_config(NotificationWindow *w, ClickConfigProvider config, void *context) {
    w->clickconfig = config;
    w->clickconfigcontext = context;
}

static void _notification_window_load(Window *window) {
    NotificationWindow *w = window->user_data;
    Layer *window_layer = window_get_root_layer(window);
    
    layer_add_child(window_layer, single_notification_layer_get_layer(&w->n1));
    
    window_set_click_config_provider_with_context(window, (ClickConfigProvider) _notification_window_click_config_provider, w);
}

static void _notification_window_unload(Window *window) {
}
