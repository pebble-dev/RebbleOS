/* protocol_notification.c
 * Protocol notification processer
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "systemapp.h"
#include "test.h"
#include "protocol_notification.h"
#include "notification_manager.h"


typedef struct {
    uint8_t unk0;
    uint8_t add_nofif;
    uint32_t flags;
    uint32_t id;
    uint32_t ancs_id;
    uint32_t ts;
    uint8_t layout;
    uint8_t attr_count;
    uint8_t action_count;
    // These follow in the buffer
    //cmd_phone_attribute *attributes;
    //cmd_phone_attribute *actions;
} __attribute__((__packed__)) cmd_phone_notify_t;



typedef struct {
    uint8_t attr_idx;
    uint16_t str_len;
} __attribute__((__packed__)) cmd_phone_attribute_hdr_t;

typedef struct {
    cmd_phone_attribute_hdr_t hdr;
    list_node node;
    uint8_t *data;
} __attribute__((__packed__)) cmd_phone_attribute_t;


typedef struct {
    uint8_t id;
    uint8_t cmd_id;
    uint8_t attr_count;
    uint8_t attr_id;
    uint16_t str_len;
} __attribute__((__packed__)) cmd_phone_action_hdr_t;

typedef struct {
    cmd_phone_action_hdr_t hdr;
    list_node node;
    uint8_t *data;
} __attribute__((__packed__)) cmd_phone_action_t;






static void _copy_and_null_term_string(uint8_t **dest, uint8_t *src, uint16_t len);

/* notification processing */

void protocol_process_legacy2_notification(const pbl_transport_packet *packet)
{
//     full_msg_t *msg;
//     notification_packet_push(data, &msg);
//     notification_show_message(msg, 5000);
}

// void notification_packet_push(uint8_t *data, full_msg_t **message)
// {
//     full_msg_t *new_msg = *message;
//     
//     /* create a real message as we are peeking at the buffer.
//      * lets be quick about this */
//     cmd_phone_notify_t *msg = (cmd_phone_notify_t *)data;
// 
//     SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X attrc %d actc %d", msg->attr_count, msg->action_count);
//     
//     new_msg = noty_calloc(1, sizeof(full_msg_t));
//     assert(new_msg);
//     new_msg->header = noty_calloc(1, sizeof(cmd_phone_notify_t));
//     memcpy(new_msg->header, msg, sizeof(cmd_phone_notify_t));
//     list_init_head(&new_msg->attributes_list_head);
//     list_init_head(&new_msg->actions_list_head);
//     
//     /* get the attributes */
//     uint8_t *p = data + sizeof(cmd_phone_notify_t);
//     for (uint8_t i = 0; i < msg->attr_count; i++)
//     {
//         
//         cmd_phone_attribute_hdr_t *att = (cmd_phone_attribute_hdr_t *)p;
//         uint8_t *data = p + sizeof(cmd_phone_attribute_hdr_t);
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X ATTR ID:%d L:%d", att->attr_idx, att->str_len);
//         cmd_phone_attribute_t *new_attr = noty_calloc(1, sizeof(cmd_phone_attribute_t));
//         /* copy the head to the new attribute */
//         memcpy(new_attr, att, sizeof(cmd_phone_attribute_hdr_t));
//         /* copy the data in now */
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X COPY");
//         /* we'll null terminate strings too as they are pascal strings and seemingly not terminated */
//         _copy_and_null_term_string(&(new_attr->data), data, att->str_len);
//        
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X NODE");
//         list_init_node(&new_attr->node);
//         list_insert_tail(&new_msg->attributes_list_head, &new_attr->node);
//         p += sizeof(cmd_phone_attribute_hdr_t) + att->str_len;
//     }
// 
//     /* get the actions */
//     for (uint8_t i = 0; i < msg->action_count; i++)
//     {
//         cmd_phone_action_hdr_t *act = (cmd_phone_action_hdr_t *)p;
//         uint8_t *data = p + sizeof(cmd_phone_action_hdr_t);
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X ACT ID:%d L:%d AID:%d ALEN:%d", act->id, act->attr_count, act->attr_id, act->str_len);
//         cmd_phone_action_t *new_act = noty_calloc(1, sizeof(cmd_phone_action_t));
//         /* copy the head to the new action */
//         memcpy(new_act, act, sizeof(cmd_phone_action_hdr_t));
//         /* copy the data in now */
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X NEW ACT ID:%d L:%d AID:%d ALEN:%d", new_act->hdr.id, new_act->hdr.attr_count, new_act->hdr.attr_id, new_act->hdr.str_len);
// 
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X COPY");
//         _copy_and_null_term_string(&new_act->data, data, act->str_len);
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X NODE");
//         list_init_node(&new_act->node);
//         list_insert_tail(&new_msg->actions_list_head, &new_act->node);
//         SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X NODE ADDED");
//         p += sizeof(cmd_phone_action_hdr_t) + act->str_len;
//     }
//     SYS_LOG("PHPKT", APP_LOG_LEVEL_INFO, "X Done");
//     *message = new_msg;
// }
// // 
// void _full_msg_free(full_msg_t *message)
// {  
//     cmd_phone_attribute_t *m;
//     list_node *l = list_get_head(&message->attributes_list_head);
//     while(l)
//     {
//         m = list_elem(l, cmd_phone_attribute_t, node);
//         list_remove(&message->attributes_list_head, &m->node);
//         /* free the string */
//         noty_free(m->data);
//         /* free the attribute */
//         noty_free(m);
//     }
//     
//     cmd_phone_action_t *a;
//     l = list_get_head(&message->actions_list_head);
//     while(l)
//     {
//         a = list_elem(l, cmd_phone_action_t, node);
//         list_remove(&message->actions_list_head, &a->node);
//         
//         /* free the string */
//         noty_free(a->data);
//         /* free the attribute */
//         noty_free(a);
//     }
//     
//     noty_free(message->header);
//     message->header = NULL;
//     noty_free(message);
//     message = NULL;
// }


/* we have pesky pascal strings. Turn them into null term strings */
static void _copy_and_null_term_string(uint8_t **dest, uint8_t *src, uint16_t len)
{
    if (src[len - 1] != '\0')
    {
        *dest = app_calloc(1, len + 1);
        assert(*dest && "Malloc Failed!");

        for(uint16_t i = 0; i < len; i++)
            *(*dest + i) = src[i];

        *(*dest + len) = '\0';
    }
    else
    {
        *dest = app_calloc(1, len);
        memcpy((uint8_t *)*dest, src, len);
    }
}
