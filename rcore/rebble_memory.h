#pragma once
/* rebble_memory.h
 * routines for Allocating memory for system and apps. 
 * App memory gets alocated ont he app's own heap where sys has a global heap
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include <string.h>
#include <stdlib.h>
#include "stdbool.h"

#define malloc system_malloc
#define calloc system_calloc
#define free vPortFree

void *system_calloc(size_t count, size_t size);
void rblos_memory_init(void);
void *system_malloc(size_t size);

void *app_malloc(size_t size);
void *app_calloc(size_t count, size_t size);
void app_free(void *mem);
