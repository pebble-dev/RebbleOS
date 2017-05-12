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

#define malloc pvPortMalloc
#define calloc pvPortCalloc
#define free vPortFree

void *pvPortCalloc(size_t count, size_t size);
void rblos_memory_init(void);
bool rblos_memory_sanity_check_app(size_t size);
void *app_malloc(size_t size);
void *app_calloc(size_t count, size_t size);
void app_free(void *mem);

size_t xPortGetMinimumEverFreeAppHeapSize( void );
size_t xPortGetFreeAppHeapSize( void );
void vPortAppFree( void *pv );
void *pvPortAppMalloc( size_t xWantedSize );
void appHeapInit(size_t xTotalHeapSize, uint8_t *e_app_stack_heap);
