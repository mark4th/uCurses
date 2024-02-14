// uC_list.h   -- uCurses linked lists
// -----------------------------------------------------------------------

#ifndef UC_LIST_H
#define UC_LIST_H

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

// -----------------------------------------------------------------------

typedef struct
{
    void *head;
    void *tail;
    uint32_t count;
} uC_list_t;

// -----------------------------------------------------------------------

typedef struct
{
    void *next;
    void *prev;
    uC_list_t *list;
    void *payload;
} uC_list_node_t;

// -----------------------------------------------------------------------

API bool uC_list_insert_node(uC_list_node_t *node1, void *payload);
API void uC_list_remove_node(uC_list_t *list, void *payload);
API bool uC_list_push_head(uC_list_t *list, void *payload);
API bool uC_list_push_tail(uC_list_t *list, void* payload);
API void *uC_list_pop_head(uC_list_t *list);
API void *uC_list_pop_tail(uC_list_t *list);
API void *uC_list_scan(uC_list_t *list);

// -----------------------------------------------------------------------

#endif // UC_LIST_H

// =======================================================================
