/* blob_db.c
 * blob database is a simple flat file store in flash/ram
 * This provides functions to query this data
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"
#include "node_list.h"
#include "timeline.h"
#include "blob_db.h"


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

typedef struct database_t {
    uint8_t id;
    const char *filename;
    uint16_t def_db_size;
} database;

static const database databases[] = {
    {
        .id = BlobDatabaseID_Test,
        .filename = "rebble/testblob",
        .def_db_size = 16384,
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
#define DB_COUNT 3
};

void _blobdb_select_items2(list_head *head, struct fd *fd, uint8_t database_id, 
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator,
                            uint16_t where_offsetof_property1, uint8_t where_property_size1, 
                            uint8_t *where_val1, Blob_Operator operator1);


static const database *_find_database(uint8_t id)
{
    for (uint8_t i = 0; i < DB_COUNT; i++)
    {
        if (databases[i].id == id)
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
            fs_seek(&fd, sizeof(hdr), FS_SEEK_CUR);
            continue;
        }
        
        /* Erased -- or skipping the first? */
        if (FLAG_SET(hdr.flags, BLOBDB_FLAG_ERASED) || next) {
            next = 0;
            fs_seek(&fd, sizeof(hdr) + hdr.key_len + hdr.data_len, FS_SEEK_CUR);
            continue;
        }
        
        /* Looks good. */
        if (hdrp)
            memcpy(hdrp, &hdr, sizeof(hdr));
        return 1;
    } while(1);
    
    return 0;
}

static int _blobdb_seek_first(struct fd *fdp, struct blobdb_hdr *hdrp) {
    return _blobdb_seek_valid(fdp, hdrp, 0);
}

static int _blobdb_seek_next(struct fd *fdp, struct blobdb_hdr *hdrp) {
    return _blobdb_seek_valid(fdp, hdrp, 1);
}

#if 0
/* XXX: in the morning */
struct blobdb_select_entry {
    uint16_t offsetof;
    uint16_t size;
    Blob_Operator operator; /* new operator: Blob_Result, which turns this into a result */
    void *val;
}
struct blobdb_select_result /* inserted into list_head */ {
    struct fd fd; /* pointer to data */
    void *key;
    uint8_t key_len;
    list_node node;
    void *result[];
}
int blobdb_select(list_head *head, struct fd *fd, struct blobdb_select_entry *entries);
void blobdb_free_results(list_head *head);
/* use like: */
{
    struct blobdb_select_entry selects[] = {
        { offsetof(timeline_item, timestamp), FIELD_SIZEOF(timeline_item, timestamp), Blob_Gtr, &from_timestamp },
        { offsetof(timeline_item, timeline_type), FIELD_SIZEOF(timeline_item, timeline_type), Blob_Eq, &val_type },
        { offsetof(timeline_item, uuid), FIELD_SIZEOF(timeline_item, uuid), Blob_Result },
        { offsetof(timeline_item, timestamp), FIELD_SIZEOF(timeline_item, timestamp), Blob_Result },
        { 0 }
    };
    int ntimeline = blobdb_select(head, timeline_fd, selects);
    [... process head ...]
    blobdb_free_results(head);
}
#endif

static int32_t _blob_db_find_item_entry(struct fd *fd, const database *db, size_t file_size, uint8_t *key, uint8_t key_size)
{
    uint8_t _tmp_key[key_size];
    size_t _data_size = 0;
    struct blobdb_hdr hdr;
    int valid;

    valid = _blobdb_seek_first(fd, &hdr);
    while (valid) {
        if (hdr.key_len != key_size) {
            valid = _blobdb_seek_next(fd, &hdr);
            continue;
        }
        
        int idx = fs_seek(fd, 0, FS_SEEK_CUR);

        fs_seek(fd, sizeof(hdr), FS_SEEK_CUR);
        if (fs_read(fd, _tmp_key, key_size) < key_size)
            break;

        /* End of file */
        if (uuid_is_int((Uuid  *)_tmp_key, 0xFF))
            break;

        if (memcmp(key, _tmp_key, key_size) == 0)
        {
            LOG_INFO("Found at index: %d", idx);
            return idx;
        }

        fs_seek(fd, hdr.data_len, FS_SEEK_CUR);
        valid = _blobdb_seek_first(fd, &hdr); /* we already seeked */
    }

    return -1;
}

