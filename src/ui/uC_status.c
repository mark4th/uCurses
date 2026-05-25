// status.c    - status bar window of one line only but you can have many
// -----------------------------------------------------------------------

#include <stdarg.h>

#include "uCurses.h"
#include "uC_win_printf.h"
#include "uC_window.h"
#include "uC_status.h"

// -----------------------------------------------------------------------

#ifdef UC_STATUS

// -----------------------------------------------------------------------

API uC_window_t *uC_add_status(uC_screen_t *scr, uint16_t width,
    uint16_t xco, uint16_t yco)
{
    int16_t f;

    if (scr == NULL)
    {
        return NULL;
    }

    uC_window_t *win = uC_win_open(width, 1);

    if (win != NULL)
    {
        // verify that this window can actually fit in
        // this position on the screen.  if not, then
        // abort mission.

        f = win_chk_pos(win, scr, xco, yco);

        if (f == -1)
        {
            uC_win_close(win);
            return NULL;
        }

        win->xco = xco;
        win->yco = yco;
        win->screen = scr;
        win->flags |= uC_WIN_LOCKED;

        uC_list_push_tail(&scr->status, win);
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

API void uC_set_status(uC_window_t *win, const char *fmt, ...)
{
    va_list args;

    if (!win) return;

    uC_win_clear(win);
    va_start(args, fmt);
    uC_win_vprintf(win, fmt, args);
    va_end(args);
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
