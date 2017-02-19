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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "rebble_memory.h"
#include "platform.h"
#include "appmanager.h"
#include "ambient.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "gui.h"
#include "display.h"
#include "menu.h"
#include "neographics.h"
#include "ugui.h"
#include "main.h"

// Public API functions as exposed through the various layers

// Reset the watchdog timer manually
void watchdog_reset(void);

#define NOWT                 0
#define SYSTEM_RUNNING_APP   1
#define SYSTEM_IN_MAIN_MENU  2


typedef struct SystemStatus {
    uint8_t booted;
    uint8_t app_mode; // like in menu
} SystemStatus;


// move into backlight
typedef struct SystemSettings {
    uint8_t backlight_intensity;
    uint8_t backlight_on_time;
    uint8_t vibrate_intensity;
    uint8_t vibrate_pattern;
} SystemSettings;


void rebbleos_init(void);
