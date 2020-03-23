/* blobdb.c
 * blob database is a simple flat file store in flash/ram
 * This provides functions to query this data
 * libRebbleOS
 *
 * Authors: Barry Carter <barry.carter@gmail.com>
 *          Joshua Wise <joshua@joshuawise.com>
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"
#include "node_list.h"
#include "timeline.h"
#include "blobdb.h"

/* Configure Logging */
#define MODULE_NAME "blobdb"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

/* BlobDBs have a few states for an entry:
 *
 *   Header empty: it's safe to write a header and an entry here.
 *
 *   Partial header write: we started to write a key and data length here,
 *   but got interrupted.  Skip over the header; no further data has been
 *   written.
 *
 *   Complete header write: The BLOBDB_FLAG_HEADER_WRITTEN flag is set. 
 *   Space has now been allocated for the rest of the data, but the data are
 *   not valid.  Skip over the header and the data.
 *
 *   Complete data: The BLOBDB_FLAG_WRITTEN flag is set.  The record is
 *   valid.
 *
 *   Erased: The BLOBDB_FLAG_ERASED flag is set.  The record should be
 *   ignored.
 */
 
#define FLAG_SET(flags, flag) ((flags & flag) == 0)
#define BLOBDB_FLAG_WRITTEN         1
#define BLOBDB_FLAG_HEADER_WRITTEN  2
#define BLOBDB_FLAG_ERASED          4

typedef struct blobdb_hdr {
    uint8_t flags;
    uint8_t key_len;
    uint16_t data_len;
} __attribute__((__packed__)) blobdb_hdr;

struct blobdb_database {
    uint8_t id;
    const char *filename;
    uint16_t def_db_size;
};

static const struct blobdb_database databases[] = {
    {
        .id = BlobDatabaseID_Test,
        .filename = "rebble/testblob",
        .def_db_size = 1024,
    },
    {
        .id = BlobDatabaseID_Notification,
        .filename = "rebble/notifstr",
        .def_db_size = 16384,
    },
    {
        .id = BlobDatabaseID_App,
        .filename = "rebble/appdb",
        .def_db_size = 16384,
    },
};

const struct blobdb_database *blobdb_open(uint16_t database_id) {
    for (int i = 0; i < sizeof(databases) / sizeof(databases[0]); i++)
    {
        if (databases[i].id == database_id)
            return &databases[i];
    }

    return NULL;
}

/* Positions the fd to the next valid header.  If no remaining valid headers
 * exist, positions the fd to the next free space in which one can write a
 * header (assuming there is enough space left in the file).  If "next" is
 * set, skips the header that the fd currently points to.
 *
 * Returns 1 if the fd points to a valid header, or 0 if it points to free
 * space (or has insufficient space left in the file for another header).
 */
static int _blobdb_seek_valid(struct fd *fdp, struct blobdb_hdr *hdrp, int next) {
    /* We work on a copy of the fd, because rewinding is expensive. */
    struct fd fd;
    struct blobdb_hdr hdr;
    
    memcpy(&fd, fdp, sizeof(fd));
    do {
        /* Put the location of the header we're about to read. */
        memcpy(fdp, &fd, sizeof(fd));
        if (fs_read(&fd, &hdr, sizeof(hdr)) < sizeof(hdr))
            break;
        
        /* Empty? */
        if ((hdr.flags == 0xFF) && (hdr.key_len == 0xFF) && (hdr.data_len == 0xFFFF)) {
            if (hdrp)
                memcpy(hdrp, &hdr, sizeof(hdr));
            return 0;
        }
        
        /* Partial header write? */
        if (((hdr.key_len != 0xFF) || (hdr.data_len != 0xFF)) &&
            !(FLAG_SET(hdr.flags, BLOBDB_FLAG_HEADER_WRITTEN) ||
              FLAG_SET(hdr.flags, BLOBDB_FLAG_WRITTEN) /* compatibility */)) {
            continue;
        }
        
        /* Erased -- or skipping the first? */
        if (FLAG_SET(hdr.flags, BLOBDB_FLAG_ERASED) || next) {
            next = 0;
            fs_seek(&fd, hdr.key_len + hdr.data_len, FS_SEEK_CUR);
            continue;
        }
        
        /* Looks good. */
        if (hdrp)
            memcpy(hdrp, &hdr, sizeof(hdr));
        return 1;
    } while(1);
    
    return 0;
}

