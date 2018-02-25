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

#define NOWT                 0
#define SYSTEM_RUNNING_APP   1
#define SYSTEM_IN_MAIN_MENU  2

typedef struct SystemStatus {
    uint8_t booted;
    uint8_t app_mode; // like in menu
} SystemStatus;

#define MODULE_DISABLED     0
#define MODULE_ENABLED      1

#define MODULE_NO_ERROR     0
#define MODULE_ERROR        1

#define MODULE_BLUETOOTH    1
#define MODULE_DISPLAY      2
#define MODULE_VIBRATE      4

#define SYSTEM_STATUS_STARTED 1


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
uint8_t rebbleos_get_system_status(void);
void rebbleos_set_system_status(uint8_t status);

SystemSettings *rebbleos_get_settings(void);
void rebbleos_module_set_status(uint8_t module, uint8_t enabled, uint8_t error);
uint8_t rebbleos_module_is_enabled(uint8_t module);
uint8_t rebbleos_module_is_error(uint8_t module);
