/* rebbleos.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "rebbleos.h"
#include "ngfxwrap.h"
#include "overlay_manager.h"


// current status of te system
SystemStatus system_status = { .booted = 0 };
SystemSettings system_settings;

void rebbleos_init(void)
{
    system_status.booted = 0;
    system_status.app_mode = SYSTEM_RUNNING_APP;
    
    rwatch_neographics_init();
    appmanager_init();
    
    overlay_window_init();
}

void rebbleos_set_system_status(uint8_t status)
{
    system_status.booted = status;
}

uint8_t rebbleos_get_system_status(void)
{
    return system_status.booted;
}
