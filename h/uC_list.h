// list.h   -- uCurses linked lists
// -----------------------------------------------------------------------

#ifndef LIST_H
#define LIST_H

// -----------------------------------------------------------------------

#include <stdint.h>

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
// visibility hidden

void node_insert(node_t *n1, node_t *n2);
void list_remove_node(list_t *l1, void *payload);
int16_t list_append_node(list_t *l, void *payload);
int16_t list_add_node(list_t *l, void *payload);
void *list_pop(list_t *list);
void *list_scan(list_t *l);

// -----------------------------------------------------------------------

#endif // LIST_H

// =======================================================================
