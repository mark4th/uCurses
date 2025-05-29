// status.c
// -----------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uCurses.h"
#include "uC_menus.h"
#include "uC_win_printf.h"
#include "uC_status.h"

// -----------------------------------------------------------------------

API uC_window_t *uC_add_status(uC_screen_t *scr,
    uint16_t w, uint16_t x, uint16_t y)
{
    uC_window_t *win = NULL;

    if ((x + w) < scr->width)
    {
        win = uC_win_open(w, 1);

        if (win != NULL)
        {
            win->blank = 0x20;
            win->xco = x;
            win->yco = y;
            win->screen = scr;
            win->flags |= WIN_LOCKED;

            uC_list_push_tail(&scr->status, win);
        }
    }

    return win;
}

// -----------------------------------------------------------------------

API void uC_clr_status(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_win_clear(win);
    }
}

// -----------------------------------------------------------------------
// print string into status window

API void uC_set_status(uC_window_t *win, char *s)
{
    if (win != NULL)
    {
        uC_win_printf(win, "%0%s", s);
    }
}

// =======================================================================
