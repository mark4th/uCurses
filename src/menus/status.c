// status.c
// -----------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uCurses.h"
#include "uC_menus.h"
#include "uC_win_printf.h"

// -----------------------------------------------------------------------
// user application allocatted

static char *status_line;

// -----------------------------------------------------------------------

void free_status(void)
{
    free(status_line);
    status_line = NULL;
}

// -----------------------------------------------------------------------

API void uC_bar_clr_status(void)
{
    if (status_line != NULL)
    {
        memset(status_line, 0x20, MAX_STATUS);
        status_line[MAX_STATUS] = '\0';
    }
}

// -----------------------------------------------------------------------

API void uC_alloc_status(void)
{
    status_line = calloc(1, MAX_STATUS + 1);

    if (status_line != NULL)
    {
        uC_bar_clr_status();
    }
}

// -----------------------------------------------------------------------
// print string into status bar.

API void uC_bar_set_status(char *s)
{
    memset(status_line, 0x20, MAX_STATUS);

    if (status_line != NULL)
    {
        snprintf(status_line, MAX_STATUS, "%s", s);
    }
    status_line[strlen(status_line)] = 0x20;
}

// -----------------------------------------------------------------------

API void uC_bar_draw_status(menu_bar_t *bar)
{
    int16_t x;
    screen_t *scr;

    if ((bar != NULL) && (status_line != NULL))
    {
        window_t *win = bar->window;
        scr = win->screen;

        // this x calculation needs work but this is close enough for now

        x = (scr->width - MAX_STATUS) - 6;

        win->attr_grp.attrs.chunk = bar->attr_grp.selected.chunk;

        uC_win_printf(win, "%@[%s]", x, 0, status_line);
    }
}

// =======================================================================
