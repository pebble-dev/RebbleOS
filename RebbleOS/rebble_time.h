#pragma once
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

struct tm *rebble_time_get_tm(void);

// a bit mask of the time units
typedef enum {
    SECOND_UNIT = 1 << 0, 
    MINUTE_UNIT = 1 << 1, 
    HOUR_UNIT   = 1 << 2, 
    DAY_UNIT    = 1 << 3, 
    MONTH_UNIT  = 1 << 4, 
    YEAR_UNIT   = 1 << 5
} TimeUnits;
