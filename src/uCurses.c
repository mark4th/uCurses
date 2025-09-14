// uCurses.c
// -----------------------------------------------------------------------

#include <locale.h>
#include <signal.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_attribs.h"
#include "uC_utils.h"
#include "uC_alloc.h"
#include "uC_screen.h"
#include "uC_window.h"
#include "uC_win_printf.h"
#include "uC_menus.h"
#include "ti_file.h"
#include "json.h"

// -----------------------------------------------------------------------

void uC_alloc_init(uC_mem_zone_t zone);
void init_winch(void);
void de_init_winch(void);

extern ti_vars_t *ti_vars;
extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------

API void uCurses_init(char *file, json_mem_t *json, void *fp)
{
#ifdef UC_JSON
    // gcc will no longer compile these sources because of some ultra
    // fucking bullshit ISO standard that mandates that you cannot be
    // allowed to smash a square peg into a square hole of the wrong
    // type.
    //
    // the "fp" variable absolutely has to be passed as void because the
    // entire fucking menu system is OPTIONAL so even if I could make
    // it of type fp_finder_t that fucking typedef is not going always
    // to exist

    // ISO C forbids conversion of object pointer to function pointer
    // type

    // who the FUCK says a void pointer is always a ponter to
    // fucking data?

    fp_finder_t finder = (fp_finder_t) fp;
#endif
    setlocale(LC_ALL, "C.UTF-8");

    uC_init_terminal();

    alloc_parse();
    alloc_info();

    init_winch();
    uC_curoff();

#ifdef UC_JSON
    if (file != NULL)
    {
        json_file_create_ui(file, finder);
    }
    else if (json != NULL)
    {
        json_mem_create_ui(json->json, json->len, finder);
    }
#else
    (void) file;
    (void) json;
    (void) fp;
#endif

#ifdef UC_MENUS
    if (active_screen->menu_bar != NULL)
    {
        menu_init_keys();
    }
#endif
}

// -----------------------------------------------------------------------

API void uCurses_deInit(void)
{
    uC_restore_terminal();
    de_init_winch();

    uC_mem_purge(uC_MEM_ZONE_JSON);
    uC_mem_purge(uC_MEM_ZONE_UI);
    uC_mem_purge(uC_MEM_ZONE_DEFAULT);
}

// =======================================================================
