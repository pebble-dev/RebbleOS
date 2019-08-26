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
#include <stdbool.h>
#include "rtoswrap.h"

#define VIBRATE_QUEUE_MAX_WAIT_TICKS  20    // chosen by fair dice roll

#define VIBRATE_QUEUE_MAX_ITEMS 5

/**
 * Initialization of default patterns. 
 * buffer: contains the sequence of pairs, where each pair is composed of a frequency (at which the motor will spin)
 *      ,and a duration of spin (in milliseconds).
 * length:is the length of the buffer
 * 
 * TODO Maybe load these dinamically at some point
 */
static const VibratePattern_t _default_vibrate_patterns[VIBRATE_CMD_MAX] = {
    // VIBRATE_CMD_PLAY_PATTERN_1
    {
        .length =  1, 
        .buffer = (const VibratePatternPair_t [])  {{.frequency = 255, .duration_ms = 1000}},                   
    },
    
    // VIBRATE_CMD_PLAY_PATTERN_2
    {
        .length =  2, 
        .buffer = (const VibratePatternPair_t [])  {{.frequency = 255, .duration_ms = 100}, 
                                                    {.frequency = 255, .duration_ms = 750}},
    },
    
    // VIBRATE_CMD_PLAY_PATTERN_3
    {
        .length = 25, 
        .buffer = (const VibratePatternPair_t [])  {{.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100},
                                                    {.frequency = 127, .duration_ms = 750},
                                                    {.frequency = 255, .duration_ms = 100}},
    },
    
    // VIBRATE_CMD_STOP
    {
        .length =  0, 
        .buffer = (const VibratePatternPair_t [])  {{.frequency = 0, .duration_ms = 0}}
    }
};

static void _enable(uint8_t enabled);
static void _set_frequency(uint16_t frequency);
static void _vibrate_thread(void *pvParameters);
static void _print_pattern(const VibratePattern_t *pattern);

THREAD_DEFINE(vibrate, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 2UL, _vibrate_thread);
QUEUE_DEFINE(vibrate, VibratePattern_t *, VIBRATE_QUEUE_MAX_ITEMS);

/*
 * Initialize the vibration controller and tasks
 */
uint8_t vibrate_init(void)
{
    int rv;
    hw_vibrate_init();

    QUEUE_CREATE(vibrate);
    THREAD_CREATE(vibrate);
    
    return 0;
}

/**
 * Execute a command
 * @param command Command to execute. Note that VIBRATE_CMD_MAX is not a valid command.
 */
void vibrate_command(VibrateCmd_t command)
{
    if (command < VIBRATE_CMD_MAX)
    {
        vibrate_play_pattern(&_default_vibrate_patterns[command]);
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
void vibrate_play_pattern(const VibratePattern_t *pattern)
{
    (void) xQueueSendToBack(_vibrate_queue, &pattern, VIBRATE_QUEUE_MAX_WAIT_TICKS);   
}

/**
 * Stop playing a pattern. If the _vibrate_thread is currently blocked by vTaskDelay,
 * the pattern will be stopped when vTaskDelay returns.
 */
void vibrate_stop(void)
{
    
    (void) xQueueSendToBack(_vibrate_queue, &_default_vibrate_patterns[VIBRATE_CMD_STOP], VIBRATE_QUEUE_MAX_WAIT_TICKS);    
}

static void _print_pattern(const VibratePattern_t *pattern)
{
//     printf(">>> vibrate @%u:\n", pattern);
//     printf("\tlength:%d, val[%d]=(.frequency=%u, .duration=%u)\n",
//            pattern->length,
//            pattern->cur_buffer_index,
//            pattern->buffer[pattern->cur_buffer_index].frequency,
//            pattern->buffer[pattern->cur_buffer_index].duration_ms
//     );
}

/**
 * Start vibratin
 */
static void _enable(uint8_t enabled)
{
    hw_vibrate_enable(enabled);
}

/**
 * Set vibration frequency
 */
static void _set_frequency(uint16_t frequency)
{
    // TODO set the motor frequency here
}

/*
 * The main task for driving patterns out of the motor
 */
static void _vibrate_thread(void *pvParameters)
{
    static const VibratePattern_t *current_pattern = &_default_vibrate_patterns[VIBRATE_CMD_STOP];

    while(1)
    {
        uint32_t buf_idx = 0;
        
        if (!xQueueReceive(_vibrate_queue, &current_pattern, portMAX_DELAY))
        {
            continue;
        }
        
        buf_idx = 0;
        
        while (buf_idx < current_pattern->length)
        {
            _set_frequency(current_pattern->buffer[buf_idx].frequency);
            _enable(true);
            vTaskDelay(current_pattern->buffer[buf_idx].duration_ms / portTICK_RATE_MS);
            _enable(false);
            
            if (xQueueReceive(_vibrate_queue, &current_pattern, 0))
            {
                // if we just received another pattern (including stop), start playing it from the beginning
                buf_idx = 0;
            }
            else
            {
                buf_idx++;
            }
            
            _print_pattern(current_pattern);
        }
        
        current_pattern = &_default_vibrate_patterns[VIBRATE_CMD_STOP];
    }
}
