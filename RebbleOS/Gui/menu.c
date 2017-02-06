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
#include "stdio.h"
#include "string.h"
#include "ugui.h"
#include "menu.h"
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

static uint8_t menu_index = 0;

void menu_init(void)
{
    printf("menu init\n");
    main_menu[0].text       = "Settings";
    main_menu[0].sub_text   = "";
    main_menu[1].text       = "Console";
    main_menu[1].sub_text   = ": Sys booted";
    main_menu[2].text       = "Info";
    main_menu[2].sub_text   = ": ";
    main_menu[3].text       = "Toys";
    main_menu[3].sub_text   = "/toys/";
}

void menu_draw_list(menu_item_t menu[], uint8_t offsetx, uint8_t offsety)
{
    char buf[25];
    
    for (uint8_t i = 0; i < 4; i++)
    {
        // build the sub text
        if (i == 2) // its the info menu, prob need to get cleverer here for auto magic
        {
            sprintf(buf, ": %d", ambient_get());
            menu[i].sub_text = buf;
        }

        menu_draw_list_item(0, i * 42, offsetx, offsety, &menu[i], (menu_index == i ? 1 : 0));
    }
}

void menu_draw_list_item(UG_S16 x, UG_S16 y, uint8_t offsetx, uint8_t offsety, menu_item_t* menu, uint8_t selected)
{
    UG_COLOR c;
    // list item is a box
    // might have a type (has subtext etc)
    
    // could be selected item eh
    if (selected)
    {
        c = C_BLUE;
        UG_SetBackcolor(C_BLUE);
        UG_SetForecolor(C_WHITE);
    }
    else
    {
        c = C_WHITE;
        UG_SetBackcolor(C_WHITE);
        UG_SetForecolor(C_BLACK);
    }
    
//     x += offsetx;
//     y += offsety;
    
    UG_FillFrame(x, y, x + 144, y + 42, c);
    // and an icon
    // and text
    UG_FontSelect(&FONT_8X14);
    UG_PutString(x + 30, y + 5, menu->text);
    // and subtext
    UG_FontSelect(&FONT_6X8);
    UG_PutString(x + 40, y + 25, menu->sub_text);
}

void menu_show(uint8_t offsetx, uint8_t offsety)
{
    menu_draw_list(main_menu, offsetx, offsety);
}

void menu_up()
{
    if (menu_index > 0)
       menu_index--;
}

void menu_down()
{
    if (menu_index < 3)
        menu_index++;
}

void menu_select()
{
    
}

void menu_back()
{
    
}


