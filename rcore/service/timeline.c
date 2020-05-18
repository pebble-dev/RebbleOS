/* timeline.c
 * emulation and api for timeline functionality.
 * Controls notifications
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "protocol_service.h"
#include "pebble_protocol.h"
#include "timeline.h"
#include "rdb.h"
#include "notification_manager.h"
#include "event_service.h"

/* Configure Logging */
#define MODULE_NAME "timeln"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

static uint16_t _process_attribute(list_head *list, void *raw_data)
{
    uint8_t *data;

    rebble_attribute *r_attr = app_calloc(1, sizeof(rebble_attribute));
    memcpy(&r_attr->timeline_attribute, raw_data, sizeof(timeline_attribute));
    uint16_t len = sizeof(timeline_attribute) + r_attr->timeline_attribute.length;

    timeline_attribute *nattr = (timeline_attribute *)raw_data;
    data = app_calloc(1, r_attr->timeline_attribute.length + 1);
    memcpy(data, nattr->data, r_attr->timeline_attribute.length);

    r_attr->data = data;
    list_init_node(&r_attr->node);

    uint8_t id = r_attr->timeline_attribute.attribute_id;
    switch (id) {
        case TimelineAttributeType_Sender:
        case TimelineAttributeType_Subject:
        case TimelineAttributeType_Message:
            if (!r_attr->timeline_attribute.length)
            {
                break;
            }
            data[r_attr->timeline_attribute.length] = 0;

            break;
        case TimelineAttributeType_SourceType:
            break;
    }
    list_insert_tail(list, &r_attr->node);
    return len;
}

void printattr(rebble_attribute *attr, const char *indent)
{
    printf("%s{\n", indent);
    printf("%s  Id: %d,\n", indent, attr->timeline_attribute.attribute_id);
    printf("%s  Length: %d,\n", indent, attr->timeline_attribute.length);
    if (attr->timeline_attribute.length) {
        if (attr->timeline_attribute.attribute_id == TimelineAttributeType_SourceType)
            printf("%s  Type: %d,\n", indent, (int)*(uint32_t *)attr->data);
        else
            printf("%s  Data: %s,\n", indent, attr->data ? (char *)attr->data : "n/a");
    }
    printf("%s},\n", indent);
}

void printblob(rebble_notification *notification)
{
    char buf[UUID_STRING_BUFFER_LENGTH];
    timeline_item *ti = &notification->timeline_item;
    printf("  Notification: {\n");
    uuid_to_string(&ti->uuid, buf);
    printf("    Uuid: %s,\n", buf);
    uuid_to_string(&ti->parent_uuid, buf);
    printf("    Parent: %s,\n", buf);
    printf("    TimeStamp: %d,\n", (int)ti->timestamp);
    printf("    Duration: %d,\n", ti->duration);
    printf("    Type: %d,\n", ti->timeline_type);
    printf("    Flags: %x,\n", ti->flags);
    printf("    Layout: %d,\n", ti->layout);
    printf("    Size: %d,\n", ti->data_size);
    printf("    Attrs: %d,\n", ti->attr_count);
    printf("    Actions: %d,\n", ti->action_count);
    printf("    Attributes: [\n");
    rebble_attribute *m;
    list_foreach(m, &notification->attributes, rebble_attribute, node)
    {
        printattr(m, "      ");
    }
    printf("    ],\n");
    printf("    Actions: [\n");
    rebble_action *a;
    list_foreach(a, &notification->actions, rebble_action, node)
    {
        printf("      {\n");
        printf("        Id: %d,\n", a->timeline_action.action_id);
        printf("        Type: %d,\n", a->timeline_action.type);
        printf("        Attrs: %d,\n", a->timeline_action.attr_count);
        printf("        Attributes: [\n");
        list_foreach(m, &a->attributes, rebble_attribute, node)
        {
            printattr(m, "          ");
        }
        printf("        ]\n");
        printf("      }\n");
    }
    printf("    ]\n  }\n");
}

