// uC_widget_view.c - widget views, containers for widgets
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_widgets.h"
#include "uC_borders.h"
#include "uC_list.h"
#include "uC_alloc.h"
#include "uC_win_printf.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------
// create a widget view as a container for one type of widget

API uC_widget_view_t *uC_widget_view_create(const char *name,
    uint16_t width, uint16_t height, uint16_t xco, uint16_t yco,
    uC_attribs_t attrs, bool scroll)
{
    uC_widget_view_t *view = uC_alloc(uC_MEM_ZONE_UI, sizeof(*view));

    if (view)
    {
        if (name)
        {
            view->name   = name;
            view->flags |= (1 << uC_VIEW_NAMED);
        }

        if (scroll)
        {
            view->flags |= (1 << uC_VIEW_SCROLL);
        }

        view->attrs   = attrs;
        view->width   = width;
        view->height  = height;
        view->xco     = xco;
        view->yco     = yco;
    }

    return view;
}

// -----------------------------------------------------------------------
// give a view a border. user must ensure there is space for it in window

API void uC_widget_view_add_border(uC_widget_view_t *view,
    uC_border_type_t bdr_type, uC_attribs_t bdr_attrs)
{
    if (view)
    {
        view->flags    |= (1 << uC_VIEW_BOXED);
        view->box_attrs = bdr_attrs;
        view->box_type  = bdr_type;
    }
}

// -----------------------------------------------------------------------

API bool uC_widget_view_add_widget(uC_widget_view_t *view,
    uC_widget_t *widget, uint16_t sequence)
{
    uC_list_node_t *n1;
    uC_widget_t *w2;

    if (!view || !widget)
    {
        return false;
    }

    // all widgets added to the view must be of the same type
    // as the first widget added to the view

    if (view->widgets.count)
    {
        n1 = uC_list_scan(&view->widgets, NULL);
        w2 = (uC_widget_t *)n1->payload;

        if (widget->type != w2->type)
        {
            return false;
        }
    }

    // scrollable views get one tab stop for the whole view,
    // set via uC_widget_vg_add_view — all widgets share that sequence

    if (view->flags & (1 << uC_VIEW_SCROLL))
    {
        widget->sequence = view->sequence;
    }
    else
    {
        if (sequence)
        {
            sync_seq(sequence);
        }
        widget->sequence = sequence ? sequence : auto_sequence();
    }

    widget->view = view;
    return uC_list_push_tail(&view->widgets, widget);
}

// -----------------------------------------------------------------------

API void uC_widget_view_remove_widget(uC_widget_view_t *view,
    uC_widget_t *widget)
{
    if (view)
    {
        uC_list_remove_node(&view->widgets, widget);
    }
}

// -----------------------------------------------------------------------

void widget_close_view(uC_widget_view_t *view)
{
    uC_widget_t *widget;

    while (view->widgets.count)
    {
        widget = uC_list_pop_head(&view->widgets);
        uC_widget_close_widget(widget);
    }

    uC_free(uC_MEM_ZONE_UI, view);
}

// -----------------------------------------------------------------------
// move the focus up one line in a scrollable view

static void view_up(void)
{
    uC_widget_view_t *view = widget_state.view;

    if (view->cy)
    {
        view->cy--;
    }
    else if (view->top)
    {
        view->top--;
    }
    else
    {
        return;
    }

    if (view->view_node->prev)
    {
        widget_state.view->view_node = view->view_node->prev;
    }
}

// -----------------------------------------------------------------------

static void next(void)
{
    if (widget_state.view->view_node->next)
    {
        widget_state.view->view_node =
            widget_state.view->view_node->next;
    }
}

// -----------------------------------------------------------------------
// move the focus down one line in a scrollable view

static void view_down(void)
{
    uint16_t n;
    uint16_t min;

    uC_widget_view_t *view;

    view = widget_state.view;

    min = (view->height < view->widgets.count)
        ? view->height
        : view->widgets.count;

    if (view->cy != min - 1)
    {
        view->cy++;
    }
    else
    {
        n = view->top + view->cy;

        if (n == view->widgets.count - 1)
        {
            return;
        }
        view->top++;
    }
    next();
}

// -----------------------------------------------------------------------

void widget_scroll_view(uint8_t k)
{
    switch (k)
    {
        case WIDGET_KEY_UP:   view_up();    break;
        case WIDGET_KEY_DOWN: view_down();  break;
    }

    if (widget_state.view->view_node)
    {
        widget_state.widget = widget_state.view->view_node->payload;
    }
}

// -----------------------------------------------------------------------
// give the widget at the specified index within a scrollable view focus

API void uC_widget_to_view_index(uC_widget_view_t *view, uint16_t index)
{
    int i;

    // call is only valid if view is scrollable

    if (!(view->flags & (1 << uC_VIEW_SCROLL)))
    {
        return;
    }

    if (!view->widgets.head)
    {
        return;
    }

    view->top             = 0;
    view->cy              = 0;
    view->view_node       = view->widgets.head;

    widget_state.widget   = view->view_node->payload;
    widget_state.view     = view;
    widget_state.sequence = widget_state.widget->sequence;

    for (i = 0; i != index ; i++)
    {
        widget_scroll_view(WIDGET_KEY_DOWN);
    }
}

// -----------------------------------------------------------------------

API void uC_widget_view_set_position(uC_widget_view_t *view,
    uint16_t xco, uint16_t yco)
{
    if (view)
    {
        view->xco = xco;
        view->yco = yco;
    }
}

// -----------------------------------------------------------------------

API uint16_t uC_widget_view_index(uC_widget_view_t *view)
{
    uint32_t index;

    if (!view || !view->widgets.count)
    {
        return 0;
    }

    index = view->top + view->cy;

    if (index >= view->widgets.count)
    {
        index = view->widgets.count - 1;
    }

    return (uint16_t)index;
}


// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
