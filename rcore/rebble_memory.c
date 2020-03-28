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

void *system_malloc(size_t size)
{
    if (appmanager_is_thread_system())
        LOG_ERROR("XXX System Malloc. Check who did this");
    return mem_heap_alloc(&mem_heaps[HEAP_SYSTEM], size);
}

void *system_calloc(size_t sz, size_t n) {
    void *p = system_malloc(sz * n);
    if (!p)
        return NULL;
    memset(p, 0, sz * n);
    return p;
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

/* Wrappers to "magically allocate" on the correct heap. */

static struct mem_heap *_heap_or_system() {
    struct mem_heap *heap = mem_thread_get_heap();
    if (!heap)
        return &mem_heaps[HEAP_SYSTEM];
    return heap;
}

static struct mem_heap *_heap_for_pointer(void *p) {
    for (int i = 0; i < HEAP_MAX; i++)
        if ((p >= mem_heaps[i].start) && (p <= (mem_heaps[i].start + mem_heaps[i].size)))
            return &mem_heaps[i];
    return NULL;
}

static void *_realloc(void *p, size_t new_size, int remote) {
    struct mem_heap *heap = _heap_or_system();
    if (p) {
        struct mem_heap *pheap = _heap_for_pointer(p);
        assert(pheap && "allocation operation on pointer that did not come from an alloc");
        assert(((pheap == heap) || remote) && "allocation operation on pointer from remote heap without acknowledging it first");
        heap = pheap;
    }
    
    if (!new_size) {
        mem_heap_free(heap, p);
        return NULL;
    } else
        return mem_heap_realloc(heap, p, new_size);
}

void *malloc(size_t sz) { return _realloc(NULL, sz, 0); }
void *realloc(void *p, size_t new_size) { return  _realloc(p, new_size, 0); }
void *remote_realloc(void *p, size_t new_size) { return  _realloc(p, new_size, 1); }
void free(void *p) { _realloc(p, 0, 0); }
void remote_free(void *p) { _realloc(p, 0, 1); }

void *calloc(size_t sz, size_t n) {
    void *p = malloc(sz * n);
    if (!p)
        return NULL;
    memset(p, 0, sz * n);
    return p;
}
