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
#include "rebble_util.h"
#include "rbl_bluetooth.h"

#define VERSION "v0.0.0.2"

// Public API functions as exposed through the various layers

// Reset the watchdog timer manually
void watchdog_reset(void);



typedef struct SystemSettings {
    uint16_t backlight_intensity;
    uint16_t backlight_on_time;
    uint16_t vibrate_intensity;
    uint16_t vibrate_pattern;
    // may need 16t
    uint8_t modules_enabled_flag;
    uint8_t modules_error_flag;
} SystemSettings;

void rebbleos_init(void);
void os_module_init_complete(uint8_t result);
SystemSettings *rebbleos_get_settings(void);

#define INIT_RESP_OK            0
#define INIT_RESP_ASYNC_WAIT    1
#define INIT_RESP_NOT_SUPPORTED 2
#define INIT_RESP_ERROR         3
