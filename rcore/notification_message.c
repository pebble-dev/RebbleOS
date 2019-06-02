/* notification_message.c
 * A really simple message storage. We give messages its own heap
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "notification_message.h"

/*
 * A little message handler
 * In the absence of a writable flash, we will store 
 * messages in here. 
 * XXX when we have write
 * 
 */
#define MSG_HEAP_SIZE 10000

static uint8_t _notification_messages_heap[MSG_HEAP_SIZE];
static qarena_t *_notification_arena;
static list_head _messages_head = LIST_HEAD(_messages_head);

static full_msg_t *_fake_message(char *text, char *action);


void messages_init(void)
{
    _notification_arena = qinit(_notification_messages_heap, MSG_HEAP_SIZE);

    /* create three samples */
//     message_add(_fake_message("RebbleOS is here!", "To Moon"));
//     message_add(_fake_message("Join the rebblution", "Dismiss"));
//     message_add(_fake_message("Missed Call: Bob", "Dismiss"));
}

static full_msg_t *_fake_message(char *text, char *action)
{
    full_msg_t *m = noty_calloc(1, sizeof(full_msg_t));
    cmd_phone_notify_t *n = noty_calloc(1, sizeof(cmd_phone_notify_t));
    m->header = n;
    m->header->attr_count = 1;
    m->header->action_count = 1;
    
    cmd_phone_attribute_t *new_attr = noty_calloc(1, sizeof(cmd_phone_attribute_t));
    cmd_phone_action_t *new_act = noty_calloc(1, sizeof(cmd_phone_action_t));
    new_attr->data = (uint8_t *)text;
    new_act->data = (uint8_t *)action;
    list_init_head(&m->attributes_list_head);
    list_init_head(&m->actions_list_head);
    list_init_node(&new_act->node);
    list_init_node(&new_attr->node);
    list_insert_tail(&m->attributes_list_head, &new_attr->node);
    list_insert_tail(&m->actions_list_head, &new_act->node);
    
    return m;
}

void message_add(full_msg_t *msg)
{
    /* we need to get the message into our list
     * it should already be allocated on our heap */
    list_init_node(&msg->node);
    list_insert_head(&_messages_head, &msg->node);
}

list_head *message_get_head(void)
{
    return &_messages_head;
}

uint16_t message_count(void)
{
    uint16_t count = 0;
    if (list_get_head(&_messages_head) == NULL)
        return 0;
    
    full_msg_t *w;
    list_foreach(w, &_messages_head, full_msg_t, node)
        count++;

    return count;
}

void *noty_calloc(size_t count, size_t size)
{
    /* uses a special qarena */
    void *x = qalloc(_notification_arena, count * size);
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}

void noty_free(void *mem)
{
    qfree(_notification_arena, mem);
}

