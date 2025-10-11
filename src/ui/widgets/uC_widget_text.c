// uC_widget_text.c
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

// currently widget name is displayed to the left of the edit box.  i
// could add an option to display the name above the edit box

void draw_textbox(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y)
{
    int i;
    int q ;
    char c;

    uC_widget_textbox_t *t = &widget->textbox;

    // display widget name using windows normal attributes

    // %@ set cursor x / y location in window
    // %s write string

    uC_win_printf(win, "%@%s",x ,y, widget->name);

    win->attrs = (widget->focused == true)
        ? widget->focus_attrs
        : widget->attrs;

    // %x set cursor x location on current line of window
    // %* write multiple repetitions of same character

    q = x + strlen(widget->name);

    uC_win_printf(win, "%x%*%x", q + 1,
        widget->width, 0x20, q + 1);

    for (i = 0; i != widget->width; i++)
    {
        q = (i + t->offset);

        // if (q >= t->size)      { break; }
        // if (q == t->count + 1) { break; }

        c = t->data[q];

        if (((i == t->cx) && (widget->focused)) || (q >= t->size))
        {
            uC_win_printf(win,
                (t->insert) ? "%R+%8%R-" : "%U+%8%U-",
                (c == '\0') ? 0x20       : c);
        }
        else
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
// which characters are valid in various bases

char * const radix_chars =
    "0123456789abcdefABCDEF"
    "ghijklmnopqrstuvwxyz"
    "GHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*()-=_+[]{}|\\\"';:',./?<> ";

// length within above string with valid characters for each base

uint8_t radix_lengths[] =
{
    2, 8, 10, 22, strlen(radix_chars)
};

// -----------------------------------------------------------------------
// veriry that input character is valid in current radix

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

static void lt(uC_widget_textbox_t *t)
{
    uint16_t half = widget_state.widget->width / 2;
    uint16_t min = (t->offset != 0) ? half : 0;

    if (t->cx != min)
    {
        t->cx--;
    }
    else if (t->offset != 0)
    {
        t->offset--;
    }
}

// -----------------------------------------------------------------------

static void rt(uC_widget_textbox_t *t)
{
    int q = (t->cx + t->offset);

    if (q != t->count)
    {
        if (t->cx != widget_state.widget->width - 1)
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

static void insert_space(uC_widget_textbox_t *t, uint8_t k)
{
    int i;

    char c2;

    for (i = t->cx; i != t->count; i++)
    {
        if (i + t->offset == t->size)
        {
            break;
        }
        c2 = t->data[i + t->offset];
        t->data[i + t->offset] = k;
        k = c2;
    }
}

// -----------------------------------------------------------------------

static uint8_t write_char(uC_widget_textbox_t *t, uint8_t k)
{
    bool f;

    f = test_char(t, k);

    int q = (t->cx + t->offset);

    if (f == false)
    {
        return k;
    }

    if (q != t->size)
    {
        if (t->insert)
        {
            insert_space(t, k);
        }

        t->data[q] = k;
        t->count++;
        rt(t);
    }

    k = 0;

    return k;
}

// -----------------------------------------------------------------------

static void del(uC_widget_textbox_t *t)
{
    int i;

    int q = (t->cx + t->offset);

    if (t->count != 0)
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

    if ((t->count != 0) && (q >= 0))
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

uint8_t handle_text(uint8_t k)
{
    uC_widget_textbox_t *t = &widget_state.widget->textbox;

    switch (k)
    {
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

API uC_widget_t *uC_widget_text_create(
    uint16_t sequence, char *data, char *name,
    uint16_t size, uC_textbox_radix_t radix,
    uint16_t width, uint8_t xco, uint8_t yco,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *w = create_widget(uC_WIDGET_TEXTBOX,
        name, sequence, xco, yco, width, attrs, focus);

    if (w != NULL)
    {
        w->textbox.data   = data;
        w->textbox.size   = size;
        w->textbox.radix  = radix;
        w->textbox.insert = true;
    }

    return w;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
