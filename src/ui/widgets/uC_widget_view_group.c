// uC_widget_view_group.c
// -----------------------------------------------------------------------

#include <stdint.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_borders.h"
#include "uC_list.h"
#include "uC_widgets.h"
#include "uC_keys.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------

bool widget_vg_contains_widget(uC_widget_vg_t *vg,
    uC_widget_t *target)
{
    uC_list_node_t *view_node;
    uC_list_node_t *widget_node;
    uC_widget_view_t *view;

    if ((vg == NULL) || (target == NULL))
    {
        return false;
    }

    for (view_node = uC_list_scan(&vg->views, NULL);
         view_node != NULL;
         view_node = uC_list_scan(NULL, view_node))
    {
        view = view_node->payload;
        for (widget_node = uC_list_scan(&view->widgets, NULL);
             widget_node != NULL;
             widget_node = uC_list_scan(NULL, widget_node))
        {
            if (widget_node->payload == target)
            {
                return true;
            }
        }
    }

    return false;
}

// -----------------------------------------------------------------------

static bool widget_shortcut_attached_active(uC_widget_t *widget)
{
    uC_list_node_t *node;
    uC_widget_vg_t *vg;
    uC_screen_t *scr;

    if ((widget == NULL) || (widget->shortcut_screen == NULL))
    {
        return false;
    }

    scr = widget->shortcut_screen;
    if ((scr->popup_vg != NULL) &&
        widget_vg_contains_widget((uC_widget_vg_t *)scr->popup_vg, widget))
    {
        vg = (uC_widget_vg_t *)scr->popup_vg;
        return (vg->flags & uC_vg_flag_inactive) == 0;
    }

    for (node = uC_list_scan(&scr->view_groups, NULL);
         node != NULL;
         node = uC_list_scan(NULL, node))
    {
        vg = node->payload;
        if (widget_vg_contains_widget(vg, widget))
        {
            return (vg->flags & uC_vg_flag_inactive) == 0;
        }
    }

    return false;
}

// -----------------------------------------------------------------------

static bool widget_shortcut_action_ready(uC_widget_t *widget)
{
    bool attached;

    if (widget == NULL)
    {
        return false;
    }

    if (widget->type != uC_WIDGET_BUTTON)
    {
        return false;
    }

    if (widget->disabled || (widget->button.letter == '\0'))
    {
        return false;
    }

    attached = widget_shortcut_attached_active(widget);
    return attached;
}

// -----------------------------------------------------------------------

static void widget_shortcut_action(void *context)
{
    uC_widget_t *widget = context;
    bool ready;

    ready = widget_shortcut_action_ready(widget);
    if (!ready)
    {
        return;
    }

    uC_widget_select_widget(widget->sequence);
    uC_set_key((uint8_t)widget->button.letter);
}

// -----------------------------------------------------------------------

static bool widget_shortcut_candidate(uC_widget_t *widget)
{
    if (widget == NULL)
    {
        return false;
    }

    if (widget->type != uC_WIDGET_BUTTON)
    {
        return false;
    }

    if (widget->button.letter == '\0')
    {
        return false;
    }

    return widget->shortcut_screen == NULL;
}

// -----------------------------------------------------------------------

static void widget_register_one_shortcut(uC_screen_t *scr,
    uC_widget_t *widget)
{
    uC_shortcut_t shortcut;
    bool candidate;
    bool registered;

    candidate = widget_shortcut_candidate(widget);
    if (!candidate)
    {
        return;
    }

    shortcut = UC_SHORTCUT(widget->button.letter);
    registered = uC_shortcut_register(scr, shortcut, widget_shortcut_action,
        widget, widget);
    if (registered)
    {
        widget->shortcut_screen = scr;
    }
}

// -----------------------------------------------------------------------

static void widget_register_view_shortcuts(uC_screen_t *scr,
    uC_widget_view_t *view)
{
    uC_list_node_t *widget_node;
    uC_widget_t *widget;

    if (view == NULL)
    {
        return;
    }

    widget_node = uC_list_scan(&view->widgets, NULL);
    while (widget_node != NULL)
    {
        widget = widget_node->payload;
        widget_register_one_shortcut(scr, widget);
        widget_node = uC_list_scan(NULL, widget_node);
    }
}

// -----------------------------------------------------------------------

static void widget_register_shortcuts(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    uC_list_node_t *view_node;
    uC_widget_view_t *view;

    if ((scr == NULL) || (vg == NULL))
    {
        return;
    }

    view_node = uC_list_scan(&vg->views, NULL);
    while (view_node != NULL)
    {
        view = view_node->payload;
        widget_register_view_shortcuts(scr, view);
        view_node = uC_list_scan(NULL, view_node);
    }
}

