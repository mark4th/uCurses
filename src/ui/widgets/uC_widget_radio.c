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

// -----------------------------------------------------------------------

uint8_t handle_radio(uint8_t k)
{
    uC_widget_t *widget;

    uint16_t bit;

    if (k == 0x20)
    {
        widget = widget_state.view->view_node->payload;
        bit = widget->radio.bit;

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
    uC_widget_t *w = create_widget(uC_WIDGET_RADIO,
        name, sequence, xco, yco, width, attrs, focus);

    if (w != NULL)
    {
        w->radio.type   = type;
        w->radio.select = select;
        w->radio.bit    = bit;
    }

    return w;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================

