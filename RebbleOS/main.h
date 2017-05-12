#pragma once
/* main.c
 * Main entry point
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "platform.h"
#include "power.h"
#include "vibrate.h"
#include "display.h"
#include "backlight.h"
#include "buttons.h"
#include "time.h"

void watchdog_init();
void watchdog_reset(void);
void hardware_init(void);
