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
#include "librebble.h"
#include "menu.h"
#include "appmanager.h"

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

#define STANDARD_MENU_COUNT 2

static int8_t menu_index = 0;
extern App apps[NUM_APPS];
    // these will get cached out so meh
void menu_init(void)
{
        // these will get cached out so meh
    printf("menu init\n");
    main_menu[0].text       = "Settings";
    main_menu[0].sub_text   = "";
    main_menu[0].image_res_id = 16;
    main_menu[1].text       = "Console";
    main_menu[1].sub_text   = ": Sys booted";
    main_menu[1].image_res_id = 24;
    main_menu[2].text       = "a";
    main_menu[2].sub_text   = "";
    main_menu[2].image_res_id = 0;
    main_menu[3].text       = "b";
    main_menu[3].sub_text   = "";
    main_menu[3].image_res_id = 0;
}

void menu_draw_list(menu_item_t menu[], uint8_t offsetx, uint8_t offsety)
{
    // Draw the standard apps
    for (uint8_t i = 0; i < STANDARD_MENU_COUNT; i++)
    {
        // build the sub text
//         if (i == 2) // its the info menu, prob need to get cleverer here for auto magic
//         {
//             sprintf(buf, ": %d", ambient_get());
//             menu[i].sub_text = buf;
//         }

        menu_draw_list_item(0, i * 42, offsetx, offsety, &menu[i], (menu_index == i ? 1 : 0));
    }
    
    uint8_t j = STANDARD_MENU_COUNT;
    for (uint8_t i = 0; i < NUM_APPS; i++)
    {
        if (apps[i].type == APP_TYPE_FACE)
        {
            menu[j].text = apps[i].name;
            menu[j].image_res_id = 25;
            menu_draw_list_item(0, j * 42, offsetx, offsety, &menu[j], (menu_index == j ? 1 : 0));
            j++;
        }
        
    }
    
}

void menu_draw_list_item(uint16_t x, uint16_t y, uint8_t offsetx, uint8_t offsety, menu_item_t* menu, uint8_t selected)
{
    GColor bg;
    // list item is a box
    // might have a type (has subtext etc)

    n_GContext *nGContext = neographics_get_global_context();

    // could be selected item eh
    if (selected)
    {
        bg = GColorBlue;
        graphics_context_set_text_color(nGContext, GColorWhite);
    }
    else
    {
        bg = GColorWhite;
        graphics_context_set_text_color(nGContext, GColorBlack);
    }

    graphics_context_set_fill_color(nGContext, bg);
    graphics_fill_rect(nGContext, GRect(x, y, x + 144, y + 42), 0, GCornerNone);
    
    // and an icon
    if (menu->image_res_id > 0)
    {
        GBitmap *gbitmap = gbitmap_create_with_resource(menu->image_res_id);
        graphics_draw_bitmap_in_rect(nGContext, gbitmap, GRect(x + 5, y + 5, 25,25)); //gbitmap->bounds.size.w, gbitmap->bounds.size.h));
        gbitmap_destroy(gbitmap);
    }

    GFont font1 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    GFont font2 = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    
    // and subtext
    if (strlen(menu->sub_text) > 0)
    {
        graphics_draw_text(nGContext, menu->sub_text, font1,
                       GRect(x + 45, y + 18, 100,25), 0,
                       0, 0);

        graphics_draw_text(nGContext, menu->text, font2,
                       GRect(x + 40, y + 0, 100,20), 0,
                       0, 0);
    }
    else
    {
        graphics_draw_text(nGContext, menu->text, font2,
                       GRect(x + 40, y + 0, 100,25), 0,
                       0, 0);
    }

}

uint16_t fl_idx = 7;

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
    if (menu_index > 1)
        appmanager_app_start(main_menu[menu_index].text);
}

void menu_back()
{
    
}


