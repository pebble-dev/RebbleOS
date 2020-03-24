/* backlight.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include "task.h" /* xTaskCreate */
#include "queue.h" /* xQueueCreate */
#include "platform.h" /* hw_backlight_set */
#include "log.h" /* KERN_LOG */
#include "backlight.h"
#include "ambient.h"
#include "rebble_memory.h"
#include "rtoswrap.h"

static void _backlight_thread(void *pvParameters);
THREAD_DEFINE(backlight, configMINIMAL_STACK_SIZE - 22, tskIDLE_PRIORITY + 2UL, _backlight_thread);

typedef struct backlight_message
{
    uint8_t cmd;
    uint16_t val1;
    uint16_t val2;
} backlight_message_t;

#define BACKLIGHT_QUEUE_SIZE 2
QUEUE_DEFINE(backlight, backlight_message_t, BACKLIGHT_QUEUE_SIZE);

static uint16_t _backlight_brightness;
static uint8_t _backlight_is_on;

/*
 * Backlight is a go
 */
uint8_t rcore_backlight_init(void)
{
    hw_backlight_init();
    
    _backlight_is_on = 0;
    _backlight_brightness = 0;
   
    QUEUE_CREATE(backlight);
    THREAD_CREATE(backlight);

    rcore_backlight_on(100, 3000);
    
    return 0;
}

// In here goes the functions to dim the backlight
// on a timer

// use the backlight as additional alert by flashing it
void rcore_backlight_on(uint16_t brightness_pct, uint16_t time)
{
    backlight_message_t msg;

    //  send the queue the backlight on task
    msg.cmd = BACKLIGHT_ON;
    msg.val1 = brightness_pct;
    msg.val2 = time;
    xQueueSendToBack(_backlight_queue, (void *)&msg, 0);
}

/*
 * Set the backlight. At the moment this is scaled to be 4000 - mid brightness
 */
static void _backlight_set_raw(uint16_t brightness)
{
    _backlight_brightness = brightness;

    // set the display pwm value
    hw_backlight_set(brightness);
}

static void _backlight_set(uint16_t brightness_pct)
{
    uint16_t brightness;
    
    brightness = 8499 / (100 / brightness_pct);
    //KERN_LOG("backl", APP_LOG_LEVEL_DEBUG, "Brightness %d", brightness);

    _backlight_set_raw(brightness);
}


static void _backlight_set_from_ambient(void)
{
    uint16_t amb, bri;
    bri = _backlight_brightness;
    
    _backlight_set_raw(0);
    // give the led in the backlight time to de-energise
    delay_us(10);
    amb = rcore_ambient_get();
    
    // hacky brightness control here...
    // if amb is near 0, it is dark.
    // amb is 3500ish at about max brightness
    if (amb > 0)
    {
        amb = (8499 / (2 * amb)) + (8499 / 2);
        // restore the backlight
        _backlight_set_raw(amb);
        return;
    }
    // restore the backlight
    _backlight_set_raw(bri);
}

/*
 * Will take care of dimmng and time light is on etc
 */
static void _backlight_thread(void *pvParameters)
{
    backlight_message_t message;
    uint32_t wait = 0;  
    TickType_t on_expiry_time = xTaskGetTickCount();
    uint8_t backlight_status = BACKLIGHT_OFF;
    uint16_t bri_scale = 100;
    uint16_t bri_it = 50;

    while(1)
    {
        if (backlight_status == BACKLIGHT_FADE)
        {
            uint16_t newbri = (_backlight_brightness - bri_scale);
            wait = 80;
            _backlight_set_raw(newbri);

            bri_it--;
            
            if (bri_it == 0)
            {
                backlight_status = BACKLIGHT_OFF;
                _backlight_set_raw(0);
            }
        }
        else if (backlight_status == BACKLIGHT_ON)
        {
            // set the queue reader to immediately return
            wait = 500;
            
//             backlight_set_from_ambient();
            _backlight_set(bri_scale);
            
            if (xTaskGetTickCount() > on_expiry_time)
            {
                backlight_status = BACKLIGHT_FADE;
                bri_scale = _backlight_brightness / 50;
                bri_it = 50; // number of steps
            }
        }
        else
        {
            // We are idle so we can sleep for a bit
            wait = portMAX_DELAY;
        }
        
        if (xQueueReceive(_backlight_queue, &message, pdMS_TO_TICKS(wait)))
        {
            switch(message.cmd)
            {
                case BACKLIGHT_FADE:
                    break;
                case BACKLIGHT_OFF:
                    break;
                case BACKLIGHT_ON:
//                     KERN_LOG("backl", APP_LOG_LEVEL_DEBUG, "Backlight ON");
                    backlight_status = BACKLIGHT_ON;
                    // timestamp the tick counter so we can stay on for
                    // the right amount of time
                    bri_scale = message.val1;
                    on_expiry_time = xTaskGetTickCount() + (message.val2 / portTICK_RATE_MS);
                    _backlight_set(message.val1);
                    break;
                ;
            }
        }
    }
}
