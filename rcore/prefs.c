/* prefs.c
 * System preferences persistence API
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#define MODULE_NAME "prefs"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

#include "rdb.h"
#include "minilib.h"
#include "debug.h"
#include "prefs.h"

int prefs_get(const uint32_t key, void *buf, uint32_t bufsz) {
    struct rdb_iter it;
    rdb_select_result_list head;
    list_init_head(&head);
    struct rdb_database *db = rdb_open(RDB_ID_PREFS);
    assert(db);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        rdb_close(db);
        return -1;
    }
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, sizeof(key) , RDB_OP_EQ, &key },
        { 0, 0, RDB_OP_RESULT_FULLY_LOAD },
        { }
    };
    int n = rdb_select(&it, &head, selectors);

    if (!n) {
        rv = -1;
        goto done;
    }
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    size_t sz = bufsz <= res->it.data_len ? bufsz : res->it.data_len;
    memcpy(buf, res->result[0], sz);
    rv = sz;
    
done:
    rdb_close(db);
    rdb_select_free_all(&head);
    return rv;
}

int prefs_put(const uint32_t key, void *buf, uint32_t bufsz) {
    struct rdb_database *db = rdb_open(RDB_ID_PREFS);
    assert(db);
    
    if (rdb_create(db) != Blob_Success)
        return -1;
    
    /* Try inserting it first? */
    if (rdb_insert(db, (uint8_t *)&key, sizeof(key), buf, bufsz) == Blob_Success) {
        rdb_close(db);
        return 0; /* Ok, success. */
    }
    
    /* Maybe it already exists and we should do an update? */
    if (rdb_update(db, (uint8_t *)&key, sizeof(key), buf, bufsz) == Blob_Success) {
        rdb_close(db);
        return 0; /* Ok, success. */
    }
    
    /* Guess this one just isn't gonna work out today. */
    rdb_close(db);
    return -1;
}