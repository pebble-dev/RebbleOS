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
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "rebbleos.h"
#include "menu.h"
#include "gui.h"


// current status of te system
SystemStatus system_status;
SystemSettings system_settings;

void rebbleos_init(void)
{
    system_status.booted = 0;
    system_status.app_mode = SYSTEM_RUNNING_APP;
    
    gui_init();
    appmanager_init();

    // set up main rebble task thread
    
    // this will be the main coordinator
}

void rebbleos_set_system_status(uint8_t status)
{
    
}
