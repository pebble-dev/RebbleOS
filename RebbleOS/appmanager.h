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


typedef struct AppMessage
{
    uint8_t message_type_id;
    void *payload;
} AppMessage;

typedef struct ButtonMessage
{
    void *callback;
    void *clickref;
    void *context;
} ButtonMessage;

#define APP_BUTTON       0
#define APP_QUIT         1

#define APP_TYPE_SYSTEM  0
#define APP_TYPE_FACE    1
#define APP_TYPE_APP     2


void appmanager_init(void);
// void appmanager_begin_suspend(void);
// void appmanager_resume(void);
void appmanager_post_button_message(ButtonMessage *bmessage);

// void app_resumed(void);
// void app_init(void);
void rbl_window_load_proc(void);
// void app_main(void);
