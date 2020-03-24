/* fs.c
 * PebbleFS read-only routines
 * RebbleOS
 */
#include <stdint.h>
#include "debug.h"
#include "minilib.h"
#include "platform.h"
#include "log.h"
#include "fs.h"
#include "fs_internal.h"
#include "flash.h"

/* XXX: should filesystem bits and bobs get split out somewhere else? 
 * Probably, but who's counting, anyway?  */

/* flag values are *cleared* to mean "true" */
#define FLASHFLAG(val, flag) (((val) & (flag)) == 0)

static void _fs_read_page_ofs(int pg, size_t ofs, void *p, size_t n) {
    flash_read_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE + ofs, (uint8_t *)p, n);
}

static int _fs_write_page_ofs(int pg, size_t ofs, const void *p, size_t n) {
    return flash_write_bytes(REGION_FS_START + pg * REGION_FS_PAGE_SIZE + ofs, (uint8_t *)p, n);
}

static void _fs_read_file_hdr(int pg, struct fs_file_hdr_with_name *p) {
    _fs_read_page_ofs(pg, 0, (void *)p, sizeof(struct fs_file_hdr_with_name));

    p->name[(MAX_FILENAME_LEN < p->hdr.filename_len) ? MAX_FILENAME_LEN : p->hdr.filename_len] = 0;
}


static uint8_t _fs_valid = 1;

/*** Page table routines. ***/

static int _fs_lastpg = -1;

/* We store the lowest erase count, and the first available page with that
 * erase count, for use with the block allocator.  */
static uint32_t _fs_best_erase_count = 0xFFFFFFFF;
static int _fs_last_allocated_page = -1;
static int _fs_free_pages = 0; /* dirty + clean */

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

static int _fs_page_table_init()
{
    struct fs_page_hdr pagehdr;
    uint8_t saw_blank_page = 0;
    uint8_t saw_page_in_outer_space = 0;
    
    int nclean = 0;
    int ndirty = 0;
    int nused = 0;
    
    /* Find the last written page, and while we're at it, populate the page
     * state table, and find the earliest, lowest, page with the smallest
     * erase count.
     *
     * XXX: Of course, this will get blown away as soon as it gets claimed
     * as the GC page, but oh well.  It's just an optimization.  */
    
    _fs_lastpg = -1;
    _fs_best_erase_count = 0xFFFFFFFF;
    _fs_last_allocated_page = -1;
    _fs_free_pages = 0;
    
    for (int pg = 0; pg < REGION_FS_N_PAGES; pg++) {
        _fs_read_page_ofs(pg, 0, &pagehdr, sizeof(pagehdr));
        
        if (pagehdr.v_0x5001 == 0xFFFF) {
            if (!saw_blank_page)
                KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "this filesystem has a blank page in it... hmm...");
            saw_blank_page = 1;
            _fs_set_page_state(pg, PageStateClean);
            _fs_free_pages++;
            continue;
        }
        
        if (pagehdr.v_0x5001 != 0x5001) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has bad header version 0x%04x; I give up", pg, pagehdr.v_0x5001);
            return -1;
        }
        
        if (FLASHFLAG(pagehdr.empty, HDR_EMPTY_ALLOCATED) && !FLASHFLAG(pagehdr.empty, HDR_EMPTY_MOREBLOCKS) && _fs_lastpg == -1) {
            _fs_lastpg = pg;
        } else if ((_fs_lastpg != -1) && FLASHFLAG(pagehdr.empty, HDR_EMPTY_ALLOCATED)) {
            if (!saw_page_in_outer_space)
                KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d is marked as allocated, but page %d had the last page marker... hmm...", pg, _fs_lastpg);
            saw_page_in_outer_space = 1;
        }
        
        if (!FLASHFLAG(pagehdr.empty, HDR_EMPTY_ALLOCATED) || /* block has never been written to */
            (/* allocated && */ !FLASHFLAG(pagehdr.status, HDR_STATUS_FILE_START) && !FLASHFLAG(pagehdr.status, HDR_STATUS_FILE_CONT)) /* block was erased, has no data now */) {
            _fs_set_page_state(pg, PageStateClean);
            if (pagehdr.wear_level_counter < _fs_best_erase_count) {
                _fs_best_erase_count = pagehdr.wear_level_counter;
                _fs_last_allocated_page = pg;
            }
            nclean++;
            _fs_free_pages++;
            continue;
        }
        
        if (FLASHFLAG(pagehdr.status, HDR_STATUS_DEAD)) {
            _fs_set_page_state(pg, PageStateDirty);
            _fs_free_pages++;
            ndirty++;
            continue;
        }
        
        if (FLASHFLAG(pagehdr.status, HDR_STATUS_FILE_START)) {
            _fs_set_page_state(pg, PageStateFileStart);
            nused++;
            continue;
        }

        if (FLASHFLAG(pagehdr.status, HDR_STATUS_FILE_CONT)) {
            _fs_set_page_state(pg, PageStateFileCont);
            nused++;
            continue;
        }
        
        /* We should have hit one of those cases. */
        assert(0);
    }
 
    KERN_LOG("flash", APP_LOG_LEVEL_INFO, "filesystem has last page %d/%d (%d used, %d dirty, %d clean; next clean %d w/ count %d)", _fs_lastpg, REGION_FS_N_PAGES, nused, ndirty, nclean, _fs_last_allocated_page, _fs_best_erase_count);
    
    return 0;
}

