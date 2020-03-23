#pragma once

#include "fs.h"

enum {
    Blob_Insert = 0x01,
    Blob_Delete = 0x04,
    Blob_Clear  = 0x05,
};

typedef enum blobdb_operator {
    Blob_None = 0,
    Blob_Gtr,
    Blob_Less,
    Blob_Eq,
    Blob_NEq,
    Blob_Result = 0x80,
    Blob_Result_FullyLoad
} blobdb_operator_t;

enum {
    Blob_Success = 0x01,
    Blob_GeneralFailure = 0x02,
    Blob_InvalidOperation = 0x03,
    Blob_InvalidDatabaseID = 0x04,
    Blob_InvalidData = 0x05,
    Blob_KeyDoesNotExist = 0x06,
    Blob_DatabaseFull = 0x07,
    Blob_DataStale = 0x08,
    Blob_NotSupported = 0x09,
    Blob_Locked = 0xA,
    Blob_TryLater = 0xB,
};

enum {
    BlobDatabaseID_Test = 0,
    BlobDatabaseID_Pin = 1,
    BlobDatabaseID_App = 2,
    BlobDatabaseID_Reminder = 3,
    BlobDatabaseID_Notification = 4,
    BlobDatabaseID_AppGlance = 11,
};

struct blobdb_database;

struct blobdb_iter {
    struct fd fd;
    uint8_t key_len;
    uint16_t data_len;
};

#define BLOBDB_SELECTOR_OFFSET_KEY 0xFFFF

struct blobdb_selector {
    uint16_t offsetof;
    uint16_t size;
    blobdb_operator_t operator;
    void *val;
};

struct blobdb_select_result {
    list_node node;
    struct blobdb_iter it; /* pointer to data */
    void *key;
    int nres;
    void *result[];
};

typedef list_head blobdb_select_result_list;

const struct blobdb_database *blobdb_open(uint16_t database_id);
uint8_t blobdb_insert(const struct blobdb_database *db, uint8_t *key, uint16_t key_size, uint8_t *data, uint16_t data_size);

/* Each returns true if the iterator points to a valid header. */
int blobdb_iter_start(const struct blobdb_database *db, struct blobdb_iter *it);
int blobdb_iter_next(struct blobdb_iter *it);

int blobdb_iter_read_key(struct blobdb_iter *it, void *key);
int blobdb_iter_read_data(struct blobdb_iter *it, int ofs, void *data, int len);

int blobdb_select(struct blobdb_iter *it, blobdb_select_result_list *head, struct blobdb_selector *selectors);
void blobdb_select_free_result(struct blobdb_select_result *res);
void blobdb_select_free_all(blobdb_select_result_list *head);

#define blobdb_select_result_foreach(res, lh) list_foreach(res, lh, struct blobdb_select_result, node)
#define blobdb_select_result_head(lh) list_elem(list_get_head(lh), struct blobdb_select_result, node)
#define blobdb_select_result_next(res, lh) list_elem(list_get_next(lh, &(res)->node), struct blobdb_select_result, node)
#define blobdb_select_result_prev(res, lh) list_elem(list_get_prev(lh, &(res)->node), struct blobdb_select_result, node)
