/* fs.c
 * PebbleFS read-only routines
 * RebbleOS
 */
#include <stdint.h>
#include "minilib.h"
#include "platform.h"
#include "log.h"
#include "fs.h"
#include "fs_internal.h"
#include "flash.h"
#include "blob_db_ramfs.h"

/* XXX: should filesystem bits and bobs get split out somewhere else? 
 * Probably, but who's counting, anyway?  */

/* flag values are *cleared* to mean "true" */
#define FLASHFLAG(val, flag) (((val) & (flag)) == 0)

static void _fs_read_page_ofs(int pg, size_t ofs, void *p, size_t n) {
    flash_read_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE + ofs, (uint8_t *)p, n);
}

static int _fs_write_page_ofs(int pg, size_t ofs, void *p, size_t n) {
    return flash_write_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE + ofs, (uint8_t *)p, n);
}

static void _fs_read_file_hdr(int pg, struct fs_file_hdr_with_name *p) {
    _fs_read_page_ofs(pg, 0, (void *)p, sizeof(struct fs_file_hdr_with_name));

    p->name[(MAX_FILENAME_LEN < p->hdr.filename_len) ? MAX_FILENAME_LEN : p->hdr.filename_len] = 0;
}


static uint8_t _fs_valid = 1;
static int _fs_lastpg = -1;

enum page_state {
    PageStateClean = 0,     /* Block is erased, and we can write to it. */
    PageStateFileStart = 1,
    PageStateFileCont = 2,
    PageStateDirty = 3      /* Block is not erased. */
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

static int _delete_file_by_pg(int pg)
{
    struct fs_page_hdr pagehdr;
    struct fs_file_hdr filehdr;
    
    /* The page must either be alive or incomplete-deleted. */
    _fs_read_page_ofs(pg, 0, &filehdr, sizeof(filehdr));
    assert(!FLASHFLAG(filehdr.status, HDR_STATUS_DEAD) || filehdr.st_delete_complete);
    
    /* Mark all pages as deallocated. */
    int curpg = pg;
    do {
        _fs_read_page_ofs(curpg, 0, &pagehdr, sizeof(pagehdr));
        pagehdr.status &= ~HDR_STATUS_DEAD;
        if (_fs_write_page_ofs(curpg, 0, &pagehdr, sizeof(pagehdr)))
            return -1;
        _fs_set_page_state(curpg, PageStateDirty);
        curpg = pagehdr.next_page;
    } while (curpg != 0xFFFF);
    
    /* Now mark it as deleted. */
    _fs_read_page_ofs(pg, 0, &filehdr, sizeof(filehdr));
    filehdr.st_delete_complete = 0x0000;
    if (_fs_write_page_ofs(pg, 0, &filehdr, sizeof(filehdr)))
        return -1;
    
    return 0;
}


/* GC routines. */

static int _gc_sector;

struct fs_gc_file_hdr {
    uint8_t version; /* XXX: what should this be? */
    uint8_t flags;
    uint16_t start_pg;
    uint32_t page_mask;
    uint8_t num_entries;
};

#define PAGES_PER_SECTOR (REGION_FS_ERASE_SIZE / REGION_FS_PAGE_SIZE)
static void _gc_find_sector()
{
    /* What we want to find is an erase-block-contiguous region of empty
     * pages with the lowest erase count.  */
    uint32_t lowest_erase_count = 0xFFFFFFFF;
    int best_sector = -1;
    
    int pg;
    
    for (pg = 0; pg <= REGION_FS_N_PAGES; pg += PAGES_PER_SECTOR) {
        int is_possible = 1;
        int is_dirty = 0;
        
        /* Is this sector empty? */
        for (int i = 0; i < PAGES_PER_SECTOR; i++) {
            enum page_state st = _fs_get_page_state(pg + i);
            if (st != PageStateClean && st != PageStateDirty) {
                is_possible = 0;
                break;
            }
            
            if (st == PageStateDirty)
                is_dirty = 1;
        }
        
        if (!is_possible)
            continue;
        
        /* Ok, read the erase count from it. */
        uint32_t cur_erase_count;
        
        struct fs_page_hdr pagehdr;
        _fs_read_page_ofs(pg, 0, &pagehdr, sizeof(pagehdr));
        cur_erase_count = pagehdr.wear_level_counter;
        
        if (is_dirty) /* We're gonna have to erase it. */
            cur_erase_count++;
        
        /* New best? */
        if (cur_erase_count < lowest_erase_count) {
            lowest_erase_count = cur_erase_count;
            best_sector = pg;
        }
    }
    
    _gc_sector = best_sector;
    assert(_gc_sector != -1);
    KERN_LOG("flash", APP_LOG_LEVEL_INFO, "chose GC sector of %d", _gc_sector);
    
    /* Mark it all as in use so the allocator doesn't come for it. */
    for (int i = 0; i < PAGES_PER_SECTOR; i++) {
        _fs_set_page_state(_gc_sector + i, PageStateDirty);
    }
}

void fs_init()
{
    /* Do a basic integrity check to see if there's any cleanup that needs
     * to be done that we don't know how to do yet.
     */
    struct fs_file_hdr_with_name buffer;
    struct fs_file_hdr *hdr = &buffer.hdr;
    int pg;
    
    KERN_LOG("flash", APP_LOG_LEVEL_INFO, "doing basic filesystem check");
    _fs_valid = 1;
    _fs_lastpg = -1;
    memset(&_fs_page_flags, 0, sizeof(_fs_page_flags));

    /* Make sure that at least the first page has the header of the right
     * version.  There might be pages with missing headers later, and we can
     * squawk about that, but the first page has to be good for there to be
     * a fileystem here.  */
    _fs_read_file_hdr(0, &buffer);
    if (hdr->v_0x5001 != 0x5001) {
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "this doesn't appear to be a Pebble filesystem %x", hdr->v_0x5001);
        _fs_valid = 0;
        return;
    }

