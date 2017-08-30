/* fs.c
 * PebbleFS read-only routines
 * RebbleOS
 */
#include <stdint.h>
#include "minilib.h"
#include "platform.h"
#include "log.h"
#include "fs.h"
#include "flash.h"


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
};

struct file_hdr {
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

struct file_hdr_with_name {
    struct file_hdr hdr;
    char name[MAX_FILENAME_LEN + 1];
};

static void _fs_read_file_hdr(int pg, struct file_hdr_with_name *p) {
    flash_read_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE, (uint8_t *)p, sizeof(struct file_hdr_with_name));
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

void fs_init()
{
    /* Do a basic integrity check to see if there's any cleanup that needs
     * to be done that we don't know how to do yet.
     */
    struct file_hdr_with_name buffer;
    struct file_hdr *hdr = &buffer.hdr;
    int pg;
    
    KERN_LOG("flash", APP_LOG_LEVEL_INFO, "doing basic filesystem check");
    _fs_valid = 1;
    memset(&_fs_page_flags, 0, sizeof(_fs_page_flags));

    /* Make sure that at least the first page has the header of the right
     * version.  There might be pages with missing headers later, and we can
     * squawk about that, but the first page has to be good for there to be
     * a fileystem here.  */
    _fs_read_file_hdr(0, &buffer);
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
        _fs_read_file_hdr(pg, &buffer);
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
    
    KERN_LOG("flash", APP_LOG_LEVEL_INFO, "checked %d pages, and it's good enough to read, at least", pg);
    
    /* test it out some ... */
    struct file file;
    struct fd fd;
    if (fs_find_file(&file, "appdb") < 0)
    {
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "no appdb???");
        return;
    }
    
    fs_open(&fd, &file);
    char b[4];
    fs_read(&fd, b, 3);
    b[3] = 0;
    KERN_LOG("flash", APP_LOG_LEVEL_DEBUG, "first 3 bytes of appdb are %s", b);
    
}

int fs_find_file(struct file *file, const char *name)
{
    /* no need to say it -- they already heard it at init time ... */
    if (!_fs_valid)
        return -1;

    struct file_hdr_with_name buffer;
    struct file_hdr *hdr = &buffer.hdr;

    for (uint16_t pg = 0; pg < REGION_FS_N_PAGES; pg++)
    {
        if (_fs_get_page_state(pg) == PageStateFileStart)
        {
            _fs_read_file_hdr(pg, &buffer);
            if (!strcmp(name, buffer.name)) {
                file->startpage = pg;
                file->size = hdr->file_size;
                file->startpofs = sizeof(struct file_hdr) + hdr->filename_len;
                return 0;
            }
        }
    }

    return -1;
}

void fs_open(struct fd *fd, struct file *file)
{
    fd->file = *file;
    
    fd->curpage = fd->file.startpage;
    fd->curpofs = fd->file.startpofs;
    
    fd->offset  = 0;
}

int fs_read(struct fd *fd, void *p, size_t bytes)
{
    size_t bytesrem;
    
    if (bytes > (fd->file.size - fd->offset))
        bytes = fd->file.size - fd->offset;
    bytesrem = bytes;

    while (bytes)
    {
        size_t n = bytesrem;
        
        if (n > (REGION_FS_PAGE_SIZE - fd->curpofs))
            n = REGION_FS_PAGE_SIZE - fd->curpofs;
        
        _fs_read_page_ofs(fd->curpage, fd->curpofs, p, n);
        
        fd->curpofs += n;
        fd->offset += n;
        bytes -= n;
        p += n;
        
        if (fd->curpofs == REGION_FS_PAGE_SIZE)
        {
            struct page_hdr hdr;
            
            _fs_read_page_ofs(fd->curpage, 0, &hdr, sizeof(hdr));
            fd->curpage = hdr.next_page; /* XXX check this */
            fd->curpofs = sizeof(hdr);
        }
    }
    
    return bytes;
}

long fs_seek(struct fd *fd, long ofs, enum seek whence)
{
    size_t newoffset;
    switch (whence)
    {
    case FS_SEEK_SET: newoffset = ofs; break;
    case FS_SEEK_CUR: newoffset = fd->offset + ofs /* XXX: overflow */; break;
    case FS_SEEK_END: newoffset = fd->file.size + ofs; break;
    }
    
    if (newoffset > fd->file.size)
        newoffset = fd->file.size;
    
    if (newoffset < fd->offset)
    {
        fd->curpage = fd->file.startpage;
        fd->curpofs = fd->file.startpofs;
        fd->offset = 0;
    }
    
    size_t delta = newoffset - fd->offset;
    while (delta)
    {
        size_t n = delta;
        
        if (n > (REGION_FS_PAGE_SIZE - fd->curpofs))
            n = REGION_FS_PAGE_SIZE - fd->curpofs;
        
        fd->curpofs += n;
        fd->offset += n;
        delta -= n;
        
        if (fd->curpofs == REGION_FS_PAGE_SIZE)
        {
            struct page_hdr hdr;
            
            _fs_read_page_ofs(fd->curpage, 0, &hdr, sizeof(hdr));
            fd->curpage = hdr.next_page; /* XXX check this */
            fd->curpofs = sizeof(hdr);
        }
    }
    
    return fd->offset;
}
