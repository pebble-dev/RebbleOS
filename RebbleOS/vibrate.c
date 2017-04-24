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

#define VIBRATE_QUEUE_MAX_WAIT_TICKS  20    // chosen by fair dice roll


/*
 * Initialization of default patterns.
 * 
 * TODO Maybe load these dinamically at some point
 */
static VibratePattern_t _defaultVibratePatterns[VIBRATE_CMD_MAX] = {
    {.length =  2, .buffer = (const uint16_t []) {255, 1000}},                   // VIBRATE_CMD_PLAY_PATTERN_1
    {.length =  6, .buffer = (const uint16_t []) {255, 750, 0, 750, 255, 750}},  // VIBRATE_CMD_PALY_PATTERN_2
    {.length = 30, .buffer = (const uint16_t []) {255, 750, 0, 750, 255, 750,
                                                  255, 750, 0, 750, 255, 750, 
                                                  255, 750, 0, 750, 255, 750, 
                                                  255, 750, 0, 750, 255, 750, 
                                                  255, 750, 0, 750, 255, 750}},  // VIBRATE_CMD_PALY_PATTERN_3
    {.length =  0, .buffer = (const uint16_t []) {}},                          // VIBRATE_CMD_STOP
};

static TaskHandle_t _vibrate_task;
static xQueueHandle _vibrate_queue;
static VibratePattern_t *currentPattern;

static void _enable(uint8_t enabled);
static void _play(void);
static void _vibrate_thread(void *pvParameters);

/*
 * Initialize the vibration controller and tasks
 */
void vibrate_init(void)
{
    int rv;
    
    hw_vibrate_init();
    
    rv = xTaskCreate(_vibrate_thread, "Vibrate", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &_vibrate_task); /* XXX: allocate statically later */
    assert(rv == pdPASS);
    
    _vibrate_queue = xQueueCreate(1, sizeof(uint8_t));
}

/**
 * Execute a command
 * @param command Command to execute. Note that VIBRATE_CMD_MAX is not a valid command.
 */
void vibrate_command(VibrateCmd_t command)
{
    if (command < VIBRATE_CMD_MAX)
    {
        vibrate_play_pattern(&_defaultVibratePatterns[command]);
    }
    else
    {
        // ya dun goofed
        assert(false);
    }
}

/**
 * Play a given pattern.
 * @param pattern Pointer to a struct containing a defined pattern.
 */
void vibrate_play_pattern(VibratePattern_t *pattern)
{
    pattern->curBufferIndex = 0;
    (void) xQueueSendToBack(_vibrate_queue, &pattern, VIBRATE_QUEUE_MAX_WAIT_TICKS);   
}

/**
 * Stop playing a pattern. If the _vibrate_thread is currently blocked by vTaskDelay,
 * the pattern will be stopped when vTaskDelay returns.
 */
void vibrate_stop(void)
{
    
    (void) xQueueSendToBack(_vibrate_queue, &_defaultVibratePatterns[VIBRATE_CMD_STOP], VIBRATE_QUEUE_MAX_WAIT_TICKS);    
}

/**
 * Start vibratin
 */
static void _enable(uint8_t enabled)
{
    hw_vibrate_enable(enabled);
}


/**
 * Play the currently selected pattern.
 */
static void _play()
{
    while (currentPattern->curBufferIndex < currentPattern->length)
    {        
        if (currentPattern->buffer[currentPattern->curBufferIndex] > 0)
        {
            _enable(true);
        }
        
        vTaskDelay(currentPattern->buffer[currentPattern->curBufferIndex] / portTICK_RATE_MS);
        _enable(false);
        
        
        if (xQueueReceive(_vibrate_queue, &currentPattern, 0))
        {
            // if we just received another pattern (including stop),
            // start playing it from the beginning
            currentPattern->curBufferIndex = 0;
        }
        else
        {
            currentPattern->curBufferIndex++;
        }
    }
    
    currentPattern = &_defaultVibratePatterns[VIBRATE_CMD_STOP];
}

/*
 * The main task for driving patterns out of the motor
 */
static void _vibrate_thread(void *pvParameters)
{
    currentPattern = &_defaultVibratePatterns[VIBRATE_CMD_STOP];
    
    while(1)
    {
        if (!xQueueReceive(_vibrate_queue, &currentPattern, portMAX_DELAY))
        {
            continue;
        }
        
        _play();
    }
}
