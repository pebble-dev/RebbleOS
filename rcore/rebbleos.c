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
#include "watchdog.h"
#include "ngfxwrap.h"
#include "overlay_manager.h"
#include "notification_manager.h"

typedef uint8_t (*mod_callback)(void);
static TaskHandle_t _os_task;
static SemaphoreHandle_t _os_init_sem;

static void _os_thread(void *pvParameters);
static void _module_init(mod_callback mod, const char *mod_name);

SystemSettings _system_settings = 
{
    .backlight_on_time = 3000,
    .backlight_intensity = 100, //%
};

void rebbleos_init(void)
{   
    xTaskCreate(_os_thread, "OS", 1920, NULL, tskIDLE_PRIORITY + 6UL, &_os_task);
    _os_init_sem = xSemaphoreCreateBinary();
}

static void _os_thread(void *pvParameters)
{
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Starting Init...");
    /* Load or init each module in an orderly fashion.
     * All modules support either sync or async loading
     * An async module might require such things as callbacks
     * or a delay before it is up. Once the module is up, it 
     * can report completion
     */
    _module_init(flash_init,            "Flash Storage");
    _module_init(vibrate_init,          "Vibro");
    _module_init(display_init,          "Display");
    _module_init(rcore_buttons_init,    "Buttons");
    rtc_init();
    rcore_time_init();
    //rcore_ambient_init("Ambiance");
    _module_init(rcore_backlight_init,  "Backlight");
    platform_init_late();
    rcore_watchdog_init_late();
    KERN_LOG("OS", APP_LOG_LEVEL_INFO,  "Watchdog is ticking");
    _module_init(bluetooth_init,        "Bluetooth");
    
    SYS_LOG("OS", APP_LOG_LEVEL_INFO,   "Init: Main hardware up. Starting OS modules");
    _module_init(resource_init,         "Resources");
    _module_init(notification_init,     "Notifications");
    _module_init(overlay_window_init,   "Overlay");
    _module_init(appmanager_init,       "Main App");

    //while(1)
        // block forever in slumber?

    // delete end done fin
    vTaskDelete( NULL );
}


static int8_t _last_init_result = -1;
static void _module_init(mod_callback mod, const char *mod_name)
{
    uint8_t mres = mod();

    /* are we going async? */
    if (mres == INIT_RESP_ASYNC_WAIT)
    {
        /* Sleep and wait for the module to come up */
        if (!xSemaphoreTake(_os_init_sem, pdMS_TO_TICKS(5000)))
        {
            SYS_LOG("OS", APP_LOG_LEVEL_ERROR, "Init: Async module Init Failed", mod_name);
            assert(!"Init: An async task failed to come up when it should!");
        }
        else
        {
            mres = _last_init_result;
        }
    }
    
    /* Wee module is up. or at least no more work to do */   
    switch(mres)
    {
        case INIT_RESP_OK:
            SYS_LOG("OS", APP_LOG_LEVEL_INFO, "Init: %s", mod_name);
            break;
        case INIT_RESP_NOT_SUPPORTED:
            SYS_LOG("OS", APP_LOG_LEVEL_WARNING, "Init: Module %s NOT supported", mod_name);
            break;
        case INIT_RESP_ERROR:
            SYS_LOG("OS", APP_LOG_LEVEL_ERROR, "Init: Module %s broken.", mod_name);
    }

    return;
    
}

void os_module_init_complete(uint8_t result)
{
    _last_init_result = result;
    xSemaphoreGive(_os_init_sem);
}

SystemSettings *rebbleos_get_settings(void)
{
    return &_system_settings;
}