static bool _compare(Blob_Operator operator, uint8_t *where_prop, uint8_t *where_val, size_t size)
{
    uint16_t i = 0;
    uint32_t val1 = 0;
    uint32_t val2 = 0;
    memcpy(&val1, where_prop, size);
    memcpy(&val2, where_val, size);
    
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
    }

    LOG_DEBUG("COMP: %d %c %d := %s", val1, type, val2, rval ? "true" : "false");
    return rval;
}

uint16_t blobdb_select_items_all(uint8_t database_id, list_head *head,
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size)
{
    return blobdb_select_items2(database_id, head,
                                 select1_offsetof_property, select1_property_size,
                                 select2_offsetof_property, select2_property_size,
                                 0, 0,
                                 NULL, 0,
                                 0, 0,
                                 NULL, 0);
}

uint16_t blobdb_select_items1(uint8_t database_id, list_head *head,
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator)
{
    return blobdb_select_items2(database_id, head,
                                 select1_offsetof_property, select1_property_size,
                                 select2_offsetof_property, select2_property_size,
                                 where_offsetof_property, where_property_size,
                                 where_val, operator,
                                 0, 0,
                                 NULL, 0);
}

uint16_t blobdb_select_items2(uint8_t database_id, list_head *head,
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator,
                            uint16_t where_offsetof_property1, uint8_t where_property_size1, 
                            uint8_t *where_val1, Blob_Operator operator1)
{
    struct fd fd;
    struct file file;

    const database *db = _find_database(database_id);
    if (fs_find_file(&file, db->filename) < 0) {
        LOG_ERROR("file not found for db %d", database_id);
        return 0;
    }
    
    fs_open(&fd, &file);
    _blobdb_select_items2(head, &fd, database_id, 
                          select1_offsetof_property, select1_property_size, 
                          select2_offsetof_property, select2_property_size, 
                          where_offsetof_property, where_property_size, 
                          where_val, operator,
                          where_offsetof_property1, where_property_size1, 
                          where_val1, operator1);

    return 0;

}

void _blobdb_select_items2(list_head *head, struct fd *fd, uint8_t database_id, 
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator,
                            uint16_t where_offsetof_property1, uint8_t where_property_size1, 
                            uint8_t *where_val1, Blob_Operator operator1)
{
    int idx = 0;
    const database *db = _find_database(database_id);
    struct blobdb_hdr hdr;
    int valid;
    
    valid = _blobdb_seek_first(fd, &hdr);
    while (valid) {
        fs_seek(fd, sizeof(hdr), FS_SEEK_CUR);
        
        struct fd kfd = *fd;
        fs_seek(fd, hdr.key_len, FS_SEEK_CUR);
        
        struct fd dfd = *fd; /* data pointer */
        fs_seek(fd, hdr.data_len, FS_SEEK_CUR);
        
        bool comp = true;
        
        /* XXX: check property sizes vs data_size */
        if (where_property_size && where_offsetof_property)
        {
            struct fd pfd = dfd;
            
            if ((where_property_size + where_offsetof_property) > hdr.data_len)
                comp = false;
            else {
                uint8_t where_prop[where_property_size];
                fs_seek(&pfd, where_offsetof_property, FS_SEEK_CUR);
                fs_read(&pfd, where_prop, where_property_size);
            
                comp = comp && _compare(operator, where_prop, where_val, where_property_size);
            }
        }
        if (where_property_size1 && where_offsetof_property1)
        {
            struct fd pfd = dfd;
            
            if ((where_property_size1 + where_offsetof_property1) > hdr.data_len)
                comp = false;
            else {
                uint8_t where_prop1[where_property_size1];
                fs_seek(&pfd, where_offsetof_property, FS_SEEK_CUR);
                fs_read(&pfd, where_prop1, where_property_size1);

                comp = comp && _compare(operator1, where_prop1, where_val1, where_property_size1);
            }
        }
        
        if (comp)
        {
            /* we have a match */
            blobdb_result_set *set = calloc(1, sizeof(blobdb_result_set));
            list_init_node(&set->node);

            struct fd pfd = dfd;
            set->select1 = calloc(1, select1_property_size);
            fs_seek(&pfd, select1_offsetof_property, FS_SEEK_CUR);
            fs_read(&pfd, set->select1, select1_property_size);

            if (select2_property_size)
            {
                pfd = dfd;
                
                set->select2 = calloc(1, select2_property_size);
                fs_seek(&pfd, select2_offsetof_property, FS_SEEK_CUR);
                fs_read(&pfd, set->select2, select2_property_size);
            }
            
            set->key_size = hdr.key_len;
            set->key = calloc(1, hdr.key_len);
            fs_read(&kfd, set->key, hdr.key_len);

            list_insert_tail(head, &set->node);
        }

        valid = _blobdb_seek_first(fd, &hdr);
    }
}

