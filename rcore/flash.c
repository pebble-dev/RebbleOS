/* flash.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"

static void _fs_init(void);

extern void hw_flash_init(void);
extern void hw_flash_read_bytes(uint32_t, uint8_t*, size_t);

// TODO
// DMA/async?
// what about apps/watchface resource loading?
// document

/// MUTEX
static SemaphoreHandle_t _flash_mutex;
static StaticSemaphore_t _flash_mutex_buf;
uint32_t _flash_get_app_slot_address(uint16_t slot_id);
extern unsigned int _ram_top;
#define portMPU_REGION_READ_WRITE (0x03UL << MPU_RASR_AP_Pos)

static struct hw_driver_ext_flash_t *_flash_driver;

// No ISR here (yet)
static hw_driver_handler_t _callack_handler = {
    .done_isr = NULL
};

void flash_init()
{
    // initialise device specific flash
    _flash_driver = (hw_driver_ext_flash_t *)driver_register((hw_driver_module_init_t)hw_flash_module_init, &_callack_handler);
    assert(_flash_driver->read_bytes && "Read is invalid");
    
//     MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
//     MPU->RNR  = 0;
//     MPU->RBAR = 0x20000000;
//     MPU->RASR = _ram_top | portMPU_REGION_READ_WRITE  | MPU_RASR_XN_Msk;
//     SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
//     MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;

    _flash_mutex = xSemaphoreCreateMutexStatic(&_flash_mutex_buf);
    
    _fs_init();
}

/*
 * Read a given number of bytes SAFELY from the flash chip
 */
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    xSemaphoreTake(_flash_mutex, portMAX_DELAY);

    _flash_driver->read_bytes(address, buffer, num_bytes);
    
    xSemaphoreGive(_flash_mutex);
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

/* XXX: should filesystem bits and bobs get split out somewhere else? 
 * Probably, but who's counting, anyway?  */

struct page_hdr {
    uint16_t v_0x5001;
    uint8_t  empty; /* 0xFF if empty, 0xFC if not empty, 0xFE if the first empty block before the rest of the device is empty? */
#define HDR_EMPTY_ALLOCATED 0x1
#define HDR_EMPTY_MOREBLOCKS 0x2
    uint8_t  status;   /* bitfield; f4, f8, ... */
#define HDR_STATUS_VALID 0x1
#define HDR_STATUS_DEAD 0x2
#define HDR_STATUS_FILE_START 0x4
#define HDR_STATUS_FILE_CONT 0x8
    uint32_t rsvd_0; /* ff ff ff ff */
    uint32_t wear_level_counter;
    uint32_t rsvd_1;
    uint32_t rsvd_2;
    uint8_t  rsvd_3;
    uint8_t  next_page_crc;
    uint16_t next_page;
    uint32_t pagehdr_crc; /* random numbers? */
    
    uint32_t file_size;
    uint8_t  flag_2; /* FF or FE; FE if there's a filename */
#define HDR_FLAG_2_HAS_FILENAME 0x1
    uint8_t  filename_len;
    uint16_t rsvd_4; /* FF FF */
    uint32_t rsvd_5;
    uint32_t filehdr_crc;
    uint16_t st_tmp_file; /* non-zero if temp file, zero if not temp file */
    uint16_t st_create_complete; /* zero if create complete, non-zero if not */
    uint16_t st_delete_complete; /* zero if delete complete, non-zero if not */
    uint8_t  v_full[26];
};

/* flag values are *cleared* to mean "true" */
#define FLASHFLAG(val, flag) (((val) & (flag)) == 0)

/* assuming that no longer files are possible, just a guess */
#define MAX_FILENAME_LEN 32

struct page_hdr_with_name {
    struct page_hdr hdr;
    char name[MAX_FILENAME_LEN + 1];
};

static void _fs_read_page_hdr(int pg, struct page_hdr_with_name *p) {
    flash_read_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE, (uint8_t *)p, sizeof(struct page_hdr_with_name));
    p->name[(MAX_FILENAME_LEN < p->hdr.filename_len) ? MAX_FILENAME_LEN : p->hdr.filename_len] = 0;
}

static void _fs_read_page_ofs(int pg, size_t ofs, void *p, size_t n) {
    flash_read_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE + ofs, (uint8_t *)p, n);
}

static uint8_t _fs_valid = 1;

enum page_state {
    PageStateUnallocated = 0,
    PageStateFileStart = 1,
    PageStateFileCont = 2,
    PageStateInvalid = 3
};

static uint8_t _fs_page_flags[(REGION_FS_N_PAGES + 3) >> 2];

static void _fs_set_page_state(uint16_t pg, enum page_state state)
{
    int offset = 6 - 2 * (pg & 3);
    _fs_page_flags[pg >> 2] &= ~(3 << offset); // clear flag bits
    _fs_page_flags[pg >> 2] |= (state << offset); // set flag bits
}

static enum page_state _fs_get_page_state(uint16_t pg)
{
    return (_fs_page_flags[pg >> 2] >> (6  - 2 * (pg & 3))) & 3;
}