/* Note that this should be immediately followed by an _fs_set_page_state. */
static int _fs_page_alloc()
{
    assert(_fs_best_erase_count != 0xFFFFFFFF);
    assert(_fs_last_allocated_page != -1);
    
    /* The general algorithm is: we look at the last allocated page, then if
     * it's taken, we iterate through the entire flash looking for an open
     * page with the best erase count.  At the same time, we write down the
     * new best erase count, and if we don't find a page with the old best
     * erase count, we iterate again with the new one.
     */
    
    int pgstart = _fs_last_allocated_page;
    int new_best_erase_count = 0xFFFFFFFF;
    
    for (int i = 0; i < REGION_FS_N_PAGES; i++) {
        struct fs_page_hdr pagehdr;
        
        int pg = (i + pgstart) % REGION_FS_N_PAGES;
        if (_fs_get_page_state(pg) != PageStateClean)
            continue;
        
        _fs_read_page_ofs(pg, 0, &pagehdr, sizeof(pagehdr));
        if (pagehdr.wear_level_counter < new_best_erase_count)
            new_best_erase_count = pagehdr.wear_level_counter;
        if (pagehdr.wear_level_counter == _fs_best_erase_count) {
            /* Looks like we're in luck. */
            _fs_last_allocated_page = pg;
            return pg;
        }
    }
    
    /* Ok, that didn't work out, but at least we have a new best erase
     * count.  Loop again with the new one. */
    _fs_best_erase_count = new_best_erase_count;
    for (int i = 0; i < REGION_FS_N_PAGES; i++) {
        struct fs_page_hdr pagehdr;

        int pg = (i + pgstart) % REGION_FS_N_PAGES;
        if (_fs_get_page_state(pg) != PageStateClean)
            continue;

        _fs_read_page_ofs(pg, 0, &pagehdr, sizeof(pagehdr));
        if (pagehdr.wear_level_counter == _fs_best_erase_count) {
            /* Finally, found one. */
            _fs_last_allocated_page = pg;
            return pg;
        }
    }
    
    return -1;
}

/*** GC routines. ***/

static int _gc_sector;

struct fs_gc_file_hdr {
    uint8_t version; /* XXX: what should this be? */
    uint8_t flags;
    uint16_t start_pg;
    uint32_t page_mask;
    uint8_t num_entries;
};

