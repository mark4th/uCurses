// uC_window_ctrl.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_window.h"

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
// close a window and free all associated allocated resources

API void uC_win_close(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_scr_win_detach(win);

        if (win->buffer != NULL)
        {
            uC_free(uC_MEM_ZONE_UI, win->buffer);
        }

        uC_free(uC_MEM_ZONE_UI, win);
    }
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
            uC_free(uC_MEM_ZONE_UI, win);
            win = NULL;
        }
    }

    return win;
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

    // the windows dimensions as stored within its structure accounts only
    // for the drawable region of the window, not its border. we need to
    // account for the border if there is one here.

    int16_t win_width;
    int16_t win_height;
    int16_t win_x;
    int16_t win_y;

    if ((win != NULL) && (x >= 0) && (y >= 0))
    {
        uC_screen_t *scr = win->screen;

        if (scr == NULL)
        {
            return rv;
        }

        win_width  = win->width;
        win_height = win->height;
        win_x      = win->xco;
        win_y      = win->yco;

        // if window is boxed account for border
        if (win->flags & WIN_BOXED)
        {
            if ((win->xco <= 0) || (win->yco < 0))
            {
                return rv;
            }
            else
            {
                win_width  += 2;
                win_height += 2;
                win_x--;
                win_y--;
            }
        }

        // ensure that window is entirely within the screen
        if ((win_x + win_width  < scr->width) &&
            (win_y + win_height < scr->height))
        {
            win->xco = x;
            win->yco = y;

            rv = 0;
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
    uC_attribs_t bdr_attrs)
{
    win->border_type = border_type;
    win->bdr_attrs   = bdr_attrs;
    win->flags      |= WIN_BOXED;
}

// =======================================================================
