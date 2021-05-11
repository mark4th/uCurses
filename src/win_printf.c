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
        p++;
    }
    else if(*p == 'b')
    {
        win_set_rgb_bg(w, r, g, b);
        p++;
    }
    xabort("Expected x or b on win_printf %r");
}

// -----------------------------------------------------------------------

static void f(void)
{
    int f;

    zero_abort();

    f = va_arg(arg, int);

    if(*p == 'c')
    {
        win_set_fg(w, f & 0xff);
        p++;
    }
    else if(*p == 's')
    {
        win_set_gray_fg(w, f % 21);
        p++;
    }
    xabort("Expected c or s on win_printf %f");
}

// -----------------------------------------------------------------------

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
        xabort("Expected c or s on win_printf %b");
    }
    p++;
}

// -----------------------------------------------------------------------

static void xy(void)
{
    int x = va_arg(arg, int);
    int y = va_arg(arg, int);

    win_cup(w, x, y);
}

// -----------------------------------------------------------------------

static void x(void)
{
    int x = va_arg(arg, int);

    win_set_cx(w, x);
}

// -----------------------------------------------------------------------

static void y(void)
{
    int y = va_arg(arg, int);

    win_set_cy(w, y);
}

// -----------------------------------------------------------------------

static void up(void)
{
    int y = va_arg(arg, int);

    while(y-- != 0)
    {
        win_scroll_up(w);
    }
}

// -----------------------------------------------------------------------

static void dn(void)
{
    int y = va_arg(arg, int);

    while(y-- != 0)
    {
        win_scroll_dn(w);
    }
}

// -----------------------------------------------------------------------

static void lt(void)
{
    int x = va_arg(arg, int);

    while(x-- != 0)
    {
        win_scroll_lt(w);
    }
}

// -----------------------------------------------------------------------

static void rt(void)
{
    int x = va_arg(arg, int);

    while(x-- != 0)
    {
        win_scroll_rt(w);
    }
}

// -----------------------------------------------------------------------

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
        xabort("Expected u, d, l or r on win_printf %c");
    }
    p++;
}

// -----------------------------------------------------------------------

static void wclear(void)
{
    win_clear(w);
}

// -----------------------------------------------------------------------

static switch_t commands[] = //
    {                        //
        { 'r', &rf }, { 'f', &f },  { 'b', &b },      { '@', &xy },
        { 'x', &x },  { 'y', &y },  { 'u', &up },     { 'd', &dn },
        { 'l', &lt }, { 'r', &rt }, { '0', &wclear }, { 'c', &c }
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

        if(*p == '\0')
        {
            return;
        }

        p++;

        command();
    }

    va_end(arg);
}

// =======================================================================
