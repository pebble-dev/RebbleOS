/* flash.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"
#include "fs.h"

extern void hw_flash_init(void);
extern void hw_flash_read_bytes(uint32_t, uint8_t*, size_t);
extern int hw_flash_erase_sync(uint32_t addr, uint32_t len);
extern int hw_flash_write_sync(uint32_t addr, uint8_t *buf, size_t len);

static SemaphoreHandle_t _flash_mutex;
static StaticSemaphore_t _flash_mutex_buf;
static SemaphoreHandle_t _flash_wait_semaphore;
static StaticSemaphore_t _flash_wait_semaphore_buf;

#include "rdb.h"

static int _insert(int key, int dsize) {
    struct rdb_database *db = rdb_open(RDB_ID_TEST);
    
    uint32_t *keyp = malloc(16);
    keyp[0] = key;
    keyp[1] = key;
    keyp[2] = key;
    keyp[3] = key;
    
    uint8_t *val = app_calloc(1, dsize);
    for (int i = 0; i < dsize; i++)
        val[i] = (key & 0xFF) ^ i;
    
    int rv = rdb_insert(db, keyp, 16, val, dsize);
    
    app_free(val);
    app_free(keyp);
    
    rdb_close(db);
    
    return rv != Blob_Success;
}


static int _retrieve(int key, int dsize, int  inverse) {
    struct rdb_database *db = rdb_open(RDB_ID_TEST);
    struct rdb_iter it;
    rdb_select_result_list head;
    int ret = 0;
    
    list_init_head(&head);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        ret = 1;
        goto fail;
    }
    
    uint32_t *keyp = malloc(16);
    keyp[0] = key;
    keyp[1] = key;
    keyp[2] = key;
    keyp[3] = key;
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, 16, RDB_OP_EQ, keyp },
        { 0, 0, RDB_OP_RESULT_FULLY_LOAD },
        { }
    };
    int n = rdb_select(&it, &head, selectors);
    if (n != 1) {
        ret = 2;
        goto fail;
    }
    
    free(keyp);
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    for (int i = 0; i < dsize; i++) {
        uint8_t exp = (key & 0xFF) ^ (inverse ? dsize - 1 - i : i);
        uint8_t rd = ((uint8_t *)res->result[0])[i];
        if (exp != rd) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "rdb_select incorrect readback key %d size %d ofs %d, should be %02x is %02x", key, dsize, i, exp, rd);
            ret = 3;
            goto fail;
        }
    }
    
    res = 0;
    
fail:
    rdb_select_free_all(&head);
    rdb_close(db);

    return ret;
}

static int _delete(int key) {
    struct rdb_database *db = rdb_open(RDB_ID_TEST);
    struct rdb_iter it;
    rdb_select_result_list head;
    
    list_init_head(&head);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv)
        return 1;

    uint32_t *keyp = malloc(16);
    keyp[0] = key;
    keyp[1] = key;
    keyp[2] = key;
    keyp[3] = key;

    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, 16, RDB_OP_EQ, keyp },
        { }
    };
    int n = rdb_select(&it, &head, selectors);
    if (n != 1)
        return 2;
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    rdb_delete(&res->it);
    
    free(keyp);
    
    rdb_close(db);
    
    return 0;
}

uint8_t flash_init()
{
    // initialise device specific flash
    hw_flash_init();
    
    _flash_mutex = xSemaphoreCreateMutexStatic(&_flash_mutex_buf);
    _flash_wait_semaphore = xSemaphoreCreateBinaryStatic(&_flash_wait_semaphore_buf);
    
    /* hack hack hack */
    fs_format();
    
    fs_init();
    
    /* hack hack hack */
    for (int i = 0; i < 32; i++) {
        KERN_LOG("flash", APP_LOG_LEVEL_INFO, "*** ROUND %d ***", i);
        assert(_insert(1, 31) == 0);
        assert(_insert(2, 43) == 0);
        assert(_retrieve(1, 31, 0) == 0);
        assert(_retrieve(2, 43, 0) == 0);
        assert(_delete(1) == 0);
        assert(_insert(3, 129) == 0);
        assert(_insert(4, 76) == 0);
        assert(_retrieve(2, 43, 0) == 0);
        assert(_delete(2) == 0);
        assert(_retrieve(3, 129, 0) == 0);
        assert(_delete(3) == 0);
        assert(_retrieve(4, 76, 0) == 0);
        assert(_delete(4) == 0);
    }
    
    panic("done");
    
    return 0;
}

/* Some platforms have brain damage, like nRF52840, and can't read from
 * flash in multiples less than 4 bytes. */
#ifndef PLATFORM_FLASH_ALIGNMENT
#define PLATFORM_FLASH_ALIGNMENT 1
#endif

