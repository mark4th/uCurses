// uC_widget.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"
#include "uC_list.h"

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

        // this is one for all widgets except the uneditable
        // text widget which can be multi line high
        // this element is fixed by caller if it is a text
        // widget

        widget->height       = 1;
    }

    return widget;
}

// -----------------------------------------------------------------------

API void uC_widget_close_widget(uC_widget_t *widget)
{
    uC_widget_view_t *view;

    if (widget != NULL)
    {
        view = widget->view;

        if (view != NULL)
        {
            uC_list_remove_node(&view->widgets, widget);
        }

        uC_free(uC_MEM_ZONE_UI, widget);
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
