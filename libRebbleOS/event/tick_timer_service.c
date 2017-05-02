/* tick_timer_service.c
 * routines for [...]
 * RebbleOS core
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"



void tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler)
{
    rebble_time_service_subscribe(tick_units, handler);
}

void tick_timer_service_unsubscribe(void)
{
    rebble_time_service_unsubscribe();
}


