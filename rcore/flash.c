/* flash.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"
#include "fs.h"

extern void hw_flash_init(void);
extern void hw_flash_read_bytes(uint32_t, uint8_t*, size_t);

static SemaphoreHandle_t _flash_mutex;
static StaticSemaphore_t _flash_mutex_buf;
static SemaphoreHandle_t _flash_wait_semaphore;
static StaticSemaphore_t _flash_wait_semaphore_buf;

uint8_t flash_init()
{
    // initialise device specific flash
    hw_flash_init();
    
    _flash_mutex = xSemaphoreCreateMutexStatic(&_flash_mutex_buf);
    _flash_wait_semaphore = xSemaphoreCreateBinaryStatic(&_flash_wait_semaphore_buf);
    fs_init();
    
    return 0;
}

/*
 * Read a given number of bytes SAFELY from the flash chip
 * DO NOT use from an ISR
 */
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    hw_flash_read_bytes(address, buffer, num_bytes);
    
    /* sit the caller being this wait lock semaphore */
    if (!xSemaphoreTake(_flash_wait_semaphore, pdMS_TO_TICKS(200)))
        panic("Got stuck behind a wait lock in flash.c");

    xSemaphoreGive(_flash_mutex);
}

void flash_dump(void)
{
    uint8_t buffer[1025];
//     xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    
    for (int i = 0; i < (16384); i++)
    {
        flash_read_bytes(i * 1024, buffer, 1024);
    
        ss_debug_write(buffer, 1024);
    }
 
 while(1);
//     xSemaphoreGive(_flash_mutex);
}

inline void flash_operation_complete(uint8_t cmd)
{
    /* Notify the task that the transmission is complete. */
    xSemaphoreGive(_flash_wait_semaphore);
}

void flash_operation_complete_isr(uint8_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* wake the flash */
    xSemaphoreGiveFromISR(_flash_wait_semaphore, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
