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

static TickType_t _boot_ticks;
static time_t _boot_time_t;

void rcore_time_init(void)
{
    struct tm *tm;
    
    /* Read the time out of the RTC, then convert to a time_t (ugh!), then
     * begin offsetting ticks in ms from there.  */
    _boot_ticks = xTaskGetTickCount();
    tm = hw_get_time();
    _boot_time_t = rcore_mktime(tm);
}

time_t rcore_mktime(struct tm *tm)
{
    return mktime(tm);
}

void rcore_localtime(struct tm *tm, time_t time)
{
    localtime_r(&time, tm);
}

void rcore_time_ms(time_t *tutc, uint16_t *ms)
{
    TickType_t ticks_since_boot = xTaskGetTickCount() - _boot_ticks;
    
    *tutc = _boot_time_t + ticks_since_boot / configTICK_RATE_HZ;
    *ms = (ticks_since_boot % configTICK_RATE_HZ) * 1000 / configTICK_RATE_HZ;
}

/*
 * Get the time as a tm struct
 */
/* XXX need one struct tm per app */
static struct tm _global_tm;
struct tm *rebble_time_get_tm(void)
{
    time_t tm;
    rcore_time_ms(&tm, NULL);
    rcore_localtime(&_global_tm, tm);
    return &_global_tm;
}

uint16_t pbl_time_ms_deprecated(time_t *tloc, uint16_t *ms)
{
    /* XXX time zones: utc vs local time */
    uint16_t _ms;
    time_t _tm;
    
    rcore_time_ms(&_tm, &_ms);
    if (tloc)
        *tloc = _tm;
    if (ms)
        *ms = _ms;
    
    return _ms;
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

    struct tm *time = rebble_time_get_tm();
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
