// uC_widget.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

// type     = BUTTON, RADIO, CHECK, TEXTBOX
// sequence = selection sequence on tab key

uC_widget_t *create_widget(uC_widget_type_t type, char *name,
    uint16_t sequence, uint16_t xco, uint16_t yco, uint16_t width,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *widget = uC_alloc(uC_MEM_ZONE_UI, sizeof(*widget));

    if (widget != NULL)
    {
        widget->type         = type;
        widget->name         = name;
        widget->sequence     = sequence;
        widget->xco          = xco;
        widget->yco          = yco;
        widget->attrs        = attrs;
        widget->focus_attrs  = focus;
        widget->width        = width;
    }

    return widget;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
