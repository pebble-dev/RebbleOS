/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "rebbleos.h"

static TaskHandle_t xBacklightTask;
static xQueueHandle xBacklightQueue;
void vBacklightTask(void *pvParameters);

struct backlight_message_t
{
    uint8_t cmd;
    uint16_t val1;
    uint16_t val2;
} backlight_message;

uint16_t backlight_brightness;
uint8_t backlight_is_on;

/*
 * Backlight is a go
 */
void backlight_init(void)
{
    hw_backlight_init();
    
    xTaskCreate(vBacklightTask, "Bl", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &xBacklightTask);    
    
    xBacklightQueue = xQueueCreate( 2, sizeof(struct backlight_message_t *));
        
    printf("Backlight Tasks Created!\n");
    backlight_is_on = 0;
    backlight_brightness = 0;

    backlight_on(100, 3000);
}

// In here goes the functions to dim the backlight
// on a timer

// use the backlight as additional alert by flashing it
void backlight_on(uint16_t brightness_pct, uint16_t time)
{
    struct backlight_message_t *message;
    printf("bl\n");
    //  send the queue the backlight on task
    message = &backlight_message;
    message->cmd = BACKLIGHT_ON;
    message->val1 = brightness_pct;
    message->val2 = time;
    xQueueSendToBack(xBacklightQueue, &message, 0);
}


void backlight_set(uint16_t brightness_pct)
{
    uint16_t brightness;
    
    brightness = 8499 / (100 / brightness_pct);
    printf("BRIGHTNESS %d\n", brightness); // display.Brightness);            

    backlight_set_raw(brightness);
}

/*
 * Set the backlight. At the moment this is scaled to be 4000 - mid brightness
 */
void backlight_set_raw(uint16_t brightness)
{
    backlight_brightness = brightness;

    // set the display pwm value
    hw_backlight_set(brightness);
}

void backlight_set_from_ambient(void)
{
    uint16_t amb, bri;
    bri = backlight_brightness;
    
    backlight_set_raw(0);
    // give the led in the backlight time to de-energise
    delay_us(10);
    amb = ambient_get();
    
    // hacky brightness control here...
    // if amb is near 0, it is dark.
    // amb is 3500ish at about max brightness
    if (amb > 0)
    {
        amb = (8499 / (2 * amb)) + (8499 / 2);
        // restore the backlight
        backlight_set_raw(amb);
        return;
    }
    // restore the backlight
    backlight_set_raw(bri);
}

/*
 * Will take care of dimmng and time light is on etc
 */
void vBacklightTask(void *pvParameters)
{
    struct backlight_message_t *message;
//     const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    uint8_t wait = 0;
    TickType_t on_expiry_time;
    uint8_t backlight_status;
    uint16_t bri_scale;
    uint16_t bri_it;
    
    while(1)
    {
        if (backlight_status == BACKLIGHT_FADE)
        {
            uint16_t newbri = (backlight_brightness - bri_scale);
            wait = 10;
            backlight_set_raw(newbri);

            bri_it--;
            
            if (bri_it == 0)
            {
                backlight_status = BACKLIGHT_OFF;
                backlight_set_raw(0);
            }
        }
        else if (backlight_status == BACKLIGHT_ON)
        {
            // set the queue reader to immediately return
            wait = 50;
            
//             backlight_set_from_ambient();
            backlight_set(bri_scale);
            
            if (xTaskGetTickCount() > on_expiry_time)
            {
                backlight_status = BACKLIGHT_FADE;
                bri_scale = backlight_brightness / 50;
                bri_it = 50; // number of steps
            }
        }
        else
        {
            // We are idle so we can sleep for a bit
            wait = 1000 / portTICK_RATE_MS;
        }
        
        if (xQueueReceive(xBacklightQueue, &message, wait))
        {
            switch(message->cmd)
            {
                case BACKLIGHT_FADE:
                    break;
                case BACKLIGHT_OFF:
                    break;
                case BACKLIGHT_ON:
                    printf("bl on %d\n", message->val1);
                    backlight_status = BACKLIGHT_ON;
                    // timestamp the tick counter so we can stay on for
                    // the right amount of time
                    bri_scale = message->val1;
                    on_expiry_time = xTaskGetTickCount() + (message->val2 / portTICK_RATE_MS);
                    backlight_set(message->val1);
                    break;
            }
        }
    }
}
