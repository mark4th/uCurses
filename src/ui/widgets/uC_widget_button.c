
// uC_widget_button.c
// -----------------------------------------------------------------------

#include <string.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_widgets.h"
#include "uC_win_printf.h"
#include "uC_utf8.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

bool justify;

// -----------------------------------------------------------------------

static bool button_letter_alpha(char c)
{
    return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}

// -----------------------------------------------------------------------

static char button_letter_upper(char c)
{
    if ((c >= 'a') && (c <= 'z'))
    {
        return (char)(c - ('a' - 'A'));
    }
    return c;
}

// -----------------------------------------------------------------------

static char button_letter_from_name(const char *name, char letter)
{
    char match;

    if ((name == NULL) || !button_letter_alpha(letter))
    {
        return '\0';
    }

    match = button_letter_upper(letter);
    while (*name != '\0')
    {
        if (button_letter_alpha(*name) &&
            (button_letter_upper(*name) == match))
        {
            return *name;
        }
        name++;
    }

    return '\0';
}

// -----------------------------------------------------------------------
// Draws the text on a button and underlines the focused-button return
// key when that key is present in the displayed name.

static void draw_btn_txt(uC_window_t *win, uint16_t x, uint16_t y,
    uint16_t width, const char *name, char letter)
{
    uint32_t codepoint;
    uint8_t len;
    bool ul = false;
    uint16_t pad;
    uint16_t remaining;
    int16_t text_width;

    if (!win || !name)
    {
        return;
    }

    remaining = widget_clear_width(win, x, y, width);
    text_width = uC_utf8_width((uint8_t *)name);
    pad = (justify || (text_width < 0) || ((uint16_t)text_width >= remaining))
       ? 0
       : (uint16_t)((remaining - (uint16_t)text_width) / 2u);

    while (pad != 0)
    {
        if (!widget_emit_clipped(win, 0x20, &remaining))
        {
            return;
        }
        pad--;
    }

    while ((*name != '\0') && (remaining != 0))
    {
        len = utf8_decode(&codepoint, (uint8_t *)name);
        if ((len == 0) || (len > 4))
        {
            break;
        }

        if (!ul && (codepoint == (uint32_t)letter))
        {
            // make sure only first instance of letter is
            // actually underlined

            ul = true;
            uC_win_printf(win, "%U+");
            if (!widget_emit_clipped(win, codepoint, &remaining))
            {
                uC_win_printf(win, "%U-");
                break;
            }
            uC_win_printf(win, "%U-");
        }
        else
        {
            if (!widget_emit_clipped(win, codepoint, &remaining))
            {
                break;
            }
        }
        name += len;
    }
}

// -----------------------------------------------------------------------

void draw_button(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y)
{
    widget_set_attrs(win, widget);

    // win->attrs = (widget->focused == true)
    //     ? widget->focus_attrs
    //     : widget->attrs;

    draw_btn_txt(win, x, y, widget->width, widget->name,
        widget->button.letter);
}

// -----------------------------------------------------------------------
// a button key has been pressed or the button itself has been pressed

uint8_t handle_button(uint8_t k)
{
    uC_widget_button_t *b;

    b = &widget_state.widget->button;

    if ((k == UC_KEY_ENTER) || (k == '\r'))
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

        if (b->letter != '\0')
        {
            return b->letter;
        }

        if (b->select)
        {
            *b->select = widget_state.widget->sequence;
        }
        return UC_KEY_NONE;
    }

    return k;
}

// -----------------------------------------------------------------------

API uC_widget_t *uC_widget_button_create(
    uint16_t *select, const char *name, char letter,
    uint16_t width, uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *widget = create_widget(uC_WIDGET_BUTTON, name,
        width, attrs, focus);

    if (widget)
    {
        // text displayed on button and optional focused-button return key

        widget->button.letter = button_letter_from_name(name, letter);
        widget->button.select = select;
    }

    return widget;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
