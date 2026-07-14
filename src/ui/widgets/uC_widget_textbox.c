// uC_widget_textbox.c
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
// which characters are valid in various bases

const char radix_chars[] =
    "0123456789abcdefABCDEF"
    "ghijklmnopqrstuvwxyz"
    "GHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*()-=_+[]{}|\\\"';:`,./?<> ";

// length within above string with valid characters for each base

uint8_t radix_lengths[] =
{
    2, 8, 10, 22, sizeof radix_chars -1
};

// -----------------------------------------------------------------------

// currently widget name is displayed to the left of the edit box.  i
// could add an option to display the name above the edit box

void draw_textbox(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y)
{
    int i;
    int x2;
    char c;

    uC_widget_textbox_t *t;

    // display widget name using windows current attributes

    // %@ set cursor x / y location in window
    // %s write string

    uC_win_printf(win, "%@%s", UC_XY(x, y), widget->name);

    widget_set_attrs(win, widget);

    // win->attrs = (widget->focused == true)
    //     ? widget->focus_attrs
    //     : widget->attrs;

    t = &widget->textbox;

    // this places the widgets edit box to the right of its name
    // should I add code here to allow exit box below name?

    // x2 = x coordinate one space beyond the end of the name string

    x2 = x + strlen(widget->name);

    // erase widgets edit box area

    // %x set cursor x location on current line of window
    // %* write multiple repetitions of same character
    // %x move cursor back to initial %x position above

    uC_win_printf(win, "%x%*%x", x2, widget->width, 0x20, x2);

    // we want to write only the part of the string that will fit
    // within the widgets assigned horizontal width, starting from
    // the current edit offset within that string.

    x2 = (t->offset);

    for (i = 0; i != widget->width; i++)
    {
        c = t->data[x2++];

        // if this widget is the one with focus and we are drawing
        // the character that is currently under the cursor then
        // draw it in either reverse video or underlined (based
        // on insert or overwrite status)

        if ((widget->focused) && t->editing && (i == t->cx))
        {
            // %R+ turn on reverse video
            // %8  print single UTF-8 character
            // %R- turn off reverse video

            // or...

            // %U+ turn on underline
            // %8  print single UTF-8 character
            // %U- turn off underline

            uC_win_printf(win, (t->insert)
                ? "%R+%8%R-"
                : "%U+%8%U-",

                // if character is null print a space
                // else print character underlined or rev

                (c == '\0') ? 0x20 : c);
        }
        else if (c != '\0')
        {
            uC_win_printf(win, "%8", c);
        }

        if (c == '\0')
        {
            break;
        }
    }
}

// -----------------------------------------------------------------------
// veriry that input character is valid in current radix

// there is one string used to test for each valid raidx.  How var into
// this string we scan is based on whtat the current radix is.  we scan
// this string and compare each character of it with the passed in
// variable k.   If k is found within that string then k is a valid
// character within the current base.

