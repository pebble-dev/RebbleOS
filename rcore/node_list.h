#pragma once
/* appmanager.c
 * Routines for a double linked node list
 * (For example usage see window.c)
 * RebbleOS
 * 
 * Author: Michael Sullivan <sully@msully.net>.
 */
#include <stddef.h>

/**
 * @brief A list traversal field that can be embedded in objects.
 */
typedef struct list_node {
    struct list_node *next;
    struct list_node *prev;
} list_node;

/**
 * @brief The head of a list
 */
typedef struct list_head {
    struct list_node node;
} list_head;

#define FIELD_SIZEOF(type, field) (sizeof(((type*)0)->field))

#define LIST_HEAD(name) { { .next = &name.node, .prev = &name.node } }
#define LIST_NODE() { .next = NULL, .prev = NULL }

#define container_of(ptr, type, member) ({                           \
            const __typeof__( ((type *)0)->member ) *__mptr = (ptr); \
            (type *)( (char *)__mptr - offsetof(type,member) );})

/**
 * @brief Recover a pointer to a 'type' given a ptr to a list_node
 * that is stored as a 'member' in 'type's. Returns NULL if ptr is NULL.
 */
#define list_elem(ptr, type, member) \
    ((ptr) ? container_of(ptr, type, member) : NULL)

#define list_foreach(pos, h, type, member) \
    for (pos = container_of((h)->node.next, type, member); \
         &pos->member != &(h)->node; \
         pos = container_of(pos->member.next, type, member))

////////////////////
/**
 * @brief List and node initialization routines
 */
static inline
void list_init_head(list_head *head) {
    head->node.next = head->node.prev = &head->node;
}

static inline
void list_init_node(list_node *node) {
    node->next = node->prev = NULL;
}

/**
 * @brief List accessors. Return NULL on end of list.
 */
static inline
list_node *list_get_next(list_head *head, list_node *node) {
    return node->next == &head->node ? NULL : node->next;
}

static inline
list_node *list_get_prev(list_head *head, list_node *node) {
    return node->prev == &head->node ? NULL : node->prev;
}

static inline
list_node *list_get_head(list_head *head) {
    return list_get_next(head, &head->node);
}

static inline
list_node *list_get_tail(list_head *head) {
    return list_get_prev(head, &head->node);
}


/**
 * @brief List insertion.
 */
static inline
void list_insert_between(list_head *head, list_node *n,
                         list_node *n1, list_node *n2) {
    (void)head;
    n->prev = n1;
    n2->prev = n;
    n->next = n2;
    n1->next = n;
}


static inline
void list_insert_before(list_head *head,
                        list_node *inq, list_node *node) {
    list_insert_between(head, node, inq->prev, inq);
}

static inline
void list_insert_after(list_head *head,
                       list_node *inq, list_node *node) {
    list_insert_between(head, node, inq, inq->next);
}

static inline
void list_insert_head(list_head *head, list_node *node) {
    list_insert_after(head, &head->node, node);
}

static inline
void list_insert_tail(list_head *head, list_node *node) {
    list_insert_before(head, &head->node, node);
}

/**
 * @brief Remove a list node.
 */
static inline
void list_remove(list_head *head, list_node *node) {
    (void)head;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    // To help catch bugs.
    node->next = node->prev = NULL;
}

