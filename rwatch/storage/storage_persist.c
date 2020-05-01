/* storage_persist.c
 * Persist data storage api
 * libRebbleOS
 *
 * Authors: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "rdb.h"
#include "storage_persist.h"
#include "appmanager_thread.h"

#define MODULE_NAME "stor"
#define MODULE_TYPE "RWAT"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

#define PERSIST_MAX_STORAGE_BYTES 4096

typedef struct app_persist_key_t {
    Uuid appid;
    uint32_t key;
} app_persist_key;

static void _create_composite_key(app_persist_key *c_key, uint32_t key)
{
    struct App *app = appmanager_get_current_app();
    assert(app);
    c_key->key = key;
    memcpy(c_key, &app->uuid, UUID_SIZE);
}

bool persist_exists(const uint32_t key)
{
    char buffer[1];
    return persist_read(key, buffer, 1) == 1;
}

int persist_get_size(const uint32_t key)
{
    struct rdb_iter it;
    rdb_select_result_list head;
    list_init_head(&head);
    
    struct rdb_database *db = rdb_open(RDB_ID_APP_PERSIST);
    assert(db);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        rdb_close(db);
        return E_ERROR;
    }
    
    app_persist_key c_key;
    _create_composite_key(&c_key, key);
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, sizeof(app_persist_key) , RDB_OP_EQ, &c_key },
        { }
    };
    
    int n = rdb_select(&it, &head, selectors);
    if (!n)
        return  E_DOES_NOT_EXIST;
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    
    if (!rdb_delete(&res->it)) {
        rdb_close(db);
        return E_ERROR;
    }
    
    rdb_close(db);
    rdb_select_free_all(&head);
    
    LOG_DEBUG("persist: size(%d)", res->it.data_len);
    return res->it.data_len;
}

bool persist_read_bool(const uint32_t key)
{
    bool b = false;
    if (!persist_read(key, &b, sizeof(bool)))
        return E_INVALID_ARGUMENT;

    LOG_DEBUG("persist: r bool(%s)", b ? "true" : "false");
    return b;
}

int32_t persist_read_int(const uint32_t key)
{
    int32_t i = 0;
    if (persist_read(key, &i, sizeof(int32_t)) != sizeof(int32_t))
        return E_INVALID_ARGUMENT;
    
    LOG_DEBUG("persist: r int(%d)", i);
    return i;
}

int persist_read_data(const uint32_t key, void * buffer, const size_t buffer_size) 
{
    return persist_read(key, buffer, buffer_size);
}

int persist_read_string(const uint32_t key, char * buffer, const size_t buffer_size)
{
    return persist_read(key, buffer, buffer_size);
}

status_t persist_write_bool(const uint32_t key, const bool value) 
{
    LOG_DEBUG("persist: w bool(%d)", value ? "true" : "false");
    return persist_write(key, &value, sizeof(bool));
}     
        
int persist_write_data(const uint32_t key, const void * data, const size_t size)
{
    /* return E_RANGE if longer? */
    return persist_write(key, data, size < PERSIST_DATA_MAX_LENGTH ? size : PERSIST_DATA_MAX_LENGTH);
}
         
int persist_write_string(const uint32_t key, const char * cstring)
{
    LOG_DEBUG("persist: w str(%s)", cstring);
    return persist_write(key, cstring, strnlen(cstring, PERSIST_STRING_MAX_LENGTH));
}
          
status_t persist_delete(const uint32_t key)
{
    struct rdb_iter it;
    rdb_select_result_list head;
    list_init_head(&head);
    struct rdb_database *db = rdb_open(RDB_ID_APP_PERSIST);
    assert(db);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        rdb_close(db);
        return E_ERROR;
    }
    
    app_persist_key c_key;
    _create_composite_key(&c_key, key);
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, sizeof(app_persist_key) , RDB_OP_EQ, &c_key },
        { }
    };
    
    int n = rdb_select(&it, &head, selectors);
    if (!n)
        return  E_DOES_NOT_EXIST;
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    
    if (!rdb_delete(&res->it)) {
        rdb_close(db);
        return E_ERROR;
    }
    
    rdb_close(db);
    return S_SUCCESS;
}

status_t persist_write_int(const uint32_t key, const int32_t value)
{
    LOG_DEBUG("persist: w int(%d)", value);
    return persist_write(key, &value, sizeof(int32_t));
}

status_t persist_write(const uint32_t key, const void *data, const size_t size)
{
    rdb_select_result_list head;
    list_init_head(&head);
    
    if (size > PERSIST_DATA_MAX_LENGTH)
        return E_INVALID_ARGUMENT;
    
    bool exists = persist_exists(key);

    struct rdb_database *db = rdb_open(RDB_ID_APP_PERSIST);
    assert(db);
    
    if (rdb_create(db) != Blob_Success)
        return E_OUT_OF_STORAGE;  
    
    app_persist_key c_key;
    _create_composite_key(&c_key, key);
    
    if (!exists) {
        if (rdb_insert(db, (uint8_t *)&c_key, sizeof(app_persist_key), data, size) != Blob_Success) {
            rdb_close(db);
            return E_ERROR;
        }
    } else {    
        if (rdb_update(db, &c_key, sizeof(c_key), data, size) != Blob_Success) {
            rdb_close(db);
            return E_ERROR;
        }
    }
    
    rdb_close(db);
    return size;
}

status_t persist_read(const uint32_t key, const void *buffer, const size_t size)
{
    struct rdb_iter it;
    rdb_select_result_list head;
    list_init_head(&head);
    struct rdb_database *db = rdb_open(RDB_ID_APP_PERSIST);
    assert(db);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        rdb_close(db);
        return E_ERROR;
    }
    
    app_persist_key c_key;
    _create_composite_key(&c_key, key);
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, sizeof(app_persist_key) , RDB_OP_EQ, &c_key },
        { 0, 0, RDB_OP_RESULT_FULLY_LOAD },
        { }
    };
    int n = rdb_select(&it, &head, selectors);

    if (!n) {
        rv =  E_DOES_NOT_EXIST;
        goto done;
    }
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    size_t sz = size <= res->it.data_len ? size : res->it.data_len;
    memcpy(buffer, res->result[0], sz);
    rv =  sz;
    
done:
    rdb_close(db);
    rdb_select_free_all(&head);
    return rv;
}
