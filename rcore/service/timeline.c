/* timeline.c
 * emulation and api for timeline functionality.
 * Controls notifications
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"
#include "timeline.h"
#include "blob_db.h"
#include "notification_manager.h"

static uint16_t _process_attribute(list_head *list, void *raw_data)
{
    uint8_t *data;

    rebble_attribute *r_attr = app_calloc(1, sizeof(rebble_attribute));
    memcpy(&r_attr->timeline_attribute, raw_data, sizeof(timeline_attribute));
    uint16_t len = sizeof(timeline_attribute) + r_attr->timeline_attribute.length;

    if (r_attr->timeline_attribute.length > 0)
    {
        timeline_attribute *nattr = (timeline_attribute *)raw_data;
        data = app_calloc(1, r_attr->timeline_attribute.length + 1);
        memcpy(data, nattr->data, r_attr->timeline_attribute.length);
    }

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
//     notification_arrived(uuid);
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


list_head *timeline_notifications(uint32_t from_timestamp)
{
    uint32_t val_type = TimelineItemType_Notification;
    list_head *lh = blobdb_select_items2(BlobDatabaseID_Notification, 
                        offsetof(timeline_item, uuid), FIELD_SIZEOF(timeline_item, uuid), 
                        offsetof(timeline_item, timestamp), FIELD_SIZEOF(timeline_item, timestamp), 
                        /* where */
                        offsetof(timeline_item, timestamp), FIELD_SIZEOF(timeline_item, timestamp),
                        (uint8_t *)&from_timestamp, Blob_Gtr,
                        offsetof(timeline_item, timeline_type), FIELD_SIZEOF(timeline_item, timeline_type),
                        (uint8_t *)&val_type, Blob_Eq);
    return lh;
}


rebble_notification *timeline_get_notification(Uuid *uuid)
{
    uint8_t *data;
    if (blobdb_select(BlobDatabaseID_Notification, (uint8_t *)uuid, &data) != Blob_Success)
        assert(0);
    rebble_notification *notif = timeline_item_process(data);
    printblob(notif);
    app_free(data);

    return notif;
}