void blobdb_resultset_destroy(list_head *lh)
{
    list_node *n = list_get_head(lh);
    while(n)
    {
        list_remove(lh, n);

        blobdb_result_set *set = list_elem(n, blobdb_result_set, node);
        free(set->select1);
        free(set->select2);
        free(set->key);
        free(set);

        n = list_get_head(lh);
    }
    free(lh);
}

int32_t _blob_db_flash_load_blob(const database *db, uint8_t *key, uint8_t key_size, uint8_t **data)
{
    struct file file;
    struct fd fd;

    if (fs_find_file(&file, db->filename) < 0)
    {
        LOG_ERROR("file not found");
        return -1;
    }
    fs_open(&fd, &file);

    int32_t idx = _blob_db_find_item_entry(&fd, db, file.size, key, key_size);
    if (idx < 0)
        return -1;

    if (!data)
        return idx;

    struct blobdb_hdr hdr;
    fs_seek(&fd, idx, FS_SEEK_SET);
    if (fs_read(&fd, &hdr, sizeof(hdr)) != sizeof(hdr))
        return -1;

    LOG_DEBUG("Data Size %d", hdr.data_len);
    *data = app_calloc(hdr.data_len, 1);
    
    fs_seek(&fd, idx + sizeof(hdr) + hdr.key_len, FS_SEEK_SET);
    if (fs_read(&fd, *data, hdr.data_len) != hdr.data_len)
        return -1;

    return idx;
}


uint8_t blobdb_select(uint16_t database_id, uint8_t *key, uint8_t key_size, uint8_t **data)
{
    const database *db = _find_database(database_id);

    if (!db)
    {
        LOG_ERROR("Invalid database id selected: %d", database_id);
        return Blob_InvalidDatabaseID;
    }

    /* check the key isn't already there. Check only the header */
    int32_t idx = _blob_db_flash_load_blob(db, key, key_size, data);
    
    if (idx < 0)
    {
        LOG_ERROR("Invalid Key: %d database id: %d", key, database_id);
        /* Key Does NOT exist! */
        return Blob_KeyDoesNotExist;
    }

    return Blob_Success;
}

