// uC_widget.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

static uint16_t next_seq = 1;

uint16_t auto_sequence(void)
{
    return next_seq++;
}

void sync_seq(uint16_t seq)
{
    if (seq >= next_seq)
        next_seq = seq + 1;
}

API void uC_widget_reset_sequence(void)
{
    next_seq = 1;
}

// -----------------------------------------------------------------------

uC_widget_t *create_widget(uC_widget_type_t type, const char *name,
    uint16_t width, uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *widget = uC_alloc(uC_MEM_ZONE_UI, sizeof(*widget));

    if (widget)
    {
        widget->type        = type;
        widget->name        = name;
        widget->attrs       = attrs;
        widget->focus_attrs = focus;
        widget->width       = width;
        widget->height      = 1;
    }

    return widget;
}

// -----------------------------------------------------------------------

API void uC_widget_set_position(uC_widget_t *widget,
    uint16_t xco, uint16_t yco)
{
    if (widget)
    {
        widget->xco = xco;
        widget->yco = yco;
    }
}

// -----------------------------------------------------------------------

API void uC_widget_close_widget(uC_widget_t *widget)
{
    uC_widget_view_t *view;

    if (widget)
    {
        view = widget->view;

        if (view)
        {
            uC_list_remove_node(&view->widgets, widget);
        }

        uC_free(uC_MEM_ZONE_UI, widget);
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
