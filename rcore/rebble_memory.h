#pragma once
/* rebble_memory.h
 * routines for Allocating memory for system and apps. 
 * App memory gets alocated ont he app's own heap where sys has a global heap
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include "semphr.h"
#include "qalloc.h"
#include <string.h>
#include <stdlib.h>
#include "stdbool.h"

void *system_calloc(size_t count, size_t size);
void rblos_memory_init(void);
void *system_malloc(size_t size);
void system_free(void *mem);

#define app_malloc malloc
#define app_calloc calloc
#define app_realloc realloc
#define app_free free
uint32_t app_heap_bytes_free(void);
uint32_t app_heap_bytes_used(void);

/* Each thread has its own "home heap". [...] */

struct mem_heap {
    void *start;
    size_t size;
    
    qarena_t *arena;
    SemaphoreHandle_t mutex;
    StaticSemaphore_t mutex_buf;
};

enum {
    HEAP_SYSTEM,
    HEAP_OVERLAY,
    HEAP_APP,
    HEAP_WORKER,
    HEAP_MAX
};

extern struct mem_heap mem_heaps[HEAP_MAX];

void mem_init();

void mem_heap_init(struct mem_heap *heap);
void *mem_heap_alloc(struct mem_heap *heap, size_t newsz);
void *mem_heap_realloc(struct mem_heap *heap, void *p, size_t newsz);
void mem_heap_free(struct mem_heap *heap, void *p);
void mem_thread_set_heap(struct mem_heap *heap);
struct mem_heap *mem_thread_get_heap();
