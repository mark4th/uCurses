// uC_widget_button.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_widgets.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------
// todo (incoming)

uint8_t handle_text(uint8_t k)
{
    (void) k;
    return k;
}

// -----------------------------------------------------------------------

API uC_widget_t *uC_widget_text_create(
    uint16_t sequence, char *data, char *name,
    uint16_t size, uC_textbox_radix_t radix,
    uint16_t width, uint8_t xco, uint8_t yco,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *w = create_widget(uC_WIDGET_TEXTBOX,
        name, sequence, xco, yco, width, attrs, focus);

    if (w != NULL)
    {
        w->textbox.data   = data;
        w->textbox.size   = size;
        w->textbox.radix  = radix;
    }

    return w;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
