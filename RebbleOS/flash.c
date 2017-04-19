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


void flash_init()
{
    hw_flash_init();
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

uint16_t flash_read16(uint32_t address)
{
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    return hw_flash_read16(address);
    xSemaphoreGive(flash_mutex);
}

uint32_t flash_read32(uint32_t address)
{
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    return hw_flash_read32(address);
    xSemaphoreGive(flash_mutex);
}
