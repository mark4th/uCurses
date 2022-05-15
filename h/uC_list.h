// list.h   -- uCurses linked lists
// -----------------------------------------------------------------------

#ifndef LIST_H
#define LIST_H

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

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
    list_t *list;
    void *payload;
} node_t;

// -----------------------------------------------------------------------

API bool list_insert_node(node_t *node1, void *payload);
API void list_remove_node(list_t *list, void *payload);
API bool list_push_head(list_t *list, void *payload);
API bool list_push_tail(list_t *list, void* payload);
API void *list_pop_head(list_t *list);
API void *list_pop_tail(list_t *list);
API void *list_scan(list_t *list);

// -----------------------------------------------------------------------

#endif // LIST_H

// =======================================================================
