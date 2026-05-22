// list_demo.c  -- stack and queue macros built on uC_list_t
// -----------------------------------------------------------------------

#include <stdio.h>

#include "uC_list.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

int main(void)
{
    uC_list_t list = { .zone = uC_MEM_ZONE_USER1 };
    const char *p;

    // stack: LIFO - items come back in reverse push order
    uC_stack_push(&list, "first");
    uC_stack_push(&list, "second");
    uC_stack_push(&list, "third");

    printf("stack (LIFO):\n");
    while (list.count)
    {
        p = uC_stack_pop(&list);
        printf("  %s\n", p);
    }

    // queue: FIFO - items come back in put order
    uC_queue_put(&list, "first");
    uC_queue_put(&list, "second");
    uC_queue_put(&list, "third");

    printf("queue (FIFO):\n");
    while (list.count)
    {
        p = uC_queue_get(&list);
        printf("  %s\n", p);
    }

    uC_mem_purge(uC_MEM_ZONE_USER1);

    return 0;
}

// =======================================================================