static int _blobdb_iter_start_from_fd(struct fd *fd, struct blobdb_iter *it) {
    it->fd = *fd;
    
    it->key_len = it->data_len = 0;
    
    struct blobdb_hdr hdr;
    if (!_blobdb_seek_valid(&it->fd, &hdr, 0))
        return 0;
    
    it->key_len = hdr.key_len;
    it->data_len = hdr.data_len;
    
    return 1;
}
    
int blobdb_iter_start(const struct blobdb_database *db, struct blobdb_iter *it) {
    struct file file;
    struct fd fd;
    struct blobdb_hdr hdr;
    
    if (fs_find_file(&file, db->filename) < 0) {
        LOG_ERROR("file not found for db %s", db->filename);
        return 0;
    }
    fs_open(&fd, &file);
    
    return _blobdb_iter_start_from_fd(&fd, it);
} 

int blobdb_iter_next(struct blobdb_iter *it) {
    it->key_len = it->data_len = 0;

    struct blobdb_hdr hdr;
    if (!_blobdb_seek_valid(&it->fd, &hdr, 1))
        return 0;
    
    it->key_len = hdr.key_len;
    it->data_len = hdr.data_len;
    
    return 1;
}

int blobdb_iter_read_key(struct blobdb_iter *it, void *key) {
    struct fd fd = it->fd;
    
    fs_seek(&fd, sizeof(struct blobdb_hdr), FS_SEEK_CUR);
    return fs_read(&fd, key, it->key_len);
}

int blobdb_iter_read_data(struct blobdb_iter *it, int ofs, void *data, int len) {
    struct fd fd = it->fd;
    
    if (it->data_len <= ofs)
        return 0;
    
    len = (it->data_len > (ofs + len)) ? len : (it->data_len - ofs);
    
    fs_seek(&fd, sizeof(struct blobdb_hdr) + it->key_len + ofs, FS_SEEK_CUR);
    return fs_read(&fd, data, len);
}

static bool _compare(blobdb_operator_t operator, uint8_t *where_prop, uint8_t *where_val, size_t size)
{
    uint16_t i = 0;
    uint32_t val1 = 0;
    uint32_t val2 = 0;
    
    if (size <= 4) {
        memcpy(&val1, where_prop, size);
        memcpy(&val2, where_val, size);
    }
    
    char type = '>';
    bool rval = false;

    switch(operator) {
        case Blob_Gtr:
            type = '>';
            if (size > 4) break;
            if (val1 > val2)
                rval = true;
            break;
        case Blob_Less:
            type = '<';
            if (size > 4) break;
            if (val1 < val2)
                rval = true;
            break;
        case Blob_Eq:
            type = '=';
            rval = true;
            for(i = 0; i < size; i++)
                if (where_prop[i] != where_val[i])
                    rval = false;
            break;
        case Blob_NEq:
            type = '!';
            for(i = 0; i < size; i++)
                if (where_prop[i] != where_val[i])
                    break;
            if (i == size)
                rval = true;
            break;
        default:
            assert(!"bad operator for _compare");
    }

    LOG_DEBUG("COMP: %d %c %d := %s", val1, type, val2, rval ? "true" : "false");
    return rval;
}

