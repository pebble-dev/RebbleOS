#pragma once
/* rebbleos.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "FreeRTOS.h"
#include "rebble_memory.h"
#include "platform.h"
#include "appmanager.h"
#include "ambient.h"
#include "task.h"
#include "semphr.h"
#include "display.h"
#include "rebble_time.h"
#include "main.h"
#include "log.h"
#include "debug.h"
#include "flash.h"
#include "resource.h"

#define VERSION "v0.0.0.2"

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
