// uC_widget.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_widgets.h"
#include "uC_list.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------
// current tab selection sequence number

static uint16_t next_seq = 1;

// -----------------------------------------------------------------------

uint16_t auto_sequence(void)
{
    return next_seq++;
}

// -----------------------------------------------------------------------

void sync_seq(uint16_t seq)
{
    if (seq >= next_seq)
    {
        next_seq = seq + 1;
    }
}

// -----------------------------------------------------------------------

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
        widget->tag         = NULL;     // no 2nd segment unless set later
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
// drop the focus the widget_state currently holds, and drop it FROM THE
// WIDGET as well.
//
// ⚠⚠ the widget's own `focused` flag is what the draw reads, and nothing
// but this clears it - so any code that repoints widget_state at a new
// widget without coming through here STRANDS the old one lit.  two
// widgets then draw with their focus attributes at once and the display
// stops saying where the cursor is.
//
// ★ a textbox also stops editing.  it holds `editing` until something
// takes focus off it, and focus is being taken off it right now.

void widget_release_focus(void)
{
    if (widget_state.vg != NULL)
    {
        widget_state.vg->window.flags &= ~uC_WIN_FOCUS;
    }

    if (widget_state.widget != NULL)
    {
        if (widget_state.widget->type == uC_WIDGET_TEXTBOX)
        {
            widget_state.widget->textbox.editing = false;
        }
        widget_state.widget->focused = false;
    }

    if (widget_state.view != NULL)
    {
        widget_state.view->view_node = NULL;
    }
}

// -----------------------------------------------------------------------

static void widget_clear_focus(uC_widget_t *widget)
{
    if (widget == NULL)
    {
        return;
    }

    widget->focused = false;

    if (widget_state.widget != widget)
    {
        return;
    }

    if (widget_state.vg != NULL)
    {
        widget_state.vg->window.flags &= ~uC_WIN_FOCUS;
    }
    if (widget_state.view != NULL)
    {
        widget_state.view->view_node = NULL;
    }

    widget_state.widget = NULL;
    widget_state.view = NULL;
    widget_state.vg = NULL;
    widget_state.sequence = 0;
}

// -----------------------------------------------------------------------

void widget_detach_widget(uC_widget_t *widget)
{
    uC_widget_view_t *view;

    if (widget == NULL)
    {
        return;
    }

    if (widget->shortcut_screen != NULL)
    {
        uC_shortcut_remove_owner(widget->shortcut_screen, widget);
        widget->shortcut_screen = NULL;
    }

    widget_clear_focus(widget);

    view = widget->view;
    if (view != NULL)
    {
        uC_list_remove_node(&view->widgets, widget);
        widget->view = NULL;
    }
}

// -----------------------------------------------------------------------

API void uC_widget_close_widget(uC_widget_t *widget)
{
    if (widget != NULL)
    {
        widget_detach_widget(widget);
        uC_free(uC_MEM_ZONE_UI, widget);
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
