#pragma once


enum {
    Blob_Insert = 0x01,
    Blob_Delete = 0x04,
    Blob_Clear  = 0x05,
};

typedef enum Blob_Operator {
    Blob_Gtr = 0x01,
    Blob_Less = 0x02,
    Blob_Eq  = 0x03,
    Blob_NEq  = 0x04,
} Blob_Operator;

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


const char *blobdb_filename_for_id(uint8_t id);
uint8_t blobdb_insert(uint16_t database_id, uint8_t *key, uint16_t key_size, uint8_t *data, uint16_t data_size);
uint8_t blobdb_select(uint16_t database_id, uint8_t *key, uint8_t key_size, uint8_t **data);
uint16_t blobdb_select_items_all(uint8_t database_id, list_head *head,
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size);

uint16_t blobdb_select_items1(uint8_t database_id, list_head *head,
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator);

uint16_t blobdb_select_items2(uint8_t database_id,  list_head *head,
                            uint16_t select1_offsetof_property, uint16_t select1_property_size, 
                            uint16_t select2_offsetof_property, uint16_t select2_property_size, 
                            uint16_t where_offsetof_property, uint8_t where_property_size, 
                            uint8_t *where_val, Blob_Operator operator,
                            uint16_t where_offsetof_property1, uint8_t where_property_size1, 
                            uint8_t *where_val1, Blob_Operator operator1);

typedef struct blobdb_result_set_t {
    uint8_t *key;
    uint16_t key_size;
    uint8_t *select1;
    uint16_t select1_size;
    uint8_t *select2;
    uint16_t select2_size;
    list_node node;
} blobdb_result_set;

typedef blobdb_result_set* ResultSetItem;

#define blobresult_foreach(rs, lh) list_foreach(rs, lh, blobdb_result_set, node)
typedef list_head* ResultSetList;


ResultSetItem blobdb_first_result(ResultSetList list_head);
ResultSetItem blobdb_next_result(ResultSetList list_head, ResultSetItem item);
ResultSetItem blobdb_prev_result(ResultSetList list_head, ResultSetItem item);
void blobdb_add_result_set_item(ResultSetList list_head, Uuid *uuid, uint32_t timestamp);
void blobdb_resultset_destroy(list_head *lh);