    /* Start off by finding the last written page. */
    uint8_t saw_blank_page = 0;
    uint8_t saw_page_in_outer_space = 0;
    for (pg = 0; pg < REGION_FS_N_PAGES; pg++) {
        _fs_set_page_state(pg, PageStateClean);
        
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
        if (FLASHFLAG(hdr->empty, HDR_EMPTY_ALLOCATED) && !FLASHFLAG(hdr->empty, HDR_EMPTY_MOREBLOCKS) && _fs_lastpg == -1) {
            _fs_lastpg = pg;
        } else if ((_fs_lastpg != -1) && FLASHFLAG(hdr->empty, HDR_EMPTY_ALLOCATED)) {
            if (!saw_page_in_outer_space)
                KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d is marked as allocated, but page %d had the last page marker... hmm...", pg, _fs_lastpg);
            saw_page_in_outer_space = 1;
        }
    }
    
    /* Do file-level cleanup. */
    for (pg = 0; pg <= _fs_lastpg; pg++) {
        _fs_read_file_hdr(pg, &buffer);

        if (!FLASHFLAG(hdr->empty, HDR_EMPTY_ALLOCATED)) {
            _fs_set_page_state(pg, PageStateClean);
            continue;
        }

        if (FLASHFLAG(hdr->status, HDR_STATUS_FILE_CONT)) {
            _fs_set_page_state(pg, PageStateFileCont);
            continue;
        }

        if (!FLASHFLAG(hdr->status, HDR_STATUS_FILE_START)) {
            _fs_set_page_state(pg, PageStateClean);
            continue;
        }

        if (!FLASHFLAG(hdr->status, HDR_STATUS_DEAD) && hdr->st_create_complete) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d incompletely created; cleaning up", pg);
            assert(_delete_file_by_pg(pg) == 0);
        }
        if (FLASHFLAG(hdr->status, HDR_STATUS_DEAD) && hdr->st_delete_complete) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d incompletely deleted; cleaning up", pg);
            assert(_delete_file_by_pg(pg) == 0);
        }

        if (hdr->filename_len > MAX_FILENAME_LEN)
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has unexpectedly long file name: %d; it may cause further errors", pg, hdr->filename_len);

        if (!strcmp(buffer.name, "GC")) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has a GC file; I can't deal with this; go boot PebbleOS to clean up first", pg);
            _fs_valid = 0;
            return;
        }

        if (FLASHFLAG(hdr->status, HDR_STATUS_DEAD)) {
            _fs_set_page_state(pg, PageStateDirty);
            continue;
        }
        
        _fs_set_page_state(pg, PageStateFileStart);
    }
    
    KERN_LOG("flash", APP_LOG_LEVEL_INFO, "checked %d pages, and it's good enough to read, at least", _fs_lastpg);
    
    _gc_find_sector();
    
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

