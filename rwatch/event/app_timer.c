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
#include "node_list.h"


/* XXX: See animation.c comment for the memory allocation story here. */

struct AppTimer {
    CoreTimer timer;
    AppTimerCallback cb;
    void *priv;
    uint8_t scheduled;
    AppTimerHandle id;
};

uint16_t _app_timer_next_free_id(void);
AppTimer *_app_timer_get_by_id(AppTimerHandle id);

void _app_timer_callback(CoreTimer *_timer)
{
    AppTimer *timer = (AppTimer *)_timer;
    
    /* If we had a real "handle" system, then we could free it here, and the
     * handle could simply be dead (at least, until reused).  */
    timer->scheduled = 0;
    
    timer->cb(timer->priv);
    
    app_free(timer);
}


AppTimerHandle app_timer_register(uint32_t ms, AppTimerCallback cb, void *priv)
{
    AppTimer *timer = app_calloc(1, sizeof(AppTimer));
    
    if (!timer)
        return 0;
   
    timer->timer.when = xTaskGetTickCount() + pdMS_TO_TICKS(ms);
    timer->timer.callback = _app_timer_callback;
    timer->cb = cb;
    timer->priv = priv;
    timer->scheduled = 1;
    timer->id = _app_timer_next_free_id();
    appmanager_timer_add(&timer->timer);

    return (AppTimerHandle)timer->id;
}

bool app_timer_reschedule(AppTimerHandle timer_handle, uint32_t ms)
{
    AppTimer *timer = _app_timer_get_by_id(timer_handle);

    if (!timer || !timer->scheduled)
        return false;
    
    appmanager_timer_remove(&timer->timer);
    timer->timer.when = xTaskGetTickCount() + pdMS_TO_TICKS(ms);
    appmanager_timer_add(&timer->timer);
    
    return true;
}

void app_timer_cancel(AppTimerHandle timer_handle)
{
    AppTimer *timer = _app_timer_get_by_id(timer_handle);
    
    if (!timer)
        return;
    
    if (timer->scheduled)
        appmanager_timer_remove(&timer->timer);
    
    app_free(timer);
}


AppTimer *_app_timer_get_by_id(AppTimerHandle id)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    CoreTimer **tnext = &_this_thread->timer_head;

    while (*tnext) {
        if (((AppTimer *)(*tnext))->id == id) {
            return (AppTimer *)*tnext;
        }
        tnext = &(*tnext)->next;
    }
    
    return NULL;
}

static uint16_t _timer = 0;
uint16_t _app_timer_next_free_id(void)
{
    return _timer++;
    
    app_running_thread *_this_thread = appmanager_get_current_thread();
    CoreTimer **tnext = &_this_thread->timer_head;
    uint16_t high = 0;
    while (*tnext) {
        if (((AppTimer *)(*tnext))->id > high)
        {
            high = ((AppTimer *)(*tnext))->id;
        }
        tnext = &(*tnext)->next;
    }
    
    return high++;
}