/*
 * Read a given number of bytes SAFELY from the flash chip
 * DO NOT use from an ISR
 */
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    int unaligned = ((uint32_t)buffer) & (PLATFORM_FLASH_ALIGNMENT - 1);
    if (unaligned) {
        int nfix = PLATFORM_FLASH_ALIGNMENT - unaligned;
        if (nfix > num_bytes)
            nfix = num_bytes;

        uint8_t extra[PLATFORM_FLASH_ALIGNMENT];
        flash_read_bytes(address, extra, PLATFORM_FLASH_ALIGNMENT);
        memcpy(buffer, extra, nfix);
        buffer += nfix;
        address += nfix;
        num_bytes -= nfix;
    }
    
    if (!num_bytes)
        return;

    xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    
    hw_flash_read_bytes(address, buffer, num_bytes & ~(PLATFORM_FLASH_ALIGNMENT - 1));
    
    /* sit the caller being this wait lock semaphore */
    if (!xSemaphoreTake(_flash_wait_semaphore, pdMS_TO_TICKS(200)))
        panic("Got stuck behind a wait lock in flash.c");

    xSemaphoreGive(_flash_mutex);
    
    if (num_bytes & (PLATFORM_FLASH_ALIGNMENT - 1)) {
        /* Clean up brain damage.  Sigh. */
        uint8_t extra[PLATFORM_FLASH_ALIGNMENT];
        uint32_t offset = num_bytes & ~(PLATFORM_FLASH_ALIGNMENT - 1);
        
        flash_read_bytes(address + offset, extra, PLATFORM_FLASH_ALIGNMENT);
        memcpy(buffer + offset, extra, num_bytes - offset);
    }
}

int flash_write_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    int rv = 0;
    
    int unaligned = ((uint32_t)buffer) & (PLATFORM_FLASH_ALIGNMENT - 1);
    if (unaligned) {
        int nfix = PLATFORM_FLASH_ALIGNMENT - unaligned;
        if (nfix > num_bytes)
            nfix = num_bytes;

        assert(PLATFORM_FLASH_ALIGNMENT <= 4);
        uint8_t extra[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        memcpy(extra, buffer, nfix);
        rv |= flash_write_bytes(address, extra, PLATFORM_FLASH_ALIGNMENT);
        buffer += nfix;
        address += nfix;
        num_bytes -= nfix;
    }
    
    if (!num_bytes)
        return rv;

    xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    if ((void *)buffer < (void *)0x20000000) /* XXX: this is correct on both STM32 and nRF52, but not guaranteed */ {
        /* DMA out of microflash doesn't work, so need to copy through SRAM */
#define FLASHRAMBUFSIZ 64
        char buf[FLASHRAMBUFSIZ];
        int nrem = num_bytes & ~(PLATFORM_FLASH_ALIGNMENT - 1);
        int ndone = 0;
        int curad = address;
        while (nrem) {
            int nthis = (nrem < FLASHRAMBUFSIZ) ? nrem : FLASHRAMBUFSIZ;
            
            memcpy(buf, buffer + ndone, nthis);
            rv |= hw_flash_write_sync(address + ndone, buf, nthis);
            
            ndone += nthis;
            nrem -= nthis;
        }
    } else {
        /* No need to copy through memory. */
        rv |= hw_flash_write_sync(address, buffer, num_bytes & ~(PLATFORM_FLASH_ALIGNMENT - 1));
    }
    
    /* writes currently are synchronous */
    
    xSemaphoreGive(_flash_mutex);
    
    if (num_bytes & (PLATFORM_FLASH_ALIGNMENT - 1)) {
        /* Clean up brain damage.  Sigh. */
        assert(PLATFORM_FLASH_ALIGNMENT <= 4);
        uint8_t extra[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        uint32_t offset = num_bytes & ~(PLATFORM_FLASH_ALIGNMENT - 1);
        
        memcpy(extra, buffer + offset, num_bytes - offset);
        rv |= flash_write_bytes(address + offset, extra, PLATFORM_FLASH_ALIGNMENT);
    }
    
    return rv;
}

int flash_erase(uint32_t address, uint32_t len)
{
    int rv;
    
    xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    rv = hw_flash_erase_sync(address, len);
    xSemaphoreGive(_flash_mutex);
    
    return rv;
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

inline void flash_operation_complete(uint8_t cmd)
{
    /* Notify the task that the transmission is complete. */
    xSemaphoreGive(_flash_wait_semaphore);
}

void flash_operation_complete_isr(uint8_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* wake the flash */
    xSemaphoreGiveFromISR(_flash_wait_semaphore, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
