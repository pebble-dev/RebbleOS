#pragma once

#include "rdb.h"

enum TimelineItemType {
    TimelineItemType_Notification = 1,
    TimelineItemType_Pin = 2,
    TimelineItemType_Reminder = 3
};


typedef struct timeline_item_t {
    Uuid uuid;
    Uuid parent_uuid;
    uint32_t timestamp;
    uint16_t duration;
    uint8_t timeline_type;
    uint16_t flags;
    uint8_t layout;
    uint16_t data_size;
    uint8_t attr_count;
    uint8_t action_count;
    uint8_t data[];
} __attribute__((__packed__)) timeline_item;

typedef struct timeline_attribute_t {
    uint8_t attribute_id;
    uint16_t length;
    uint8_t data[];
} __attribute__((__packed__)) timeline_attribute;

enum {
    TimelineAttributeType_Sender = 1,
    TimelineAttributeType_Subject = 2,
    TimelineAttributeType_Message = 3,
    TimelineAttributeType_SourceType = 4,
    TimelineAttributeType_Icon = 6,
};

enum {
    TimelineAction_AncsDismiss = 1,
    TimelineAction_Generic = 2,
    TimelineAction_Response = 3,
    TimelineAction_Dismiss = 4,
    TimelineAction_HTTP = 5,
    TimelineAction_Snooze = 6,
    TimelineAction_OpenWatchapp = 7,
    TimelineAction_Empty = 8,
    TimelineAction_Remove = 9,
    TimelineAction_OpenPin = 10,
};

enum {
    TimelineNotificationSource_Twitter = 6,
    TimelineNotificationSource_Facebook = 11,
    TimelineNotificationSource_Email = 19,
    TimelineNotificationSource_SMS = 45,
};

typedef struct timeline_action_t {
    uint8_t action_id;
    uint8_t type;
    uint8_t attr_count;
    uint8_t data[];
} __attribute__((__packed__)) timeline_action;


typedef struct rebble_notification_t {
    timeline_item timeline_item;
    list_head attributes;
    list_head actions;
} rebble_notification;

typedef struct rebble_attribute_t {
    timeline_attribute timeline_attribute;
    uint8_t *data;
    list_node node;
} rebble_attribute;

typedef struct rebble_action_t {
    timeline_action timeline_action;
    uint8_t *data;
    list_head attributes;
    list_node node;
} rebble_action;


rebble_notification *timeline_item_process(void *data);
rdb_select_result_list *timeline_notifications(uint32_t from_timestamp);
rebble_notification *timeline_get_notification(Uuid *uuid);
void timeline_notification_arrived(Uuid *uuid);
void timeline_destroy(rebble_notification *notification);
rebble_notification *timeline_item_process(void *data);
