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

#include "librebble.h"

typedef void(*TickHandler)(struct tm *tick_time, TimeUnits units_changed);
void tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler);
void tick_timer_service_unsubscribe(void);


TimeUnits time_units;
TickHandler tick_handler;


void tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler)
{
    time_units = tick_units;
    tick_handler = handler;
}

void tick_timer_service_unsubscribe(void)
{
    tick_handler = NULL;
    time_units = 0;
}

/*
 * trigger the tick callback
 */
void tick_timer_callback_trigger(struct tm *tick_time, TimeUnits tick_units)
{
    if (tick_handler != NULL &&
        (time_units & tick_units))
    {
        ((TickHandler)(tick_handler))(tick_time, tick_units);
    }
}
