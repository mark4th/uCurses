// uC_widget_check.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_win_printf.h"
#include "uC_widgets.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;
extern uint16_t radio_on[];
extern uint16_t radio_off[];

// -----------------------------------------------------------------------
// ★★★★★ HOW MANY OF A VIEW'S BOXES ARE TICKED - COUNTED, NEVER TALLIED.
//
// ⚠⚠ A TALLY WOULD BE A SECOND FACT.  a checkbox points at a bit the
// APPLICATION owns, and an application writes those bits itself - it is
// how a form is loaded from a record - so a counter maintained in here
// would be right until the first time anything else set one.  the bits
// are the state; this reads them.
//
// ⓘ one walk of one view's widget list, on a key press and on a draw.
// a view holds what fits on a screen.

static uint16_t view_checks_used(uC_widget_view_t *view)
{
    uC_list_node_t *n;
    uC_widget_t *w;
    uint16_t used = 0;

    if (view == NULL)
    {
        return 0;
    }

    n = uC_list_scan(&view->widgets, NULL);

    while (n != NULL)
    {
        w = (uC_widget_t *)n->payload;

        if ((w != NULL) && (w->type == uC_WIDGET_CHECK) &&
            (w->check.select != NULL) &&
            ((*w->check.select & (1u << w->check.bit)) != 0))
        {
            used++;
        }

        n = uC_list_scan(NULL, n);
    }

    return used;
}

// -----------------------------------------------------------------------

API uint16_t uC_widget_view_checks_used(uC_widget_view_t *view)
{
    return view_checks_used(view);
}

// -----------------------------------------------------------------------
// ★★★★ WOULD TICKING THIS BOX BE REFUSED - and it is ONE function
// because the draw and the key handler must not answer it differently.
// a box that ghosts and then toggles anyway is worse than no bound at
// all.
//
// ⚠ OFF IS NEVER REFUSED.  a full list you cannot untick is a trap
// rather than a bound - there would be no way back out of it.

API bool uC_widget_check_refused(uC_widget_t *widget)
{
    uC_widget_view_t *view;

    if ((widget == NULL) || (widget->type != uC_WIDGET_CHECK) ||
        (widget->check.select == NULL))
    {
        return false;
    }

    view = widget->view;

    if ((view == NULL) || (view->check_max == 0))
    {
        return false;
    }

    if ((*widget->check.select & (1u << widget->check.bit)) != 0)
    {
        return false;
    }

    return (view_checks_used(view) >= view->check_max);
}

// -----------------------------------------------------------------------
// ⚠ GHOSTED, NOT DISABLED - the row keeps its place in the tab order
// and its focus bar, and only the ink dims.  ★ the textbox draw reads
// grey the same way: *"grey says disabled"*, which is exactly what a box
// that cannot be ticked has to say.
//
// ⓘ TOWARDS THE BACKGROUND rather than towards black, so it dims on a
// light theme as well as on a dark one.

static void check_ghost(uC_window_t *win)
{
    uint16_t fg;
    uint16_t bg;

    if ((win->attrs.flags.bits & uC_ATTR_FLAG_GRAY_FG) != 0)
    {
        fg = win->attrs.fg_gray;

        bg = ((win->attrs.flags.bits & uC_ATTR_FLAG_GRAY_BG) != 0)
            ? win->attrs.bg_gray
            : 0;

        uC_win_set_gray_fg(win, (uint8_t)((fg + bg) / 2));
        return;
    }

    uC_win_set_fg(win, uC_COLOR_GRAY);
}

// -----------------------------------------------------------------------
// draw checkbox widget

void draw_check(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y)
{
    uint16_t c;
    uint16_t remaining;

    c = ((*widget->check.select & (1 << widget->check.bit)))
       ? radio_on[widget->check.type]
       : radio_off[widget->check.type];

    widget_set_attrs(win, widget);

    if (uC_widget_check_refused(widget))
    {
        check_ghost(win);
    }

    remaining = widget_clear_width(win, x, y, widget->width);

    // optional tag segment, drawn BEFORE the checkbox in its own fg colour
    // (its bg is left as-is so it shares the focus / field background)
    if (widget->tag != NULL)
    {
        uC_attribs_t save = win->attrs;

        (widget->tag_attrs.flags.bits & uC_ATTR_FLAG_GRAY_FG)
            ? uC_win_set_gray_fg(win, widget->tag_attrs.fg_gray)
            : uC_win_set_fg(win, widget->tag_attrs.fg);

        widget_puts_clipped(win, widget->tag, &remaining);
        win->attrs = save;
    }

    if (widget_emit_clipped(win, c, &remaining))
    {
        widget_emit_clipped(win, 0x20, &remaining);
        widget_puts_clipped(win, widget->name, &remaining);
    }
}

// -----------------------------------------------------------------------

uint8_t handle_check(uint8_t k)
{
    uC_widget_t *widget;

    uint16_t bit;

    if (!widget_state.view->view_node)
    {
        return k;
    }
    if (k == 0x20)
    {
        widget = widget_state.view->view_node->payload;

        // ⚠⚠ THE CAP IS ENFORCED HERE AND NOWHERE ELSE.  the mouse
        // arrives through this same call, so one test covers both ways
        // a box can be pressed.

        if (uC_widget_check_refused(widget))
        {
            return k;
        }

        bit = widget->check.bit;

        *widget->check.select ^= (1 << bit);
    }

    return k;
}

// -----------------------------------------------------------------------

API uC_widget_t *uC_widget_check_create(
    uint32_t *select, uint16_t bit, const char *name,
    uC_radio_type_t type, uint16_t width,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *w = create_widget(uC_WIDGET_CHECK,
        name, width, attrs, focus);

    if (w)
    {
        w->check.type   = type;
        w->check.select = select;
        w->check.bit    = bit;
    }

    return w;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
