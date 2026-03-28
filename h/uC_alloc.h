// uC_alloc.h
// -----------------------------------------------------------------------

#ifndef UC_ALLOC_H
#define UC_ALLOC_H

// -----------------------------------------------------------------------

#include <stddef.h>

#include "uCurses.h"

// -----------------------------------------------------------------------

typedef enum
{
    uC_MEM_ZONE_DEFAULT,
    uC_MEM_ZONE_UI,         // screen, widnows, widgets etc
    uC_MEM_ZONE_JSON,
    uC_MEM_ZONE_USER1,
    uC_MEM_ZONE_USER2,

    uC_MEM_ZONES
} __attribute__((__packed__)) uC_mem_zone_t;

// -----------------------------------------------------------------------

API size_t uC_zone_query(uC_mem_zone_t zone);
API void uC_mem_purge(uC_mem_zone_t zone);
API void *uC_alloc(uC_mem_zone_t zone, size_t size);
API void uC_free(uC_mem_zone_t zone, void *addr);

// -----------------------------------------------------------------------

#endif // UC_ALLOC_H

// =======================================================================
