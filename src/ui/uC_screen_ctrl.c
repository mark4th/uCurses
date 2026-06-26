// uC_screen_ctrl.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_keys.h"
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

    if ((scr == NULL) || (scr->width <= 0) || (scr->height <= 0))
    {
        return -1;
    }

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
        scr->shortcuts_enabled = true;

        scr->windows.zone = uC_MEM_ZONE_UI;
        scr->status.zone  = uC_MEM_ZONE_UI;
        scr->shortcuts.zone = uC_MEM_ZONE_UI;

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

API void uC_scr_enable_shortcuts(uC_screen_t *scr)
{
    if (scr != NULL)
    {
        scr->shortcuts_enabled = true;
    }
}

// -----------------------------------------------------------------------

API void uC_scr_disable_shortcuts(uC_screen_t *scr)
{
    if (scr != NULL)
    {
        scr->shortcuts_enabled = false;
    }
}

// -----------------------------------------------------------------------

API bool uC_scr_shortcuts_enabled(uC_screen_t *scr)
{
    return (scr != NULL) && scr->shortcuts_enabled;
}

// -----------------------------------------------------------------------
// close all widgets, views and view groups associated with screen

#ifdef UC_WIDGETS

API void uC_scr_close_view_groups(uC_screen_t *scr)
{
    uC_widget_vg_t *vg;

    if (scr != NULL)
    {
        if (scr->popup_vg != NULL)
        {
            vg = (uC_widget_vg_t *)scr->popup_vg;
            scr->popup_vg = NULL;
            uC_widget_vg_close(vg);
        }

        while (scr->view_groups.count != 0)
        {
            vg = uC_list_pop_head(&scr->view_groups);
            uC_widget_vg_close(vg);
        }
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

#ifdef UC_POPUPS
        uC_win_close(scr->popup);
        scr->popup = NULL;
        uC_win_close(scr->too_small_popup);
        scr->too_small_popup = NULL;
#endif

#ifdef UC_WIDGETS
        uC_scr_close_view_groups(scr);
#endif

#ifdef UC_MENUS
        uC_menu_bar_close(scr);
#endif

        uC_shortcut_clear(scr);

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
        .flags.bits = (uC_ATTR_FLAG_GRAY_FG | uC_ATTR_FLAG_GRAY_BG),
        .fg         = uC_GRAY_12,
        .bg         = uC_GRAY_06,
    };

    if ((win != NULL) && (scr != NULL))
    {
        win->xco         = 1;
        win->yco         = 1;
        win->width       = scr->width - 2;
        win->height      = scr->height - 2;
        win->flags       = (uC_WIN_BOXED | uC_WIN_LOCKED);
        win->border_type = uC_BDR_SINGLE;
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
// within the screen you might not need one of these but if the windows
// you define do not take up the entire width and height of the console
// you will have to clear the display prior to drawing your view or what
// ever was there before you launched your application will still be
// visible

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
    int16_t x, int16_t y)
{
    int16_t width;
    int16_t height;

    width  = win->width;
    height = win->height;

    // if window is boxed account for border
    if (win->flags & uC_WIN_BOXED)
    {
        // if the border exists then the lowest X / Y coordinate
        // the window can be placed at is 1 / 1.

        if ((x < 1) || (y < 1))
        {
            return -1;
        }

        // count the border as part of the windows width and
        // position for the following calculation

        width  += 2;
        height += 2;
        x--;
        y--;
    }

    x += width;
    y += height;

    // ensure that window is entirely within the screen

    return ((x <= scr->width) && (y <= scr->height))
        ? 0 : -1;
}

// -----------------------------------------------------------------------
// set the minimum usable screen dimensions

API void uC_scr_set_min_size(uC_screen_t *scr, int16_t width, int16_t height)
{
    if (scr != NULL)
    {
        scr->min_width  = width;
        scr->min_height = height;
    }
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
#ifdef UC_POPUPS
            if (scr->popup == win)
            {
                scr->popup = NULL;
            }
            if (scr->too_small_popup == win)
            {
                scr->too_small_popup = NULL;
            }
#endif
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

        f = uC_list_push_tail(&scr->windows, win);

        if (f == true)
        {
            win->screen = scr;
        }
    }
}

#ifdef UC_POPUPS
// -----------------------------------------------------------------------
// attach a popup window to a screen

API void uC_scr_popup_attach(uC_screen_t *scr, uC_window_t *win)
{
    int16_t rv;

    uC_scr_win_detach(win);

    if ((scr != NULL) && (win != NULL))
    {
        if (scr->popup != NULL)
        {
            uC_scr_popup_detach(scr->popup);
        }

        rv = win_chk_pos(win, scr, win->xco, win->yco);
        if (rv != 0)
        {
            return;
        }

        scr->popup = win;
        win->screen = scr;
    }
}

// -----------------------------------------------------------------------
// detach a popup window from its parent screen

API void uC_scr_popup_detach(uC_window_t *win)
{
    uC_scr_win_detach(win);
}

// -----------------------------------------------------------------------
// cancel the currently attached popup window and widget popup, if any

API void uC_scr_popup_cancel(uC_screen_t *scr)
{
    if (scr == NULL)
    {
        return;
    }

    if (scr->popup != NULL)
    {
        uC_scr_popup_detach(scr->popup);
    }

#ifdef UC_WIDGETS
    if (scr->popup_vg != NULL)
    {
        uC_widget_popup_detach((uC_widget_vg_t *)scr->popup_vg);
    }
#endif
}
#endif

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
        scr->selected->flags &= ~uC_WIN_FOCUS;
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
            win->flags     |= uC_WIN_FOCUS;
            break;
        }
        n1 = uC_list_scan(NULL, n1);
    }
}

// =======================================================================
