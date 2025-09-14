// uC_widget.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

uC_widget_t *create_widget(uC_widget_type_t type,
    char *name, uint16_t sequence,
    uint16_t xco, uint16_t yco, uint16_t width,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *w = uC_alloc(uC_MEM_ZONE_UI, sizeof(*w));

    if (w != NULL)
    {
        w->type         = type;
        w->name         = name;
        w->sequence     = sequence;
        w->xco          = xco;
        w->yco          = yco;
        w->attrs        = attrs;
        w->focus_attrs  = focus;
        w->width        = width;
    }

    return w;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
