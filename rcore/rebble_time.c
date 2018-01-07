/* rebble_time.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "rebbleos.h"

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

TickType_t rcore_time_to_ticks(time_t t, uint16_t ms) {
    if (t < _boot_time_t)
        return 0;
    return (t - _boot_time_t) * configTICK_RATE_HZ + pdMS_TO_TICKS(ms);
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

uint16_t pbl_time_deprecated(time_t *tloc)
{
    /* XXX time zones: utc vs local time */
    uint16_t _ms;
    time_t _tm;
    
    rcore_time_ms(&_tm, &_ms);
    if (tloc)
        *tloc = _tm;
    
    return _ms;
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

int pbl_clock_is_24h_style()
{
    // XXX: Obviously, everybody wants 24h time.  Why would they use a
    // developer operating system if not?
    return 1;
}
