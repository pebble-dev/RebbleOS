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