#define PAGES_PER_SECTOR (REGION_FS_ERASE_SIZE / REGION_FS_PAGE_SIZE)
static void _fs_gc_find_sector()
{
    /* What we want to find is an erase-block-contiguous region of empty
     * pages with the lowest erase count.  */
    uint32_t lowest_erase_count = 0xFFFFFFFF;
    int best_sector = -1;
    
    int pg;
    
    for (pg = 0; pg < REGION_FS_N_PAGES; pg += PAGES_PER_SECTOR) {
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
    KERN_LOG("flash", APP_LOG_LEVEL_INFO, "chose GC sector of %d, with erase count %d", _gc_sector, lowest_erase_count);
    
    /* Mark it all as in use so the allocator doesn't come for it. */
    for (int i = 0; i < PAGES_PER_SECTOR; i++) {
        _fs_set_page_state(_gc_sector + i, PageStateDirty);
    }
}

static void _fs_gc_trigger()
{
    panic("GC not yet supported");
}

/*** File creation and deletion. ***/

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
    if (_fs_page_table_init()) {
        _fs_valid = 0;
        return;
    }
    
    /* Do file-level cleanup. */
    for (pg = 0; pg <= _fs_lastpg; pg++) {
        if ((_fs_get_page_state(pg) != PageStateFileStart) &&
            (_fs_get_page_state(pg) != PageStateDirty))
            continue;
        
        _fs_read_file_hdr(pg, &buffer);

        if (!FLASHFLAG(hdr->status, HDR_STATUS_FILE_START)) { /* Only dead file starts need be considered. */
            continue;
        }

        /* Cases we might need to clean up from. */
        if (!FLASHFLAG(hdr->status, HDR_STATUS_DEAD) && hdr->st_create_complete) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d incompletely created; cleaning up", pg);
            assert(_delete_file_by_pg(pg) == 0);
            continue;
        }
        if (FLASHFLAG(hdr->status, HDR_STATUS_DEAD) && hdr->st_delete_complete) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d incompletely deleted; cleaning up", pg);
            assert(_delete_file_by_pg(pg) == 0);
            continue;
        }
        if (hdr->st_tmp_file) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d is a temp file; cleaning up", pg);
            assert(_delete_file_by_pg(pg) == 0);
            continue;
        }

        if (hdr->filename_len > MAX_FILENAME_LEN)
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has unexpectedly long file name: %d; it may cause further errors", pg, hdr->filename_len);

        if (!strcmp(buffer.name, "GC")) {
            KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "page %d has a GC file; I can't deal with this; go boot PebbleOS to clean up first", pg);
            _fs_valid = 0;
            return;
        }
    }
    
    _fs_gc_find_sector();
    
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
                file->flags = FILE_HAS_DIRENT;
                return 0;
            }
        }
    }

    return -1;
}

static uint16_t _fs_verily_alloc_page(enum page_state st)
{
    int pg = _fs_page_alloc();
    if (pg < 0)
    {
        _fs_gc_trigger();
        pg = _fs_page_alloc();
    }
    assert(pg >= 0);
    
    _fs_set_page_state(pg, st);
    if (pg > _fs_lastpg)
        _fs_lastpg = pg;
    
    return (uint16_t) pg;
}

struct fd *fs_creat_replacing(struct fd *fd, const char *name, size_t bytes, const struct file *previous /* can be NULL */)
{
    /* XXX: Take a fs mutex around this entire function. */
    size_t fs_bytes = bytes + (name ? strlen(name) : 0) + sizeof(struct fs_file_hdr) - sizeof(struct fs_page_hdr);
    size_t bytes_in_pg = REGION_FS_PAGE_SIZE - sizeof(struct fs_page_hdr);
    size_t npgs = fs_bytes / bytes_in_pg + ((fs_bytes % bytes_in_pg) ? 1 : 0);
    
