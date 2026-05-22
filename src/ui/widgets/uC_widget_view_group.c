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

extern widget_state_t widget_state;

// -----------------------------------------------------------------------

API uC_widget_vg_t *uC_widget_vg_create(const char *name,
    uint16_t width, uint16_t height, uint16_t xco, uint16_t yco,
    uC_attribs_t attrs)
{
    uC_widget_vg_t *vg = uC_alloc(uC_MEM_ZONE_UI, sizeof(*vg));

    if (vg)
    {
        vg->window.display_name = name;

        vg->window.width         = width;
        vg->window.height        = height;
        vg->window.xco           = xco;
        vg->window.yco           = yco;
        vg->window.attrs         = attrs;

        // no scrolling this window

        vg->window.flags         = uC_WIN_LOCKED;

        // this name will not be displayed unless you also give the
        // view group a border but it is assumed that if you supplied
        // a name then you intend to do so.

        if (name)
        {
            vg->window.flags |= uC_WIN_NAMED;
        }

        vg->window.blank         = 0x20;
    }

    return vg;
}

// -----------------------------------------------------------------------
// if you want the view group to have a border...

API void uC_widget_vg_add_border(uC_widget_vg_t *vg,
    uC_border_type_t bdr_type,
    uC_attribs_t bdr_attrs, uC_attribs_t focus_attrs)
{
    if (vg)
    {
        vg->window.flags |= uC_WIN_BOXED;

        // a view group window always has focus when it contains the
        // widget that has focus

        vg->window.border_type = bdr_type;
        vg->window.bdr_attrs   = bdr_attrs;
        vg->window.focus_attrs = focus_attrs;
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_attach(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    int16_t rv;

    if (scr && vg)
    {
        rv = win_alloc(&vg->window);

        if (!rv)
        {
            vg->window.screen = scr;
            uC_win_clear(&vg->window);
            uC_list_push_tail(&scr->view_groups, vg);

            widget_state.screen = scr;

            if (!widget_state.sequence)
            {
                uC_widget_select_widget(1);
            }
        }
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_detach(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    if (vg)
    {
        uC_list_remove_node(&scr->view_groups, vg);
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_close(uC_widget_vg_t *vg)
{
    uC_widget_view_t *view;

    if (vg)
    {
        while (vg->views.count)
        {
            view = uC_list_pop_head(&vg->views);
            widget_close_view(view);
        }

        uC_win_close(&vg->window);
        uC_free(uC_MEM_ZONE_UI, vg);
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_add_view(uC_widget_vg_t *vg, uC_widget_view_t *v,
    uint16_t sequence)
{
    if (v)
    {
        v->sequence = sequence ? sequence : auto_sequence();
    }
    uC_list_push_tail(&vg->views, v);
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
