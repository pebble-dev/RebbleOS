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

static int8_t _menu_index = 3;
MenuItem _main_menu[4];

void menu_init(void)
{
    
    printf("menu init\n");
    _main_menu[0].text       = "Settings";
    _main_menu[0].sub_text   = "";
    _main_menu[0].image_res_id = 16;
    _main_menu[1].text       = "Dump Flash";
    _main_menu[1].sub_text   = "Device will lock";
    _main_menu[1].image_res_id = 24;
}

void menu_draw_list(MenuItem menu[], uint8_t offsetx, uint8_t offsety)
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

        menu_draw_list_item(0, i * 42, offsetx, offsety, &menu[i], (_menu_index == i ? 1 : 0));
    }
        
    // find the app
    App *node = app_manager_get_apps_head();
    
    while(node)
    {
        printf("NODE: %s %d %d\n", node->name, node->next, strncmp("91 Dub 4.0", node->name, strlen(node->name)));

        if (!strncmp(node->name, "NiVZ", strlen(node->name)))
        {
            // match!
            _main_menu[2].text = node->name;
            _main_menu[2].sub_text = 0;
            _main_menu[2].image_res_id = 25;
            menu_draw_list_item(0, 2 * 42, offsetx, offsety, &_main_menu[2], (_menu_index == 2 ? 1 : 0));
        }
        
        if (!strncmp(node->name, "91 Dub 4.0", strlen(node->name)))
        {
            // match!
            _main_menu[3].text = node->name;
            _main_menu[3].sub_text = 0;
            _main_menu[3].image_res_id = 25;
            menu_draw_list_item(0, 3 * 42, offsetx, offsety, &_main_menu[3], (_menu_index == 3 ? 1 : 0));
        }
        
        if (node->next == NULL)
        {
            printf("Done iter!\n");
        }
        node = node->next;
    }

    /*
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
        
    }*/
    
}

void menu_draw_list_item(uint16_t x, uint16_t y, uint8_t offsetx, uint8_t offsety, MenuItem* menu, uint8_t selected)
{
    GColor bg;
    // list item is a box
    // might have a type (has subtext etc)

    n_GContext *nGContext = neographics_get_global_context();

    // could be selected item eh
    if (selected)
    {
        bg = GColorRed;
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
        graphics_draw_bitmap_in_rect(nGContext, gbitmap, GRect(x + 5, y + 5, 25,25));
        gbitmap_destroy(gbitmap);
    }

    // these will get cached out so meh
    GFont font1 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    GFont font2 = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    
    // and text/subtext
    if (menu->sub_text != NULL && strlen(menu->sub_text) > 0)
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
    menu_draw_list(_main_menu, offsetx, offsety);
}

void menu_up()
{
    if (_menu_index > 0)
       _menu_index--;
}

void menu_down()
{        
    if (_menu_index < 3)
        _menu_index++;
}

void menu_select()
{
    if (_menu_index == 1)
    {
        // dump flash
        flash_dump();
    }
    else if (_menu_index > 1)
    {
        appmanager_app_start(_main_menu[_menu_index].text);
    }
}

void menu_back()
{
    
}


