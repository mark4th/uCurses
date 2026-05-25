// uC_win_printf.c     printf strings into uC_window_t
// -----------------------------------------------------------------------
//
// Format specifiers (none are standard printf specifiers):
//
//   Cursor movement within window:
//     %@(x,y)   move cursor to absolute (x, y)
//     %x(x)     move cursor to column x on current line
//     %y(y)     move cursor to row y in current column
//     %cu       move cursor up one line
//     %cd       move cursor down one line (scrolls if at bottom and unlocked)
//     %cl       move cursor left one column (wraps to line above)
//     %cr       move cursor right one column (wraps to line below)
//
//   Window operations:
//     %P(x,y)   move window to (x, y) within parent screen
//     %0        clear window and home cursor
//     %e        carriage return / line feed (scrolls if at bottom and unlocked)
//
//   Scroll / pan (n = number of lines or columns):
//     %up(n)    scroll window up n lines   (clears bottom n lines)
//     %dn(n)    scroll window down n lines (clears top n lines)
//     %lt(n)    pan window left n columns  (clears rightmost n columns)
//     %rt(n)    pan window right n columns (clears leftmost n columns)
//
//   Foreground color:
//     %fc(c)      8-bit palette color index
//     %fs(gray)   grayscale index
//     %rf(r,g,b)  24-bit RGB
//
//   Background color:
//     %bc(c)      8-bit palette color index
//     %bs(gray)   grayscale index
//     %rb(r,g,b)  24-bit RGB
//
//   Text attributes:
//     %B+  enable bold        %B-  disable bold
//     %U+  enable underline   %U-  disable underline
//     %R+  enable reverse     %R-  disable reverse
//
//   Output:
//     %s(str)        write string at cursor
//     %8(codepoint)  write single UTF-8 character
//     %*(n,c)        write character c exactly n times
//
// NOTE: %0 is "clear window", NOT printf-style zero-padding.
//       "%02d" hits %0 and clears the window — use snprintf then %s instead.

// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_switch.h"
#include "uC_window.h"
#include "uC_utf8.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------
// this file is a blatant MISRA C violation :)
// -----------------------------------------------------------------------

static va_list arg;
static const char *p;
static uC_window_t *w;

// -----------------------------------------------------------------------
// write string into specified window at its current cursor location

API void uC_win_puts(uC_window_t *win, const char *p)
{
    uint8_t skip;
    uint32_t codepoint;

    while (*p != '\0')
    {
        skip = utf8_decode(&codepoint, (uint8_t *)p);
        uC_win_emit(win, codepoint);
        p += skip;
    }
}

// =======================================================================
// control specifiers
// =======================================================================

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
    uC_win_set_cx(w, va_arg(arg, int));
}

// -----------------------------------------------------------------------
// %y   set cursor y in window

static void y(void)
{
    uC_win_set_cy(w, va_arg(arg, int));
}

// -----------------------------------------------------------------------
// %cu %cd %cl %cr    move cursor up, down, left or right in window

static void c(void)
{
    switch (*p)
    {
        case 'u': uC_win_crsr_up(w); break;
        case 'd': uC_win_crsr_dn(w); break;
        case 'l': uC_win_crsr_lt(w); break;
        case 'r': uC_win_crsr_rt(w); break;
        default:
            uC_abort("Expected u, d, l or r on win_printf %c");
    }
    p++;
}

// -----------------------------------------------------------------------
// %P  set window X/Y position within screen

static void P(void)
{
    int x = va_arg(arg, int);
    int y = va_arg(arg, int);

    uC_win_set_pos(w, x, y);
}

// -----------------------------------------------------------------------
// %up  scroll window up specified amount

static void up(void)
{
    int y = va_arg(arg, int);

    if (*p == 'p')
    {
        uC_win_scroll_up_n(w, (int16_t)y);
        p++;
        return;
    }
    uC_abort("Expected p on win_printf %u");
}

// -----------------------------------------------------------------------
// %dn   scroll window down specified amount

static void dn(void)
{
    int y = va_arg(arg, int);

    if (*p == 'n')
    {
        uC_win_scroll_dn_n(w, (int16_t)y);
        p++;
        return;
    }
    uC_abort("Expected n on win_printf %d");
}

// -----------------------------------------------------------------------
// %lt   scroll window left specified amount

static void lt(void)
{
    int x = va_arg(arg, int);

    if (*p == 't')
    {
        uC_win_scroll_lt_n(w, (int16_t)x);
        p++;
        return;
    }
    uC_abort("Expected t on win_printf %l");
}

