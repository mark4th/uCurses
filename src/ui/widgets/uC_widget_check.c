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

    remaining = widget_clear_width(win, x, y, widget->width);
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
        bit    = widget->check.bit;

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