    if (previous && !(previous->flags & FILE_HAS_DIRENT)) {
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "cannot replace file %s that ain't a real file", name);
        assert(0);
        return NULL;
    }
    
    KERN_LOG("flash", APP_LOG_LEVEL_DEBUG, "preparing to create file %s with %d pages (%d bytes)", name, npgs, bytes);
    if (_fs_free_pages < npgs)
    {
        KERN_LOG("flash", APP_LOG_LEVEL_ERROR, "not enough free storage for file");
        return NULL;
    }
    
    _fs_free_pages -= npgs;
    
    uint16_t startpg = 0;
    uint16_t pg; /* We have to do this here, since we also need a next page.  Sigh. */
    struct fs_file_hdr filehdr; /* We keep this around since we'll use it to rewrite the create_complete later. */
    
    pg = _fs_verily_alloc_page(PageStateFileStart);
    for (size_t pgn = 0; pgn < npgs; pgn++)
    {
        uint16_t nextpg = 0xFFFF;
        if (pgn != (npgs - 1))
            nextpg = _fs_verily_alloc_page(PageStateFileCont);
    
        struct fs_page_hdr pagehdr;
        _fs_read_page_ofs(pg, 0, &pagehdr, sizeof(pagehdr));
        
        if (pgn == 0)
        {
            /* Set up the whole file header. */
            memset(&filehdr, 0xFF, sizeof(filehdr));
            
            filehdr.v_0x5001 = 0x5001;
            filehdr.empty = pagehdr.empty & ~HDR_EMPTY_MOREBLOCKS;
            filehdr.status &= ~(HDR_STATUS_VALID | HDR_STATUS_FILE_START);
            /* XXX: update 'empty' status for someone else? */
            filehdr.wear_level_counter = pagehdr.wear_level_counter;
            filehdr.next_page = nextpg;
            filehdr.next_page_crc = fs_next_page_crc(nextpg);
            filehdr.pagehdr_crc = fs_pagehdr_crc((struct fs_page_hdr *)&filehdr);
            
            filehdr.file_size = bytes;
            if (name)
                filehdr.flag_2 &= ~HDR_FLAG_2_HAS_FILENAME;
            assert(strlen(name) < MAX_FILENAME_LEN);
            filehdr.filename_len = strlen(name);
            
            int rv = _fs_write_page_ofs(pg, 0, &filehdr, sizeof(filehdr));
            assert(rv >= 0);
            
            if (name) {
                rv = _fs_write_page_ofs(pg, sizeof(filehdr), name, strlen(name));
                assert(rv >= 0);
            }
            
            KERN_LOG("flash", APP_LOG_LEVEL_DEBUG, "wrote start page at %d", pg);
            
            /* Write it down for later to finish the create_complete op. */
            startpg = pg;
        } else {
            pagehdr.empty &= ~HDR_EMPTY_MOREBLOCKS;
            pagehdr.status &= ~(HDR_STATUS_VALID | HDR_STATUS_FILE_START);
            pagehdr.next_page = nextpg;
            pagehdr.next_page_crc = fs_next_page_crc(nextpg);
            pagehdr.pagehdr_crc = fs_pagehdr_crc(&pagehdr);
            
            int rv = _fs_write_page_ofs(pg, 0, &pagehdr, sizeof(pagehdr));
            assert(rv >= 0);
            
            KERN_LOG("flash", APP_LOG_LEVEL_DEBUG, "wrote continuation page %d at %d", pgn, pg);
        }
        
        pg = nextpg;
    }
    
    filehdr.st_create_complete = 0x0;
    int rv = _fs_write_page_ofs(startpg, 0, &filehdr, sizeof(filehdr));
    assert(rv >= 0);
    
    fd->file.startpage = startpg;
    fd->file.startpofs = (name ? strlen(name) : 0) + sizeof(struct fs_file_hdr);
    fd->file.size = bytes;
    fd->file.flags = FILE_HAS_DIRENT;
    
    fd->curpage = fd->file.startpage;
    fd->curpofs = fd->file.startpofs;
    fd->offset = 0;
    
    memset(&fd->replaces, 0x0, sizeof(fd->replaces));
    if (previous)
        fd->replaces = *previous;
    
    KERN_LOG("flash", APP_LOG_LEVEL_DEBUG, "file create complete");
    
    return fd;
}

struct fd *fs_creat(struct fd *fd, const char *name, size_t bytes)
{
    return fs_creat_replacing(fd, name, bytes, NULL);
}

