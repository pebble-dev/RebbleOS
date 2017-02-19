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
#include "snowy_rtc.h"  // <-- todo switch to using rebbleo time.c
#include "ugui.h"

void rbl_get_time(char *buf)
{
    hw_get_time_str(buf);
}

struct tm *rbl_get_tm(void)
{
    return hw_get_time();
}

void rbl_draw_fill_screen(uint8_t colour)
{
    if (colour == RBL_BLACK)
        UG_FillScreen(C_BLACK);
}

void rbl_draw_text(uint8_t x, uint8_t y, char *text)
{
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    
    UG_PutString(x, y, text);
}

void rbl_set_font_size()
{
    UG_FontSelect(&FONT_10X16);
}

void rbl_draw(void)
{
    display_draw();
}
