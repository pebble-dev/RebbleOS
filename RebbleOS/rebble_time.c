/* rebble_time.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

static TimeUnits _time_units;
static TickHandler _tick_handler;
static TickMessage _tick_message;

void rebble_time_callback_trigger(struct tm *tick_time, TimeUnits tick_units, BaseType_t *xHigherPriorityTaskWoken);


/*
 * Get the time as a tm struct
 */
struct tm *rebble_time_get_tm(void)
{
    return hw_get_time();
}

/*
 * Set the handler and unit type to the global handler
 */
void rebble_time_service_subscribe(TimeUnits tick_units, TickHandler handler)
{
    _time_units = tick_units;
    _tick_handler = handler;
    rtc_set_timer_interval(tick_units);
}

/*
 * Null all handles and stop the RTC
 */
void rebble_time_service_unsubscribe(void)
{
    _tick_handler = NULL;
    _time_units = 0;
    rtc_disable_timer_interval();
}

void rebble_time_service_disable_timer()
{
    rtc_disable_timer_interval();
}

/* 
 * Callback from the RTC core to tell us that we have a tick
 */
void rebble_time_rtc_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    struct tm *time = hw_get_time();
    TimeUnits tick_units = SECOND_UNIT;
    rebble_time_callback_trigger(time, tick_units, &xHigherPriorityTaskWoken);
    
    if(xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/*
 * trigger the tick callback
 */
void rebble_time_callback_trigger(struct tm *tick_time, TimeUnits tick_units, BaseType_t *xHigherPriorityTaskWoken)
{
    // only callback is we are looking for this mask
    if (_tick_handler != NULL ) //&&
        //(_time_units & tick_units))
    {
        // we need to malloc this as it will be passed as a pointer to the queue
        // Once the work has been done it will need to be freed
        _tick_message.callback = _tick_handler;
        _tick_message.tick_time = tick_time;
        _tick_message.tick_units = tick_units;
        appmanager_post_tick_message(&_tick_message, xHigherPriorityTaskWoken);
    }
}
