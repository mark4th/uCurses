// uC_widget_keys.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_terminfo.h"
#include "uC_screen.h"
#include "uC_alloc.h"
#include "uC_widgets.h"
#include "uC_window.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern uC_kh_t user_key_actions;
extern uC_screen_t *active_screen;

uC_kh_t saved_key_actions;
API widget_state_t widget_state;

// ----------------------------------------------------------------

static uint8_t handle_widget(uint8_t k)
{
    if (widget_state.widget == NULL)
    {
        return k;
    }

    // if the currently focused widget view is scrollable

    if (widget_state.view != NULL)
    {
        if (widget_state.view->flags & (1 << uC_VIEW_SCROLL))
        {
            // if the key pressed is a cursor movement then scroll
            // the view, otherwise fall through

            if ((k == WIDGET_KEY_UP) || (k == WIDGET_KEY_DOWN))
            {
                if (widget_state.view != NULL)
                {
                    widget_scroll_view(k);
                }
                return k;
            }
        }
    }

    switch (widget_state.widget->type)
    {
        case uC_WIDGET_NONE:    return k;
        case uC_WIDGET_BUTTON:  return handle_button(k);
        case uC_WIDGET_RADIO:   return handle_radio(k);
        case uC_WIDGET_CHECK:   return handle_check(k);
        case uC_WIDGET_TEXTBOX: return handle_textbox(k);
    }

    return k;
}

// -----------------------------------------------------------------------

static uint8_t widget_key(void)
{
    uint8_t k = 0;

    // make this call block till a key is pressed
    // I may remove this from here and make it up to the user
    // application to decide if it should block within its own
    // key loop

    while (uC_test_keys() == 0)
    {
        ;
    }

    // this while loop ensures that when an escape sequence key
    // is pressed and translated into a single character we
    // read and interpret that single character

    while (uC_test_keys() != 0)
    {
        k = uC_key();

        if (k != 0x1b)
        {
            if (k == 0x88)
            {
                k = tab_prev_widget();
            }
            else
            {
                k = (k == 0x09)
                    ? tab_next_widget()
                    : handle_widget(k);
            }
        }
    }

    return k;
}

// -----------------------------------------------------------------------
// any key that returns an escape sequence instead of a single character
// is processed differently by the uCurses keyboard handler.   each key
// that is currently recognized by uCurses (and not all are) will have a
// user definable handler which will be called for that key press.  The
// handlers for the keys needed by the widget loop are given below.
//
// Each of these functions will inject a single value back into the
// uCurses keyboard input buffer to be handled as a single key press.
// this injected value will be returned to widget_key() above.
// The enumeration in uC_widgets.h contains values that cannot be returned
// by a normal keypress so are safe to use

// stuff one of the above uCurses black magic key values into the keyboard
// input buffers to be returned to widget_key() above

static void widget_key_up(void)     { uC_set_key(WIDGET_KEY_UP);     }
static void widget_key_down(void)   { uC_set_key(WIDGET_KEY_DOWN);   }
static void widget_key_left(void)   { uC_set_key(WIDGET_KEY_LEFT);   }
static void widget_key_right(void)  { uC_set_key(WIDGET_KEY_RIGHT);  }
static void widget_key_insert(void) { uC_set_key(WIDGET_KEY_INSERT); }
static void widget_key_delete(void) { uC_set_key(WIDGET_KEY_DELETE); }
static void widget_key_home(void)   { uC_set_key(WIDGET_KEY_HOME);   }
static void widget_key_end(void)    { uC_set_key(WIDGET_KEY_END);    }

// -----------------------------------------------------------------------
// the other option is to simply have each of these perform the desired
// action but then each and every single one of these would need to base
// that action on which widget type is currently active.   The code to
// arbitrate this already exists above so we only need to implement it one
// time this way.

static void set_widget_key_actions(void)
{
    saved_key_actions = uC_alloc_kh();

    uC_set_key_action(K_CUU1, widget_key_up);
    uC_set_key_action(K_CUD1, widget_key_down);
    uC_set_key_action(K_CUB1, widget_key_left);
    uC_set_key_action(K_CUF1, widget_key_right);
    uC_set_key_action(K_ICH1, widget_key_insert);
    uC_set_key_action(K_DCH1, widget_key_delete);
    uC_set_key_action(K_HOME, widget_key_home);
    uC_set_key_action(K_END,  widget_key_end);

    // as long as there are any widgets active the menu system is
    // disabled.

    uC_set_key_action(K_F10, uC_noop);
}

// -----------------------------------------------------------------------

// there might be a problem with the widget state variable maintaining a
// memory of which widgets had focus on the previos call to this function
// because all widgets are destroyed when the call is made by the app to
// uC_widget_vg_close() and recreated prior to calling this function
// again.

API char uC_widget_main(void)
{
    char k;

    if (widget_state.vg != NULL)
    {
        if ((widget_state.vg->flags & uC_vg_flag_ignore) != 0)
        {
            widget_state.sequence = 0;
        }
    }

    set_widget_key_actions();

    k = widget_key();

    uC_release_kh(saved_key_actions);

    return k;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
