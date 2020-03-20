/* flash.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "FreeRTOS.h"
#include "display.h"
#include "platform.h"
#include "semphr.h"
#include "flash.h"
#include <string.h>
#include "fs.h"

extern void hw_flash_init(void);
extern void hw_flash_read_bytes(uint32_t, uint8_t*, size_t);
extern int hw_flash_erase_sync(uint32_t addr, uint32_t len);
extern int hw_flash_write_sync(uint32_t addr, uint8_t *buf, size_t len);

static SemaphoreHandle_t _flash_mutex;
static StaticSemaphore_t _flash_mutex_buf;
static SemaphoreHandle_t _flash_wait_semaphore;
static StaticSemaphore_t _flash_wait_semaphore_buf;

uint8_t flash_init()
{
    // initialise device specific flash
    hw_flash_init();
    
    _flash_mutex = xSemaphoreCreateMutexStatic(&_flash_mutex_buf);
    _flash_wait_semaphore = xSemaphoreCreateBinaryStatic(&_flash_wait_semaphore_buf);
    
    fs_init();
    
    return 0;
}

/* Some platforms have brain damage, like nRF52840, and can't read from
 * flash in multiples less than 4 bytes. */
#ifndef PLATFORM_FLASH_DMA_ALIGNMENT
#define PLATFORM_FLASH_DMA_ALIGNMENT 1
#endif

/* Some platforms require that flash writes do not span multiple "pages",
 * and therefore we require that we break up writes such that (addr + len) &
 * PAGE_MASK == addr & PAGE_MASK.
 */
#ifndef PLATFORM_FLASH_PAGE_MASK
#define PLATFORM_FLASH_PAGE_MASK 0
#endif

#ifndef PLATFORM_FLASH_PAGE_SIZE
#define PLATFORM_FLASH_PAGE_SIZE 0xFFFFFFFF
#endif

// #define FLASH_NOISY_WRITE_ALIGNMENT

/*
 * Read a given number of bytes SAFELY from the flash chip
 * DO NOT use from an ISR
 */
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    int unaligned = ((uint32_t)buffer) & (PLATFORM_FLASH_DMA_ALIGNMENT - 1);
    if (unaligned) {
        int nfix = PLATFORM_FLASH_DMA_ALIGNMENT - unaligned;
        if (nfix > num_bytes)
            nfix = num_bytes;

        uint8_t extra[PLATFORM_FLASH_DMA_ALIGNMENT];
        flash_read_bytes(address, extra, PLATFORM_FLASH_DMA_ALIGNMENT);
        memcpy(buffer, extra, nfix);
        buffer += nfix;
        address += nfix;
        num_bytes -= nfix;
    }
    
    if (!num_bytes)
        return;

    xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    
    hw_flash_read_bytes(address, buffer, num_bytes & ~(PLATFORM_FLASH_DMA_ALIGNMENT - 1));
    
    /* sit the caller being this wait lock semaphore */
    if (!xSemaphoreTake(_flash_wait_semaphore, pdMS_TO_TICKS(200)))
        panic("Got stuck behind a wait lock in flash.c");

    xSemaphoreGive(_flash_mutex);
    
    if (num_bytes & (PLATFORM_FLASH_DMA_ALIGNMENT - 1)) {
        /* Clean up brain damage.  Sigh. */
        uint8_t extra[PLATFORM_FLASH_DMA_ALIGNMENT];
        uint32_t offset = num_bytes & ~(PLATFORM_FLASH_DMA_ALIGNMENT - 1);
        
        flash_read_bytes(address + offset, extra, PLATFORM_FLASH_DMA_ALIGNMENT);
        memcpy(buffer + offset, extra, num_bytes - offset);
    }
}

static int _flash_write_aligned_paged(uint32_t addr, uint8_t *buf, size_t len) {
#ifdef FLASH_NOISY_WRITE_ALIGNMENT
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "    paged aligned write bytes 0x%08x: %d bytes", addr, len);
#endif

    assert(((addr + len - 1) & PLATFORM_FLASH_PAGE_MASK) == (addr & PLATFORM_FLASH_PAGE_MASK));
    assert((len & (PLATFORM_FLASH_DMA_ALIGNMENT - 1)) == 0);
    assert((((uint32_t)buf) & (PLATFORM_FLASH_DMA_ALIGNMENT - 1)) == 0);
    
    int rv = 0;

    xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    
    if ((void *)buf < (void *)0x20000000) /* XXX: this is correct on both STM32 and nRF52, but not guaranteed */ {
        /* On nRF52, DMA out of microflash doesn't work, so need to copy
         * through SRAM. */
#define FLASHRAMBUFSIZ 64
        static uint8_t rambuf[FLASHRAMBUFSIZ];
        int nrem = len;
        int ndone = 0;
        while (nrem) {
            int nthis = (nrem < FLASHRAMBUFSIZ) ? nrem : FLASHRAMBUFSIZ;
            
            memcpy(rambuf, buf + ndone, nthis);
            rv |= hw_flash_write_sync(addr + ndone, rambuf, nthis);
            
            ndone += nthis;
            nrem -= nthis;
        }
    } else {
        /* No need to copy through memory. */
        rv |= hw_flash_write_sync(addr, buf, len);
    }

    xSemaphoreGive(_flash_mutex);
        
    return rv;
}

