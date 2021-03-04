// list.c   - simple linked lists
// -----------------------------------------------------------------------

#include <stdlib.h>

#include "h/list.h"

// -----------------------------------------------------------------------
// insert node n2 into list after node n1

static void node_insert(node_t *n1, node_t *n2)
{
    node_t *t;
    list_t *l;

    l = (list_t *) n1->parent;
    n2->parent = l;

    t = (node_t *)(n1->next);

    n2->next = t;
    if(NULL == t)
    {
        l->tail = n2;
    }
    else
    {
        t->prev = n2;
    }
    n1->next = n2;
    n2->prev = n1;
}

// -----------------------------------------------------------------------

static void node_remove(node_t *n1)
{
    node_t *t1;
    node_t *t2;
    list_t *l;

    l = n1->parent;

    t1 = (node_t *)(n1->prev);
    t2 = (node_t *)(n1->next);

    if(l->head == n1) { l->head = t2; }
    if(l->tail == n1) { l->tail = t1; }

    if(NULL != t1) { t1->next = t2; }
    if(NULL != t2) { t2->prev = t1; }

    n1->next = n1->prev = NULL;
    n1->parent = NULL;

    l->count--;
}

// -----------------------------------------------------------------------
// remove node from list that contains specified payload

void list_remove_node(list_t *l1, void *payload)
{
    node_t *n1;

    n1 = (node_t *)l1->head;

    while(NULL != n1)
    {
        if(n1->payload == payload)
        {
            node_remove(n1);
            free(n1);
            return;
        }
    }
    // no node with specified payload was found.  do what here?
    // silently ignore? log it? abort mission?
}

// -----------------------------------------------------------------------

bool list_append_node(list_t *l, void *payload)
{
    node_t *n1;

    n1 = (node_t *) malloc(sizeof(node_t));
    if(NULL == n1)
    {
        return false;
    }

    n1->payload = payload;

    if(NULL == l->head)
    {
        l->head = l->tail = n1;
        n1->next = n1->prev = NULL;
        n1->parent = l;
    }
    else
    {
        node_insert((node_t *)(l->tail), n1);
        l->tail = n1;
    }

    l->count++;

    return true;
}

// -----------------------------------------------------------------------
// remove tail item from list returning its payload

void *list_pop(list_t *list)
{
    void *result = NULL;
    node_t *n;
    if(0 != list->count)
    {
        n = list->tail;
        list->tail = n->prev;
        list->count--;
        result = n->payload;
    }
    return result;
}

// =======================================================================
