// win_printf.c  printf strings into window_t
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

static va_list arg;
static char *p;
static window_t *w;

// -----------------------------------------------------------------------
// write string into specified window at its currnt cursor location

void win_puts(window_t *win, char *p)
{
    uint8_t skip;
    int32_t codepoint;

    while(*p != '\0')
    {
        skip = utf8_decode(&codepoint, p);
        win_emit(win, codepoint);
        p += skip;
    }
}

// -----------------------------------------------------------------------

static void zero_abort(void)
{
    if(*p == '\0')
    {
        xabort("Unexpected end of win_printf format string");
    }
}

// -----------------------------------------------------------------------
// %rf   set 24 bit rgb fg

static void rf(void)
{
    int r, g, b;

    zero_abort();

    r = va_arg(arg, int) & 0xff;
    g = va_arg(arg, int) & 0xff;
    b = va_arg(arg, int) & 0xff;

    if(*p == 'f')
    {
        win_set_rgb_fg(w, r, g, b);
    }
    else if(*p == 'b')
    {
        win_set_rgb_bg(w, r, g, b);
    }
    else
    {
        xabort("Expected x or b on win_printf %%r");
    }

    p++;
}

// -----------------------------------------------------------------------
// %fc ir %fs  set foreground color or gray scale

static void f(void)
{
    int f;

    zero_abort();

    f = va_arg(arg, int);

    if(*p == 'c')
    {
        win_set_fg(w, f & 0xff);
    }
    else if(*p == 's')
    {
        win_set_gray_fg(w, f % 21);
    }
    else
    {
        xabort("Expected c or s on win_printf %%f");
    }
    p++;
}

// -----------------------------------------------------------------------
// %bc or %bs  set backgorund color or gray scale

static void b(void)
{
    int b;

    zero_abort();

    b = va_arg(arg, int);

    if(*p == 'c')
    {
        win_set_bg(w, b & 0xff);
    }
    else if(*p == 's')
    {
        win_set_bg(w, b % 21);
    }
    else
    {
        xabort("Expected c or s on win_printf %%b");
    }

    p++;
}

// -----------------------------------------------------------------------
// %@    set cursor x/y within window

static void xy(void)
{
    int x = va_arg(arg, int);
    int y = va_arg(arg, int);

    win_cup(w, x, y);
}

// -----------------------------------------------------------------------
// %x   set cursor x in window

static void x(void)
{
    int x = va_arg(arg, int);

    win_set_cx(w, x);
}

// -----------------------------------------------------------------------
// %y   set cursor y in window

static void y(void)
{
    int y = va_arg(arg, int);

    win_set_cy(w, y);
}

// -----------------------------------------------------------------------
// %u  scroll window up specified amount

static void up(void)
{
    int y = va_arg(arg, int);

    while(y-- != 0)
    {
        win_scroll_up(w);
    }
}

// -----------------------------------------------------------------------
// %d   scroll window down specified amount

static void dn(void)
{
    int y = va_arg(arg, int);

    while(y-- != 0)
    {
        win_scroll_dn(w);
    }
}

// -----------------------------------------------------------------------
// %l   scroll window left specified amount

static void lt(void)
{
    int x = va_arg(arg, int);

    while(x-- != 0)
    {
        win_scroll_lt(w);
    }
}

// -----------------------------------------------------------------------
// %r    scroll window right specified amount

static void rt(void)
{
    int x = va_arg(arg, int);

    while(x-- != 0)
    {
        win_scroll_rt(w);
    }
}

// -----------------------------------------------------------------------
// %cu  %cd %cl %cr    move cursor up, down, left or right in window

static void c(void)
{
    if(*p == 'u')
    {
        win_crsr_up(w);
    }
    else if(*p == 'd')
    {
        win_crsr_dn(w);
    }
    else if(*p == 'l')
    {
        win_crsr_lt(w);
    }
    else if(*p == 'r')
    {
        win_crsr_rt(w);
    }
    else
    {
        xabort("Expected u, d, l or r on win_printf %%c");
    }
    p++;
}

// -----------------------------------------------------------------------
// %0    clear window

static void wclear(void)
{
    win_clear(w);
}

// -----------------------------------------------------------------------
// just a wrapper for puts which is at the top of this file

static void u_puts(void)
{
    char *s = va_arg(arg, char *);
    win_puts(w, s);
}

// -----------------------------------------------------------------------

static void bold(void)
{
    if(*p == '+')
    {
        win_set_bold(w);
    }
    else if(*p == '-')
    {
        win_clr_bold(w);
    }
    else
    {
        xabort("Expected + or - win_printf %%B");
    }

    p++;
}

// -----------------------------------------------------------------------

static void uline(void)
{
    if(*p == '+')
    {
        win_set_ul(w);
    }
    else if(*p == '-')
    {
        win_clr_ul(w);
    }
    else
    {
        xabort("Expected + or - win_printf %%U");
    }

    p++;
}

// -----------------------------------------------------------------------

static void rev(void)
{
    if(*p == '+')
    {
        win_set_rev(w);
    }
    else if(*p == '-')
    {
        win_clr_rev(w);
    }
    else
    {
        xabort("Expected + or - win_printf %%R");
    }

    p++;
}

// -----------------------------------------------------------------------

static switch_t commands[] = //
    {                        //
        { 'r', &rf },   { 'f', &f },     { 'b', &b },      { '@', &xy },
        { 'x', &x },    { 'y', &y },     { 'u', &up },     { 'd', &dn },
        { 'l', &lt },   { 'r', &rt },    { '0', &wclear }, { 'c', &c },
        { 'B', &bold }, { 'U', &uline }, { 'R', rev },     { 's', u_puts }
    };

#define COMMANDS sizeof(commands) / sizeof(commands[0])

// -----------------------------------------------------------------------

static INLINE void command(void)
{
    zero_abort();
    re_switch(commands, COMMANDS, *p++);
}

// -----------------------------------------------------------------------
// window string writing and window attribute control

// if you need to use normal printf format tags you must first sprintf
// your string into a buffer and escape the format tags within it that
// you want passed to this function...

void win_printf(window_t *win, char *format, ...)
{
    int32_t codepoint;
    int8_t skip;

    va_start(arg, format);

    p = format;
    w = win;

    while(*p != '\0')
    {
        while((*p != '%') && (*p != '\0'))
        {
            skip = utf8_decode(&codepoint, p);
            win_emit(win, codepoint);
            p += skip;
        }

        if(*p != '\0')
        {
            p++;
            command();
        }
    }

    va_end(arg);
}

// =======================================================================
