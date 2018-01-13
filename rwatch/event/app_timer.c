/* app_timer.c
 * implementation of PebbleOS app timer service
 * libRebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "app_timer.h"
#include "appmanager.h"
#include "task.h"
#include "rebble_memory.h"

/* XXX: See animation.c comment for the memory allocation story here. */

struct AppTimer {
    CoreTimer timer;
    AppTimerCallback cb;
    void *priv;
    int scheduled;
};

void _app_timer_callback(CoreTimer *_timer)
{
    AppTimer *timer = (AppTimer *)_timer;
    
    /* If we had a real "handle" system, then we could free it here, and the
     * handle could simply be dead (at least, until reused).  */
    timer->scheduled = 0;
    
    timer->cb(timer->priv);
}

AppTimer *app_timer_register(uint32_t ms, AppTimerCallback cb, void *priv)
{
    AppTimer *timer = app_calloc(1, sizeof(*timer));
    
    if (!timer)
        return NULL;
    
    timer->timer.when = xTaskGetTickCount() + pdMS_TO_TICKS(ms);
    timer->timer.callback = _app_timer_callback;
    timer->cb = cb;
    timer->priv = priv;
    timer->scheduled = 1;
    appmanager_timer_add(&timer->timer);
    
    return timer;
}

bool app_timer_reschedule(AppTimer *timer, uint32_t ms)
{
    if (!timer->scheduled)
        return false;
    
    appmanager_timer_remove(&timer->timer);
    timer->timer.when = xTaskGetTickCount() + pdMS_TO_TICKS(ms);
    appmanager_timer_add(&timer->timer);
    
    return true;
}

void app_timer_cancel(AppTimer *timer)
{
    if (timer->scheduled)
        appmanager_timer_remove(&timer->timer);
    app_free(timer);
}