void fs_open(struct fd *fd, const struct file *file)
{
    fd->file = *file;
    
    fd->curpage = fd->file.startpage;
    fd->curpofs = fd->file.startpofs;
    
    fd->offset  = 0;
}

void fs_file_from_file(struct file *file, const struct file *from, size_t offset, size_t len)
{
    struct fd fd;
    
    fs_open(&fd, from);
    fs_seek(&fd, offset, FS_SEEK_SET);
    
    *file = *from;
    file->flags &= ~FILE_HAS_DIRENT;
    file->startpage = fd.curpage;
    file->startpofs = fd.curpofs;

    if (file->size < offset)
        file->size = 0;
    else
        file->size -= offset;
    if (file->size > len)
        file->size = len;
}

void fs_file_from_flash(struct file *file, size_t addr, size_t len) 
{
    file->flags = FILE_IS_RAW_FLASH;
    file->startpage = 0;
    file->startpofs = addr;
    file->size = len;
}

void fs_mark_written(struct fd *fd)
{
    int rv;
    
    /* The only 'close' operation is to clean up tmpfile status, and to
     * erase the old file.  So we do that.  */
    if (fd->replaces.size)
    {
        /* First off, mark the old one as staged for deletion. */
        struct fs_file_hdr filehdr;
        _fs_read_page_ofs(fd->replaces.startpage, 0, &filehdr, sizeof(filehdr));
        filehdr.status &= ~HDR_STATUS_DEAD;
        rv = _fs_write_page_ofs(fd->replaces.startpage, 0, &filehdr, sizeof(filehdr));
        assert(rv >= 0);
        
        /* XXX: There is a race condition, built into the Pebble filesystem here. */
        
        /* Then, mark the new one as not-a-tempfile. */
        _fs_read_page_ofs(fd->file.startpage, 0, &filehdr, sizeof(filehdr));
        filehdr.st_tmp_file = 0x0;
        rv = _fs_write_page_ofs(fd->file.startpage, 0, &filehdr, sizeof(filehdr));
        assert(rv >= 0);
        
        /* Then shoot the old one in the head completely. */
        _delete_file_by_pg(fd->replaces.startpage);
    }
    
    memset(&fd, 0, sizeof(fd));
}

int fs_read(struct fd *fd, void *p, size_t bytes)
{
    size_t bytesrem;
    
    if (bytes > (fd->file.size - fd->offset))
        bytes = fd->file.size - fd->offset;
    bytesrem = bytes;
    
    if (fd->file.flags & FILE_IS_RAW_FLASH)
    {
        /* Special case. */
        flash_read_bytes(fd->curpofs, p, bytes);
        fd->curpofs += bytes;
        fd->offset += bytes;
        return bytes;
    }

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

int fs_write(struct fd *fd, const void *p, size_t bytes)
{
    size_t bytesrem;
    
    assert(!(fd->file.flags & FILE_IS_RAW_FLASH) && "cannot write to raw flash");
    
    if (bytes > (fd->file.size - fd->offset))
        bytes = fd->file.size - fd->offset;
    bytesrem = bytes;

    while (bytesrem)
    {
        size_t n = bytesrem;
        
        if (n > (REGION_FS_PAGE_SIZE - fd->curpofs))
            n = REGION_FS_PAGE_SIZE - fd->curpofs;
        
        _fs_write_page_ofs(fd->curpage, fd->curpofs, p, n);
        
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
    size_t newoffset;
    switch (whence)
    {
    case FS_SEEK_SET: newoffset = ofs; break;
    case FS_SEEK_CUR: newoffset = fd->offset + ofs /* XXX: overflow */; break;
    case FS_SEEK_END: newoffset = fd->file.size + ofs; break;
    }
    
    if (newoffset > fd->file.size)
        newoffset = fd->file.size;
    
    if (fd->file.flags & FILE_IS_RAW_FLASH)
    {
        fd->curpofs = fd->file.startpofs + newoffset;
        fd->offset = newoffset;
        return newoffset;
    }
    
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

long fs_size(struct fd *fd)
{
    return fd->file.size;
}
