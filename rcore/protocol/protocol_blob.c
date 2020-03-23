/* protocol_blob.c
 * Emulation of the Blob protocol
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"
#include "protocol_service.h"
#include "blobdb.h"
#include "timeline.h"

typedef struct blob_db_t {
    uint8_t command;
    uint16_t token;
    uint8_t database_id;
} __attribute__((__packed__)) pcol_blob_db;

typedef struct pcol_blob_db_key_t {
    struct blob_db_t blobdb;
    uint8_t key_size;
    uint8_t key[];
} __attribute__((__packed__)) pcol_blob_db_key;

typedef struct {
    struct pcol_blob_db_key_t blobdb;
    uint16_t value_size;
    uint8_t value[];
} __attribute__((__packed__)) pcol_blob_db_insert;

typedef struct {
    pcol_blob_db_key blobdb;
} __attribute__((__packed__)) pcol_blob_db_delete;

typedef struct {
    uint16_t token;
    uint8_t response;
} __attribute__((__packed__)) pcol_blob_db_response;

void printblock(void *data, size_t len)
{
    for(int i = 0; i < len; i++)
        printf("%02x", *(((uint8_t *)data) + i));
}

uint8_t blob_process(pcol_blob_db_key *blob, void *data, uint16_t data_size)
{
    switch(blob->blobdb.database_id) {
        case BlobDatabaseID_AppGlance:
        case BlobDatabaseID_Pin:
        case BlobDatabaseID_Reminder:
        case BlobDatabaseID_Test:
            break;
        case BlobDatabaseID_App:
            appmanager_app_loader_init_n();
            break;
        case BlobDatabaseID_Notification:
            timeline_notification_arrived((Uuid *)blob->key);
            break;
    }

    return Blob_Success;
}

uint8_t blob_insert(pcol_blob_db_key *blob)
{
    pcol_blob_db_insert *iblob = (pcol_blob_db_insert *)blob;
    void *val_sz_start = (void *)blob + sizeof(pcol_blob_db_key) + blob->key_size;
    uint8_t val_sz = *((uint8_t *)val_sz_start);

    printf("  ValueSize: %d, %d\n", val_sz, blob->key_size);
    void *val_start = val_sz_start + sizeof(iblob->value_size);

    /* Pre-process */
    if (blob->blobdb.database_id == BlobDatabaseID_App)
    {
        /* Apps keys are the app id. Get one and monkey with the key */
        uint32_t newappid = appmanager_get_next_appid();
        memcpy(blob->key, &newappid, 4);
        blob->key_size = 4;
    }
        
    /* insert to database */
    const struct blobdb_database *db = blobdb_open(blob->blobdb.database_id);
    if (!db)
        return Blob_InvalidDatabaseID;
    
    int rv = blobdb_insert(db, blob->key, blob->key_size, val_start, val_sz);
    if (rv != Blob_Success)
        return rv;

    return blob_process((pcol_blob_db_key *)&blob->blobdb, val_start, val_sz);
}

void blob_delete(pcol_blob_db_delete *blob)
{
}

void protocol_process_blobdb(const RebblePacket packet)
{
    uint8_t *data = packet_get_data(packet);
    pcol_blob_db_key *blob = (pcol_blob_db_key *)data;
    printf("Blob:  %d{\n", blob->blobdb.command);
    printf("  Token: %d,\n", blob->blobdb.token);
    printf("  DbId: %d,\n", blob->blobdb.database_id);
    printf("  KeySize: %d,\n", blob->key_size);
    printf("  Key: ");
    printblock(&data[offsetof(pcol_blob_db_key, key)], blob->key_size);
    printf(",\n  Command: ");

    uint8_t ret = 0;
    switch(blob->blobdb.command) {
        case Blob_Insert:
            printf("  INSERT,\n");
            ret = blob_insert(blob);
            break;
        case Blob_Delete:
            break;
        default:
            break;
    }
    printf("}\n");

    pcol_blob_db_response response;
    response.token = blob->blobdb.token;
    response.response = ret;
    SYS_LOG("pblob", APP_LOG_LEVEL_INFO, "Done: Send Response: token %d, %d\n", response.token, response.response);

    /* Reply back with the cookie */
    rebble_protocol_send(packet_get_endpoint(packet), (void *)&response, sizeof(pcol_blob_db_response));
    packet_destroy(packet);
}
