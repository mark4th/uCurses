// uC_widget_scan.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

// every widget must have a unique tab select sequence value.  these
// values must start at 1 and have no gaps.   the tab sequence is utterly
// independent of which view or view group a widgit is associated with.
// if the widget is within a view or view group that is currently attached
// to the active_screen then these functions will find it.

// when a widgit is associated with a scrollable view then it is the
// entire view that the tab sequence is associated with.  However, the tab
// sequence value is held in the first widget linked to that view.  the
// tab sequence value of every other widgit in that view is ignored.

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern uC_screen_t *active_screen;
extern widget_state_t widget_state;

// -----------------------------------------------------------------------
// scan all widgets within view for a widget whith given sequence number

static bool scan_view(uC_widget_view_t *view, uint16_t sequence)
{
    uC_list_node_t *n1;
    uC_widget_t *widget;

    n1 = uC_list_scan(&view->widgets, NULL);

    while (n1 != NULL)
    {
        widget = (uC_widget_t *)n1->payload;
        widget->focused = false;

        if (widget->sequence == sequence)
        {
            widget->focused = true;
            widget_state.widget = widget;

            if (view->view_node == NULL)
            {
                view->view_node = n1;
            }

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

    while (n1 != NULL)
    {
        view = (uC_widget_view_t *)n1->payload;

        f = scan_view(view, sequence);
        if (f == true)
        {
            widget_state.view = view;
            return true;
        }

        n1 = uC_list_scan(NULL, n1);
    }
    return false;
}

// -----------------------------------------------------------------------
// scan all view groups for widget with given sequence number

static bool find_widget(uint16_t sequence)
{
    bool f = false;
    bool result;

    uC_list_node_t *n1;
    uC_widget_vg_t *vg;

    n1 = uC_list_scan(&active_screen->view_groups, NULL);

    while (n1 != NULL)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        // assume that this view group does not contain the
        // widget that is receiving focus

        vg->window.flags &= ~WIN_FOCUS;

        if (f == false)
        {
            f = scan_vg(vg, sequence);

            if (f == true)
            {
                result = f;
                vg->window.flags |= WIN_FOCUS;
                widget_state.vg = vg;
                widget_state.sequence = sequence;
            }
        }
        n1 = uC_list_scan(NULL, n1);
    }

    uC_scr_draw_screen(active_screen);

    return result;
}

// -----------------------------------------------------------------------

void tab_next_widget(void)
{
    bool f;

    uint16_t sequence = widget_state.sequence + 1;
    widget_state.sequence = 0;

    f = find_widget(sequence);

    if (f == false)
    {
        find_widget(0);
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
