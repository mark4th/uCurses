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

API uC_widget_view_t *uC_widget_view_create(uint8_t *name,
    uint16_t width, uint16_t height, uint16_t xco, uint16_t yco,
    uC_attribs_t attrs, bool scroll)
{
    uC_widget_view_t *view = uC_alloc(uC_MEM_ZONE_UI, sizeof(*view));

    if (view != NULL)
    {
        if (name != NULL)
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
    if (view != NULL)
    {
        view->flags    |= (1 << uC_VIEW_BOXED);
        view->box_attrs = bdr_attrs;
        view->box_type  = bdr_type;
    }
}

// -----------------------------------------------------------------------

API bool uC_widget_view_add_widget(uC_widget_view_t *view,
    uC_widget_t *widget)
{
    uC_list_node_t *n1;
    uC_widget_t *w2;

    if ((view == NULL) || (widget == NULL))
    {
        return false;
    }

    // all widgets added to the view must be of the same type
    // as the first widget added to the view

    if (view->widgets.count != 0)
    {
        n1 = uC_list_scan(&view->widgets, NULL);
        w2 = (uC_widget_t *)n1->payload;

        if (widget->type != w2->type)
        {
            return false;
        }
    }

    if (view->flags & (1 << uC_VIEW_SCROLL))
    {
        if (view->sequence == 0)
        {
            view->sequence = widget->sequence;
        }
    }

    widget->view = view;
    return uC_list_push_tail(&view->widgets, widget);
}

// -----------------------------------------------------------------------

void widget_close_view(uC_widget_view_t *view)
{
    uC_widget_t *widget;

    do
    {
        widget = uC_list_pop_head(&view->widgets);
        uC_free(uC_MEM_ZONE_UI, widget);
    } while (widget != NULL);

    uC_free(uC_MEM_ZONE_UI, view);
}

// -----------------------------------------------------------------------
// move the focus up one line in a scrollable view

static void view_up(void)
{
    if (widget_state.view->cy != 0)
    {
        widget_state.view->cy--;
        widget_state.view->view_node =
            widget_state.view->view_node->prev;
    }
    else if (widget_state.view->top != 0)
    {
        widget_state.view->top--;
        widget_state.view->view_node =
            widget_state.view->view_node->prev;
    }
}

// -----------------------------------------------------------------------

static void next(void)
{
    if (widget_state.view->view_node != NULL)
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
    uC_widget_view_t *view;

    view = widget_state.view;

    if ((view->cy != view->height - 1) &&
        (view->cy != view->widgets.count - 1))
    {
        view->cy++;
        next();
    }
    else
    {
        n = view->top + view->cy;

        if (n != view->widgets.count - 1)
        {
            view->top++;
            next();
        }
    }
}

// -----------------------------------------------------------------------

void widget_scroll_view(uint8_t k)
{
    switch (k)
    {
        case WIDGET_KEY_UP:      view_up();    break;
        case WIDGET_KEY_DOWN:    view_down();  break;
    }

    if (widget_state.view->view_node != NULL)
    {
        widget_state.widget = widget_state.view->view_node->payload;
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
