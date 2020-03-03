/* blob_db_test.c
 * tests for blob database
 * RebbleOS
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"
#include "node_list.h"
#include "timeline.h"
#include "blob_db.h"
#include "test.h"
#include "debug.h"

#define MODULE_NAME "blobdb"
#define MODULE_TYPE "TEST"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

#include <string.h>

#ifdef REBBLEOS_TESTING

TEST(blobdb_basic) {
    uint8_t *d;
    int rv;
    uint8_t uuid1[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8_t uuid2[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
    
    LOG_INFO("blobdb_select(uuid1) should fail");
    rv = blobdb_select(BlobDatabaseID_Test, uuid1, &d);
    if (rv != Blob_KeyDoesNotExist) {
        LOG_ERROR("blobdb_select(uuid1) init state returned data");
        return TEST_FAIL;
    }
    
    uint8_t hdr[24] = {0xAA, 0x55};
    uint8_t data[4] = {1, 2, 3, 4};
    memcpy(hdr + 4, uuid1, 16);
    hdr[4 + 16] = 4;
    LOG_INFO("blobdb_insert(uuid1) should succeed");
    rv = blobdb_insert(BlobDatabaseID_Test, hdr, sizeof(hdr), data, sizeof(data));
    if (rv != Blob_Success) {
        LOG_ERROR("blobdb_insert(uuid1) failed");
        *artifact = rv;
        return TEST_FAIL;
    }
    
    LOG_INFO("blobdb_insert(uuid1) should fail");
    rv = blobdb_insert(BlobDatabaseID_Test, hdr, sizeof(hdr), data, sizeof(data));
    if (rv == Blob_Success) {
        LOG_ERROR("blobdb_insert(uuid1) double insert succeeded");
        *artifact = rv;
        return TEST_FAIL;
    }
    
    
    *artifact = 0;
    return TEST_PASS;
}

#endif
