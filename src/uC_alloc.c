// uC_alloc.c  - memory allocations tracking
// -----------------------------------------------------------------------

#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>

#include "uCurses.h"
#include "uC_utils.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

#define INIT_MEM_SIZE (512)      // array of 512 pointers = 4k

// -----------------------------------------------------------------------

typedef struct
{
    size_t alloc_count;
    size_t alloc_max;

    // this is actually a pointer to an array of void * pointers
    // but C is too LAME to allow operations on void pointers.  I'm
    // sure there is a reason for this but as Chuck Moore says
    //
    // "Data types are a crutch for POOR PROGRAMMERS"

    uint64_t *alloc_array;
} uC_mem_array_t;

// -----------------------------------------------------------------------
// memory zones are a way to compartmentalize various memory allocations
// so you can free up all allocations associated with that specific set of
// allocations without ripping the rug out from under any other zone.
//
// for example, you could deallocate every allocation associated with your
// user interface so it can be rebuilt from the ground up afer a winch
// signal.

static uC_mem_array_t *uC_memory[uC_MEM_ZONES] = { NULL };

// -----------------------------------------------------------------------
// free all allocations in uC_memory->alloc_array[]

API void uC_mem_purge(uC_mem_zone_t zone)
{
    uint64_t *p;

    if (uC_memory[zone] != NULL)
    {
        if (uC_memory[zone]->alloc_array != NULL)
        {
            p = uC_memory[zone]->alloc_array;
            while (uC_memory[zone]->alloc_count--)
            {
                free((void *)*p++);
            }
            free(uC_memory[zone]->alloc_array);
        }
        free(uC_memory[zone]);
        uC_memory[zone] = NULL;
    }
}

// -----------------------------------------------------------------------

static void mem_abort(void)
{
    uC_abort("Out of Memory!");
}

// -----------------------------------------------------------------------

void uC_alloc_init(uC_mem_zone_t zone)
{
    if (uC_memory[zone] == NULL)
    {
        uC_memory[zone] = calloc(1, sizeof(*uC_memory[zone]));

        uC_memory[zone]->alloc_array = (uint64_t *)(calloc(INIT_MEM_SIZE,
            sizeof(uint64_t)));

        if (uC_memory[zone]->alloc_array == NULL)
        {
            mem_abort();
        }

        uC_memory[zone]->alloc_max = INIT_MEM_SIZE;
    }
}

// -----------------------------------------------------------------------

API void *uC_alloc(uC_mem_zone_t zone, size_t size)
{
    size_t new_size;
    uint64_t *p;

    // allocates new chunks of 4k but this is really only helpful if
    // the original allocation was on a 4k boundry and I have no idea
    // if thats guaranteed or not - hmm wonder if I should look it up?

    if (uC_memory[zone]->alloc_count == uC_memory[zone]->alloc_max)
    {
        new_size = uC_memory[zone]->alloc_count + INIT_MEM_SIZE;

        p = realloc(uC_memory[zone]->alloc_array,
            new_size * sizeof(uint64_t));

        if (p == NULL)
        {
            mem_abort();
        }

        uC_memory[zone]->alloc_array = p;
        uC_memory[zone]->alloc_max = new_size;
    }

    // if this fails it is not a mem_abort because the caller
    // might be handling it

    p = (void *)&uC_memory[zone]->alloc_array[uC_memory[zone]->alloc_count];

    *p = (uint64_t)calloc(1, size);
    if (*p != 0)
    {
        uC_memory[zone]->alloc_count++;
    }
    return (void *)*p;
}

// -----------------------------------------------------------------------

API void uC_free(uC_mem_zone_t zone, void *addr)
{
    size_t i;
    uint64_t p = (uint64_t)addr;

    for (i = 0; i < uC_memory[zone]->alloc_count; i++)
    {
        if (uC_memory[zone]->alloc_array[i] == p)
        {
            free((void *)p);

            if (i != uC_memory[zone]->alloc_count)
            {
                uC_memory[zone]->alloc_array[i] =
                    uC_memory[zone]->alloc_array[uC_memory[zone]
                        ->alloc_count - 1];
            }
            uC_memory[zone]->alloc_count--;
        }
    }
}

// =======================================================================
