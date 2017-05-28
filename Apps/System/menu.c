/* menu.c
 * Poor mans menu to stop gap us
 * 
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include "rebbleos.h"
#include "librebble.h"
#include "menu.h"
#include "appmanager.h"
#include "ngfxwrap.h"

extern void graphics_draw_bitmap_in_rect(GContext*, GBitmap*, GRect);
extern void flash_dump(void);

/*
// menu

This is teh suckiest. All hard coded (in the worst way) until we get proper menu support
coded up

*/

#define STANDARD_MENU_COUNT 4

static int8_t _menu_index = 0;
MenuItem _main_menu[4];
static char *_selected_menu_name;

#define MENU_MAIN       0
#define MENU_WATCH      1
#define MENU_CONSOLE    2

uint8_t _menu_type = MENU_MAIN;

void menu_init(void)
{
    
    printf("menu init\n");
    _main_menu[0].text       = "Watchfaces";
    _main_menu[0].sub_text   = "Will scan flash";
    _main_menu[0].image_res_id = 25;
    _main_menu[1].text       = "Dump Flash";
    _main_menu[1].sub_text   = "Device will lock";
    _main_menu[1].image_res_id = 24;
    _main_menu[2].text       = "RebbleOS";
    _main_menu[2].sub_text   = "... v0.0.0.1";
    _main_menu[2].image_res_id = 24;
    _main_menu[3].text       = "... Soon (TM)";
    _main_menu[3].sub_text   = "";
    _main_menu[3].image_res_id = 25;
}

void menu_draw_list(MenuItem menu[], uint8_t offsetx, uint8_t offsety)
{
    // Draw the standard apps
    for (uint8_t i = 0; i < STANDARD_MENU_COUNT; i++)
    {
        if (_menu_index == i)
            _selected_menu_name = menu[i].text;
        menu_draw_list_item(0, i * 42, offsetx, offsety, &menu[i], (_menu_index == i ? 1 : 0));
    }
     
}

void menu_draw_watch_list()
{
    // loop through all apps
    App *node = app_manager_get_apps_head();
    uint8_t i = 0;
    n_GContext *nGContext = rwatch_neographics_get_global_context();
    
    graphics_context_set_fill_color(nGContext, GColorBlue);
    graphics_fill_rect(nGContext, GRect(0, 0, 144, 168), 0, GCornerNone);
    
    while(node)
    {
        if ((!strcmp(node->name, "System")) ||
            (!strcmp(node->name, "TrekV2")) ||
            (!strcmp(node->name, "watchface")))
        {
            node = node->next;
            continue;
        }
        MenuItem mi;
        mi.text = node->name;
        mi.sub_text = 0;
        mi.image_res_id = 25;
        menu_draw_list_item(0, i * 42, 0, 0, &mi, (_menu_index == i ? 1 : 0));

        if (_menu_index == i)
            _selected_menu_name = node->name;
        
        node = node->next;

        // we show 4 for now
        if (i >= 3)
            break;
        
        i++;
    }
}

void menu_draw_list_item(uint16_t x, uint16_t y, uint8_t offsetx, uint8_t offsety, MenuItem* menu, uint8_t selected)
{
    GColor bg;
    // list item is a box
    // might have a type (has subtext etc)

    n_GContext *nGContext = rwatch_neographics_get_global_context();

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
    if (_menu_type == MENU_MAIN)
        menu_draw_list(_main_menu, offsetx, offsety);
    else if (_menu_type == MENU_WATCH)
        menu_draw_watch_list();
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
    if (_menu_type == MENU_WATCH)
    {
        appmanager_app_start(_selected_menu_name);
        return;
    }
    
    if (_menu_index == 0)
    {
        // submenu watchfaces
        _menu_type = MENU_WATCH;
    }
    else if (_menu_index == 1)
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
    if (_menu_type == MENU_WATCH)
    {
        _menu_type = MENU_MAIN;
    }        
    else
    {
        appmanager_app_start("Simple");
        return;
    }

    // TODO quit back to watchface
}

