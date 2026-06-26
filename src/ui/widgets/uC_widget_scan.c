// uC_widget_scan.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------

static void scan_tab_candidate_in_vg(uC_widget_vg_t *vg, uint16_t current,
    bool forward, uint16_t *candidate, uint16_t *wrap)
{
    uC_list_node_t *n2, *n3;
    uC_widget_view_t *view;
    uC_widget_t *widget;

    if ((vg == NULL) || (candidate == NULL) || (wrap == NULL) ||
        (vg->flags & uC_vg_flag_inactive))
    {
        return;
    }

    n2 = uC_list_scan(&vg->views, NULL);
    while (n2)
    {
        view = (uC_widget_view_t *)n2->payload;
        n3 = uC_list_scan(&view->widgets, NULL);

        while (n3)
        {
            widget = (uC_widget_t *)n3->payload;
            if (widget->disabled)
            {
                n3 = uC_list_scan(NULL, n3);
                continue;
            }

            if (forward)
            {
                if (((*wrap == 0) || (widget->sequence < *wrap)))
                {
                    *wrap = widget->sequence;
                }
                if ((widget->sequence > current) &&
                    ((*candidate == 0) || (widget->sequence < *candidate)))
                {
                    *candidate = widget->sequence;
                }
            }
            else
            {
                if (widget->sequence > *wrap)
                {
                    *wrap = widget->sequence;
                }
                if ((widget->sequence < current) &&
                    (widget->sequence > *candidate))
                {
                    *candidate = widget->sequence;
                }
            }

            n3 = uC_list_scan(NULL, n3);
        }

        n2 = uC_list_scan(NULL, n2);
    }
}

// -----------------------------------------------------------------------
// scan widgets within view for one whith specified sequence number

static bool scan_view(uC_widget_view_t *view, uint16_t sequence)
{
    uC_list_node_t *n1;
    uC_widget_t *widget;

    n1 = uC_list_scan(&view->widgets, NULL);

    while (n1)
    {
        widget = (uC_widget_t *)n1->payload;

        if (widget->sequence == sequence)
        {
            if (widget->disabled)
            {
                return false;
            }
            widget->focused = true;
            widget_state.widget = widget;

            // this is saved here so we can cursor up and down
            // through the list in a scrollable view

            view->view_node = n1;

            return true;
        }

        n1 = uC_list_scan(NULL, n1);
    }

    return false;
}

// -----------------------------------------------------------------------
// scan all views within view group for widget with given sequence number

static bool scan_vg(uC_widget_vg_t *vg, uint16_t sequence)
{
    bool f;

    uC_list_node_t *n1;
    uC_widget_view_t *view;

    n1 = uC_list_scan(&vg->views, NULL);

    while (n1)
    {
        view = (uC_widget_view_t *)n1->payload;

        f = scan_view(view, sequence);

        if (f)
        {
            vg->window.flags     |= uC_WIN_FOCUS;

            widget_state.view     = view;
            widget_state.vg       = vg;
            widget_state.sequence = sequence;

            return true;
        }

        n1 = uC_list_scan(NULL, n1);
    }
    return false;
}

// -----------------------------------------------------------------------
// scan all view groups for widget with given sequence number

API bool uC_widget_select_widget(uint16_t sequence)
{
    bool f = false;

    uC_list_node_t *n1;
    uC_widget_vg_t *vg;

    // remove focus from currently focussed view group window etc.

    if (widget_state.vg)
    {
        widget_state.vg->window.flags &= ~uC_WIN_FOCUS;
        if ((widget_state.widget != NULL) &&
            (widget_state.widget->type == uC_WIDGET_TEXTBOX))
        {
            widget_state.widget->textbox.editing = false;
        }
        widget_state.widget->focused   = false;

        // this is a pointer to the list node of the widget
        // within a scrollable view that has the focus.  scrolling
        // up and down simply means doing node->next or node->prev.
        // if the view losing focus is not actually a scrollable then
        // meh
        //
        // all widgets within a scrollable view share the same
        // tab sequence number which is stored in the first
        // widget attached to the view.

        widget_state.view->view_node = NULL;
    }

    // scan through all view groups, all views within those
    // view groups and all widgets within those views...

    if ((widget_state.screen != NULL) &&
        (widget_state.screen->popup_vg != NULL))
    {
        vg = (uC_widget_vg_t *)widget_state.screen->popup_vg;
        if (!(vg->flags & uC_vg_flag_inactive))
        {
            f = scan_vg(vg, sequence);
        }
    }

    n1 = f ? NULL : uC_list_scan(&widget_state.screen->view_groups, NULL);

    while (n1)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        if (!(vg->flags & uC_vg_flag_inactive))
        {
            f = scan_vg(vg, sequence);

            if (f)
            {
                break;
            }
        }
        n1 = uC_list_scan(NULL, n1);
    }

    if (!f)
    {
        widget_state.vg     = NULL;
        widget_state.view   = NULL;
        widget_state.widget = NULL;
    }

    return f;
}

// -----------------------------------------------------------------------
// choose the next/previous real active widget sequence. Sequences may be
// sparse, so this scans actual widgets once instead of probing every integer
// between the current sequence and the highest assigned sequence.

static uint16_t tab_candidate(bool forward)
{
    uint16_t current = widget_state.sequence;
    uint16_t candidate = 0;
    uint16_t wrap = 0;
    uC_list_node_t *n1;
    uC_widget_vg_t *vg;

    if ((widget_state.screen != NULL) &&
        (widget_state.screen->popup_vg != NULL))
    {
        scan_tab_candidate_in_vg(
            (uC_widget_vg_t *)widget_state.screen->popup_vg, current,
            forward, &candidate, &wrap);
        return candidate ? candidate : wrap;
    }

    n1 = uC_list_scan(&widget_state.screen->view_groups, NULL);

    while (n1)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        scan_tab_candidate_in_vg(vg, current, forward, &candidate, &wrap);

        n1 = uC_list_scan(NULL, n1);
    }

    return candidate ? candidate : wrap;
}

// -----------------------------------------------------------------------
// tab key: advance to next widget, wrapping from last back to first

uint8_t tab_next_widget(void)
{
    uint16_t sequence = tab_candidate(true);

    widget_state.sequence = 0;
    if (sequence != 0)
    {
        uC_widget_select_widget(sequence);
    }

    return 0x09;
}

// -----------------------------------------------------------------------
// shift-tab: go to previous widget, wrapping from first back to last

uint8_t tab_prev_widget(void)
{
    uint16_t sequence = tab_candidate(false);

    widget_state.sequence = 0;
    if (sequence != 0)
    {
        uC_widget_select_widget(sequence);
    }

    return UC_KEY_BACKTAB;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