// -----------------------------------------------------------------------

static void widget_remove_one_shortcut(uC_widget_t *widget)
{
    if (widget == NULL)
    {
        return;
    }

    if (widget->shortcut_screen == NULL)
    {
        return;
    }

    uC_shortcut_remove_owner(widget->shortcut_screen, widget);
    widget->shortcut_screen = NULL;
}

// -----------------------------------------------------------------------

static void widget_remove_view_shortcuts(uC_widget_view_t *view)
{
    uC_list_node_t *widget_node;
    uC_widget_t *widget;

    if (view == NULL)
    {
        return;
    }

    widget_node = uC_list_scan(&view->widgets, NULL);
    while (widget_node != NULL)
    {
        widget = widget_node->payload;
        widget_remove_one_shortcut(widget);
        widget_node = uC_list_scan(NULL, widget_node);
    }
}

// -----------------------------------------------------------------------

static void widget_remove_shortcuts(uC_widget_vg_t *vg)
{
    uC_list_node_t *view_node;
    uC_widget_view_t *view;

    if (vg == NULL)
    {
        return;
    }

    view_node = uC_list_scan(&vg->views, NULL);
    while (view_node != NULL)
    {
        view = view_node->payload;
        widget_remove_view_shortcuts(view);
        view_node = uC_list_scan(NULL, view_node);
    }
}

// -----------------------------------------------------------------------

static void widget_set_vg_inactive(uC_widget_vg_t *vg, bool inactive)
{
    if (vg == NULL)
    {
        return;
    }

    if (inactive)
    {
        vg->flags |= uC_vg_flag_inactive;
    }
    else
    {
        vg->flags &= ~uC_vg_flag_inactive;
    }
}

// -----------------------------------------------------------------------

static void widget_modal_deactivate_others(uC_screen_t *scr,
    uC_widget_vg_t *popup)
{
    uC_list_node_t *node;
    uC_list_node_t *next;
    uC_widget_vg_t *vg;

    if ((scr == NULL) || (popup == NULL))
    {
        return;
    }

    node = uC_list_scan(&scr->view_groups, NULL);
    while (node != NULL)
    {
        next = uC_list_scan(NULL, node);
        vg = node->payload;
        if (vg == popup)
        {
            node = next;
            continue;
        }

        vg->popup_saved_inactive =
            (vg->flags & uC_vg_flag_inactive) != 0;
        widget_set_vg_inactive(vg, true);
        node = next;
    }
}

// -----------------------------------------------------------------------

static void widget_modal_restore_others(uC_screen_t *scr,
    uC_widget_vg_t *popup)
{
    uC_list_node_t *node;
    uC_list_node_t *next;
    uC_widget_vg_t *vg;

    if (scr == NULL)
    {
        return;
    }

    node = uC_list_scan(&scr->view_groups, NULL);
    while (node != NULL)
    {
        next = uC_list_scan(NULL, node);
        vg = node->payload;
        if (vg == popup)
        {
            node = next;
            continue;
        }

        widget_set_vg_inactive(vg, vg->popup_saved_inactive);
        node = next;
    }
}

// -----------------------------------------------------------------------

static bool widget_clear_focus_for_vg(uC_widget_vg_t *vg)
{
    if ((vg == NULL) || (widget_state.vg != vg))
    {
        return false;
    }

    vg->window.flags &= ~uC_WIN_FOCUS;
    if (widget_state.widget != NULL)
    {
        if (widget_state.widget->type == uC_WIDGET_TEXTBOX)
        {
            widget_state.widget->textbox.editing = false;
        }
        widget_state.widget->focused = false;
    }
    if (widget_state.view != NULL)
    {
        widget_state.view->view_node = NULL;
    }

    widget_state.vg = NULL;
    widget_state.view = NULL;
    widget_state.widget = NULL;
    widget_state.sequence = 0;

    return true;
}

// -----------------------------------------------------------------------

