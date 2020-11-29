/* appmanager_app_timers.c
 * The main runloop and runloop handlers for the main foregound App
 * RebbleOS
 * 
 * Author: Joshua Wise <joshua@joshuawise.com>
 *         Barry Carter <barry.carter@gmail.com>.
 */

#include <stdlib.h>
#include "display.h"
#include "appmanager.h"
#include "main.h"
#include "overlay_manager.h"
#include "notification_manager.h"

void timer_init(void)
{

}

/* Timer util */
TickType_t appmanager_timer_get_next_expiry(CoreTimer *timer_head)
{
    TickType_t next_timer;

    if (timer_head) {
        TickType_t curtime = xTaskGetTickCount();
        if (curtime > timer_head->when) {
            next_timer = 0;
        }
        else
            next_timer = timer_head->when - curtime;
    } else {
        next_timer = -1; /* Just block forever. */
    }

    return next_timer;
}

void appmanager_timer_expired(CoreTimer **timer_head, CoreTimer *timer)
{
    /* We woke up because we hit a timer expiry.  Dequeue first,
     * then invoke -- otherwise someone else could insert themselves
     * at the head, and we would wrongfully dequeue them!  */
    assert(timer);

    if (!timer->callback) {
        /* assert(!"BAD"); // actually this is pretty bad. I've seen this 
         * happen only once before when the app draw was happening while the
         * ovelay thread was coming up. The ov thread memory was memset to 0. */
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "Bad Callback!");
        *timer_head = timer->next;
        return;
    }

    *timer_head = timer->next;
    timer->callback(timer);
}

/* 
 * Always adds to the running app's queue.  Note that this is only
 * reasonable to do from the app thread: otherwise, you can race with the
 * check for the timer head.
 */
void appmanager_timer_add(CoreTimer **timer_head, CoreTimer *timer)
{
    CoreTimer **tnext = timer_head;

    /* until either the next pointer is null (i.e., we have hit the end of
     * the list), or the thing that the next pointer points to is further in
     * the future than we are (i.e., we want to insert before the thing that
     * the next pointer points to)
     */
    while (*tnext && (timer->when > (*tnext)->when)) {
        tnext = &((*tnext)->next);
    }

    timer->next = *tnext;
    *tnext = timer;
}

void appmanager_timer_remove(CoreTimer **timer_head, CoreTimer *timer)
{
    CoreTimer **tnext = timer_head;

    while (*tnext) {
        if (*tnext == timer) {
            *tnext = timer->next;
            return;
        }
        tnext = &(*tnext)->next;
    }

    assert(!"appmanager_timer_remove did not find timer in list");
}

