// uC_widget_radio.c
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
// draw a radio button

void draw_radio(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y)
{
    uint16_t c;

    if ((win != NULL) && (widget !=NULL))
    {
        c = ((*widget->radio.select & (1 << widget->radio.bit)))
            ? radio_on[widget->radio.type]
            : radio_off[widget->radio.type];

        widget_set_attrs(win, widget);

        // win->attrs = (widget->focused == true)
        //     ? widget->focus_attrs
        //     : widget->attrs;

        // %@ set cursor x / y within window
        // %* write multiple repetitions of char
        // %x set cursor x location on current window line
        // %8 emit single utf8 character
        // %s write string

        uC_win_printf(win, "%@%*%x%8 %s",
            x, y, widget->width, 0x20,
            x, c, widget->name);
    }
}

// -----------------------------------------------------------------------

uint8_t handle_radio(uint8_t k)
{
    uC_widget_t *widget;

    uint16_t bit;

    if (k == 0x20)
    {
        widget = widget_state.view->view_node->payload;
        bit    = widget->radio.bit;

        *widget->radio.select &= (1 << bit);
        *widget->radio.select ^= (1 << bit);
    }

    return k;
}

// -----------------------------------------------------------------------

API uC_widget_t *uC_widget_radio_create(uint16_t sequence,
    uint32_t *select, uint16_t bit, char *name,
    uC_radio_type_t type,
    uint16_t width, uint16_t xco, uint16_t yco,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *widget = create_widget(uC_WIDGET_RADIO,
        name, sequence, xco, yco, width, attrs, focus);

    if (widget != NULL)
    {
        widget->radio.type   = type;
        widget->radio.select = select;
        widget->radio.bit    = bit;
    }

    return widget;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================

