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
    main_menu[0].text = "Settings";
    main_menu[1].text = "Console";
    main_menu[2].text = "Info";
    main_menu[3].text = "Toys";
}

void menu_draw_list(menu_item_t menu[], uint8_t offsetx, uint8_t offsety)
{
    UG_FontSelect(&FONT_8X14);
    
    for (uint8_t i = 0; i < 4; i++)
    {
        printf("asdasd %d\n", menu_index);
        menu_draw_list_item(0, i * 43, offsetx, offsety, menu[i].text, (menu_index == i ? 1 : 0));
    }
}

void menu_draw_list_item(UG_S16 x, UG_S16 y, uint8_t offsetx, uint8_t offsety, char* text, uint8_t selected)
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
    
    UG_FillFrame(x, y, x + 144, y + 43, c);
    // and an icon
    // and text
    UG_PutString(x + 30, y + 5, text);
    // and subtext
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