static void _fs_init()
{
    /* Do a basic integrity check to see if there's any cleanup that needs
     * to be done that we don't know how to do yet.
     */
    struct page_hdr_with_name buffer;
    struct page_hdr *hdr = &buffer.hdr;
    int pg;
    
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "doing basic filesystem check");
    _fs_valid = 1;
    memset(&_fs_page_flags, 0, sizeof(_fs_page_flags));

    /* Make sure that at least the first page has the header of the right
     * version.  There might be pages with missing headers later, and we can
     * squawk about that, but the first page has to be good for there to be
     * a fileystem here.  */
    _fs_read_page_hdr(0, &buffer);
    if (hdr->v_0x5001 != 0x5001) {
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "this doesn't appear to be a Pebble filesystem");
        _fs_valid = 0;
        return;
    }
    
    /* Make sure that all pages have headers of the right version and are "in
     * the right order", and aren't half-dead.
     */
    int lastpg = -1;
    uint8_t saw_blank_page = 0;
    uint8_t saw_page_in_outer_space = 0;
    for (pg = 0; pg < REGION_FS_N_PAGES; pg++)
    {
        _fs_read_page_hdr(pg, &buffer);
        if (hdr->v_0x5001 == 0xFFFF) {
            if (!saw_blank_page)
                KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "this filesystem has a blank page in it... hmm...");
            saw_blank_page = 1;
            continue;
        }
        if (hdr->v_0x5001 != 0x5001) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has bad header version 0x%04x; I give up", pg, hdr->v_0x5001);
            _fs_valid = 0;
            return;
        }
        if (hdr->status == 0xFE && lastpg != -1) {
            lastpg = pg;
        }
        if ((lastpg != -1) && FLASHFLAG(hdr->empty, HDR_EMPTY_ALLOCATED)) {
            if (!saw_page_in_outer_space)
                KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d is marked as allocated, but page %d had the last page marker... hmm...", pg, lastpg);
            saw_page_in_outer_space = 1;
        }
        
        /* The rest of the checks only apply to an allocated page. */
        if (!FLASHFLAG(hdr->empty, HDR_EMPTY_ALLOCATED))
            continue;

        if (FLASHFLAG(hdr->status, HDR_STATUS_FILE_CONT))
            _fs_set_page_state(pg, PageStateFileCont);

        if (!FLASHFLAG(hdr->status, HDR_STATUS_FILE_START))
            continue;

        if (!FLASHFLAG(hdr->status, HDR_STATUS_DEAD) && hdr->st_create_complete) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d creation not complete; I can't deal with this; go boot PebbleOS to clean up first", pg);
            _fs_valid = 0;
            return;
        }
        if (FLASHFLAG(hdr->status, HDR_STATUS_DEAD) && hdr->st_delete_complete) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d deletion not complete; I can't deal with this; go boot PebbleOS to clean up first", pg);
            _fs_valid = 0;
            return;
        }

        if (hdr->filename_len > MAX_FILENAME_LEN)
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has unexpectedly long file name: %d; it may cause further errors", pg, hdr->filename_len);

        if (!strcmp(buffer.name, "GC")) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has a GC file; I can't deal with this; go boot PebbleOS to clean up first", pg);
            _fs_valid = 0;
            return;
        }

        _fs_set_page_state(pg, PageStateFileStart);
    }
    
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "checked %d pages, and it's good enough to read, at least", pg);
}


void flash_load_app_header(uint16_t app_id, ApplicationHeader *header)
{
    flash_read_bytes(_flash_get_app_slot_address(app_id), (uint8_t *)header, sizeof(ApplicationHeader));
}

void flash_load_app(uint16_t app_id, uint8_t *buffer, size_t count)
{
    flash_read_bytes(_flash_get_app_slot_address(app_id), buffer, count);
}

uint32_t _flash_get_app_slot_address(uint16_t slot_id)
{
    // I still don't really get the flash layout. sometimes apps appear in different pages
    if (slot_id < 8)
        return APP_SLOT_0_START + (slot_id * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 16)
        return APP_SLOT_8_START + (slot_id - 8 * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 24)
        return APP_SLOT_16_START + ((slot_id - 16) * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;
    else if (slot_id < 32)
        return APP_SLOT_24_START + ((slot_id - 24) * APP_SLOT_SIZE) + APP_HEADER_BIN_OFFSET;

    return 0;
}

uint32_t flash_get_resource_address(uint16_t slot_id)
{
    // get the address for this slot, then work backwards in 0x2000 block
    // increments looking for the resources
    uint32_t faddr = _flash_get_app_slot_address(slot_id);
    uint32_t block_size = 0x2000;

    // get the header identifier. it sits before the app
    AppTypeHeader app_header, fr;
    flash_read_bytes(faddr - sizeof(AppTypeHeader), (uint8_t *)&app_header, sizeof(AppTypeHeader));
    KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "%s", app_header.address);

    for (uint32_t i = faddr - block_size; i > (faddr - (block_size * 10)); i -= block_size)
    {
        flash_read_bytes(i - 13, (uint8_t *)&fr, sizeof(AppTypeHeader));
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "Resource %s %x", fr.address, i);
        // check to see if this block contains our header
        if (!strncmp(app_header.address, fr.address, 8))
        {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "Found Resource %s %x", fr.address, i);
            return i;
        }
    }

    return 0;
}

bool fs_find_file(File *file, const char *name)
{
    if (!_fs_valid)
    {
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "fs_find_file used with invalid fs; ignoring request");
        return false;
    }

    struct page_hdr_with_name buffer;
    struct page_hdr *hdr = &buffer.hdr;

    for (uint16_t pg = 0; pg < REGION_FS_N_PAGES; pg++)
    {
        if (_fs_get_page_state(pg) == PageStateFileStart)
        {
            _fs_read_page_hdr(pg, &buffer);
            if (!strcmp(name, buffer.name)) {
                file->page = pg;
                file->next_page = hdr->next_page;
                file->size = hdr->file_size;
                file->offset = sizeof(struct page_hdr) + hdr->filename_len;
                return true;
            }
        }
    }

    return false;
}