static bool test_char(uC_widget_textbox_t *t, uint8_t k)
{
    uint8_t len = radix_lengths[t->radix];

    while (len--)
    {
        if (radix_chars[len] == k)
        {
            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------
// move the edit cursor left within the widget

static void lt(uC_widget_textbox_t *t)
{
    uint16_t half = widget_state.widget->width / 2;
    uint16_t min  = t->offset ? half : 0;

    if (t->cx > min)
    {
        t->cx--;
    }
    else if (t->offset)
    {
        t->offset--;
    }
}

// -----------------------------------------------------------------------

static void rt(uC_widget_textbox_t *t)
{
    int16_t half = widget_state.widget->width / 2;
    int q        = (t->cx + t->offset);

    if (q < t->count)
    {
        if (t->cx <= half)
        {
            t->cx++;
        }
        else if (q != t->size)
        {
            t->offset++;
        }
    }
}

// -----------------------------------------------------------------------

static void insert_char(uC_widget_textbox_t *t, uint8_t k)
{
    int i;
    int cursor = t->offset + t->cx;
    uint8_t c2;

    // if the cursor is at the end of the string and there is still space
    // left for more characters

    if (cursor == t->count)
    {
        if (cursor != t->size)
        {
            t->data[cursor] = k;
            t->cx++;
        }
        return;
    }

    for (i = cursor; i != t->size; i++)
    {
        c2 = t->data[cursor];
        t->data[cursor++] = k;
        if (k == '\0')
        {
            break;
        }
        k = c2;
    }
}

// -----------------------------------------------------------------------

static uint8_t write_char(uC_widget_textbox_t *t, uint8_t k)
{
    bool f;
    int q;
    uint8_t saved_cx;
    uint8_t saved_offset;

    f = test_char(t, k);

    if (!f)
    {
        return k;
    }

    q = (t->offset + t->cx);
    saved_cx = t->cx;
    saved_offset = t->offset;

    if (q != t->size)
    {
        if (t->insert)
        {
            insert_char(t, k);
            t->count++;
        }
        else
        {
            t->data[q] = k;
            if (q == t->count)
            {
                t->count++;
            }
        }
        if (t->advance_cursor)
        {
            rt(t);
        }
        else
        {
            t->cx = saved_cx;
            t->offset = saved_offset;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

static void del(uC_widget_textbox_t *t)
{
    int i;

    int q = (t->cx + t->offset);

    if (q < t->count)
    {
        for (i = q; i != t->count; i++)
        {
            t->data[i] = t->data[i + 1];
            t->data[i + 1] = 0;
        }
        t->count--;
    }
}

// -----------------------------------------------------------------------

static void bs(uC_widget_textbox_t *t)
{
    int q = (t->cx + t->offset - 1);

    if (t->count && (q >= 0))
    {
        while (q != t->count)
        {
            t->data[q] = t->data[q + 1];
            q++;
        }
        t->data[q] = 0;
        t->count--;
        lt(t);
    }
}

// -----------------------------------------------------------------------

static void home(uC_widget_textbox_t *t)
{
    t->cx     = 0;
    t->offset = 0;
}

// -----------------------------------------------------------------------

static void end(uC_widget_textbox_t *t)
{
    if (!t->advance_cursor && t->count != 0)
    {
        if (t->count >= widget_state.widget->width)
        {
            t->cx = widget_state.widget->width - 1;
            t->offset = t->count - widget_state.widget->width;
        }
        else
        {
            t->cx = t->count - 1;
            t->offset = 0;
        }
        return;
    }

    if (t->count >= widget_state.widget->width)
    {
        t->cx     = widget_state.widget->width - 1;
        t->offset = t->count - t->cx;
    }
    else
    {
        t->offset = 0;
        t->cx     = t->count;
    }
}

// -----------------------------------------------------------------------

uint8_t handle_textbox(uint8_t k)
{
    uC_widget_textbox_t *t = &widget_state.widget->textbox;

    if (!t->editing)
    {
        if ((k == 0x0a) || (k == 0x0d))
        {
            t->editing = true;
            end(t);
            return 0;
        }
        return k;
    }

    switch (k)
    {
        case 0x0a:
        case 0x0d:
        case 0x1b:
            t->editing = false;
            k = 0;
            break;
        case WIDGET_KEY_LEFT:   lt(t);   break;
        case WIDGET_KEY_RIGHT:  rt(t);   break;
        case WIDGET_KEY_DELETE: del(t);  break;
        case WIDGET_KEY_BS:     bs(t);   break;
        case WIDGET_KEY_HOME:   home(t); break;
        case WIDGET_KEY_END:    end(t);  break;
        case WIDGET_KEY_INSERT:
            t->insert = !t->insert;
            break;

        default:
            k = write_char(t, k);
    }

    return k;
}

// -----------------------------------------------------------------------

// data must point to a buffer of size+1 bytes. size is the character
// capacity; the extra byte holds the null terminator at data[size].

API uC_widget_t *uC_widget_textbox_create(
    char *data, const char *name,
    uint16_t size, uC_textbox_radix_t radix,
    uint16_t width, uC_attribs_t attrs, uC_attribs_t focus)
{
    uint16_t count = 0;
    uint8_t capacity = (size > UINT8_MAX) ? UINT8_MAX : (uint8_t)size;
    uC_widget_t *widget = create_widget(uC_WIDGET_TEXTBOX,
        name, width, attrs, focus);

    if (widget)
    {
        widget->textbox.data   = data;
        widget->textbox.size   = capacity;
        widget->textbox.radix  = radix;
        widget->textbox.insert = true;
        widget->textbox.editing = false;
        widget->textbox.advance_cursor = true;

        while ((count < capacity) && (data[count] != '\0'))
        {
            count++;
        }
        widget->textbox.count = (uint8_t)count;
    }

    return widget;
}

// -----------------------------------------------------------------------
// Controls whether a successful character write advances the edit cursor.

API void uC_widget_textbox_set_cursor_advance(uC_widget_t *widget,
    bool enabled)
{
    if (widget != NULL && widget->type == uC_WIDGET_TEXTBOX)
    {
        widget->textbox.advance_cursor = enabled;
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
