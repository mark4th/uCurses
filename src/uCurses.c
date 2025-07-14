// uCurses.c
// -----------------------------------------------------------------------

#include <locale.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_ti_file.h"
#include "uC_attribs.h"
#include "uC_utils.h"
#include "uC_alloc.h"

void uC_alloc_init(uC_mem_zone_t zone);

// -----------------------------------------------------------------------

API void uCurses_init(void)
{
    // initialize allocation tracking
    uC_alloc_init(uC_MEM_ZONE_DEFAULT);
    uC_alloc_init(uC_MEM_ZONE_UI);

    alloc_parse();           // initialie terminfo parser
    alloc_info();

    setlocale(LC_ALL, "C.UTF-8");
    uC_curoff();
    uC_init_terminal();
}

// -----------------------------------------------------------------------

API void uCurses_deInit(void)
{
    uC_restore_terminal();
    free_info();
    uC_mem_purge(uC_MEM_ZONE_UI);
    uC_mem_purge(uC_MEM_ZONE_DEFAULT);
}

// =======================================================================
