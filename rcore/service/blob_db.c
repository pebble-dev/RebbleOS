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
#include "blob_db_ramfs.h"


/* Configure Logging */
#define MODULE_NAME "blobdb"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR



typedef struct database_t {
    uint8_t id;
    const char *filename;
    uint16_t hdr_size;
    uint16_t key_offset;
    uint8_t key_size;
    uint16_t data_size_offset;
    uint16_t data_size_bytes;
} database;

static const database databases[] = {
    {
        .id = BlobDatabaseID_Test,
        .filename = NULL,
    },
    {
        .id = BlobDatabaseID_Notification,
        .filename = "notifstr",
        .hdr_size = sizeof(timeline_item), 
        .key_offset = offsetof(timeline_item, uuid),
        .key_size = sizeof(Uuid),
        .data_size_offset = offsetof(timeline_item, data_size),
        .data_size_bytes = FIELD_SIZEOF(timeline_item, data_size)
    },
#define DB_COUNT 2
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

static int32_t _blob_db_find_item_entry(struct fd *fd, const database *db, size_t file_size, uint8_t *key, uint8_t key_size)
{
    int idx = 0;
    char buf[UUID_STRING_BUFFER_LENGTH];
    uint8_t _tmp_key[key_size];
    size_t _data_size = 0;

    while(idx < file_size)
    {
        fs_seek(fd, idx + db->key_offset, FS_SEEK_SET);
        fs_read(fd, _tmp_key, key_size);
        fs_seek(fd, idx + db->data_size_offset, FS_SEEK_SET);
        fs_read(fd, &_data_size, db->data_size_bytes);

        /* End of file */
        if (uuid_is_int((Uuid  *)_tmp_key, 0xFF))
            break;

        if (uuid_equal((Uuid  *)key, (Uuid  *)_tmp_key))
        {
            uuid_to_string((Uuid  *)_tmp_key, buf);
            LOG_INFO("Found: %s %d", buf, idx);
            return idx;
        }

        uuid_to_string((Uuid  *)_tmp_key, buf);
        LOG_DEBUG("NOTIF: %s %d", buf, idx);

        idx += db->hdr_size + _data_size;
        fs_seek(fd, idx, FS_SEEK_SET);
    }

    LOG_DEBUG("DB SIZE: %d", idx);
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

list_head *blobdb_select_items_all(uint8_t database_id, 
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size)
{
    return blobdb_select_items2(database_id, 
                                 select1_offsetof_property, select1_property_size,
                                 select2_offsetof_property, select2_property_size,
                                 0, 0,
                                 NULL, 0,
                                 0, 0,
                                 NULL, 0);
}

list_head *blobdb_select_items1(uint8_t database_id, 
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator)
{
    return blobdb_select_items2(database_id, 
                                 select1_offsetof_property, select1_property_size,
                                 select2_offsetof_property, select2_property_size,
                                 where_offsetof_property, where_property_size,
                                 where_val, operator,
                                 0, 0,
                                 NULL, 0);
}

list_head *blobdb_select_items2(uint8_t database_id, 
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator,
                            uint16_t where_offsetof_property1, uint8_t where_property_size1, 
                            uint8_t *where_val1, Blob_Operator operator1)
{
    struct fd fd;
    struct file file;
    fs_open(&fd, &file);
    list_head *head = app_calloc(1, sizeof(list_head));
    list_init_head(head);
    const database *db = _find_database(database_id);
    if (fs_find_file(&file, db->filename) >= 0)
    {
        _blobdb_select_items2(head, &fd, database_id, 
                            select1_offsetof_property, select1_property_size, 
                            select2_offsetof_property, select2_property_size, 
                            where_offsetof_property, where_property_size, 
                            where_val, operator,
                            where_offsetof_property1, where_property_size1, 
                            where_val1, operator1);
    }
    else
    {
        LOG_ERROR("Not in flash. Checking RAMFS");
//         return head;
    }

    ramfs_open(&fd, &file, database_id);
    _blobdb_select_items2(head, &fd, database_id, 
                            select1_offsetof_property, select1_property_size, 
                            select2_offsetof_property, select2_property_size, 
                            where_offsetof_property, where_property_size, 
                            where_val, operator,
                            where_offsetof_property1, where_property_size1, 
                            where_val1, operator1);

    return head;

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
    char buf[UUID_STRING_BUFFER_LENGTH];
    const database *db = _find_database(database_id);
    uint8_t _tmp_key[db->key_size];

    while(idx < fd->file.size)
    {
        fs_seek(fd, idx + db->key_offset, FS_SEEK_SET);
        fs_read(fd, &_tmp_key, db->key_size);

        uint8_t done = 0;

        /* End of file */
        if (uuid_is_int((Uuid *)_tmp_key, 0xFF))
            break;

        uuid_to_string((Uuid  *)_tmp_key, buf);
        LOG_DEBUG("KEY: %s %d %d", buf, idx, operator);

        bool comp1 = true;
        bool comp2 = true;

        if (where_property_size && where_offsetof_property)
        {
            uint8_t where_prop[where_property_size];
            fs_seek(fd, idx + where_offsetof_property, FS_SEEK_SET);
            fs_read(fd, where_prop, where_property_size);

            comp1 = _compare(operator, where_prop, where_val, where_property_size);
        }
        if (where_property_size1 && where_offsetof_property1)
        {
            uint8_t where_prop1[where_property_size1];
            fs_seek(fd, idx + where_offsetof_property1, FS_SEEK_SET);
            fs_read(fd, where_prop1, where_property_size1);

            comp2 = _compare(operator1, where_prop1, where_val1, where_property_size1);
        }

        if (comp1 && comp2)
        {
            /* we have a match */
            blobdb_result_set *set = app_calloc(1, sizeof(blobdb_result_set));
            list_init_node(&set->node);

            set->select1 = app_calloc(1, select1_property_size);
            fs_seek(fd, idx + select1_offsetof_property, FS_SEEK_SET);
            fs_read(fd, set->select1, select1_property_size);

            if (select2_property_size)
            {
                set->select2 = app_calloc(1, select2_property_size);
                fs_seek(fd, idx + select2_offsetof_property, FS_SEEK_SET);
                fs_read(fd, set->select2, select2_property_size);
            }

            list_insert_tail(head, &set->node);
        }

        size_t data_size = 0;
        fs_seek(fd, idx + db->data_size_offset, FS_SEEK_SET);
        fs_read(fd, &data_size, db->data_size_bytes);

        idx += db->hdr_size + data_size;
        fs_seek(fd, idx, FS_SEEK_SET);
    }

//     return head;
}

void blobdb_resultset_destroy(list_head *lh)
{
    list_node *n = list_get_head(lh);
    while(n)
    {
        list_remove(lh, n);

        blobdb_result_set *set = list_elem(n, blobdb_result_set, node);
        app_free(set->select1);
        app_free(set->select2);
        app_free(set);

        n = list_get_head(lh);
    }
    app_free(lh);
}

int32_t _blob_db_flash_load_blob(const database *db, Uuid *uuid, uint8_t **data)
{
    struct file file;
    struct fd fd;

    if (fs_find_file(&file, db->filename) < 0)
    {
        LOG_ERROR("file not found");
//         return -1;
        /* try ramfs */
        ramfs_open(&fd, &file, db->id);
    }
    else
    {
        fs_open(&fd, &file);
    }

    int32_t idx = _blob_db_find_item_entry(&fd, db, file.size, (uint8_t *)uuid, sizeof(Uuid));
    if (idx < 0)
        return -1;

    if (!data)
        return idx;

    fs_seek(&fd, idx, FS_SEEK_SET);
    *data = app_calloc(1, db->hdr_size);
    if (fs_read(&fd, *data, db->hdr_size) != db->hdr_size)
        return -1;

    uint32_t data_size = 0;
    fs_seek(&fd, idx + db->hdr_size, FS_SEEK_SET);
    memcpy(&data_size, *data + db->data_size_offset, db->data_size_bytes);
    LOG_DEBUG("Data Size %d", (uint16_t)data_size);
    *data = app_realloc(*data, db->hdr_size + data_size);

    if (fs_read(&fd, *data + db->hdr_size, data_size) != data_size)
        return -1;

    return idx;
}


uint8_t blobdb_select(uint16_t database_id, uint8_t *key, uint8_t **data)
{
    const database *db = _find_database(database_id);

    char buf[UUID_STRING_BUFFER_LENGTH];
    uuid_to_string((Uuid *)key, buf);
    LOG_DEBUG("SELECT: %s", buf);

    if (!db)
    {
        LOG_ERROR("Invalid database id selected: %d", database_id);
        return Blob_InvalidDatabaseID;
    }

    /* check the key isn't already there. Check only the header */
    int32_t idx = _blob_db_flash_load_blob(db, (Uuid *)key, data);
    
    if (idx < 0)
    {
        /* Key Does NOT exist! */
        return Blob_KeyDoesNotExist;
    }

    return Blob_Success;
}

uint8_t blobdb_insert(uint16_t database_id, uint8_t *key, uint16_t key_size, uint8_t *data, uint16_t data_size)
{
    LOG_INFO("KEY: %d", key[0]);
    Uuid uuid = UuidMakeFromBEBytes(key);

    const database *db = _find_database(database_id);

    if (!db)
    {
        LOG_ERROR("Invalid database id selected: %d", database_id);
        return Blob_InvalidDatabaseID;
    }

    /* check the key isn't already there. Check only the header */
    if (_blob_db_flash_load_blob(db, &uuid, NULL) >= 0)
        /* Key already exists! */
        return Blob_InvalidData;

    /* XXX TODO some more checks */

    /* fs_write_to_database_file_danger_danger(void *danger, int zone) */
    
    /* in this case we are going to append the message to a fake fs. yay? */
    struct file file;
    struct fd fd;

    ramfs_open(&fd, &file, database_id);
    ramfs_write(&fd, data, data_size);

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
    blobdb_result_set *set = app_calloc(1, sizeof(blobdb_result_set));
    list_init_node(&set->node);

    set->select1 = app_calloc(1, sizeof(Uuid));
    set->select2 = app_calloc(1, sizeof(uint32_t));
    set->select1_size = sizeof(Uuid);
    set->select2_size = sizeof(uint32_t);

    memcpy(set->select1, uuid, sizeof(Uuid));
    memcpy(set->select2, &timestamp, sizeof(uint32_t));

    list_insert_head(list_head, &set->node);
}
