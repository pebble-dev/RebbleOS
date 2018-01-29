#pragma once

typedef struct node {
    void *data;
    struct node *next;
} node_t;


void node_add(node_t **head, void *data);
void node_remove(node_t **head, void *data);
node_t *node_find(node_t **head, void *data);
