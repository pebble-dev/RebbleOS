/* rdb.c
 * {rebble, record, [un]reliable} database: a key-value record store in flash
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
#include "rdb.h"

/* Configure Logging */
#define MODULE_NAME "rdb"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

/* RDBs have a few states for an entry:
 *
 *   Header empty: it's safe to write a header and an entry here.
 *
 *   Partial header write: we started to write a key and data length here,
 *   but got interrupted.  Skip over the header; no further data has been
 *   written.
 *
 *   Complete header write: The RDB_FLAG_HEADER_WRITTEN flag is set. 
 *   Space has now been allocated for the rest of the data, but the data are
 *   not valid.  Skip over the header and the data.
 *
 *   Complete data: The RDB_FLAG_WRITTEN flag is set.  The record is
 *   valid.
 *
 *   Erased: The RDB_FLAG_ERASED flag is set.  The record should be
 *   ignored.
 */
 
#define FLAG_SET(flags, flag) ((flags & flag) == 0)
#define RDB_FLAG_WRITTEN         1
#define RDB_FLAG_HEADER_WRITTEN  2
#define RDB_FLAG_ERASED          4

typedef struct rdb_hdr {
    uint8_t flags;
    uint8_t key_len;
    uint16_t data_len;
} __attribute__((__packed__)) rdb_hdr;

struct rdb_database {
    uint8_t id;
    const char *filename;
    uint16_t def_db_size;
    int locked;
    SemaphoreHandle_t mutex;
    StaticSemaphore_t mutex_buf;
};

static struct rdb_database databases[] = {
    {
        .id = RDB_ID_TEST,
        .filename = "rebble/rdbtest",
        .def_db_size = 1024,
    },
    {
        .id = RDB_ID_NOTIFICATION,
        .filename = "rebble/notifstr",
        .def_db_size = 16384,
    },
    {
        .id = RDB_ID_APP,
        .filename = "rebble/appdb",
        .def_db_size = 16384,
    },
    {
        .id = RDB_ID_APP_PERSIST,
        .filename = "rebble/apppersistdb",
        .def_db_size = 16384,
    },
};

struct rdb_database *rdb_open(uint16_t database_id) {
    for (int i = 0; i < sizeof(databases) / sizeof(databases[0]); i++)
    {
        if (databases[i].id == database_id) {
            /* Avoid race on mutex setup. */
            taskENTER_CRITICAL();
            if (!databases[i].mutex)
                databases[i].mutex = xSemaphoreCreateMutexStatic(&databases[i].mutex_buf);
            taskEXIT_CRITICAL();
            
            xSemaphoreTake(databases[i].mutex, portMAX_DELAY);
            databases[i].locked = 1;
            return &databases[i];
        }
    }

    return NULL;
}

void rdb_close(struct rdb_database *db) {
    assert(db->locked);
    db->locked = 0;
    xSemaphoreGive(db->mutex);
}

/* Positions the fd to the next valid header.  If no remaining valid headers
 * exist, positions the fd to the next free space in which one can write a
 * header (assuming there is enough space left in the file).  If "next" is
 * set, skips the header that the fd currently points to.
 *
 * Returns 1 if the fd points to a valid header, or 0 if it points to free
 * space (or has insufficient space left in the file for another header).
 */
