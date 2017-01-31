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
#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "ugui.h"
/*
// menu

// draw menu
// - items
// scroll up / down

// flow
 button is pressed
  button handler calls to see if anyone is handling this request
    if in app
      up = "health"
      down = timeline
      select = menu
      back nothing (app can bind this?)
    if in menu
      forward commands


gui controller

receive events
    button pressed
    in menu
        scroll
        choose next screen
    in app
        notify app
*/

void menu_draw_list(void)
{
    // has list items
    // render them
    menu_draw_list_item(0, 0, "Settings");
    menu_draw_list_item(40, 0, "Console");
    menu_draw_list_item(80, 0, "Info");
    menu_draw_list_item(120, 0, "Info");
}

void menu_draw_list_item(UG_S16 x, UG_S16 y, char* text)
{
    // list item is a box
    // might have a type (has subtext etc)
    UG_DrawFrame(x, y, x + 40, y + 144, C_RED);
    // and an icon
    // and text
    UG_PutString(x, y, text);
    // and subtext
}
/*
void appTask()
{

    while(1)
    {
        // dequeue
        if (button_press)
        {
            // we are in the menu, so draw menu-y things
            if (APP_STATUS_MENU)
            {
                if (menu.scrollup)
            }
        }
    }
}
*/