static int _flash_write_paged(uint32_t addr, uint8_t *buf, size_t len) {
#ifdef FLASH_NOISY_WRITE_ALIGNMENT
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "  paged write bytes 0x%08x: %d bytes", addr, len);
#endif
    assert(((addr + len - 1) & PLATFORM_FLASH_PAGE_MASK) == (addr & PLATFORM_FLASH_PAGE_MASK));
    
    int rv = 0;
    
    /* Leading edge DMA alignment... */
    int unaligned = ((uint32_t)buf) & (PLATFORM_FLASH_DMA_ALIGNMENT - 1);
    if (unaligned) {
        int nfix = PLATFORM_FLASH_DMA_ALIGNMENT - unaligned;
        if (nfix > len)
            nfix = len;
        
        /* We know the input data lives entirely in one page, but for small
         * writes, we can get ourselves into trouble here.  For instance, if
         * we get called to do a 2-byte write, *and* we have to realign,
         * *and* the bytes were the last two bytes of the page, then we
         * could end up doing a 4-byte write, which breaks the invariant
         * above.  So even though it "should be fine", since we're writing
         * ones, we instead right-justify in such a case, like we do below.
         */
        int justify = 0;
        if ((addr & ~PLATFORM_FLASH_PAGE_MASK) > (PLATFORM_FLASH_PAGE_SIZE / 2)) {
            justify = PLATFORM_FLASH_DMA_ALIGNMENT - nfix;
        }

        assert(PLATFORM_FLASH_DMA_ALIGNMENT <= 4);
        uint8_t extra[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        memcpy(extra + justify, buf, nfix);

        rv |= _flash_write_aligned_paged(addr - justify, extra, PLATFORM_FLASH_DMA_ALIGNMENT);
        buf += nfix;
        addr += nfix;
        len -= nfix;
    }
    
    if (!len)
        return rv;
    
    rv |= _flash_write_aligned_paged(addr, buf, len & ~(PLATFORM_FLASH_DMA_ALIGNMENT - 1));

    /* ... and trailing edge DMA alignment. */
    if (len & (PLATFORM_FLASH_DMA_ALIGNMENT - 1)) {
        assert(PLATFORM_FLASH_DMA_ALIGNMENT <= 4);
        uint8_t extra[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        uint32_t offset = len & ~(PLATFORM_FLASH_DMA_ALIGNMENT - 1);
        
        /* We want to line up the offset that we write to flash such that
         * the last written byte is the *last* byte (i.e., right-align it). 
         * This way, we avoid opening up a new page.  We do this by defining
         * "realign" such that it is how much padding we need at left. 
         * Consider writing the following 15-byte sequence on a 4-byte-align
         * target:
         *
         * ofs   012345678901234
         * ofs%4 012301230123012
         * data  ABCDEFGHIJKLMNO
         *
         * "Offset" in this case needs to be 12, since that's the first byte
         * that we'll write.  The remaining bytes are (len - offset), which
         * is 3.  So we need to add PLATFORM_FLASH_DMA_ALIGNMENT - remaining
         * bytes of padding -- that's 1 -- to the left; we call that
         * "realign".
         */
        int realign = PLATFORM_FLASH_DMA_ALIGNMENT - (len - offset);
        
        memcpy(extra + realign, buf + offset, len - offset);
        rv |= _flash_write_aligned_paged(addr + offset - realign, extra, PLATFORM_FLASH_DMA_ALIGNMENT);
    }
    
    return rv;
}

int flash_write_bytes(uint32_t addr, uint8_t *buf, size_t len)
{
#ifdef FLASH_NOISY_WRITE_ALIGNMENT
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "write bytes 0x%08x: %d bytes", addr, len);
#endif
    
    int rv;
    
    while (len) {
        int nthis = len;
        int npagemax = PLATFORM_FLASH_PAGE_SIZE - (addr & ~PLATFORM_FLASH_PAGE_MASK);
        if (npagemax < nthis)
            nthis = npagemax;
        
                rv = _flash_write_paged(addr, buf, nthis);
        if (rv)
            return rv;

        addr += nthis;
        buf += nthis;
        len -= nthis;
    }
    
    return 0;
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
