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
#include "notification_manager.h"


// current status of te system
SystemStatus system_status = { .booted = 0 };
SystemSettings _system_settings = 
{
    .backlight_on_time = 3000,
    .backlight_intensity = 100, //%
};

void rebbleos_init(void)
{
    system_status.booted = 0;
    system_status.app_mode = SYSTEM_RUNNING_APP;
    
    resource_init();   
    notification_init();
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

SystemSettings *rebbleos_get_settings(void)
{
    return &_system_settings;
}

void rebbleos_module_set_status(uint8_t module, uint8_t enabled, uint8_t error)
{
    if (enabled)
        _system_settings.modules_enabled_flag |= 1 << module;
    else
        _system_settings.modules_enabled_flag &= ~(1 << module);
    
    if (error)
        _system_settings.modules_error_flag |= 1 << module;
}

uint8_t rebbleos_module_is_enabled(uint8_t module)
{
    return (_system_settings.modules_enabled_flag >> module ) & 1;
}

uint8_t rebbleos_module_is_error(uint8_t module)
{
    return (_system_settings.modules_error_flag >> module ) & 1;
}
