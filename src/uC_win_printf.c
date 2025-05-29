// win_printf.c  printf strings into uC_window_t
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_switch.h"
#include "uC_window.h"
#include "uC_utf8.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------
// this file is a blatant MISRA C violation :
// -----------------------------------------------------------------------

static va_list arg;
static char *p;
static uC_window_t *w;

// -----------------------------------------------------------------------
// write string into specified window at its currnt cursor location

API void uC_win_puts(uC_window_t *win, char *p)
{
    uint8_t skip;
    int32_t codepoint;

    while (*p != '\0')
    {
        skip = utf8_decode(&codepoint, p);
        uC_win_emit(win, codepoint);
        p += skip;
    }
}

// -----------------------------------------------------------------------
// %rf   set 24 bit rgb fg / bg

static void rf(void)
{
    int r, g, b;

    if (*p == 'f' || *p == 'b')
    {
        r = va_arg(arg, int) & 0xff;
        g = va_arg(arg, int) & 0xff;
        b = va_arg(arg, int) & 0xff;

        (*p == 'f')
            ? uC_win_set_rgb_fg(w, r, g, b)
            : uC_win_set_rgb_bg(w, r, g, b);
        p++;
        return;
    }
    xabort("Expected f or b on win_printf %r");
}

// -----------------------------------------------------------------------
// %fc or %fs  set foreground color or gray scale

static void f(void)
{
    int f;

    if (*p == 'c' || *p == 's')
    {
        f = va_arg(arg, int);
        (*p == 'c')
            ? uC_win_set_fg(w, f)
            : uC_win_set_gray_fg(w, f % 21);
        p++;
        return;
    }
    xabort("Expected c or s on win_printf %f");
}

// -----------------------------------------------------------------------
// %bc or %bs  set backgorund color or gray scale

static void b(void)
{
    int b;

    b = va_arg(arg, int);

    if (*p == 'c' || *p == 's')
    {
        (*p == 'c')
            ? uC_win_set_bg(w, b)
            : uC_win_set_gray_bg(w, b % 23);
        p++;
        return;
    }
    xabort("Expected c or s on win_printf %b");
}

// -----------------------------------------------------------------------
// %@    set cursor x/y within window

static void xy(void)
{
    int x = va_arg(arg, int);
    int y = va_arg(arg, int);

    uC_win_cup(w, x, y);
}

// -----------------------------------------------------------------------
// %x   set cursor x in window

static void x(void)
{
    int x = va_arg(arg, int);

    uC_win_set_cx(w, x);
}

// -----------------------------------------------------------------------
// %y   set cursor y in window

static void y(void)
{
    int y = va_arg(arg, int);

    uC_win_set_cy(w, y);
}

// -----------------------------------------------------------------------
// %u  scroll window up specified amount

static void up(void)
{
    int y = va_arg(arg, int);

    while (y-- != 0)
    {
        uC_win_scroll_up(w);
    }
}

// -----------------------------------------------------------------------
// %d   scroll window down specified amount

static void dn(void)
{
    int y = va_arg(arg, int);

    while (y-- != 0)
    {
        uC_win_scroll_dn(w);
    }
}

// -----------------------------------------------------------------------
// %l   scroll window left specified amount

static void lt(void)
{
    int x = va_arg(arg, int);

    while (x-- != 0)
    {
        uC_win_scroll_lt(w);
    }
}

// -----------------------------------------------------------------------
// %r    scroll window right specified amount

static void rt(void)
{
    int x = va_arg(arg, int);

    while (x-- != 0)
    {
        uC_win_scroll_rt(w);
    }
}

// -----------------------------------------------------------------------
// %8    draw UTF-8 codepoint

static void utf8(void)
{
    int cc = va_arg(arg, int);
    uC_win_emit(w, cc);
}

// -----------------------------------------------------------------------
// %cu  %cd %cl %cr    move cursor up, down, left or right in window

static void c(void)
{
    switch (*p)
    {
        case 'u' :   uC_win_crsr_up(w); break;
        case 'd' :   uC_win_crsr_dn(w); break;
        case 'l' :   uC_win_crsr_lt(w); break;
        case 'r' :   uC_win_crsr_rt(w); break;
        default :
            xabort("Expected u, d, l or r on win_printf %c");
    }
    p++;
}

// -----------------------------------------------------------------------
// %0    clear window

static void wclear(void)
{
    uC_win_clear(w);
}

// -----------------------------------------------------------------------
// just a wrapper for puts which is at the top of this file

static void u_puts(void)
{
    char *s = va_arg(arg, char *);
    uC_win_puts(w, s);
}

// -----------------------------------------------------------------------

static void bold(void)
{
    if (*p == '+' || *p == '-')
    {
        (*p == '+')
            ? uC_win_set_bold(w)
            : uC_win_clr_bold(w);
        p++;
        return;
    }
    xabort("Expected + or - win_printf %%B");
}

// -----------------------------------------------------------------------

static void uline(void)
{
    if (*p == '+' || *p == '-')
    {
        (*p == '+')
            ? uC_win_set_ul(w)
            : uC_win_clr_ul(w);
        p++;
        return;
    }
    xabort("Expected + or - win_printf %U");
}

// -----------------------------------------------------------------------

static void rev(void)
{
    if (*p == '+' || *p == '-')
    {
       (*p == '+')
            ? uC_win_set_rev(w)
            : uC_win_clr_rev(w);
        p++;
        return;
    }
    xabort("Expected + or - win_printf %R");
}

// -----------------------------------------------------------------------
// %e   write eol to window

static void e(void)
{
   uC_win_emit(w, 0x0d);
}

// -----------------------------------------------------------------------
// emit same char a number of times

static void star(void)
{
    int l = va_arg(arg, int);
    int c = va_arg(arg, int);

    while (l != 0)
    {
        uC_win_emit(w, c);
        l--;
    }
}

// -----------------------------------------------------------------------

static uC_switch_t commands[] =
{
    { 'r', &rf   }, { 'f', &f     }, { 'b', &b      }, { '@', &xy },
    { 'x', &x    }, { 'y', &y     }, { 'u', &up     }, { 'd', &dn },
    { 'l', &lt   }, { 'r', &rt    }, { '0', &wclear }, { 'c', &c },
    { 'B', &bold }, { 'U', &uline }, { 'R', rev     }, { 's', u_puts },
    { '8', &utf8 }, { 'e', &e     }, { '*', star    }
};

#define COMMANDS sizeof(commands) / sizeof(commands[0])

// -----------------------------------------------------------------------

static void command(void)
{
    uC_switch(commands, COMMANDS, *p++);
}

// -----------------------------------------------------------------------
// window string writing and window attribute control

// if you need to use normal printf format tags you must first sprintf
// your string into a buffer and escape the format tags within it that
// you want passed to this function...

API void uC_win_printf(uC_window_t *win, char *format, ...)
{
    int32_t codepoint;
    int8_t skip;

    va_start(arg, format);

    p = format;
    w = win;

    while (*p != '\0')
    {
        while ((*p != '%') && (*p != '\0'))
        {
            skip = utf8_decode(&codepoint, p);
            uC_win_emit(win, codepoint);
            p += skip;
        }

        if (*p != '\0')
        {
            p++;
            command();
        }
    }

    va_end(arg);
}

// =======================================================================
