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
 #include "librebble.h"
 
typedef struct {
    char *text;
    char *sub_text;
    uint16_t image_res_id;
    
} menu_item_t;

menu_item_t main_menu[4];

void menu_init(void);
void menu_draw_list(menu_item_t menu[], uint8_t offsetx, uint8_t offsety);
void menu_draw_list_item(uint16_t x, uint16_t y, uint8_t offsetx, uint8_t offsety, menu_item_t* menu, uint8_t selected);
void menu_show(uint8_t offsetx, uint8_t offsety);
void menu_up(void);
void menu_down(void);
void menu_back(void);
void menu_select(void);
