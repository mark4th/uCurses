// uCurses.c
// -----------------------------------------------------------------------

#include <locale.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_ti_file.h"
#include "uC_attribs.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

API void uCurses_init(void)
{
    alloc_parse();           // initialie terminfo parser
    alloc_info();
    alloc_attr_grp();

    setlocale(LC_ALL, "C.UTF-8");
    uC_curoff();
    uC_init_terminal();
}

// -----------------------------------------------------------------------

API void uCurses_deInit(void)
{
    free_parse();
    free_info();
    free_attr_grp();
    uC_restore_terminal();
}

// =======================================================================
