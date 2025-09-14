
// uC_screen_ctrl.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_menus.h"
#include "uC_borders.h"
#include "uC_attribs.h"

extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------

int16_t scr_alloc(uC_screen_t *scr)
{
    size_t size;
    cell_t *p1, *p2;

    size = (scr->width * scr->height) * sizeof(*p1);

    // allocate buffers 1 and 2 for screen

    p1 = uC_alloc(uC_MEM_ZONE_UI, size);
    p2 = uC_alloc(uC_MEM_ZONE_UI, size);

    if ((p1 == NULL) || (p2 == NULL))
    {
        uC_free(uC_MEM_ZONE_UI, p1);
        uC_free(uC_MEM_ZONE_UI, p2);
        return -1;          // so is safe
    }

    scr->buffer1 = p1;
    scr->buffer2 = p2;

    return 0;
}

// -----------------------------------------------------------------------

API uC_screen_t *uC_scr_open(int16_t width, int16_t height)
{
    uC_screen_t *scr;

    scr = uC_alloc(uC_MEM_ZONE_UI, sizeof(*scr));

    if (scr != NULL)
    {
        scr->width  = width;
        scr->height = height;

        scr->windows.zone = uC_MEM_ZONE_UI;
        scr->status.zone  = uC_MEM_ZONE_UI;

        // allocate screen buffer based on width / height

        if (scr_alloc(scr) != 0)
        {
            uC_free(uC_MEM_ZONE_UI, scr);
            scr = NULL;
        }
    }

    active_screen = scr;

    return scr;
}

// -----------------------------------------------------------------------
// deallocate all structures attached to screen

API void uC_scr_close(uC_screen_t *scr)
{
    uC_window_t *win;

    if (scr != NULL)
    {
        uC_free(uC_MEM_ZONE_UI, scr->buffer1);
        uC_free(uC_MEM_ZONE_UI, scr->buffer2);

        scr->buffer1 = NULL;
        scr->buffer2 = NULL;

        // safe to call this on a null window
        uC_win_close(scr->backdrop);

        do
        {
            win = uC_list_pop_head(&scr->windows);
            uC_win_close(win);
        } while (win != NULL);

        // do
        // {
        //     close all widget views
        // } while(...);

#ifdef UC_MENUS
        uC_bar_close(scr);
#endif

        do
        {
            win = uC_list_pop_head(&scr->status);
            uC_win_close(win);
        } while (win != NULL);

        uC_free(uC_MEM_ZONE_UI, scr);
    }

    // technically we could have more than one screen but that would need
    // some kind of list push when opening a new one and a list pop here
    // todo?

    active_screen = NULL;
}

// -----------------------------------------------------------------------

void init_backdrop(uC_screen_t *scr, uC_window_t *win)
{
    if ((win != NULL) && (scr != NULL))
    {
        win->xco      = 1;
        win->yco      = 1;
        win->width    = scr->width - 2;
        win->height   = scr->height - 2;
        win->flags    = (WIN_BOXED | WIN_LOCKED);
        win->blank    = SOLID;
        win->screen   = scr;
        scr->backdrop = win;
    }
 }

// -----------------------------------------------------------------------
// add a backdrop window to the screen

// a backdrop is a window you usually do not draw into that gives a place
// to draw other things over without it any windows that move leave trails
// behind but with it they do not.  This window always takes up the entire
// size of the screen and is always framed with a single line border

// if you dont want any of your windows to move up / down / left / right
// within the screen you might not need one of these.

API void uC_scr_add_backdrop(uC_screen_t *scr)
{
    uC_window_t *win;

    uC_attribs_t bdr_attrs =
    {
        .flags.bits = (ATTR_FLAG_GRAY_FG | ATTR_FLAG_GRAY_BG),
        .fg         = uC_GRAY_12,
        .bg         = uC_GRAY_06,
    };

    if (scr != NULL)
    {
        win = uC_win_open(scr->width - 2, scr->height - 2);

        if (win != NULL)
        {
            win->bdr_attrs   = bdr_attrs;
            win->border_type = BDR_SINGLE;

            uC_win_set_gray_fg(win, uC_GRAY_12);
            init_backdrop(scr, win);
            uC_win_clear(win);
        }
    }
}

// -----------------------------------------------------------------------
// attach a window to a screen

API void uC_scr_win_attach(uC_screen_t *scr, uC_window_t *win)
{
    uC_screen_t *scr2;
    win->screen = scr;
    bool f;

    if (win->screen != NULL)
    {
        scr2 = win->screen;
        uC_list_remove_node(&scr2->windows, win);
    }

    f = uC_list_push_tail(&scr->windows, win);

    if (f != true)
    {
        // log error here?
        // insert more ram to continue!
    }
}

// -----------------------------------------------------------------------
// detach window from its parent screen

API void uC_scr_win_detach(uC_window_t *win)
{
    uC_screen_t *scr = win->screen;

    if (scr != NULL)
    {
        uC_list_remove_node(&scr->windows, win);
        win->screen = NULL;
    }
}

// =======================================================================
