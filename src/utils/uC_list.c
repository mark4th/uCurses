// uC_list.c   - simple linked lists
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_list.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------
// walk through every node in the list, return next node on each call

// caller must extract payload from node
// first call must pass list to be scanned
// subsequent calls must pass null for the list

API uC_list_node_t *uC_list_scan(uC_list_t *list, uC_list_node_t *n1)
{
    return (list != NULL)
        ? list->head
        : (n1 != NULL)
            ? n1->next
            : NULL;
}

// -----------------------------------------------------------------------
// insert node2 into list after node1

API bool uC_list_insert_node(uC_list_node_t *node1, void *payload)
{
    uC_list_node_t *tmp, *node2;
    uC_list_t *list;
    uC_mem_zone_t zone;

    bool result = false;

    if (node1 != NULL)
    {
        list = node1->list;
        zone = list->zone;

        if (list == NULL)
        {
            return result;
        }

        node2 = uC_alloc(zone, sizeof(*node2));

        if (node2 != NULL)
        {
            node2->payload = payload;
            node2->list    = list;
            tmp            = node1->next;
            node2->next    = tmp;

            (tmp == NULL)
                ? (list->tail = node2)
                : (tmp->prev  = node2);

            node1->next = node2;
            node2->prev = node1;

            list->count++;

            result = true;
        }
    }

    return result;
}

// -----------------------------------------------------------------------

static void node_remove(uC_list_node_t *node1)
{
    uC_list_node_t *tmp1, *tmp2;
    uC_list_t *list;

    if (node1 == NULL)
    {
        return;
    }

    list = node1->list;

    if (list == NULL)
    {
        return;
    }

    tmp1 = node1->prev;
    tmp2 = node1->next;

    if (node1 == list->head)
    {
        list->head = tmp2;
    }
    if (node1 == list->tail)
    {
        list->tail = tmp1;
    }

    if (tmp1 != NULL)
    {
        tmp1->next = tmp2;
    }
    if (tmp2 != NULL)
    {
        tmp2->prev = tmp1;
    }

    node1->next = node1->prev = NULL;
    node1->list = NULL;

    list->count--;
}

// -----------------------------------------------------------------------

API void uC_list_remove_node(uC_list_t *list, void *payload)
{
    uC_list_node_t *node1;

    if (list == NULL)
    {
        return;
    }

    node1 = uC_list_scan(list, NULL);

    while (node1 != NULL)
    {
        if (node1->payload == payload)
        {
            node_remove(node1);
            uC_free(list->zone, node1);
            break;
        }
        node1 = uC_list_scan(NULL, node1);
    }
}

// -----------------------------------------------------------------------

API bool uC_list_push_head(uC_list_t *list, void *payload)
{
    uC_mem_zone_t zone;

    uC_list_node_t *node1, *tmp;

    if (list == NULL)
    {
        return false;
    }

    zone  = list->zone;

    node1 = uC_alloc(zone, sizeof(*node1));

    if (node1 == NULL)
    {
        return false;
    }

    node1->list    = list;
    node1->payload = payload;

    if (list->head == NULL)
    {
        list->head = list->tail = node1;
    }
    else
    {
        tmp         = list->head;
        tmp->prev   = node1;
        node1->next = tmp;
        list->head  = node1;
    }

    list->count++;

    return true;
}

// -----------------------------------------------------------------------

API bool uC_list_push_tail(uC_list_t *list, void* payload)
{
    uC_mem_zone_t zone;
    uC_list_node_t *node1, *tmp;

    if (list == NULL)
    {
        return false;
    }

    zone  = list->zone;
    node1 = uC_alloc(zone, sizeof(*node1));

    if (node1 == NULL)
    {
        return false;
    }

    node1->list    = list;
    node1->payload = payload;

    if (list->head == NULL)
    {
        list->head = list->tail = node1;
    }
    else
    {
        tmp         = list->tail;
        tmp->next   = node1;
        node1->prev = tmp;
        list->tail  = node1;
    }

    list->count++;

    return true;
}

// -----------------------------------------------------------------------

static void *list_pop(uC_list_t *list, bool whence)
{
    void *payload = NULL;
    uC_list_node_t *node1;

    if((list != NULL) && (list->count != 0))
    {
        node1   = (whence) ? list->head : list->tail;
        payload = node1->payload;
        node_remove(node1);
        uC_free(list->zone, node1);
    }
    return payload;
}

// -----------------------------------------------------------------------

API void *uC_list_pop_head(uC_list_t *list)
{
    return list_pop(list, true);
}

// -----------------------------------------------------------------------

API void *uC_list_pop_tail(uC_list_t *list)
{
    return list_pop(list, false);
}

// -----------------------------------------------------------------------
// todo: insert_after() ??

API bool uC_list_insert_before(uC_list_node_t *n1, void *payload)
{
    uC_mem_zone_t zone;
    uC_list_node_t *n2;
    uC_list_node_t *prev;
    uC_list_t *list;

    if (n1 == NULL)
    {
        return false;
    }

    list = n1->list;
    zone = list->zone;

    if (n1 == list->head)
    {
        return uC_list_push_head(list, payload);
    }

    n2 = uC_alloc(zone, sizeof(*n2));
    if (n2 == NULL)
    {
        return false;
    }

    n2->list    = list;
    n2->payload = payload;
    prev = n1->prev;

    n2->prev    = prev;
    prev->next  = n2;
    n2->next    = n1;
    n1->prev    = n2;

    list->count++;

    return true;
}

// =======================================================================
