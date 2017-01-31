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
#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "platform.h"
#include "vibrate.h"
#include "task.h"
#include "semphr.h"


static TaskHandle_t xVibratePatternTask;
static xQueueHandle xQueue;

/*
 * Initialise the vibration controller and tasks
 */
void vibrate_init(void)
{
    hw_vibrate_init();
    
    xTaskCreate(vVibratePatternTask, "Vibrate", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &xVibratePatternTask);    
    
    xQueue = xQueueCreate( 10, sizeof(uint8_t) );
}

/*
 * Start vibratin
 */
void vibrate_enable(uint8_t enabled)
{
    hw_vibrate_enable(enabled);
}

/*
 * Quickly and dirtily pop out a pattern on the motor
 */
void vibrate_pattern(uint16_t *pattern, uint8_t pattern_length)
{
    // set the timer 12 CH2 to pwm to vibrate
    
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
            vibrate_enable(1);
        vTaskDelay(pattern[i+1] / portTICK_RATE_MS);
        vibrate_enable(0);
    }
}

/*
 * The main task for driving patterns out of the motor
 */
void vVibratePatternTask(void *pvParameters)
{
    uint8_t data;

    while(1)
    {       
        if (xQueueReceive(xQueue, &data, 0))
        {
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
            vibrate_pattern(pattern, len);
        }
        else
        {
            // shouldn't happen
        }        
    }
}