static void widget_vg_release_window(uC_widget_vg_t *vg)
{
    if (vg == NULL)
    {
        return;
    }

    if (vg->window.buffer != NULL)
    {
        uC_free(uC_MEM_ZONE_UI, vg->window.buffer);
        vg->window.buffer = NULL;
    }
    vg->window.screen = NULL;
}

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
        if (vg->window.screen != NULL)
        {
            uC_widget_vg_detach(vg->window.screen, vg);
        }

        rv = win_chk_pos(&vg->window, scr, vg->window.xco, vg->window.yco);
        if (rv != 0)
        {
            return;
        }

        rv = (vg->window.buffer != NULL) ? 0 : win_alloc(&vg->window);

        if (!rv)
        {
            vg->window.screen = scr;
            uC_win_clear(&vg->window);
            if (!uC_list_push_tail(&scr->view_groups, vg))
            {
                vg->window.screen = NULL;
                return;
            }
            widget_register_shortcuts(scr, vg);

            widget_state.screen = scr;

            if (!widget_state.sequence)
            {
                if (!uC_widget_select_widget(1))
                {
                    tab_next_widget();
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_detach(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    if (vg)
    {
        bool had_focus;

        if (scr == NULL)
        {
            scr = vg->window.screen;
        }

        if ((scr != NULL) && (scr->popup_vg == vg))
        {
            uC_widget_popup_detach(vg);
            return;
        }

        widget_remove_shortcuts(vg);
        had_focus = widget_clear_focus_for_vg(vg);

        if (scr != NULL)
        {
            uC_list_remove_node(&scr->view_groups, vg);
        }
        vg->window.screen = NULL;

        if (had_focus && (widget_state.screen != NULL))
        {
            uC_widget_select_widget(1);
        }
    }
}

// -----------------------------------------------------------------------

API void uC_widget_vg_close(uC_widget_vg_t *vg)
{
    uC_widget_view_t *view;

    if (vg)
    {
        uC_widget_vg_detach(vg->window.screen, vg);

        while (vg->views.count)
        {
            view = uC_list_pop_head(&vg->views);
            widget_close_view(view);
        }

        widget_vg_release_window(vg);
        uC_free(uC_MEM_ZONE_UI, vg);
    }
}

// -----------------------------------------------------------------------

API bool uC_widget_popup_attach(uC_screen_t *scr, uC_widget_vg_t *vg)
{
    int16_t rv;

    if ((scr == NULL) || (vg == NULL))
    {
        return false;
    }

    if (vg->window.screen != NULL)
    {
        uC_widget_vg_detach(vg->window.screen, vg);
    }

    if (scr->popup_vg != NULL)
    {
        uC_widget_popup_detach((uC_widget_vg_t *)scr->popup_vg);
    }

    rv = win_chk_pos(&vg->window, scr, vg->window.xco, vg->window.yco);
    if (rv != 0)
    {
        return false;
    }

    rv = (vg->window.buffer != NULL) ? 0 : win_alloc(&vg->window);
    if (rv != 0)
    {
        return false;
    }

    widget_clear_focus_for_vg(widget_state.vg);
    widget_modal_deactivate_others(scr, vg);
    widget_set_vg_inactive(vg, false);

    vg->window.screen = scr;
    uC_win_clear(&vg->window);
    scr->popup_vg = vg;
    widget_register_shortcuts(scr, vg);
    widget_state.screen = scr;

    if (!uC_widget_select_widget(1))
    {
        tab_next_widget();
    }

    return true;
}

// -----------------------------------------------------------------------

API void uC_widget_popup_detach(uC_widget_vg_t *vg)
{
    uC_screen_t *scr;
    bool had_focus;

    if (vg == NULL)
    {
        return;
    }

    scr = vg->window.screen;
    if ((scr == NULL) || (scr->popup_vg != vg))
    {
        return;
    }

    widget_remove_shortcuts(vg);
    had_focus = widget_clear_focus_for_vg(vg);
    scr->popup_vg = NULL;
    vg->window.screen = NULL;
    widget_modal_restore_others(scr, vg);

    if (had_focus && (widget_state.screen != NULL))
    {
        uC_widget_select_widget(1);
    }
}

// -----------------------------------------------------------------------

static bool widget_view_fits_group(uC_widget_vg_t *vg, uC_widget_view_t *v)
{
    uint32_t right;
    uint32_t bottom;

    if ((vg == NULL) || (v == NULL))
    {
        return false;
    }

    if ((v->flags & (1 << uC_VIEW_BOXED)) &&
        ((v->xco == 0) || (v->yco == 0)))
    {
        return false;
    }

    right = (uint32_t)v->xco + (uint32_t)v->width;
    bottom = (uint32_t)v->yco + (uint32_t)v->height;

    return (right <= (uint32_t)vg->window.width) &&
        (bottom <= (uint32_t)vg->window.height);
}

// -----------------------------------------------------------------------

API bool uC_widget_vg_add_view(uC_widget_vg_t *vg, uC_widget_view_t *v,
    uint16_t sequence)
{
    if (!widget_view_fits_group(vg, v))
    {
        return false;
    }

    if (sequence)
    {
        sync_seq(sequence);
    }
    v->sequence = sequence ? sequence : auto_sequence();
    return uC_list_push_tail(&vg->views, v);
}

#endif // UC_WIDGETS

// =======================================================================
