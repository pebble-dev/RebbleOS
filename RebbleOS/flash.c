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
#include "platform.h"
#include "flash.h"

// TODO
// DMA/async?
// what about apps/watchface resource loading?
// document


/// MUTEX
static SemaphoreHandle_t flash_mutex;
uint32_t _flash_get_app_slot_address(uint8_t slot_id);
extern unsigned int _ram_top;
#define portMPU_REGION_READ_WRITE (0x03UL << MPU_RASR_AP_Pos)
void flash_init()
{
    hw_flash_init();
//     MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
//     MPU->RNR  = 0;
//     MPU->RBAR = 0x20000000;
//     MPU->RASR = _ram_top | portMPU_REGION_READ_WRITE  | MPU_RASR_XN_Msk;
//     SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
//     MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;

    flash_mutex = xSemaphoreCreateMutex();
}

/*
 * Read a given number of bytes SAFELY from the flash chip
 */
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    hw_flash_read_bytes(address, buffer, num_bytes);
    xSemaphoreGive(flash_mutex);
}

void flash_dump(void)
{
    uint8_t buffer[1025];
//     xSemaphoreTake(flash_mutex, portMAX_DELAY);
    
    for (int i = 0; i < (16384); i++)
    {
        flash_read_bytes(i * 1024, buffer, 1024);
    
        ss_debug_write(buffer, 1024);
    }
 
 while(1);
//     xSemaphoreGive(flash_mutex);
}


void flash_load_app_header(uint8_t app_id, ApplicationHeader *header)
{
    flash_read_bytes(_flash_get_app_slot_address(app_id), header, sizeof(ApplicationHeader));
}

void flash_load_app(uint8_t app_id, uint8_t *buffer, size_t count)
{
    flash_read_bytes(_flash_get_app_slot_address(app_id), buffer, count);
}

uint32_t _flash_get_app_slot_address(uint8_t slot_id)
{
    // I still don't really get the flash layout. sometimes apps appear in different pages
    if (slot_id < 8)
        return APP_SLOT_1_START + (slot_id * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 16)
        return APP_SLOT_9_START + (slot_id - 8 * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 24)
        return APP_SLOT_17_START + ((slot_id - 16) * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
}


BssInfo flash_get_bss(uint8_t slot_id)
{
    BssInfo bss;
    flash_read_bytes(_flash_get_app_slot_address(slot_id) + sizeof(ApplicationHeader) - 2, &bss, 8);
    return bss;
}