rebble_notification *timeline_item_process(void *data)
{
    size_t len;
    rebble_notification *notification = app_calloc(1, sizeof(rebble_notification ));
    assert(notification);

    list_init_head(&notification->attributes);
    list_init_head(&notification->actions);

    timeline_item *ti = (timeline_item *)(data);
    memcpy(&notification->timeline_item, ti, sizeof(timeline_item));

    uint16_t ptr = 0;
    for (uint16_t i = 0; i < ti->attr_count; i++)
    {
        uint16_t len = _process_attribute(&notification->attributes, ti->data + ptr);
        ptr += len;
    }

    for (uint16_t i = 0; i < ti->action_count; i++)
    {
        timeline_action *act = (timeline_action *)(ti->data + ptr);
        rebble_action *r_action = app_calloc(1, sizeof(rebble_action));
        assert(r_action);
        memcpy(&r_action->timeline_action, act, sizeof(timeline_action));
        list_init_head(&r_action->attributes);
        list_insert_tail(&notification->actions, &r_action->node);

        ptr += sizeof(timeline_action);

        for (uint16_t j = 0; j < act->attr_count; j++)
        {
            uint16_t len = _process_attribute(&r_action->attributes, ti->data + ptr);
            ptr += len;
        }
    }

    if (ti->timeline_type == TimelineItemType_Notification)
    {
        // send notification on?
    }

    return notification;
}

void timeline_notification_arrived(Uuid *uuid)
{
    Uuid *uuidc = calloc(1, sizeof(Uuid));
    memcpy(uuidc, uuid, UUID_SIZE);
    event_service_post(EventServiceCommandNotification, (void *)uuidc, remote_free);
}

void timeline_destroy(rebble_notification *notification)
{
    timeline_item *timeline = &notification->timeline_item;
    list_node *n = list_get_head(&notification->attributes);
    while(n)
    {
        list_remove(&notification->attributes, n);

        rebble_attribute *ra = list_elem(n, rebble_attribute, node);
        if (ra->timeline_attribute.length > 0)
            app_free(ra->data);
        app_free(ra);

        n = list_get_head(&notification->attributes);
    }

    n = list_get_head(&notification->actions);
    while(n)
    {
        rebble_action *ract = list_elem(n, rebble_action, node);
        list_node *m = list_get_head(&ract->attributes);
        while(m)
        {
            rebble_attribute *ratt = list_elem(m, rebble_attribute, node);
            list_remove(&ract->attributes, m);
            app_free(ratt->data);
            app_free(ratt);
            m = list_get_head(&ract->attributes);
        }
        list_remove(&notification->actions, n);
        app_free(ract);
        n = list_get_head(&notification->actions);
    }

    app_free(notification);
}


rdb_select_result_list *timeline_notifications(uint32_t from_timestamp)
{
    uint32_t val_type = TimelineItemType_Notification;
    rdb_select_result_list *head = app_calloc(1, sizeof(rdb_select_result_list));
    list_init_head(head);
    
    struct rdb_database *db = rdb_open(RDB_ID_NOTIFICATION);
    struct rdb_iter it;
    
    if (rdb_iter_start(db, &it) == 0)
        return head;
    
    struct rdb_selector selectors[] = {
        { offsetof(timeline_item, timestamp), FIELD_SIZEOF(timeline_item, timestamp), RDB_OP_GREATER, &from_timestamp },
        { offsetof(timeline_item, timeline_type), FIELD_SIZEOF(timeline_item, timeline_type), RDB_OP_EQ, &val_type },
        { offsetof(timeline_item, uuid), FIELD_SIZEOF(timeline_item, uuid), RDB_OP_RESULT },
        { offsetof(timeline_item, timestamp), FIELD_SIZEOF(timeline_item, timestamp), RDB_OP_RESULT },
        { }
    };
    
    rdb_select(&it, head, selectors);
    
    rdb_close(db);

    return head;
}

rebble_notification *timeline_get_notification(Uuid *uuid)
{
    rdb_select_result_list head;
    list_init_head(&head);
    
    struct rdb_database *db = rdb_open(RDB_ID_NOTIFICATION);
    struct rdb_iter it;
    
    if (rdb_iter_start(db, &it) == 0)
        assert(!"rdb open on notif db failed");

    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, sizeof(Uuid), RDB_OP_EQ, uuid },
        { 0, 0, RDB_OP_RESULT_FULLY_LOAD },
        { }
    };
    
    if (rdb_select(&it, &head, selectors) != 1)
        assert(!"get_notification on nonexistant notif uuid");
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    rebble_notification *notif = timeline_item_process(res->result[0]);
    printblob(notif);
    rdb_select_free_all(&head);
    
    rdb_close(db);

    return notif;
}

