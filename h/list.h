// list.h   -- uCurses linked lists
// -----------------------------------------------------------------------

#pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>

// -----------------------------------------------------------------------

typedef struct
{
    void *head;
    void *tail;
    uint16_t count;
} list_t;

// -----------------------------------------------------------------------

typedef struct
{
    void *next;
    void *prev;
    void *payload;
    list_t *parent;
} node_t;

// -----------------------------------------------------------------------

void node_insert(node_t *n1, node_t *n2);
void list_remove_node(list_t *l1, void *payload);
int16_t list_append_node(list_t *l, void *payload);
int16_t list_add_node(list_t *l, void *payload);
void *list_pop(list_t *list);
void *list_scan(list_t *l);

// =======================================================================
