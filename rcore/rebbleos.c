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
#include "power.h"
#include "qemu.h"
#include "blob_db_ramfs.h"

typedef uint8_t (*mod_callback)(void);
static TaskHandle_t _os_task;
static SemaphoreHandle_t _os_init_sem;

static void _os_thread(void *pvParameters);
static void _module_init(mod_callback mod, const char *mod_name);


/* The variable used to hold the queue's data structure. */

typedef struct os_msg_t {
    char *data;
    size_t len;
} os_msg;

#define _QUEUE_LENGTH    2
#define _ITEM_SIZE       sizeof( os_msg )

static StaticQueue_t _os_queue;
static uint8_t _os_queue_buf[_QUEUE_LENGTH * _ITEM_SIZE];
static QueueHandle_t _os_queue_handle;


SystemSettings _system_settings = 
{
    .backlight_on_time = 3000,
    .backlight_intensity = 100, //%
};


void rebbleos_init(void)
{   
    _os_init_sem = xSemaphoreCreateBinary();
    _os_queue_handle = xQueueCreateStatic( _QUEUE_LENGTH,
                                 _ITEM_SIZE,
                                 _os_queue_buf,
                                 &_os_queue );

    configASSERT( _os_queue_handle );
    
    xTaskCreate(_os_thread, "OS", 1920, NULL, tskIDLE_PRIORITY + 6UL, &_os_task);
}

char buf[100];
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
    power_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Power Init");
    SYS_LOG("OS", APP_LOG_LEVEL_INFO,   "Init: Main hardware up. Starting OS modules");
    _module_init(resource_init,         "Resources");
    ramfs_init();
    _module_init(notification_init,     "Notifications");
    _module_init(overlay_window_init,   "Overlay");
    _module_init(appmanager_init,       "Main App");

    /* This is a runloop for all generic OS related stuff. */
    
    os_msg msg;
    uint8_t count = 0;
    while(1)
    {
        /* block forever in slumber? */
        if(xQueueReceive(_os_queue_handle, &(msg), pdMS_TO_TICKS(1000)))
        {
            bluetooth_send((uint8_t *)msg.data, msg.len);
        }
        
        /* Update the power detection. When interrupts are found remove me */
        power_update_charge_mode();
                
        if (count == 10)
        {
            /* get battery voltage */
            power_update_battery();
            count = 0;
            continue;
        }
        count++;
    }

    /* delete end done fin */
    vTaskDelete( NULL );
}

void send_os_msg(char *data, size_t len)
{
    os_msg msg = {
        .data = data,
        .len = len
    };
    xQueueSendFromISR(_os_queue_handle, &msg, NULL);
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
            if (strncmp(mod_name, "Bluetooth", 9) == 0)
            {
                SYS_LOG("OS", APP_LOG_LEVEL_DEBUG, "Bluetooth failed. Assuming QEMU host.");
                _module_init(qemu_init, "QEMU");
            }
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
