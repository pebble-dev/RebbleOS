#pragma once
/* rebble_time.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include <time.h>

// a bit mask of the time units
typedef enum {
    SECOND_UNIT = 1 << 0, 
    MINUTE_UNIT = 1 << 1, 
    HOUR_UNIT   = 1 << 2, 
    DAY_UNIT    = 1 << 3, 
    MONTH_UNIT  = 1 << 4, 
    YEAR_UNIT   = 1 << 5
} TimeUnits;

typedef void(*TickHandler)(struct tm *tick_time, TimeUnits units_changed);
void rebble_time_service_subscribe(TimeUnits tick_units, TickHandler handler);
void rebble_time_service_unsubscribe(void);
void rebble_time_service_disable_timer();

void rcore_time_init(void);
time_t rcore_mktime(struct tm *tm);
void rcore_localtime(struct tm *tm, time_t time);
void rcore_time_ms(time_t *tutc, uint16_t *ms);

// private
void rebble_time_rtc_isr(void);
struct tm *rebble_time_get_tm(void);
int pbl_clock_is_24h_style();
