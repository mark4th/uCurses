// uC_list.h   -- uCurses linked lists
// -----------------------------------------------------------------------

#ifndef UC_LIST_H
#define UC_LIST_H

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "uC_alloc.h"

// -----------------------------------------------------------------------

typedef struct
{
    void *head;             // head and tail both point to uC_list_node_t
    void *tail;
    uint32_t count;         // number of items in the list
    uC_mem_zone_t zone;     // all nodes allocated within this zone
} __attribute__((__packed__)) uC_list_t;

// -----------------------------------------------------------------------

typedef struct
{
    void *prev;             // prev and next are botth uC_list_node_t
    void *next;
    uC_list_t *list;        // parent list of this node
    void *payload;          // data associated with this node
} __attribute__((__packed__)) uC_list_node_t;

// -----------------------------------------------------------------------

API bool uC_list_insert_node(uC_list_node_t *node1, void *payload);
API void uC_list_remove_node(uC_list_t *list, void *payload);
API bool uC_list_push_head(uC_list_t *list, void *payload);
API bool uC_list_push_tail(uC_list_t *list, void* payload);
API void *uC_list_pop_head(uC_list_t *list);
API void *uC_list_pop_tail(uC_list_t *list);
API bool uC_list_insert_before(uC_list_node_t *n1, void *payload);

API uC_list_node_t *uC_list_scan(uC_list_t *list, uC_list_node_t *n1);

// -----------------------------------------------------------------------

#endif // UC_LIST_H

// =======================================================================
