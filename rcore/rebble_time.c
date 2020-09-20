/* rebble_time.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "rebbleos.h"
#include "strftime.h"

/* Configure Logging */
#define MODULE_NAME "rtime"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR


static TickType_t _boot_ticks;
static time_t _boot_time_t;

static struct tm _global_tm;
struct tm *boot_time_tm;
static char _tz_name[8];
static uint8_t _utc_offset;

void rcore_time_init(void)
{
    struct tm *tm;

    /* Read the time out of the RTC, then convert to a time_t (ugh!), then
     * begin offsetting ticks in ms from there.  */
    boot_time_tm = tm = hw_get_time();
    _boot_time_t = rcore_mktime(tm);
    /* reset boot ticks ms to 0 same as wall time above (ugh!!)*/
    _boot_ticks = rcore_time_to_ticks(_boot_time_t, 0);
    
    tz_init();
}

time_t rcore_mktime(struct tm *tm)
{
    return mktime(tm);
}

void rcore_set_time(time_t time)
{
    time_t now;
    struct tm tm;
    
    rcore_time_ms(&now, NULL);
    localtime_r(&time, &tm);
    hw_set_time(&tm);

    _boot_time_t -= (now - time);
}

void rcore_set_utc_offset(uint8_t offset)
{
    _utc_offset = offset;
}

void rcore_set_tz_name(char *tz_name, uint8_t len)
{
    memcpy(_tz_name, tz_name, len < 8 ? len : 8);
    _tz_name[len] = 0;
}

time_t rcore_get_time(void)
{
    return pbl_time_t_deprecated(NULL);
}

uint16_t rcore_time_ms(time_t *tutc, uint16_t *ms)
{
    TickType_t ticks_since_boot = xTaskGetTickCount() - _boot_ticks;

    if (tutc)
        *tutc = _boot_time_t + (ticks_since_boot / configTICK_RATE_HZ);

    uint16_t new_ms = (ticks_since_boot % configTICK_RATE_HZ) * 1000 / configTICK_RATE_HZ;

    if (ms)
        *ms = new_ms;

//     LOG_DEBUG("TUTC %d %d %d %d", _boot_ticks, new_ms, *tutc, _boot_time_t + ticks_since_boot / configTICK_RATE_HZ);
    return new_ms;
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

struct tm *rebble_time_get_tm(void)
{
    time_t tm;
    rcore_time_ms(&tm, NULL);
    localtime_r(&tm, &_global_tm);
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

time_t pbl_time_t_deprecated(time_t *tloc)
{
    /* XXX time zones: utc vs local time */
    uint16_t _ms;
    time_t _tm;
    
    rcore_time_ms(&_tm, NULL);
    localtime_r(&_tm, &_global_tm);

    if (tloc)
        *tloc = _tm;
    
    return _tm;
}

int pbl_clock_is_24h_style()
{
    // XXX: Obviously, everybody wants 24h time.  Why would they use a
    // developer operating system if not?
    return 1;
}


time_t clock_to_timestamp(WeekDay day, int hour, int minute)
{
    struct tm *time_now = rebble_time_get_tm();
    uint8_t wday = time_now->tm_wday;
    uint8_t dd = day - 1 - wday;

    if (dd < 0)
        dd = 7 - dd;

    if (((int)day == 0 && hour < time_now->tm_hour) || // set to TODAY
         (!dd && hour < time_now->tm_hour)) // logically today
        dd += 7;

    uint32_t secs = (minute * 60) + (hour * 360) + (dd * 86400);

    time_now->tm_sec += secs;
    return mktime(time_now);
}

double difftime(time_t end, time_t beginning)
{
    return (double)end - (double)beginning;
}

TickType_t get_boot_tick(void)
{
    return _boot_ticks;
}
