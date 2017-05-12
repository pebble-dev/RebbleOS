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
#include "rebbleos.h"
#include "platform.h"
#include "flash.h"

void resource_init();
void resource_load_id(uint16_t resource_id, uint8_t *buffer);
// ResHandle resource_get_handle(uint32_t resource_id);
size_t resource_size(ResHandle handle);
void resource_load(ResHandle resource_handle, uint8_t *buffer);
uint8_t *resource_fully_load_id(uint16_t resource_id);
uint8_t *resource_fully_load_res(ResHandle res_handle);
ResHandle resource_get_handle_app(uint32_t resource_id, uint16_t slot_id);
uint8_t *resource_fully_load_res_app(ResHandle res_handle, uint16_t slot_id);
