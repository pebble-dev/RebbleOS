/* rdb_test.c
 * tests for rdb database
 * RebbleOS
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"
#include "node_list.h"
#include "timeline.h"
#include "rdb.h"
#include "test.h"
#include "debug.h"

#define MODULE_NAME "rdb"
#define MODULE_TYPE "TEST"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

#include <string.h>

#ifdef REBBLEOS_TESTING

static int _insert(int key, int dsize) {
    struct rdb_database *db = rdb_open(RDB_ID_TEST);
    
    uint8_t *val = app_calloc(1, dsize);
    for (int i = 0; i < dsize; i++)
        val[i] = (key & 0xFF) ^ i;
    
    int rv = rdb_insert(db, (void *)&key, 4, val, dsize);
    
    app_free(val);
    
    rdb_close(db);
    
    return rv != Blob_Success;
}

static int _retrieve(int key, int dsize) {
    struct rdb_database *db = rdb_open(RDB_ID_TEST);
    struct rdb_iter it;
    rdb_select_result_list head;
    int ret = 0;
    
    list_init_head(&head);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        ret = 1;
        goto fail;
    }
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, 4, RDB_OP_EQ, &key },
        { 0, 0, RDB_OP_RESULT_FULLY_LOAD },
        { }
    };
    int n = rdb_select(&it, &head, selectors);
    if (n != 1) {
        ret = 2;
        goto fail;
    }
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    for (int i = 0; i < dsize; i++) {
        uint8_t exp = (key & 0xFF) ^ i;
        uint8_t rd = ((uint8_t *)res->result[0])[i];
        if (exp != rd) {
            LOG_ERROR("rdb_select incorrect readback key %d size %d ofs %d, should be %02x is %02x", key, dsize, i, exp, rd);
            ret = 3;
            goto fail;
        }
    }
    
    res = 0;
    
fail:
    rdb_select_free_all(&head);
    rdb_close(db);

    return ret;
}

static int _delete(int key) {
    struct rdb_database *db = rdb_open(RDB_ID_TEST);
    struct rdb_iter it;
    rdb_select_result_list head;
    
    list_init_head(&head);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv)
        return 1;
    
    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, 4, RDB_OP_EQ, &key },
        { }
    };
    int n = rdb_select(&it, &head, selectors);
    if (n != 1)
        return 2;
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    rdb_delete(&res->it);
    
    rdb_close(db);
    
    return 0;
}

TEST(rdb_basic) {
    LOG_INFO("rdb_select(1) should fail");
    if (_retrieve(1, 16) == 0) {
        LOG_ERROR("rdb_select(1) init state returned data");
        return TEST_FAIL;
    }
    
    if (_insert(1, 16) != 0) {
        LOG_ERROR("rdb_insert(1) failed");
        return TEST_FAIL;
    }
    
    if (_insert(1, 16) == 0) {
        LOG_ERROR("double rdb_insert(1) succeeded");
        return TEST_FAIL;
    }

    if (_insert(2, 16) != 0) {
        LOG_ERROR("rdb_insert(2) failed");
        return TEST_FAIL;
    }

    if (_retrieve(1, 16) != 0) {
        LOG_ERROR("rdb_retrieve(1) failed");
        return TEST_FAIL;
    }

    if (_delete(1) != 0)  {
        LOG_ERROR("rdb_delete(1) failed");
        return TEST_FAIL;
    }
    
    if (_retrieve(1, 16) == 0) {
        LOG_ERROR("rdb_retrieve(1) succeeded after delete");
        return TEST_FAIL;
    }

    if (_retrieve(2, 16) != 0) {
        LOG_ERROR("rdb_retrieve(2) failed");
        return TEST_FAIL;
    }
    
    *artifact = 0;
    return TEST_PASS;
}

TEST(rdb_fill) {
    int i;
    
    for (i = 0; i < 1024; i++) {
        if (_insert(i, 128) != 0) {
            LOG_INFO("rdb_insert(%d) failed", i);
            break;
        }
    }
    
    int nents = i;
    for (i = 0; i < nents; i++)  {
        if (_retrieve(i, 128) != 0) {
            LOG_ERROR("failure on retrieve(%d)", i);
            break;
        }
    }
    
    LOG_INFO("verified %d records", nents);
    
    if (_delete(0) != 0) {
        LOG_ERROR("rdb_delete(0) failed");
        return TEST_FAIL;
    }
    
    if (_insert(nents, 128) != 0) {
        LOG_ERROR("rdb_insert(nents) failed");
        return TEST_FAIL;
    }

    LOG_INFO("inserted to replace");
    
    for (i = 1; i <= nents; i++)  {
        if (_retrieve(i, 128) != 0) {
            LOG_ERROR("failure on retrieve(%d)", i);
            break;
        }
    }

    LOG_INFO("verified %d records", nents);

    *artifact = 0;
    return TEST_PASS;
}

#endif
