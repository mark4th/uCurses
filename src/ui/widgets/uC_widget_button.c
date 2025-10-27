
// uC_widget_button.c
// -----------------------------------------------------------------------

#include <string.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_widgets.h"
#include "uC_win_printf.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------
// draws the text on a button and underlines the keyboard shortcut key
// if there is one (the shortcut key does not actually need to be part
// of the button name but it would be more helpful if it was :)

static void draw_btn_txt(uC_window_t *win, uint16_t x, uint16_t y,
    uint16_t width, char *name, char key)
{
    char c;
    bool ul = false;
    uint16_t pad = (width / 2) - (strlen(name) / 2);

    // %@ set cursor location in window
    // %* emit single char multiple times
    // %x set cursor X location on current line
    // %* emit single char multiple times

    uC_win_printf(win, "%@%*%x%*",
        x, y, width, 0x20, x, pad, 0x20);

    while ((*name != '\0') && (width-- != 0))
    {
        c = *name++;

        if ((ul != true) && (*name == key))
        {
            // make sure only first instance of letter is
            // actually underlined

            ul = true;

            // %U+ turn on underlining of text
            // %8  output a single char
            // %U- turn underling of text off

            uC_win_printf(win, "%U+%8%U-", c);

            continue;
        }
        uC_win_emit(win, c);
    }
}

// -----------------------------------------------------------------------

void draw_button(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y)
{
    win->attrs = (widget->focused == true)
        ? widget->focus_attrs
        : widget->attrs;

    draw_btn_txt(win, x, y, widget->width, widget->name,
        widget->button.letter);
}

// -----------------------------------------------------------------------
// a button key has been pressed or the button itself has been pressed

uint8_t handle_button(uint8_t k)
{
    uC_widget_button_t *b;

    b = &widget_state.widget->button;

    if (k == 0x0a)
    {
        // there are two ways the application code can determine which
        // button was pressed.   If the button has a key character
        // assigned that key letter is returned as if it had been a
        // key press.

        // if the button widget has an assigned *select then the tab
        // sequence value of the button will be written in to this
        // address.

        // note: if the button is part of a scrollable view then only
        // the first button in that view has a valid tab sequence
        // value.

        k = b->letter;

        if (b->select != NULL)
        {
            *b->select = widget_state.widget->sequence;
        }
    }

    return k;
}

// -----------------------------------------------------------------------

API uC_widget_t *uC_widget_button_create(uint16_t sequence,
    uint16_t *select, char *name, char letter,
    uint16_t width, uint8_t xco, uint8_t yco,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *w = create_widget(uC_WIDGET_BUTTON, name,
        sequence, xco, yco, width, attrs, focus);

    if (w != NULL)
    {
        // text displayed on button and optional keybord shortcut

        w->button.letter = letter;
        w->button.select = select;
    }

    return w;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
