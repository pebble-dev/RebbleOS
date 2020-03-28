/* rebble_memory.c
 * routines for Allocating memory for system and apps. 
 * App memory gets allocated in it's relevant arena heap. System has the rest
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

/* Configure Logging */
#define MODULE_NAME "mem"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_ERROR //RBL_LOG_LEVEL_NONE

void *system_calloc(size_t count, size_t size)
{
    if (!appmanager_is_thread_system())
        LOG_ERROR("XXX System Calloc. Check who did this");

    void *p = mem_heap_alloc(&mem_heaps[HEAP_SYSTEM], count * size);
    if (!p)
        return NULL;
    memset(p, 0, count * size);
    return p;
}

void *system_malloc(size_t size)
{
    if (appmanager_is_thread_system())
        LOG_ERROR("XXX System Malloc. Check who did this");
    return mem_heap_alloc(&mem_heaps[HEAP_SYSTEM], size);
}

void *pvPortMalloc(size_t size) {
    return system_malloc(size);
}

void system_free(void *mem)
{
    if (appmanager_is_thread_system())
        LOG_ERROR("XXX System Free. Check who did this");
    return mem_heap_free(&mem_heaps[HEAP_SYSTEM], mem);
}

void vPortFree(void *p) {
    return system_free(p);
}

void *app_malloc(size_t size)
{
    return app_calloc(1, size);    
}

void *app_calloc(size_t count, size_t size)
{
    assert(mem_thread_get_heap());
    void *p = mem_heap_alloc(mem_thread_get_heap(), count * size);
    if (!p) {
        LOG_ERROR("app_calloc(%d) failed", count * size);
        return NULL;
    }
    memset(p, 0, count * size);
    return p;
}

void app_free(void *p)
{
    assert(mem_thread_get_heap());
    mem_heap_free(mem_thread_get_heap(), p);
}

void *app_realloc(void *mem, size_t new_size)
{
    assert(mem_thread_get_heap());
    return mem_heap_realloc(mem_thread_get_heap(), mem, new_size);
}

uint32_t app_heap_bytes_free(void)
{
    assert(mem_thread_get_heap());

    assert(mem_thread_get_heap()->arena); /* XXX: lock */
    return qfreebytes(mem_thread_get_heap()->arena);
}

uint32_t app_heap_bytes_used(void)
{
    assert(mem_thread_get_heap());

    assert(mem_thread_get_heap()->arena); /* XXX: lock */
    return qusedbytes(mem_thread_get_heap()->arena);
}

/* Thread-local heap implementation. */


/* Define the available heaps. */

static                     uint8_t _heap_system[MEMORY_SIZE_SYSTEM];
static MEM_REGION_HEAP_OVL uint8_t _heap_overlay[MEMORY_SIZE_OVERLAY_HEAP];
static                     uint8_t _heap_app[MEMORY_SIZE_APP_HEAP];
static MEM_REGION_HEAP_WRK uint8_t _heap_worker[MEMORY_SIZE_WORKER_HEAP];

struct mem_heap mem_heaps[HEAP_MAX] = {
    [HEAP_SYSTEM]  = { _heap_system,  MEMORY_SIZE_SYSTEM },
    [HEAP_OVERLAY] = { _heap_overlay, MEMORY_SIZE_OVERLAY_HEAP },
    [HEAP_APP]     = { _heap_app,     MEMORY_SIZE_APP_HEAP },
    [HEAP_WORKER]  = { _heap_worker,  MEMORY_SIZE_WORKER_HEAP } 
};

void mem_init() {
    mem_heap_init(&mem_heaps[HEAP_SYSTEM]);
}

void mem_heap_init(struct mem_heap *heap) {
    memset(heap->start, 0, heap->size);
    heap->mutex = xSemaphoreCreateMutexStatic(&heap->mutex_buf);
    heap->arena = qinit(heap->start, heap->size);
}

void *mem_heap_alloc(struct mem_heap *heap, size_t newsz) {
    return mem_heap_realloc(heap, NULL, newsz);
}

void *mem_heap_realloc(struct mem_heap *heap, void *p, size_t newsz) {
    assert(heap->arena);
    
    xSemaphoreTake(heap->mutex, portMAX_DELAY);
    void *rp = qrealloc(heap->arena, p, newsz);
    xSemaphoreGive(heap->mutex);

    return rp;
}

void mem_heap_free(struct mem_heap *heap, void *p) {
    assert(heap->arena);
    
    xSemaphoreTake(heap->mutex, portMAX_DELAY);
    qfree(heap->arena, p);
    xSemaphoreGive(heap->mutex);
}

void mem_thread_set_heap(struct mem_heap *heap) {
    vTaskSetThreadLocalStoragePointer(NULL /* this task */, FREERTOS_TLS_CUR_HEAP, heap);
}

struct mem_heap *mem_thread_get_heap() {
    return pvTaskGetThreadLocalStoragePointer(NULL /* this task */, FREERTOS_TLS_CUR_HEAP);
}
