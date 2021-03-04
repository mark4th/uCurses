// list.h   -- uCurses linked lists
// -----------------------------------------------------------------------

    #pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>
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
    void *payload;
    list_t *parent;
} node_t;

// -----------------------------------------------------------------------

void list_remove_node(list_t *l1, void *payload);
bool list_append_node(list_t *l, void *payload);
void *list_pop(list_t *list);

// =======================================================================
