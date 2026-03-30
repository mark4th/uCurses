// uC_alloc.c  - memory allocations tracking
// -----------------------------------------------------------------------

#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>

#include "uCurses.h"
#include "uC_utils.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

#define INIT_MEM_SIZE (512)

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
// so you can free up all allocations associated with a specific zone
// without ripping the rug out from under any other.

static uC_mem_array_t *uC_memory[uC_MEM_ZONES] = { NULL };

// -----------------------------------------------------------------------

static void mem_abort(void)
{
    uC_abort("Out of Memory!");
}

// -----------------------------------------------------------------------
// free all allocations in specified zone and discards the zone itself

API void uC_mem_purge(uC_mem_zone_t zone)
{
    uint64_t *p;
    uC_mem_array_t *z = uC_memory[zone];

    if (z != NULL)
    {
        p = z->alloc_array;

        if (p != NULL)
        {
            while (z->alloc_count--)
            {
                free((void *)*p++);
            }
            free(z->alloc_array);
        }

        free(z);
        uC_memory[zone] = NULL;
    }
}

// -----------------------------------------------------------------------

// useful for debugging deallocatioins within the library itself, did a
// specific module free up all its allocations without needing to "purge"
// any unaccounted for allocations?

API size_t uC_zone_query(uC_mem_zone_t zone)
{
    uC_mem_array_t *z = uC_memory[zone];

    return (z != NULL)
        ? z->alloc_count
        : 0;
}

// -----------------------------------------------------------------------
// if someone attempts to make an allocation on an uninitialized zone then
// this code will be called first, allowing said allocation.

static void alloc_init(uC_mem_zone_t zone)
{
    uC_mem_array_t *z = calloc(1, sizeof(*z));
    uC_ASSERT(z != NULL, "Out of Memory!");

    uC_memory[zone] = z;

    z->alloc_max   = INIT_MEM_SIZE;
    z->alloc_array = calloc(INIT_MEM_SIZE, sizeof(uint64_t));

    if (z->alloc_array == NULL)
    {
        free(z);
        mem_abort();
    }
}

// -----------------------------------------------------------------------

static bool check_zone_full(uC_mem_zone_t zone)
{
    uint64_t *p;
    size_t new_size;

    uC_mem_array_t *z = uC_memory[zone];

    if (z->alloc_count == z->alloc_max)
    {
        new_size = z->alloc_count + INIT_MEM_SIZE;

        p = realloc(z->alloc_array, new_size * sizeof(uint64_t));

        if (p != NULL)
        {
            z->alloc_array = p;
            z->alloc_max   = new_size;
        }
        return (p != NULL);
    }

    return true;
}

// -----------------------------------------------------------------------

API void *uC_alloc(uC_mem_zone_t zone, size_t size)
{
    bool f;
    uint64_t *p = NULL;
    uC_mem_array_t *z;

    z = uC_memory[zone];

    if (z == NULL)
    {
        alloc_init(zone);
        z = uC_memory[zone];
    }

    // grow zone if need be
    f = check_zone_full(zone);

    if (f == true)
    {
        // if this fails it is not a mem_abort because the caller
        // might be handling it

        p = (void *)&z->alloc_array[z->alloc_count];

        *p = (uint64_t)calloc(1, size);

        if (*p != 0)
        {
            z->alloc_count++;
        }
    }

    return (void *)*p;
}

// -----------------------------------------------------------------------

API void uC_free(uC_mem_zone_t zone, void *addr)
{
    size_t i;
    uC_mem_array_t *z = uC_memory[zone];

    for (i = 0; i < z->alloc_count; i++)
    {
        if (z->alloc_array[i] == (uint64_t)addr)
        {
            free(addr);
            z->alloc_count--;

            if (i != z->alloc_count)
            {
                z->alloc_array[i] = z->alloc_array[z->alloc_count];
            }
            z->alloc_array[z->alloc_count] = 0;
        }
    }
}

// =======================================================================