typedef struct timeline_action_request_t {
    uint8_t command;
    Uuid uuid;
    uint8_t action;
    uint8_t attr_count;
    uint8_t attributes[];
} __attribute__((__packed__)) timeline_action_request;


typedef struct rebble_action_response_t {
    timeline_action_request response_item;
    uint8_t *data;
    list_head attributes;
} rebble_action_response;

enum {
    TimelineActionRequest_WithPayload,
    TimelineActionRequest_Open,
    TimelineActionRequest_Dismiss,
    TimelineActionRequest_DismissAll,
    TimelineActionRequest_Mute,
};

enum {
    TimelineActionCommand_Invoke = 2
};

enum {
    TimelineActionAttribute_ReplyText = 1,
    TimelineActionAttribute_ReplyTo = 12,
};

static uint16_t _timeline_attribute_create_to_buf(uint8_t attribute_id, uint8_t *data, uint8_t data_len, uint8_t *buf)
{   
    /* header */
    buf[0] = attribute_id;
    buf[1] = data_len;
    /* attr data */
    memcpy(buf + 2, data, data_len);
    
    return data_len + 2;
}


void timeline_action_send(uint8_t timeline_action, Uuid *uuid, const char *reply_text, const char *reply_to)
{
    int r_len = strlen((char *)reply_text);
    int t_len = strlen((char *)reply_to);
    int attr_count = 0;

    if (reply_text) 
    {
        attr_count++;
        r_len++;
    }
    if (reply_to)
    {
        attr_count++;
        t_len++;
    }
    
    int total_len = sizeof(timeline_action_request) + 
                            (attr_count * (sizeof(timeline_attribute)) + 
                            r_len + t_len);
    RebblePacket packet = packet_create(WatchProtocol_TimelineAction, total_len);
    timeline_action_request *action = (timeline_action_request *)packet_get_data(packet);
    
    action->command = TimelineActionCommand_Invoke;
    memcpy(&action->uuid, uuid, UUID_SIZE);
    action->action = timeline_action;
    action->attr_count = attr_count;

    uint32_t pkt_len = 0;
    
    if (reply_text)
        pkt_len += _timeline_attribute_create_to_buf(TimelineActionAttribute_ReplyText, 
                                              (uint8_t *)reply_text, r_len - 1, action->attributes);

    if (reply_to)
        pkt_len += _timeline_attribute_create_to_buf(TimelineActionAttribute_ReplyTo, 
                                              (uint8_t *)reply_to, t_len - 1, action->attributes + pkt_len);

    packet_send(packet);
}

enum {
    TimelineActionResponseCommand_Ack,
    TimelineActionResponseCommand_Nack,
};

enum {
    TimelineActionResponseCommand_Response = 0x11
};

/* We get an action back with some attributes */
void protocol_process_timeline_action_response(const RebblePacket packet)
{
    //notify_andwhatnot(packet->data);
}

void timeline_action_response_process(const RebblePacket packet)
{
//     rebble_action_response *response = app_calloc(1, sizeof(rebble_action_response));
//     assert(response);
// 
//     list_init_head(&response->attributes);
// 
//     timeline_action_request *ti = (timeline_action_request *)(data);
//     memcpy(&response->response_item, ti, sizeof(timeline_action_request));
//     
//     timeline_action_request *resp = &response->response_item;
//     rebble_packet *rp = rebble_protocol_get_awaiting_by_uuid(&response->response_item.uuid);    
//     switch(resp->command)
//     {
//         case TimelineActionResponseCommand_Response:
//             switch(resp->action)
//             {
//                 case TimelineActionResponseCommand_Nack:
//                     rebble_protocol_resend_packet(rp);
//                     break;
//                 case TimelineActionResponseCommand_Ack:
//                     rebble_protocol_remove_packet(rp);
//                     /* parse the attributes out */
//                     /* attr with id subject, and id with icon */                    
//                     uint16_t ptr = 0;
//                     for (uint16_t i = 0; i < resp->attr_count; i++)
//                     {
//                         uint16_t len = _process_attribute(&response->attributes, ti->attributes + ptr);
//                         ptr += len;
//                     }
//             }
//             break;
//     }
}

void timeline_action_response_destroy()
{
    // XXX
}