uint8_t blobdb_insert(uint16_t database_id, uint8_t *key, uint16_t key_size, uint8_t *data, uint16_t data_size)
{
    const database *db = _find_database(database_id);

    if (!db)
    {
        LOG_ERROR("Invalid database id selected: %d", database_id);
        return Blob_InvalidDatabaseID;
    }

    /* check the key isn't already there. Check only the header */
    if (_blob_db_flash_load_blob(db, key, key_size, NULL) >= 0)
        /* Key already exists! */
        return Blob_InvalidData;

    /* XXX TODO some more checks */

    /* fs_write_to_database_file_danger_danger(void *danger, int zone) */
    
    /* in this case we are going to append the message to a fake fs. yay? */
    struct file file;
    struct fd fd;
    struct blobdb_hdr hdr;
    
    if (fs_find_file(&file, db->filename) >= 0)
        fs_open(&fd, &file);
    else {
        LOG_ERROR("blobdb database %d (%s) does not exist, so I'm going to go create it, wish me luck", database_id, db->filename);
        if (fs_creat(&fd, db->filename, db->def_db_size) == NULL) {
            LOG_ERROR("nope, that did not work either, I give up");
            return Blob_DatabaseFull;
        }
        fs_mark_written(&fd);
    }

    int valid = _blobdb_seek_first(&fd, NULL);
    while (valid)
        valid = _blobdb_seek_next(&fd, NULL);
    
    int pos = fs_seek(&fd, 0, FS_SEEK_CUR);
    struct fd hfd = fd;
    if (pos + sizeof(struct blobdb_hdr) + key_size + data_size > fd.file.size) {
        /* XXX: try gc'ing the blob */
        LOG_ERROR("not enough space %d %d for new entry", fd.file.size, pos); //+ sizeof(struct blobdb_hdr) + key_size + data_size);
        return Blob_DatabaseFull;
    }

    /* Carefully start by writing a header. */
    hdr.flags = 0xFF;
    hdr.key_len = key_size;
    hdr.data_len = data_size;
    if (fs_write(&fd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to write header");
        return Blob_GeneralFailure;
    }
    
    /* Mark the header as completely written. */
    hdr.flags &= ~BLOBDB_FLAG_HEADER_WRITTEN;
    fd = hfd;
    if (fs_write(&fd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to write header");
        return Blob_GeneralFailure;
    }
    
    /* Write the data. */
    if (fs_write(&fd, key, key_size) < key_size) {
        LOG_ERROR("failed to write key");
        return Blob_GeneralFailure;
    }
    if (fs_write(&fd, data, data_size) < data_size) {
        LOG_ERROR("failed to write data");
        return Blob_GeneralFailure;
    }
    
    /* Mark the data as written. */
    fd = hfd;
    hdr.flags &= ~BLOBDB_FLAG_WRITTEN;
    if (fs_write(&fd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOG_ERROR("failed to update header");
        return Blob_GeneralFailure;
    }

    return Blob_Success;
}



uint8_t *blobdb_delete(uint8_t *key, uint16_t keylen)
{
    /* hah, good one */
    return NULL;
}



/* API Utility */
ResultSetItem blobdb_first_result(ResultSetList list_head)
{
    return list_elem(list_get_head(list_head), blobdb_result_set, node);
}

ResultSetItem blobdb_next_result(ResultSetList list_head, ResultSetItem item)
{
    return list_elem(list_get_next(list_head, &item->node), blobdb_result_set, node);
}

ResultSetItem blobdb_prev_result(ResultSetList list_head, ResultSetItem item)
{
    return list_elem(list_get_prev(list_head, &item->node), blobdb_result_set, node);
}

/* Fakey fake becuase we can't write fs */
void blobdb_add_result_set_item(ResultSetList list_head, Uuid *uuid, uint32_t timestamp)
{
    blobdb_result_set *set = calloc(1, sizeof(blobdb_result_set));
    list_init_node(&set->node);

    set->select1 = calloc(1, sizeof(Uuid));
    set->select2 = calloc(1, sizeof(uint32_t));
    set->select1_size = sizeof(Uuid);
    set->select2_size = sizeof(uint32_t);

    memcpy(set->select1, uuid, sizeof(Uuid));
    memcpy(set->select2, &timestamp, sizeof(uint32_t));

    list_insert_head(list_head, &set->node);
}