int blobdb_select(struct blobdb_iter *it, list_head/*<blobdb_select_result>*/ *head, struct blobdb_selector *selectors) {
    int n = 0;
    
    int valid = 1;
    for (; valid; valid = blobdb_iter_next(it)) {
        bool comp = true;
        struct blobdb_selector *selp;
        int nrv = 0;
        
        /* Do comparisons first. */
        for (selp = selectors; selp->operator; selp++) {
            if (selp->operator >= Blob_Result) {
                nrv++;
                continue;
            }
            
            uint8_t prop[selp->size];
            if (selp->offsetof == BLOBDB_SELECTOR_OFFSET_KEY) {
                if (it->key_len != selp->size) {
                    comp = false;
                    break;
                }
                if (blobdb_iter_read_key(it, prop) != selp->size) {
                    comp = false;
                    break;
                }
            } else {
                if (blobdb_iter_read_data(it, selp->offsetof, prop, selp->size) != selp->size) {
                    comp = false;
                    break;
                }
            }
            
            if (!_compare(selp->operator, prop, selp->val, selp->size)) {
                comp = false;
                break;
            }
        }
        
        if (!comp)
            continue;
        
        /* Now construct and add a result. */
        struct blobdb_select_result *res = app_calloc(1, sizeof(struct blobdb_select_result) + nrv * sizeof(void *));
        if (!res)
            return n;
        
        res->it = *it;
        res->key = app_calloc(1, it->key_len);
        if (!res->key) {
            app_free(res);
            return n;
        }
        
        /* Construct result values. */
        int crv = 0;
        for (selp = selectors; selp->operator; selp++) {	
            void *r;
            
            if (selp->operator < Blob_Result)
                continue;
            
            if (selp->operator == Blob_Result) {
                r = app_calloc(1, selp->size);
                if (!r)
                    break;
                if (blobdb_iter_read_data(it, selp->offsetof, r, selp->size) != selp->size) {
                    app_free(r);
                    break;
                }
            } else if (selp->operator == Blob_Result_FullyLoad) {
                r = app_calloc(1, it->data_len);
                if (!r)
                    break;
                if (blobdb_iter_read_data(it, 0, r, it->data_len) != it->data_len) {
                    app_free(r);
                    break;
                }
            }
            
            res->result[crv] = r;
            crv++;
        }
        
        /* if one fails, clean it all up */
        if (crv != nrv) {
            for (int i = 0; i < crv; i++)
                app_free(res->result[i]);
            app_free(res);
            return n;
        }
        
        res->nres = nrv;
        list_insert_tail(head, &res->node);
        
        n++;
    }
    
    return n;
}

void blobdb_select_free_result(struct blobdb_select_result *res) {
    for (int i = 0; i < res->nres; i++)
        app_free(res->result[i]);
    app_free(res->key);
    app_free(res);
}

void blobdb_select_free_all(list_head *head)
{
    list_node *n;
    while((n = list_get_head(head)))
    {
        list_remove(head, n);

        struct blobdb_select_result *res = list_elem(n, struct blobdb_select_result, node);
        blobdb_select_free_result(res);
    }
}

int _blobdb_gc_for_bytes(const struct blobdb_database *db, struct fd *fd, int bytes) {
    fs_seek(fd, 0, FS_SEEK_SET);
    
    /* Count up how many bytes we stand to liberate. */
    struct blobdb_iter it;
    int valid;
    int used = 0;
    for (valid = _blobdb_iter_start_from_fd(fd, &it); valid; valid = blobdb_iter_next(&it))
        used += sizeof(struct blobdb_hdr) + it.key_len + it.data_len;
    int tlen = fs_seek(&it.fd, 0, FS_SEEK_CUR);
    int fsz = fs_size(fd);
    LOG_INFO("gc: blobdb %s will have %d/%d bytes used after, has %d/%d bytes used", db->filename, used, fsz, tlen, fsz);
    
    if (bytes >= (fsz - used)) {
        LOG_ERROR("gc: which is not enough space for a %d byte entry, sadly");
        return 0;
    }
    
    /* Ok, here we go. */
    valid = _blobdb_iter_start_from_fd(fd, &it);
    struct file oldfile = fd->file;
    if (fs_creat_replacing(fd, db->filename, db->def_db_size, &oldfile) == NULL) {
        LOG_ERROR("gc: failed to create new file");
        return 0;
    }
    for (; valid; valid = blobdb_iter_next(&it)) {
        int nbytes = sizeof(struct blobdb_hdr) + it.key_len + it.data_len;
        struct fd from = it.fd;
        while (nbytes) {
            uint8_t buf[16];
            size_t ibytes = nbytes < 16 ? nbytes : 16;
            
            if (fs_read(&from, buf, ibytes) != ibytes) {
                LOG_ERROR("gc: failed to read");
                return 0;
            }
            if (fs_write(fd, buf, ibytes) != ibytes) {
                LOG_ERROR("gc: failed to write");
                return 0;
            }
            
            nbytes -= ibytes;
        }
    }
    fs_mark_written(fd); /* oldfile is now dead */
    
    return 1;
}

