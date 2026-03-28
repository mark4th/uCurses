// uC_screen_ctrl.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_menus.h"
#include "uC_borders.h"
#include "uC_attribs.h"
#include "uC_utils.h"

#ifdef UC_WIDGETS
#include "uC_widgets.h"
#endif // UC_WIDGETS

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
        uC_ui_free(p1);
        uC_ui_free(p2);
        return -1;
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
            uC_ui_free(scr);
            scr = NULL;
        }
    }

    active_screen = scr;

    return scr;
}

// -----------------------------------------------------------------------
// close all widgets, views and view groups associated with screen

#ifdef UC_WIDGETS

static void close_view_groups(uC_screen_t *scr)
{
    uC_widget_vg_t *vg;

    while (scr->view_groups.count != 0)
    {
        vg = uC_list_pop_head(&scr->view_groups);
        uC_widget_vg_close(vg);
    }
}

#endif // UC_WIDGETS

// -----------------------------------------------------------------------
// deallocate all structures attached to screen

API void uC_scr_close(uC_screen_t *scr)
{
    uC_window_t *win;

    if (scr != NULL)
    {
        uC_ui_free(scr->buffer1);
        uC_ui_free(scr->buffer2);

        scr->buffer1 = NULL;
        scr->buffer2 = NULL;

        // safe to call this on a null window
        uC_win_close(scr->backdrop);

        while (scr->windows.count != 0)
        {
            win = (uC_window_t *)uC_list_pop_head(&scr->windows);
            uC_win_close(win);
        }

#ifdef UC_WIDGETS
        close_view_groups(scr);
#endif

#ifdef UC_MENUS
        uC_menu_bar_close(scr);
#endif

        do
        {
            win = uC_list_pop_head(&scr->status);
            uC_win_close(win);
        } while (win != NULL);

        uC_ui_free(scr);
    }

    // technically we could have more than one screen but that would need
    // some kind of list push when opening a new one and a list pop here
    // todo?

    if (scr == active_screen)
    {
        active_screen = NULL;
    }
}

// -----------------------------------------------------------------------

void init_backdrop(uC_screen_t *scr, uC_window_t *win)
{
    uC_attribs_t bdr_attrs =
    {
        .flags.bits = (ATTR_FLAG_GRAY_FG | ATTR_FLAG_GRAY_BG),
        .fg         = uC_GRAY_12,
        .bg         = uC_GRAY_06,
    };

    if ((win != NULL) && (scr != NULL))
    {
        win->xco         = 1;
        win->yco         = 1;
        win->width       = scr->width - 2;
        win->height      = scr->height - 2;
        win->flags       = (WIN_BOXED | WIN_LOCKED);
        win->border_type = BDR_SINGLE;
        win->blank       = 0x20;
        win->screen      = scr;
        win->bdr_attrs   = bdr_attrs;

        uC_win_set_gray_fg(win, uC_GRAY_12);

        scr->backdrop = win;
    }
 }

// -----------------------------------------------------------------------
// add a backdrop window to the screen

// a backdrop is a window you usually do not draw into that gives a place
// to draw other things over. without it any windows that move leave trails
// behind but with it they do not.  This window always takes up the entire
// size of the screen and is always framed with a single line border

// if you dont want any of your windows to move up / down / left / right
// within the screen you might not need one of these.

API void uC_scr_add_backdrop(uC_screen_t *scr)
{
    uC_window_t *win;

    if (scr != NULL)
    {
        win = uC_win_open(scr->width - 2, scr->height - 2);

        if (win != NULL)
        {
            init_backdrop(scr, win);
            uC_win_clear(win);
        }
    }
}

// -----------------------------------------------------------------------
// verify that a windows position is within bounds

int16_t win_chk_pos(uC_window_t *win, uC_screen_t *scr,
    uint16_t x, uint16_t y)
{
    int16_t xx;
    int16_t yy;
    int16_t width;
    int16_t height;

    width  = win->width;
    height = win->height;

    // if window is boxed account for border
    if (win->flags & WIN_BOXED)
    {
        // of the border exists then the lowest X / Y coordinate
        // the window can be placed at is 1 / 1.

        if ((x <= 0) || (y < 0))
        {
            return -1;
        }
        else
        {
            // count the border as part of the windows width and
            // position for the following calculation

            width  += 2;
            height += 2;
            x--;
            y--;
        }
    }

    xx = (x + width);
    yy = (y + height);

    // ensure that window is entirely within the screen
    return ((xx <= scr->width) && (yy <= scr->height))
        ? 0 : -1;
}

// -----------------------------------------------------------------------
// detach window from its parent screen

API void uC_scr_win_detach(uC_window_t *win)
{
    uC_screen_t *scr;

    if (win != NULL)
    {
        scr = win->screen;

        if (scr != NULL)
        {
            uC_list_remove_node(&scr->windows, win);
            win->screen = NULL;
        }
    }
}

// -----------------------------------------------------------------------
// attach a window to a screen

API void uC_scr_win_attach(uC_screen_t *scr, uC_window_t *win)
{
    int16_t rv;
    bool f;

    // attaching a window to a screen detaches it from any screen that it
    // is already attached to.  If multi screen is ever implemented then
    // moving a window between screens will siply entail attaching that
    // window to the new screen.

    uC_scr_win_detach(win);

    if ((scr != NULL) && (win != NULL))
    {
        rv = win_chk_pos(win, scr, win->xco, win->yco);
        if (rv != 0)
        {
            return;
        }

        // if (win->screen != NULL)
        // {
        //     uC_list_remove_node(&scr->windows, win);
        // }

        f = uC_list_push_tail(&scr->windows, win);

        if (f == true)
        {
            win->screen = scr;
        }
    }
}

// -----------------------------------------------------------------------

API void uC_scr_win_tab_next(uC_screen_t *scr)
{
    uC_list_node_t *n1;
    uC_window_t *win;
    int16_t order;

    if (scr == NULL)
    {
        return;
    }

    order = scr->tab_order + 1;

    // remove focus from current window if there is one
    if (scr->selected != NULL)
    {
        scr->selected->flags &= ~WIN_FOCUS;
        scr->tab_order = 0;
    }

    n1 = uC_list_scan(&scr->windows, NULL);

    while (n1 != NULL)
    {
        win = n1->payload;

        if (win->tab_order == order)
        {
            scr->selected   = win;
            scr->tab_order  = order;
            win->flags     |= WIN_FOCUS;
            break;
        }
        n1 = uC_list_scan(NULL, n1);
    }
}

// =======================================================================
