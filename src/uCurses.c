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
#include "uC_mouse.h"
#include "ti_file.h"
#include "json.h"

// -----------------------------------------------------------------------

void uC_alloc_init(uC_mem_zone_t zone);
void init_winch(void);
void de_init_winch(void);
void menu_set_screen(uC_screen_t *scr);

extern ti_vars_t *ti_vars;
extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------

API uC_screen_t *uCurses_init(char *file, json_mem_t *json, fp_finder_t fp)
{
#ifdef UC_MENUS
    uint16_t width;
    uint16_t height;
#endif

    setlocale(LC_ALL, "C.UTF-8");

    uC_init_terminal();

    alloc_parse();
    alloc_info();

    init_winch();
    uC_curoff();
    uC_smkx();

#ifdef UC_WIDGETS
    uC_mouse_enable();
#endif

#ifdef UC_JSON
    if (file != NULL)
    {
        json_file_create_ui(file, fp);
    }
    else if (json != NULL)
    {
        json_mem_create_ui(json->json, json->len, fp);
    }
#else
    (void) file;
    (void) json;
    (void) fp;
#endif

#ifdef UC_MENUS
    if (active_screen != NULL)
    {
        if (active_screen->menu_bar != NULL)
        {
            menu_set_screen(active_screen);
            uC_menu_init_keys();
        }
    }
    else
    {
        uC_get_console_size(&width, &height);
        active_screen = uC_scr_open(width, height);
        uC_scr_add_backdrop(active_screen);
    }
#endif

    return active_screen;
}

// -----------------------------------------------------------------------

API void uCurses_deInit(void)
{
#ifdef UC_WIDGETS
    uC_mouse_disable();
#endif
    uC_restore_terminal();
    de_init_winch();

    uC_mem_purge(uC_MEM_ZONE_JSON);
    uC_mem_purge(uC_MEM_ZONE_UI);
    uC_mem_purge(uC_MEM_ZONE_DEFAULT);
}

// =======================================================================
