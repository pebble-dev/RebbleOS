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

#define KEYLEN 4

static int _insert(int key, int dsize) {
    struct rdb_database *db = rdb_open(RDB_ID_TEST);

    KERN_LOG("flash", APP_LOG_LEVEL_INFO, ">>> INSERT %d, %d", key, dsize);
    
    uint32_t *keyp = malloc(16);
    keyp[0] = key;
    keyp[1] = key;
    keyp[2] = key;
    keyp[3] = key;
    
    uint8_t *val = app_calloc(1, dsize);
    for (int i = 0; i < dsize; i++)
        val[i] = (key & 0xFF) ^ i;
    
    int rv = rdb_insert(db, keyp, KEYLEN, val, dsize);
    
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

    KERN_LOG("flash", APP_LOG_LEVEL_INFO, ">>> RETRIEVE %d, %d", key, dsize);
    
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
        { RDB_SELECTOR_OFFSET_KEY, KEYLEN, RDB_OP_EQ, keyp },
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
        { RDB_SELECTOR_OFFSET_KEY, KEYLEN, RDB_OP_EQ, keyp },
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

struct write {
    uint32_t addr;
    uint32_t len;
    char *data;
};

/* for i in write*; do echo '{' $(echo $i | sed -e 's/.*-//'), $(wc -c $i | cut -d' ' -f1), \"$(xxd -c 256 -p $i | sed -e 's/\(..\)/\\x\1/g')\" '}',; done */
static struct write writes[] = {
/*{ 2170880, 28, "\x01\x50\xfe\xff\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" },
{ 2162688, 76, "\x01\x50\xfc\xfa\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x15\x9f\xd3\x8c\x00\x04\x00\x00\xfe\x0e\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" },
{ 2162764, 14, "\x72\x65\x62\x62\x6c\x65\x2f\x72\x64\x62\x74\x65\x73\x74" },
{ 2162776, 4, "\x73\x74\xff\xff" },
{ 2162688, 76, "\x01\x50\xfc\xfa\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x15\x9f\xd3\x8c\x00\x04\x00\x00\xfe\x0e\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" },
{ 2162688, 76, "\x01\x50\xfc\xfa\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x15\x9f\xd3\x8c\x00\x04\x00\x00\xfe\x0e\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" },
{ 2162778, 4, "\xff\x04\xbe\x00" },
{ 2162778, 4, "\xfd\x04\xbe\x00" },
{ 2162782, 4, "\x02\x00\x00\x00" },*/
{ 2162786, 190, "\x02\x03\x00\x01\x06\x07\x04\x05\x0a\x0b\x08\x09\x0e\x0f\x0c\x0d\x12\x13\x10\x11\x16\x17\x14\x15\x1a\x1b\x18\x19\x1e\x1f\x1c\x1d\x22\x23\x20\x21\x26\x27\x24\x25\x2a\x2b\x28\x29\x2e\x2f\x2c\x2d\x32\x33\x30\x31\x36\x37\x34\x35\x3a\x3b\x38\x39\x3e\x3f\x3c\x3d\x42\x43\x40\x41\x46\x47\x44\x45\x4a\x4b\x48\x49\x4e\x4f\x4c\x4d\x52\x53\x50\x51\x56\x57\x54\x55\x5a\x5b\x58\x59\x5e\x5f\x5c\x5d\x62\x63\x60\x61\x66\x67\x64\x65\x6a\x6b\x68\x69\x6e\x6f\x6c\x6d\x72\x73\x70\x71\x76\x77\x74\x75\x7a\x7b\x78\x79\x7e\x7f\x7c\x7d\x82\x83\x80\x81\x86\x87\x84\x85\x8a\x8b\x88\x89\x8e\x8f\x8c\x8d\x92\x93\x90\x91\x96\x97\x94\x95\x9a\x9b\x98\x99\x9e\x9f\x9c\x9d\xa2\xa3\xa0\xa1\xa6\xa7\xa4\xa5\xaa\xab\xa8\xa9\xae\xaf\xac\xad\xb2\xb3\xb0\xb1\xb6\xb7\xb4\xb5\xba\xbb\xb8\xb9\xbe\xbf" },
/*{ 2162778, 4, "\xfc\x04\xbe\x00" },*/
{ 0 }
};

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
#if 0
    for (int i = 0; i < 32; i++) {
        KERN_LOG("flash", APP_LOG_LEVEL_INFO, "*** ROUND %d ***", i);
        assert(_insert(2, 190) == 0);
        assert(_retrieve(2, 190, 0) == 0);
        assert(_delete(2) == 0);
    }
#endif
    for (int i = 0; writes[i].addr; i++) {
        void *ibuf = malloc(writes[i].len);
        void *obuf = malloc(writes[i].len);
        
        memcpy(ibuf, writes[i].data, writes[i].len);
        KERN_LOG("flash", APP_LOG_LEVEL_INFO, "WR %d -> %08x", writes[i].len, writes[i].addr);
        flash_write_bytes(writes[i].addr, ibuf, writes[i].len);
        
        flash_read_bytes(0x210000, obuf, 2);
        KERN_LOG("flash", APP_LOG_LEVEL_INFO, "version number is now %04x", *(uint16_t *)obuf);
        //flash_read_bytes(writes[i].addr, obuf, writes[i].len);
        //KERN_LOG("flash", APP_LOG_LEVEL_INFO, "rv %d", memcmp(ibuf, obuf, writes[i].len));
        
        free(ibuf);
        free(obuf);
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
    
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "write bytes 0x%08x: %d bytes", address, num_bytes);

    int unaligned = ((uint32_t)buffer) & (PLATFORM_FLASH_ALIGNMENT - 1);
    if (unaligned) {
        int nfix = PLATFORM_FLASH_ALIGNMENT - unaligned;
        if (nfix > num_bytes)
            nfix = num_bytes;

        assert(PLATFORM_FLASH_ALIGNMENT <= 4);
        uint8_t extra[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        memcpy(extra, buffer, nfix);
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "fixing up unaligned write");
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
