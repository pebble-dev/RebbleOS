/* ambient.c
 * routines for [...]
 * RebbleOS
 *
 * Authors: Barry Carter <barry.carter@gmail.com>, Joshua Wise <joshua@joshuawise.com>
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "platform.h"
#include "semphr.h"
#include "ambient.h"

static SemaphoreHandle_t _ambient_mutex;
static StaticSemaphore_t _ambient_mutex_buf;

void rblcore_ambient_init(void)
{
    hw_ambient_init();
    _ambient_mutex = xSemaphoreCreateMutexStatic(&_ambient_mutex_buf);
}

uint16_t rblcore_ambient_get(void)
{
    // take multiple samples
    // use a simple moving average filter to get the settle
    
    // disable the backlight biefly to get a good clean samples
    // of the real world
    
    // XXX: only turn on the ambient light sensor every once in a while, to
    // avoid burning battery by calling in a loop
    
    uint16_t val;
    
    xSemaphoreTake(_ambient_mutex, portMAX_DELAY);
    val = hw_ambient_get();
    xSemaphoreGive(_ambient_mutex);
    
    return val;
}