static int _rdb_seek_valid(struct fd *fdp, struct rdb_hdr *hdrp, int next) {
    /* We work on a copy of the fd, because rewinding is expensive. */
    struct fd fd;
    struct rdb_hdr hdr;
    
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
            !(FLAG_SET(hdr.flags, RDB_FLAG_HEADER_WRITTEN) ||
              FLAG_SET(hdr.flags, RDB_FLAG_WRITTEN) /* compatibility */)) {
            continue;
        }
        
        /* Erased -- or skipping the first? */
        if (FLAG_SET(hdr.flags, RDB_FLAG_ERASED) || next) {
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

static int _rdb_iter_start_from_fd(struct fd *fd, struct rdb_iter *it) {
    it->fd = *fd;
    
    it->key_len = it->data_len = 0;
    
    struct rdb_hdr hdr;
    if (!_rdb_seek_valid(&it->fd, &hdr, 0))
        return 0;
    
    it->key_len = hdr.key_len;
    it->data_len = hdr.data_len;
    
    return 1;
}
    
int rdb_iter_start(const struct rdb_database *db, struct rdb_iter *it) {
    struct file file;
    struct fd fd;
    struct rdb_hdr hdr;
    
    assert(db->locked);
    
    if (fs_find_file(&file, db->filename) < 0) {
        LOG_ERROR("file not found for db %s", db->filename);
        return 0;
    }
    fs_open(&fd, &file);
    
    return _rdb_iter_start_from_fd(&fd, it);
} 

int rdb_iter_next(struct rdb_iter *it) {
    it->key_len = it->data_len = 0;

    struct rdb_hdr hdr;
    if (!_rdb_seek_valid(&it->fd, &hdr, 1))
        return 0;
    
    it->key_len = hdr.key_len;
    it->data_len = hdr.data_len;
    
    return 1;
}

int rdb_iter_read_key(struct rdb_iter *it, void *key) {
    struct fd fd = it->fd;
    
    fs_seek(&fd, sizeof(struct rdb_hdr), FS_SEEK_CUR);
    return fs_read(&fd, key, it->key_len);
}

int rdb_iter_read_data(struct rdb_iter *it, int ofs, void *data, int len) {
    struct fd fd = it->fd;
    
    if (it->data_len <= ofs)
        return 0;
    
    len = (it->data_len > (ofs + len)) ? len : (it->data_len - ofs);
    
    fs_seek(&fd, sizeof(struct rdb_hdr) + it->key_len + ofs, FS_SEEK_CUR);
    return fs_read(&fd, data, len);
}

static bool _compare(rdb_operator_t operator, uint8_t *where_prop, uint8_t *where_val, size_t size)
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
        case RDB_OP_GREATER:
            type = '>';
            if (size > 4) break;
            if (val1 > val2)
                rval = true;
            break;
        case RDB_OP_LESS:
            type = '<';
            if (size > 4) break;
            if (val1 < val2)
                rval = true;
            break;
        case RDB_OP_EQ:
            type = '=';
            rval = true;
            for(i = 0; i < size; i++)
                if (where_prop[i] != where_val[i])
                    rval = false;
            break;
        case RDB_OP_NEQ:
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

int rdb_select(struct rdb_iter *it, list_head/*<rdb_select_result>*/ *head, struct rdb_selector *selectors) {
    int n = 0;
    
    int valid = 1;
    for (; valid; valid = rdb_iter_next(it)) {
        bool comp = true;
        struct rdb_selector *selp;
        int nrv = 0;
        
        /* Do comparisons first. */
        for (selp = selectors; selp->operator; selp++) {
            if (selp->operator >= RDB_OP_RESULT) {
                nrv++;
                continue;
            }
            
            uint8_t prop[selp->size];
            if (selp->offsetof == RDB_SELECTOR_OFFSET_KEY) {
                if (it->key_len != selp->size) {
                    comp = false;
                    break;
                }
                if (rdb_iter_read_key(it, prop) != selp->size) {
                    comp = false;
                    break;
                }
            } else {
                if (rdb_iter_read_data(it, selp->offsetof, prop, selp->size) != selp->size) {
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
        struct rdb_select_result *res = calloc(1, sizeof(struct rdb_select_result) + nrv * sizeof(void *));
        if (!res)
            return n;
        
        res->it = *it;
        res->key = calloc(1, it->key_len);
        if (!res->key) {
            free(res);
            return n;
        }
        
        /* Construct result values. */
        int crv = 0;
        for (selp = selectors; selp->operator; selp++) {	
            void *r;
            
            if (selp->operator < RDB_OP_RESULT)
                continue;
            
            if (selp->operator == RDB_OP_RESULT) {
                r = calloc(1, selp->size);
                if (!r)
                    break;
                if (rdb_iter_read_data(it, selp->offsetof, r, selp->size) != selp->size) {
                    free(r);
                    break;
                }
            } else if (selp->operator == RDB_OP_RESULT_FULLY_LOAD) {
                r = calloc(1, it->data_len);
                if (!r)
                    break;
                if (rdb_iter_read_data(it, 0, r, it->data_len) != it->data_len) {
                    free(r);
                    break;
                }
            }
            
            res->result[crv] = r;
            crv++;
        }
        
        /* if one fails, clean it all up */
        if (crv != nrv) {
            for (int i = 0; i < crv; i++)
                free(res->result[i]);
            free(res);
            return n;
        }
        
        res->nres = nrv;
        list_insert_tail(head, &res->node);
        
        n++;
    }
    
    return n;
}

void rdb_select_free_result(struct rdb_select_result *res) {
    for (int i = 0; i < res->nres; i++)
        free(res->result[i]);
    free(res->key);
    free(res);
}

void rdb_select_free_all(list_head *head)
{
    list_node *n;
    while((n = list_get_head(head)))
    {
        list_remove(head, n);

        struct rdb_select_result *res = list_elem(n, struct rdb_select_result, node);
        rdb_select_free_result(res);
    }
}

int _rdb_gc_for_bytes(const struct rdb_database *db, struct fd *fd, int bytes) {
    fs_seek(fd, 0, FS_SEEK_SET);
    
    /* Count up how many bytes we stand to liberate. */
    struct rdb_iter it;
    int valid;
    int used = 0;
    for (valid = _rdb_iter_start_from_fd(fd, &it); valid; valid = rdb_iter_next(&it))
        used += sizeof(struct rdb_hdr) + it.key_len + it.data_len;
    int tlen = fs_seek(&it.fd, 0, FS_SEEK_CUR);
    int fsz = fs_size(fd);
    LOG_INFO("gc: rdb %s will have %d/%d bytes used after, has %d/%d bytes used", db->filename, used, fsz, tlen, fsz);
    
    if (bytes >= (fsz - used)) {
        LOG_ERROR("gc: which is not enough space for a %d byte entry, sadly");
        return 0;
    }
    
    /* Ok, here we go. */
    valid = _rdb_iter_start_from_fd(fd, &it);
    struct file oldfile = fd->file;
    if (fs_creat_replacing(fd, db->filename, db->def_db_size, &oldfile) == NULL) {
        LOG_ERROR("gc: failed to create new file");
        return 0;
    }
    for (; valid; valid = rdb_iter_next(&it)) {
        int nbytes = sizeof(struct rdb_hdr) + it.key_len + it.data_len;
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

int rdb_create(const struct rdb_database *db)
{
    struct file file;
    if (fs_find_file(&file, db->filename) < 0) {
        struct fd fd;
        LOG_ERROR("rdb %s does not exist, so I'm going to go create it, wish me luck", db->filename);
        if (fs_creat(&fd, db->filename, db->def_db_size) == NULL) {
            LOG_ERROR("nope, that did not work either, I give up");
            return Blob_DatabaseFull;
        }
        fs_mark_written(&fd);
    }
    return Blob_Success;
}

int rdb_insert(const struct rdb_database *db, uint8_t *key, uint16_t key_size, uint8_t *data, uint16_t data_size)
{
    struct rdb_iter it;
    assert(db->locked);
    
    /* Create the db if it doesn't already exist. */      
    int rv = rdb_create(db);
    if (rv != Blob_Success)
        return rv;    
    
    /* Iterate looking for dup keys, then position us at eof. */
    int valid;
    for (valid = rdb_iter_start(db, &it); valid; valid = rdb_iter_next(&it)) {
        if (it.key_len != key_size)
            continue;
        
        uint8_t rdkey[key_size];
        if (rdb_iter_read_key(&it, rdkey) < key_size)
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
    if (pos + sizeof(struct rdb_hdr) + key_size + data_size > it.fd.file.size) {
        if (!_rdb_gc_for_bytes(db, &it.fd, sizeof(struct rdb_hdr) + key_size + data_size)) {
            LOG_ERROR("not enough space %d %d for new entry", it.fd.file.size, pos); //+ sizeof(struct rdb_hdr) + key_size + data_size);
            return Blob_DatabaseFull;
        }
    }

    /* Carefully start by writing a header. */
    struct rdb_hdr hdr;
    struct fd hfd = it.fd;
    
    hdr.flags = 0xFF;
    hdr.key_len = key_size;
    hdr.data_len = data_size;
    if (fs_write(&hfd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to write header");
        return Blob_GeneralFailure;
    }
    
    /* Mark the header as completely written. */
    hdr.flags &= ~RDB_FLAG_HEADER_WRITTEN;
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
    hdr.flags &= ~RDB_FLAG_WRITTEN;
    if (fs_write(&hfd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to update header");
        return Blob_GeneralFailure;
    }
    
    return Blob_Success;
}

int rdb_update(const struct rdb_database *db, const uint8_t *key, const uint16_t key_size, const uint8_t *data, const size_t data_size)
{
    struct rdb_iter it;
    rdb_select_result_list head;
    assert(db);
    list_init_head(&head);

    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        return Blob_InvalidDatabaseID;
    }
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, key_size, RDB_OP_EQ, key },
        { }
    };

    if (!rdb_select(&it, &head, selectors))
        return Blob_KeyDoesNotExist;

    /* Always delete and re-insert */    
    struct rdb_select_result *res;
    rdb_select_result_foreach(res, &head) {
        if (!rdb_delete(&res->it) ||
            !rdb_insert(db, key, res->it.key_len, data, data_size)) {
            rdb_select_free_all(&head);
            LOG_ERROR("update: failed to create value");
            return Blob_GeneralFailure;
        }
    }
    
    rdb_select_free_all(&head);
    return Blob_Success;
}

int rdb_delete(struct rdb_iter *it)
{
    struct fd fd = it->fd;
    struct rdb_hdr hdr;
    
    if (fs_read(&fd, &hdr, sizeof(hdr)) < sizeof(hdr))
        return Blob_GeneralFailure;
    hdr.flags &= ~RDB_FLAG_ERASED;
    
    fd = it->fd;
    if (fs_write(&fd, &hdr, sizeof(hdr)) < sizeof(hdr))
        return Blob_GeneralFailure;

    return Blob_Success;
}
