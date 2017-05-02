/* vibrate.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include "debug.h"
#include "stdio.h"
#include "string.h"
#include "platform.h"
#include "vibrate.h"
#include "task.h"
#include "semphr.h"


static uint16_t VIBRO_SHORT[] = { 255, 1000 };
static uint16_t VIBRO_TAP_TAP[] = { 255, 750, 0, 750, 255, 750 };

static TaskHandle_t _vibrate_task;
static xQueueHandle _vibrate_queue;

static void _vibrate_thread(void *pvParameters);

/*
 * Initialise the vibration controller and tasks
 */
void vibrate_init(void)
{
    int rv;
    
    hw_vibrate_init();
    
    rv = xTaskCreate(_vibrate_thread, "Vibrate", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &_vibrate_task); /* XXX: allocate statically later */
    assert(rv == pdPASS);
    
    _vibrate_queue = xQueueCreate(1, sizeof(uint8_t));
}

/*
 * Start vibratin
 */
static void _enable(uint8_t enabled)
{
    hw_vibrate_enable(enabled);
}

/*
 * Quickly and dirtily pop out a pattern on the motor
 */
static void _play_pattern(uint16_t *pattern, uint8_t pattern_length)
{
    // set the timer 12 CH2 to pwm to vibrate
    // TODO FIX
    return;
    // low rumble
    // touble tap
    // medium
    // high
    // and drill
    
    for (uint8_t i = 0; i < pattern_length; i += 2)
    {
        // vibrate for x time
        // until this is variable...
        if (pattern[i] > 0)
            _enable(1);
        vTaskDelay(pattern[i+1] / portTICK_RATE_MS);
        _enable(0);
    }
}

/*
 * The main task for driving patterns out of the motor
 */
static void _vibrate_thread(void *pvParameters)
{
    uint8_t data;

    while(1)
    {       
        if (!xQueueReceive(_vibrate_queue, &data, portMAX_DELAY))
            continue;

        uint8_t len;
        uint16_t *pattern;
        
        switch(data)
        {
            case VIBRATE_CMD_STOP:
                // slam the brakes on somehow
                break;
            case VIBRATE_CMD_PATTERN_1:
                // big vibrate
                pattern = VIBRO_SHORT;
                len = 1;
                break;
            case VIBRATE_CMD_PATTERN_2:
                // tap tap
                pattern = VIBRO_TAP_TAP;
                len = 3;
                break;
        }
        _play_pattern(pattern, len);
    }
}
