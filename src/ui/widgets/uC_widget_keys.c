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

uC_kh_t saved_key_actions;
API widget_state_t widget_state;

// ----------------------------------------------------------------
// if current view is scrollable and k is a cursor up or dowwn...

static bool check_scrollable(uint8_t k)
{
    // view is a known non null here

    if (widget_state.view->flags & (1 << uC_VIEW_SCROLL))
    {
        if ((k == WIDGET_KEY_UP) || (k == WIDGET_KEY_DOWN))
        {
            widget_scroll_view(k);
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------

static uint8_t handle_widget_key(uint8_t k)
{
    bool f;

    if (!widget_state.widget)
    {
        return k;
    }

    // handle case where the current view is scrollable and
    // either cursor up or cursor down have been pressed

    if (widget_state.view)
    {
        f = check_scrollable(k);
        if (f)
        {
            return k;
        }
    }

    // other than cursor up / down within a scrollable view (wich can
    // contain any widget type) each widget type module contains its
    // own keyboard handler

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

static uint8_t _widget_key(void)
{
    uint8_t k;

    k = uC_key();    // blocks till a key is pressed

    switch (k)
    {
        case 0x09: tab_next_widget(); break;
        case 0x88: tab_prev_widget(); break;

        default:
            k = handle_widget_key(k);
    }

    return k;
}

// -----------------------------------------------------------------------

static uint8_t widget_key(void)
{
    uint8_t k = 0;

    // this while loop ensures that when an escape sequence key
    // is pressed and the uCurses keyboard handler translates that
    // into a single character we will read and interpret that single
    // character within this call to widget_key().  this negates type
    // ahead

    while (uC_test_keys())
    {
        k = _widget_key();
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

API char uC_widget_main(void)
{
    char k;

    if (widget_state.vg)
    {
        if (widget_state.vg->flags & uC_vg_flag_ignore)
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
