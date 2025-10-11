// uC_window_ctrl.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_window.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------
// allocate buffer for window contents

int16_t win_alloc(uC_window_t *win)
{
    int16_t rv = -1;        // assume failure
    cell_t *p;
    size_t size;

    if (win != NULL)
    {
        size = (win->width * win->height) * sizeof(cell_t);
        p = uC_alloc(uC_MEM_ZONE_UI, size);

        if (p != NULL)
        {
            win->buffer = p;
            rv = 0;
        }
    }

    return rv;
}

// -----------------------------------------------------------------------
// open a new window of given dimensions

API uC_window_t *uC_win_open(int16_t width, int16_t height)
{
    uC_window_t *win = uC_alloc(uC_MEM_ZONE_UI, sizeof(*win));

    if (win != NULL)
    {
        win->width  = width;
        win->height = height;

        // win_alloc() uses width/height to determine how much
        // space needs to be allocated for the buffers

        if (win_alloc(win) == 0)
        {
            // todo: make default attributes variables
            // available to the application
            win->attrs.fg = DEFAULT_FG;
            win->attrs.bg = DEFAULT_BG;
            win->blank = 0x20;

            uC_win_clear(win);
        }
        else
        {
            uC_ui_free(win);
            win = NULL;
        }
    }

    return win;
}

// -----------------------------------------------------------------------
// close a window and free all associated allocated resources

API void uC_win_close(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_scr_win_detach(win);

        if (win->buffer != NULL)
        {
            uC_ui_free(win->buffer);
        }

        uC_ui_free(win);
    }
}

// -----------------------------------------------------------------------
// pop window to front in view

API void uC_win_pop(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_screen_t *scr = win->screen;

        uC_scr_win_detach(win);
        uC_scr_win_attach(scr, win);
    }
}

// -----------------------------------------------------------------------
// push window to back

API void uC_win_push(uC_window_t *win)
{
    bool f;

    if (win != NULL)
    {
        uC_screen_t *scr = win->screen;
        uC_scr_win_detach(win);

        f = uC_list_push_head(&scr->windows, win);
        if (f != true)
        {
            // guru meditation
        }
    }
}

// -----------------------------------------------------------------------
// set new x/y position of window within parent screen

// window must fit entirely within the screen

API int16_t uC_win_set_pos(uC_window_t *win, int16_t x, int16_t y)
{
    int16_t rv = -1;

    uC_screen_t *scr;

    // the windows dimensions as stored within its structure accounts only
    // for the drawable region of the window, not its border. we need to
    // account for the border if there is one here.

    if ((win != NULL) && (x >= 0) && (y >= 0))
    {
        scr = win->screen;

        if (scr != NULL)
        {
            rv = win_chk_pos(win, scr, x, y);
            if (rv == 0)
            {
                win->xco = x;
                win->yco = y;
            }
        }
    }

    return rv;
}

// -----------------------------------------------------------------------

API void uC_win_set_flag(uC_window_t *win, win_flags_t flag)
{
    if (win != NULL)
    {
        win->flags |= flag;
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_flag(uC_window_t *win, win_flags_t flag)
{
    if (win != NULL)
    {
        win->flags &= ~flag;
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_border(uC_window_t *win, uint16_t border_type,
    uC_attribs_t bdr_attrs, uC_attribs_t focus_attrs)
{
    win->border_type = border_type;
    win->bdr_attrs   = bdr_attrs;
    win->focus_attrs = focus_attrs;
    win->flags      |= WIN_BOXED;
}

// =======================================================================
