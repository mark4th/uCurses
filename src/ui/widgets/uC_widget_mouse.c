// uC_widget_mouse.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_mouse.h"
#include "uC_widgets.h"
#include "uC_screen.h"
#include "uC_list.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------
// walk vg → views → widgets to find which widget (if any) contains (sx,sy)
// returns the widget's sequence number, or 0 if none hit

static uint16_t widget_hit_test(int16_t sx, int16_t sy)
{
    uC_list_node_t   *n1, *n2, *n3;
    uC_widget_vg_t   *vg;
    uC_widget_view_t *view;
    uC_widget_t      *widget;

    int16_t vg_inner_x;
    int16_t vg_inner_y;
    int16_t view_inner_x;
    int16_t view_inner_y;
    int16_t view_inner_h;
    int16_t wx, wy;
    int16_t boxed;

    n1 = uC_list_scan(&widget_state.screen->view_groups, NULL);

    while (n1)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        if (vg->flags & uC_vg_flag_inactive)
        {
            n1 = uC_list_scan(NULL, n1);
            continue;
        }

        vg_inner_x = vg->window.xco +
            ((vg->window.flags & uC_WIN_BOXED) ? 1 : 0);
        vg_inner_y = vg->window.yco +
            ((vg->window.flags & uC_WIN_BOXED) ? 1 : 0);

        n2 = uC_list_scan(&vg->views, NULL);

        while (n2)
        {
            view  = (uC_widget_view_t *)n2->payload;
            boxed = (view->flags & (1 << uC_VIEW_BOXED)) ? 1 : 0;

            view_inner_x = vg_inner_x + view->xco + boxed;
            view_inner_y = vg_inner_y + view->yco + boxed;
            view_inner_h = view->height - (boxed ? 2 : 0);

            n3 = uC_list_scan(&view->widgets, NULL);

            while (n3)
            {
                widget = (uC_widget_t *)n3->payload;

                if (!widget->disabled &&
                    widget->yco >= view->top &&
                    widget->yco < view->top + view_inner_h)
                {
                    wx = view_inner_x + (int16_t)widget->xco;
                    wy = view_inner_y + (int16_t)widget->yco - (int16_t)view->top;

                    if (sy == wy && sx >= wx && sx < wx + (int16_t)widget->width)
                    {
                        return widget->sequence;
                    }
                }

                n3 = uC_list_scan(NULL, n3);
            }

            n2 = uC_list_scan(NULL, n2);
        }

        n1 = uC_list_scan(NULL, n1);
    }

    return 0;
}

// -----------------------------------------------------------------------
// dispatch a parsed mouse event: left-click transfers focus and activates

void uC_widget_mouse_handle(void)
{
    int16_t  sx  = uC_mouse_event.x - 1;
    int16_t  sy  = uC_mouse_event.y - 1;
    uint16_t seq;

    if (!widget_state.screen) return;

    if (uC_mouse_event.button != UC_MOUSE_LEFT || !uC_mouse_event.pressed)
        return;

    seq = widget_hit_test(sx, sy);
    if (!seq) return;

    if (seq != widget_state.sequence)
    {
        uC_widget_select_widget(seq);
    }

    // textboxes just receive focus from the click; button/radio/check
    // treat SPACE as their activation key
    if (widget_state.widget && widget_state.widget->type != uC_WIDGET_TEXTBOX)
    {
        uC_set_key(0x20);
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
