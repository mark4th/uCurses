// list.c   - simple linked lists
// -----------------------------------------------------------------------

#include <stdlib.h>

#include "uCurses.h"
#include "uC_list.h"

// -----------------------------------------------------------------------
// insert node n2 into list after node n1

void node_insert(node_t *n1, node_t *n2)
{
    node_t *t;
    list_t *l;

    l = n1->parent;         // make n2's parent the same as n1's
    n2->parent = l;

    t = (node_t *)n1->next; // going to insert after node n1

    n2->next = t;

    (t == NULL)             // if n2 was tail then
        ? (l->tail = n2)    // n1 is now the tail
        : (t->prev = n2);   // else link back from t to n2

    n1->next = n2;          // link n1 ->> n2
    n2->prev = n1;          // link n1 <<- n2

    l->count++;
}

// -----------------------------------------------------------------------
// add new node to head of list

static void node_add(list_t *l, node_t *n1)
{
    n1->parent = l;
    node_t *t;

    if (l->head == NULL)
    {
        l->head = l->tail = n1;
    }
    else
    {
        t        = l->head;
        t->prev  = n1;
        n1->next = t;
        l->head  = n1;
    }
    l->count++;
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

    if (l->head == n1) { l->head = t2;  }
    if (l->tail == n1) { l->tail = t1;  }
    if (t1 != NULL)    { t1->next = t2; }
    if (t2 != NULL)    { t2->prev = t1; }

    n1->next   = n1->prev = NULL;
    n1->parent = NULL;

    l->count--;
}

// -----------------------------------------------------------------------
// remove node from list that contains specified payload

void list_remove_node(list_t *l1, void *payload)
{
    node_t *n1;

    n1 = (node_t *)l1->head;

    while (n1 != NULL)
    {
        if (n1->payload == payload)
        {
            node_remove(n1);
            free(n1);
            return;
        }
        n1 = n1->next;
    }
    // no node with specified payload was found.  do what here?
    // silently ignore? log it? abort mission?
}

// -----------------------------------------------------------------------
// append a new node onto end of list

int16_t list_append_node(list_t *l, void *payload)
{
    node_t *n1, *t;

    n1 = calloc(1, sizeof(node_t));
    if (n1 == NULL) { return -1; }

    n1->payload = payload;

    if (l->head == NULL)
    {
        l->head    = l->tail  = n1;
        n1->next   = n1->prev = NULL;
        n1->parent = l;
    }
    else
    {
        n1->parent = l;
        t          = l->tail;
        t->next    = n1;
        n1->prev   = t;
        l->tail    = n1;
        n1->next   = NULL;
    }
    l->count++;

    return 0;
}

// -----------------------------------------------------------------------
// add a new node to start of list

int16_t list_add_node(list_t *l, void *payload)
{
    node_t *n1;

    n1 = calloc(1, sizeof(node_t));

    if (n1 == NULL) { return -1; }

    n1->payload = payload;

    node_add(l, n1);
    return 0;
}

// -----------------------------------------------------------------------
// remove tail item from list returning its payload

void *list_pop(list_t *list)
{
    void *result = NULL;
    node_t *n1;

    if (list->count != 0)
    {
        n1 = list->tail;
        result = n1->payload;
        node_remove(n1);
        free(n1);
    }

    return result;
}

// -----------------------------------------------------------------------

void *list_scan(list_t *l)
{
    static node_t *node;

    void *x = NULL;         // the unknown factor

    if (l != NULL)          // initiating scan?
    {
        node = l->head;
    }

    if (node != NULL)
    {
        x = node->payload;
        node = node->next;
    }
    return x;
}

// =======================================================================
