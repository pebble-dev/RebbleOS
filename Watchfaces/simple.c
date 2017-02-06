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
#include "stdio.h"
#include "string.h"

const char *app_name = "Simple";

void draw_atime(void);
void app_main(void);
void app_init(void);

void app_init(void)
{
}

void app_main(void)
{
    draw_atime();
}

const char *app_getname()
{
    return app_name;
}

void app_suspending()
{
    printf("WF: Going to sleep\n");
}

void app_resumed(void)
{
    printf("WF: Wakeup\n");
    rbl_draw_fill_screen(RBL_BLACK);
    draw_atime();
}

void draw_atime(void)
{
    char time[20];
    uint16_t val;
    rbl_draw_fill_screen(RBL_BLACK);
    rbl_get_time(time);
    printf("WF: Hello %s.\n", time);
    rbl_set_font_size();
    rbl_draw_text(40, 75, time);
    
    val = ambient_get();
    sprintf(time, "Light %d", val);
    rbl_draw_text(30, 120, time);
    rbl_draw();
}

