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
#include "debug.h"
#include "stdio.h"
#include "string.h"
#include "platform.h"
#include "vibrate.h"
#include "task.h"
#include "semphr.h"
#include <stdbool.h>

typedef struct
{
   const VibrateCmd_t command;
   size_t curBufferIndex;
   const size_t length;
   const uint16_t * const buffer;
} VibrateTrack_t;

/*
 * Definitions of all tracks.
 * 
 * TODO Dinamically load these.
 */
static VibrateTrack_t _vibrateTracks[VIBRATE_CMD_MAX] = {
   {.command = VIBRATE_CMD_STOP     , .length = 1, .buffer = (const uint16_t []) {100}},
   {.command = VIBRATE_CMD_PATTERN_1, .length = 2, .buffer = (const uint16_t []) {255, 1000}},
   {.command = VIBRATE_CMD_PATTERN_2, .length = 6, .buffer = (const uint16_t []) {255, 750, 0, 750, 255, 750}},
};

static TaskHandle_t _vibrate_task;
static xQueueHandle _vibrate_queue;

static void _vibrate_thread(void *pvParameters);
static void _enable(uint8_t enabled);
static void _play_track(VibrateTrack_t *pattern, bool doRepeat);
static void _rewind_track(VibrateTrack_t* pattern);
static void _rewind_all_tracks(void);

/*
 * Initialise the vibration controller and tasks
 */
void vibrate_init(void)
{
    int rv;
        
    hw_vibrate_init();
    _rewind_all_tracks();
    
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
 * Play a given pattern.
 */
static void _play_track(VibrateTrack_t *pattern, bool doRepeat)
{
    assert(pattern->curBufferIndex <= pattern->length);
      
    if(pattern->curBufferIndex < pattern->length)
    {
        if (pattern->buffer[pattern->curBufferIndex] > 0)
        {
            _enable(true);
        }
       
        vTaskDelay(pattern->buffer[pattern->curBufferIndex] / portTICK_RATE_MS);
       
        _enable(false);
        pattern->curBufferIndex++;
    }
    else if (doRepeat)
    {
        pattern->curBufferIndex = 0;
    }
}

/*
 * Reset all patterns to their default state
 */
static void _rewind_all_tracks()
{
    for(VibrateCmd_t i = 0; i < VIBRATE_CMD_MAX; i++)
    {
        _vibrateTracks[i].curBufferIndex = 0;
    }
}

/*
 * Set a track's position to 0
 */
static void _rewind_track(VibrateTrack_t* pattern)
{
    pattern->curBufferIndex = 0;
}

/*
 * The main task for driving patterns out of the motor
 */
static void _vibrate_thread(void *pvParameters)
{
    VibrateCmd_t command;
    static VibrateTrack_t *currentTrack = &_vibrateTracks[VIBRATE_CMD_STOP];
   
    while(1)
    {       
        if (xQueueReceive(_vibrate_queue, &command, portMAX_DELAY))
        {
            if (command < VIBRATE_CMD_MAX)
            {
                currentTrack = &_vibrateTracks[command];
                _rewind_track(currentTrack);
            }
        }
         
        _play_track(currentTrack, false);
    }
}
