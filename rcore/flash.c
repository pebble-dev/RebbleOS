/* flash.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"

extern void hw_flash_init(void);
extern void hw_flash_read_bytes(uint32_t, uint8_t*, size_t);

// TODO
// DMA/async?
// what about apps/watchface resource loading?
// document

/// MUTEX
static SemaphoreHandle_t _flash_mutex;
static StaticSemaphore_t _flash_mutex_buf;
uint32_t _flash_get_app_slot_address(uint16_t slot_id);
extern unsigned int _ram_top;
#define portMPU_REGION_READ_WRITE (0x03UL << MPU_RASR_AP_Pos)

static struct hw_driver_ext_flash_t *_flash_driver;

// No ISR here (yet)
static hw_driver_handler_t _callack_handler = {
    .done_isr = NULL
};

void flash_init()
{
    // initialise device specific flash
    _flash_driver = (hw_driver_ext_flash_t *)driver_register((hw_driver_module_init_t)hw_flash_module_init, &_callack_handler);
    assert(_flash_driver->read_bytes && "Read is invalid");
    
//     MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
//     MPU->RNR  = 0;
//     MPU->RBAR = 0x20000000;
//     MPU->RASR = _ram_top | portMPU_REGION_READ_WRITE  | MPU_RASR_XN_Msk;
//     SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
//     MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;

    _flash_mutex = xSemaphoreCreateMutexStatic(&_flash_mutex_buf);
}

/*
 * Read a given number of bytes SAFELY from the flash chip
 */
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    xSemaphoreTake(_flash_mutex, portMAX_DELAY);

    _flash_driver->read_bytes(address, buffer, num_bytes);
    
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


void flash_load_app_header(uint16_t app_id, ApplicationHeader *header)
{
    flash_read_bytes(_flash_get_app_slot_address(app_id), (uint8_t *)header, sizeof(ApplicationHeader));
}

void flash_load_app(uint16_t app_id, uint8_t *buffer, size_t count)
{
    flash_read_bytes(_flash_get_app_slot_address(app_id), buffer, count);
}

uint32_t _flash_get_app_slot_address(uint16_t slot_id)
{
    // I still don't really get the flash layout. sometimes apps appear in different pages
    if (slot_id < 8)
        return APP_SLOT_0_START + (slot_id * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 16)
        return APP_SLOT_8_START + (slot_id - 8 * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 24)
        return APP_SLOT_16_START + ((slot_id - 16) * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 32)
        return APP_SLOT_24_START + ((slot_id - 24) * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;

    return 0;
}

uint32_t flash_get_resource_address(uint16_t slot_id)
{
    // get the address for this slot, then work backwards in 0x2000 block 
    // increments looking for the resources
    uint32_t faddr = _flash_get_app_slot_address(slot_id);
    uint32_t block_size = 0x2000;
    
    // get the header identifier. it sits before the app
    AppTypeHeader app_header, fr;
    flash_read_bytes(faddr - sizeof(AppTypeHeader), (uint8_t *)&app_header, sizeof(AppTypeHeader));
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "%s", app_header.address);
    
    for (uint32_t i = faddr - block_size; i > (faddr - (block_size * 10)); i -= block_size)
    {
        flash_read_bytes(i - 13, (uint8_t *)&fr, sizeof(AppTypeHeader));
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "Resource %s %x", fr.address, i);
        // check to see if this block contains our header
        if (!strncmp(app_header.address, fr.address, 8))
        {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "Found Resource %s %x", fr.address, i);
            return i;
        }
    }
    
    return 0;
}
