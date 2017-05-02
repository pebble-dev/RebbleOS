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


void app_log_trace(uint8_t level, const char *filename, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    printf("%x\n",fmt);
    // TODO get defines
    if (level == APP_LOG_LEVEL_DEBUG)
        printf("DEBUG ");
    else 
        printf("INFO "); 
    printf(filename, ar);
    printf(fmt, ar);
    printf("\n");
    va_end(ar);
}

void app_log(uint8_t lvl, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    printf(fmt, ar);
    printf("\n");
    va_end(ar);
}
