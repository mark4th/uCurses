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

static uint16_t view_widget_width(uC_widget_view_t *view)
{
    uC_list_node_t *node;
    uC_widget_t *widget;

    if (view == NULL)
    {
        return 0;
    }

    node = uC_list_scan(&view->widgets, NULL);
    if (node == NULL)
    {
        return 0;
    }

    widget = (uC_widget_t *)node->payload;
    return widget->width;
}

// -----------------------------------------------------------------------

static uint16_t view_visible_widgets(uC_widget_view_t *view)
{
    uint16_t visible;
    uint16_t width;

    if ((view == NULL) || (view->widgets.count == 0))
    {
        return 0;
    }

    if (view->orientation == uC_VIEW_HORIZONTAL)
    {
        width = view_widget_width(view);
        visible = (width != 0) ? (view->width / width) : 0;
    }
    else
    {
        visible = view->height;
    }

    if (visible > view->widgets.count)
    {
        visible = view->widgets.count;
    }

    return visible;
}

// -----------------------------------------------------------------------

static bool view_horizontal_widgets_valid(uC_widget_view_t *view)
{
    uint16_t width = 0;
    uC_list_node_t *node;
    uC_widget_t *widget;

    if ((view == NULL) || !(view->flags & (1 << uC_VIEW_SCROLL)) ||
        (view->width == 0))
    {
        return false;
    }

    node = uC_list_scan(&view->widgets, NULL);
    while (node)
    {
        widget = (uC_widget_t *)node->payload;

        if ((widget->type == uC_WIDGET_TEXTBOX) ||
            (widget->height != 1) || (widget->width == 0) ||
            (widget->width > view->width))
        {
            return false;
        }

        if (width == 0)
        {
            width = widget->width;
        }
        else if (widget->width != width)
        {
            return false;
        }

        node = uC_list_scan(NULL, node);
    }

    return true;
}

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

API bool uC_widget_view_set_orientation(uC_widget_view_t *view,
    uC_view_orientation_t orientation)
{
    uint16_t index;
    widget_state_t saved_state;

    if ((view == NULL) ||
        ((orientation != uC_VIEW_VERTICAL) &&
         (orientation != uC_VIEW_HORIZONTAL)))
    {
        return false;
    }

    if ((orientation == uC_VIEW_HORIZONTAL) &&
        !view_horizontal_widgets_valid(view))
    {
        return false;
    }

    if (view->orientation == orientation)
    {
        return true;
    }

    index = uC_widget_view_index(view);
    view->orientation = orientation;
    view->top = 0;
    view->cy = 0;

    if (view->widgets.count != 0)
    {
        saved_state = widget_state;
        uC_widget_to_view_index(view, index);
        if (saved_state.view != view)
        {
            widget_state = saved_state;
        }
    }

    return true;
}

// -----------------------------------------------------------------------

API bool uC_widget_view_add_widget(uC_widget_view_t *view,
    uC_widget_t *widget, uint16_t sequence)
{
    uC_list_node_t *n1;
    uC_widget_t *w2 = NULL;

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

    if (view->orientation == uC_VIEW_HORIZONTAL)
    {
        if ((widget->type == uC_WIDGET_TEXTBOX) ||
            (widget->height != 1) || (widget->width == 0) ||
            (widget->width > view->width))
        {
            return false;
        }

        if (view->widgets.count && (widget->width != w2->width))
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
    if ((view != NULL) && (widget != NULL) && (widget->view == view))
    {
        widget_detach_widget(widget);
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
// move the focus to the previous widget in a scrollable view

static void view_previous(void)
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

// move the focus to the next widget in a scrollable view

static void view_next(void)
{
    uint16_t index;
    uint16_t visible;
    uC_widget_view_t *view = widget_state.view;

    visible = view_visible_widgets(view);
    if ((visible == 0) || (view->view_node == NULL) ||
        (view->view_node->next == NULL))
    {
        return;
    }

    index = view->top + view->cy;

    if (index >= view->widgets.count - 1)
    {
        return;
    }

    if (view->cy + 1 < visible)
    {
        view->cy++;
    }
    else
    {
        view->top++;
    }

    view->view_node = view->view_node->next;
}

// -----------------------------------------------------------------------

void widget_scroll_view(uint8_t k)
{
    uC_widget_view_t *view = widget_state.view;

    if (view == NULL)
    {
        return;
    }

    if (view->orientation == uC_VIEW_HORIZONTAL)
    {
        if (k == WIDGET_KEY_LEFT)
        {
            view_previous();
        }
        else if (k == WIDGET_KEY_RIGHT)
        {
            view_next();
        }
    }
    else if (k == WIDGET_KEY_UP)
    {
        view_previous();
    }
    else if (k == WIDGET_KEY_DOWN)
    {
        view_next();
    }

    if (view->view_node)
    {
        widget_state.widget = view->view_node->payload;
    }
}

// -----------------------------------------------------------------------
// give the widget at the specified index within a scrollable view focus

API void uC_widget_to_view_index(uC_widget_view_t *view, uint16_t index)
{
    uint16_t i;
    uint16_t visible;
    uint16_t max_top;
    uC_list_node_t *node;

    // call is only valid if view is scrollable

    if (!view || !(view->flags & (1 << uC_VIEW_SCROLL)))
    {
        return;
    }

    if (!view->widgets.head)
    {
        return;
    }

    if (index >= view->widgets.count)
    {
        index = view->widgets.count - 1;
    }

    visible = view_visible_widgets(view);
    if (visible == 0)
    {
        return;
    }

    max_top = view->widgets.count - visible;

    if (view->top > max_top)
    {
        view->top = max_top;
    }

    // Keep the current viewport when the target is already visible.
    // Otherwise scroll only far enough to reveal it at the nearest edge.

    if (index < view->top)
    {
        view->top = index;
        view->cy = 0;
    }
    else if (index >= view->top + visible)
    {
        view->top = index - visible + 1;
        view->cy = visible - 1;
    }
    else
    {
        view->cy = index - view->top;
    }

    node = view->widgets.head;
    for (i = 0; (i < index) && node; i++)
    {
        node = node->next;
    }

    if (node)
    {
        view->view_node       = node;
        widget_state.widget   = node->payload;
        widget_state.view     = view;
        widget_state.sequence = widget_state.widget->sequence;
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
