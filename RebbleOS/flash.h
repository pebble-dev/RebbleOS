#pragma once
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

/* flash regions have moved to platform.h / platform_config.h */

#define RES_COUNT           0x00
#define RES_CRC             0x04
#define RES_TABLE_START     0x0C


/*
 
 @4c
*/
typedef struct ResourceHeader {
    char resource_name[13];
    uint32_t resource_list_count;
    uint32_t resource_list_crc_maybe;
    uint32_t unknownoffset;
} __attribute__((__packed__)) ResourceHeader;
 

void flash_test(uint16_t resource_id);
void flash_init(void);
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes);
void flash_load_app(uint16_t app_id, uint8_t *buffer, size_t count);
void flash_load_app_header(uint16_t app_id, ApplicationHeader *header);