int blobdb_insert(const struct blobdb_database *db, uint8_t *key, uint16_t key_size, uint8_t *data, uint16_t data_size)
{
    struct blobdb_iter it;

    /* Create the db if it doesn't already exist. */
    struct file file;
    if (fs_find_file(&file, db->filename) < 0) {
        struct fd fd;
        LOG_ERROR("blobdb %s does not exist, so I'm going to go create it, wish me luck", db->filename);
        if (fs_creat(&fd, db->filename, db->def_db_size) == NULL) {
            LOG_ERROR("nope, that did not work either, I give up");
            return Blob_DatabaseFull;
        }
        fs_mark_written(&fd);
    }
    
    /* Iterate looking for dup keys, then position us at eof. */
    int valid;
    for (valid = blobdb_iter_start(db, &it); valid; valid = blobdb_iter_next(&it)) {
        if (it.key_len != key_size)
            continue;
        
        uint8_t rdkey[key_size];
        if (blobdb_iter_read_key(&it, rdkey) < key_size)
            return Blob_GeneralFailure;
        
        /* XXX: Overwrite the dup key, rather than failing.  Do this by
         * writing a new one at the end, then marking this one as deleted. 
         * Special care needed if there are multiple interrupted overwrites.
         */
        if (memcmp(key, rdkey, key_size) == 0) {
            return Blob_InvalidData;
        }
    }

    int pos = fs_seek(&it.fd, 0, FS_SEEK_CUR);
    if (pos + sizeof(struct blobdb_hdr) + key_size + data_size > it.fd.file.size) {
        if (!_blobdb_gc_for_bytes(db, &it.fd, sizeof(struct blobdb_hdr) + key_size + data_size)) {
            LOG_ERROR("not enough space %d %d for new entry", it.fd.file.size, pos); //+ sizeof(struct blobdb_hdr) + key_size + data_size);
            return Blob_DatabaseFull;
        }
    }

    /* Carefully start by writing a header. */
    struct blobdb_hdr hdr;
    struct fd hfd = it.fd;
    
    hdr.flags = 0xFF;
    hdr.key_len = key_size;
    hdr.data_len = data_size;
    if (fs_write(&hfd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to write header");
        return Blob_GeneralFailure;
    }
    
    /* Mark the header as completely written. */
    hdr.flags &= ~BLOBDB_FLAG_HEADER_WRITTEN;
    hfd = it.fd;
    if (fs_write(&hfd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to write header");
        return Blob_GeneralFailure;
    }
    
    /* Write the data. */
    if (fs_write(&hfd, key, key_size) < key_size) {
        LOG_ERROR("failed to write key");
        return Blob_GeneralFailure;
    }
    if (fs_write(&hfd, data, data_size) < data_size) {
        LOG_ERROR("failed to write data");
        return Blob_GeneralFailure;
    }
    
    /* Mark the data as written. */
    hfd = it.fd;
    hdr.flags &= ~BLOBDB_FLAG_WRITTEN;
    if (fs_write(&hfd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to update header");
        return Blob_GeneralFailure;
    }
    
    return Blob_Success;
}

int blobdb_delete(struct blobdb_iter *it)
{
    struct fd fd = it->fd;
    struct blobdb_hdr hdr;
    
    if (fs_read(&fd, &hdr, sizeof(hdr)) < sizeof(hdr))
        return Blob_GeneralFailure;
    hdr.flags &= ~BLOBDB_FLAG_ERASED;
    
    fd = it->fd;
    if (fs_write(&fd, &hdr, sizeof(hdr)) < sizeof(hdr))
        return Blob_GeneralFailure;

    return Blob_Success;
}
