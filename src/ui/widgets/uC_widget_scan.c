// uC_widget_scan.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern uC_screen_t *active_screen;
extern widget_state_t widget_state;

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

    n1 = uC_list_scan(&active_screen->view_groups, NULL);

    while (n1)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        if (!(vg->flags & uC_vg_flag_ignore))
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
// find the highest sequence number among all active widgets

static uint16_t max_sequence(void)
{
    uint16_t max = 0;
    uC_list_node_t *n1, *n2, *n3;
    uC_widget_vg_t *vg;
    uC_widget_view_t *view;
    uC_widget_t *widget;

    n1 = uC_list_scan(&active_screen->view_groups, NULL);

    while (n1)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        if (!(vg->flags & uC_vg_flag_ignore))
        {
            n2 = uC_list_scan(&vg->views, NULL);

            while (n2)
            {
                view = (uC_widget_view_t *)n2->payload;
                n3 = uC_list_scan(&view->widgets, NULL);

                while (n3)
                {
                    widget = (uC_widget_t *)n3->payload;

                    if (widget->sequence > max)
                    {
                        max = widget->sequence;
                    }

                    n3 = uC_list_scan(NULL, n3);
                }

                n2 = uC_list_scan(NULL, n2);
            }
        }

        n1 = uC_list_scan(NULL, n1);
    }

    return max;
}

// -----------------------------------------------------------------------
// tab key: advance to next widget, wrapping from last back to first

uint8_t tab_next_widget(void)
{
    uint16_t sequence = widget_state.sequence + 1;

    widget_state.sequence = 0;

    if (!uC_widget_select_widget(sequence))
    {
        uC_widget_select_widget(1);
    }

    return 0x09;
}

// -----------------------------------------------------------------------
// shift-tab: go to previous widget, wrapping from first back to last

uint8_t tab_prev_widget(void)
{
    uint16_t current  = widget_state.sequence;
    uint16_t sequence = (current > 1) ? current - 1 : max_sequence();

    widget_state.sequence = 0;
    uC_widget_select_widget(sequence);

    return 0x88;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