// -----------------------------------------------------------------------
// %rt    scroll window right specified amount

static void rt(void)
{
    int x = va_arg(arg, int);

    if (*p == 't')
    {
        uC_win_scroll_rt_n(w, (int16_t)x);
        p++;
        return;
    }
    uC_abort("Expected t or f or b on win_printf %r");
}

// -----------------------------------------------------------------------
// %0    clear window

static void wclear(void)
{
    uC_win_clear(w);
}

// =======================================================================
// attribute specifiers
// =======================================================================

// -----------------------------------------------------------------------
// %rf or %rb  set 24-bit rgb fg / bg   or %rt (delegates to rt above)

static void r(void)
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
    rt();
}

// -----------------------------------------------------------------------
// %fc or %fs  set foreground indexed color or gray scale

static void f(void)
{
    int f;

    if (*p == 'c' || *p == 's')
    {
        f = va_arg(arg, int);
        (*p == 'c')
            ? uC_win_set_fg(w, f)
            : uC_win_set_gray_fg(w, f % 23);
        p++;
        return;
    }
    uC_abort("Expected c or s on win_printf %f");
}

// -----------------------------------------------------------------------
// %bc or %bs  set background indexed color or gray scale

static void b(void)
{
    int b;

    if (*p == 'c' || *p == 's')
    {
        b = va_arg(arg, int);
        (*p == 'c')
            ? uC_win_set_bg(w, b)
            : uC_win_set_gray_bg(w, b % 23);
        p++;
        return;
    }
    uC_abort("Expected c or s on win_printf %b");
}

// -----------------------------------------------------------------------
// %B+ or %B-   turn bold on or off

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
    uC_abort("Expected + or - win_printf %%B");
}

// -----------------------------------------------------------------------
// %U+ or %U-   turn underline on or off

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
    uC_abort("Expected + or - win_printf %U");
}

// -----------------------------------------------------------------------
// %R+ or %R-   turn reverse video on or off

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
    uC_abort("Expected + or - win_printf %R");
}

// =======================================================================
// output specifiers
// =======================================================================

// -----------------------------------------------------------------------
// %s   print string into window

static void u_puts(void)
{
    const char *s = va_arg(arg, const char *);
    uC_win_puts(w, s);
}

// -----------------------------------------------------------------------
// %8    draw UTF-8 codepoint

static void utf8(void)
{
    uC_win_emit(w, va_arg(arg, int));
}

// -----------------------------------------------------------------------
// %e   write eol to window

static void e(void)
{
    uC_win_emit(w, 0x0d);
}

// -----------------------------------------------------------------------
// %*   emit same char n times

static void star(void)
{
    int l = va_arg(arg, int);
    int c = va_arg(arg, int);

    while (l-- != 0) { uC_win_emit(w, c); }
}

// =======================================================================

static uC_switch_t commands[] =
{
    // control: cursor
    { '@', &xy     }, { 'x', &x      }, { 'y', &y     },
    { 'c', &c      }, { 'P', &P      },
    // control: scroll
    { 'u', &up     }, { 'd', &dn     }, { 'l', &lt    },
    // control: window
    { '0', &wclear },
    // attribs: color
    { 'r', &r      }, { 'f', &f      }, { 'b', &b     },
    // attribs: style
    { 'B', &bold   }, { 'U', &uline  }, { 'R', rev    },
    // output
    { 's', u_puts  }, { '8', &utf8   }, { 'e', &e     },
    { '*', star    }
};

#define COMMANDS sizeof(commands) / sizeof(commands[0])

// -----------------------------------------------------------------------

static void specifier(void)
{
    uC_switch(commands, COMMANDS, *p++);
}

// -----------------------------------------------------------------------

static void do_win_printf(void)
{
    uint32_t codepoint;
    uint8_t  skip;

    while (*p != '\0')
    {
        while ((*p != '%') && (*p != '\0'))
        {
            skip = utf8_decode(&codepoint, (uint8_t *)p);
            uC_win_emit(w, codepoint);
            p += skip;
        }

        if (*p != '\0')
        {
            p++;
            specifier();
        }
    }
}

// -----------------------------------------------------------------------

API void uC_win_vprintf(uC_window_t *win, const char *format, va_list args)
{
    p = format;
    w = win;
    va_copy(arg, args);
    do_win_printf();
    va_end(arg);
}

// -----------------------------------------------------------------------

API void uC_win_printf(uC_window_t *win, const char *format, ...)
{
    p = format;
    w = win;
    va_start(arg, format);
    do_win_printf();
    va_end(arg);
}

// =======================================================================
