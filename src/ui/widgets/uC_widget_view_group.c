// uC_widget_view_group.c
// -----------------------------------------------------------------------

#include <stdint.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_borders.h"
#include "uC_list.h"
#include "uC_widgets.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

API uC_widget_vg_t *uC_widget_vg_create(char *name,
    uint16_t width, uint16_t height,
    uint16_t xco, uint16_t yco,
    uC_attribs_t attrs)
{
    uC_widget_vg_t *vg = uC_alloc(uC_MEM_ZONE_UI, sizeof(*vg));

    if (vg != NULL)
    {
        vg->window.display_name = name;

        vg->window.width         = width;
        vg->window.height        = height;
        vg->window.xco           = xco;
        vg->window.yco           = yco;
        vg->window.attrs         = attrs;
        // no scrolling this window
        vg->window.flags         = WIN_LOCKED;

        vg->window.blank         = 0x20;
    }

    return vg;
}

// -----------------------------------------------------------------------
// if you want the view group to have a border...

API void uC_widget_vg_add_border(uC_widget_vg_t *vg,
    uC_border_type_t bdr_type,
    uC_attribs_t bdr_attrs,
    uC_attribs_t focus_attrs)
{
    if (vg != NULL)
    {
        vg->window.flags |= WIN_BOXED;
        // a view group window always has focus when it is active
        vg->window.border_type = bdr_type;
        vg->window.bdr_attrs   = bdr_attrs;
        vg->window.focus_attrs = focus_attrs;
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_attach(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    int16_t rv;

    if ((scr != NULL) && (vg != NULL))
    {
        rv = win_alloc(&vg->window);

        if (rv == 0)
        {
            vg->window.screen = scr;
            uC_win_clear(&vg->window);
            uC_list_push_tail(&scr->view_groups, vg);
        }
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_detach(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    uC_list_remove_node(&scr->view_groups, vg);
}

// -----------------------------------------------------------------------

API void uC_widget_vg_close(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    uC_widget_view_t *view;

    do
    {
        view = uC_list_pop_head(&vg->views);
        if (view != NULL)
        {
            widget_close_view(view);
            uC_free(uC_MEM_ZONE_UI, view);
        }
    } while (view != NULL);

    uC_list_remove_node(&scr->view_groups, vg);
    uC_win_close(&vg->window);
    uC_free(uC_MEM_ZONE_UI, vg);
}

// -----------------------------------------------------------------------

API void uC_widget_vg_add_view(uC_widget_vg_t *vg, uC_widget_view_t *v)
{
    uC_list_push_tail(&vg->views, v);
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
