/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */ 
#include "rebbleos.h"

TimeUnits time_units;
TickHandler tick_handler;
TickMessage tick_message;

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
    time_units = tick_units;
    tick_handler = handler;
    rtc_set_timer_interval(tick_units);
}

/*
 * Null all handles and stop the RTC
 */
void rebble_time_service_unsubscribe(void)
{
    tick_handler = NULL;
    time_units = 0;
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
    if (tick_handler != NULL ) //&&
        //(time_units & tick_units))
    {
        // we need to malloc this as it will be passed as a pointer to the queue
        // Once the work has been done it will need to be freed
        tick_message.callback = tick_handler;
        tick_message.tick_time = tick_time;
        tick_message.tick_units = tick_units;
        appmanager_post_tick_message(&tick_message, xHigherPriorityTaskWoken);
    }
}