int fs_format()
{
    int rv;
    
    _fs_valid = 0;
    _fs_lastpg = 0;
    
    rv = flash_erase(REGION_FS_START, REGION_FS_N_PAGES * REGION_FS_PAGE_SIZE);
    if (rv)
        return rv;
    
    struct fs_page_hdr hdr;
    memset(&hdr, 0xFF, sizeof(hdr));
    hdr.v_0x5001 = 0x5001;
    
    /* XXX: PebbleOS in theory formats this to 0xFE, but in reality, flash
     * dumps that we see all have 0xFF here?  */
    hdr.empty = 0xFF;
    
    hdr.wear_level_counter = 0x0001;
    
    for (int pg = 0; pg < REGION_FS_N_PAGES; pg++) {
        rv = flash_write_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE, (uint8_t *)&hdr, sizeof(hdr));
        if (rv)
            return rv;
    }
    
    _fs_valid = 1;
    
    return 0;
}

int fs_find_file(struct file *file, const char *name)
{
    /* no need to say it -- they already heard it at init time ... */
    if (!_fs_valid)
        return -1;

    struct fs_file_hdr_with_name buffer;
    struct fs_file_hdr *hdr = &buffer.hdr;

    for (uint16_t pg = 0; pg <= _fs_lastpg; pg++)
    {
        if (_fs_get_page_state(pg) == PageStateFileStart)
        {
            _fs_read_file_hdr(pg, &buffer);
            if (!strcmp(name, buffer.name)) {
                file->startpage = pg;
                file->size = hdr->file_size;
                file->startpofs = sizeof(struct fs_file_hdr) + hdr->filename_len;
                file->is_ramfs = 0;
                return 0;
            }
        }
    }

    return -1;
}

void fs_open(struct fd *fd, const struct file *file)
{
    fd->file = *file;
    
    fd->curpage = fd->file.startpage;
    fd->curpofs = fd->file.startpofs;
    
    fd->offset  = 0;
}

int fs_read(struct fd *fd, void *p, size_t bytes)
{
    if (fd->file.is_ramfs)
        return ramfs_read(fd, p, bytes);

    size_t bytesrem;
    
    if (bytes > (fd->file.size - fd->offset))
        bytes = fd->file.size - fd->offset;
    bytesrem = bytes;

    while (bytesrem)
    {
        size_t n = bytesrem;
        
        if (n > (REGION_FS_PAGE_SIZE - fd->curpofs))
            n = REGION_FS_PAGE_SIZE - fd->curpofs;
        
        _fs_read_page_ofs(fd->curpage, fd->curpofs, p, n);
        
        fd->curpofs += n;
        fd->offset += n;
        bytesrem -= n;
        p += n;
        
        if (fd->curpofs == REGION_FS_PAGE_SIZE)
        {
            struct fs_page_hdr hdr;
            
            _fs_read_page_ofs(fd->curpage, 0, &hdr, sizeof(hdr));
            fd->curpage = hdr.next_page; /* XXX check this */
            fd->curpofs = sizeof(hdr);
        }
    }
    
    return bytes;
}

long fs_seek(struct fd *fd, long ofs, enum seek whence)
{
    if (fd->file.is_ramfs)
        return ramfs_seek(fd, ofs, whence);

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
            struct fs_page_hdr hdr;
            
            _fs_read_page_ofs(fd->curpage, 0, &hdr, sizeof(hdr));
            fd->curpage = hdr.next_page; /* XXX check this */
            fd->curpofs = sizeof(hdr);
        }
    }
    
    return fd->offset;
}
