// uCurses.c
// -----------------------------------------------------------------------

#include <locale.h>

#include "uCurses.h"
#include "terminfo.h"
#include "ti_file.h"
#include "attribs.h"
#include "utils.h"

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
