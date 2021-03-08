// win_printf.c  printf strings into window_t
// -----------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#include "h/tui.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

void win_puts(window_t *win, char *p)
{
    uint8_t skip;
    uint32_t codepoint;

    while('\0' != *p)
    {
        skip = utf8_decode(&codepoint, p);
        win_emit(win, codepoint);
        p += skip;
    }
}

// -----------------------------------------------------------------------
// window string writing and window attribute control

void win_printf(window_t *win, char* format, ...)
{
    char *p;
    va_list arg;

    uint32_t codepoint;
    uint8_t skip;

    uint8_t r, g, b, f, x, y;

    va_start(arg, format);

    for(p = format; *p != '\0'; p++)
    {
        while(*p != '%')
        {
            skip = utf8_decode(&codepoint, p);
            win_emit(win, codepoint);
            p += skip;
            if(*p == '\0') { break; }
        }

        if(*p == '\0') { break; }

        p++;

        switch(*p)
        {
            case 'r':
                r = va_arg(arg, int) % 0x100;
                g = va_arg(arg, int) % 0x100;
                b = va_arg(arg, int) % 0x100;

                switch(p[1])
                {
                    case 'f':    // %fg = set foreground color
                       win_set_rgb_fg(win, r, g, b);
                       p++;
                       break;
                    case 'b':   // %of = set grayscale foreground color
                       win_set_rgb_bg(win, r, g, b);
                       p++;
                       break;
                    default:
                       // must specify %Rf or %Rb
                       // what do do if f/b missing?
                       // silently aborting printf here
                       return;
                }
                break;

            case 'f':
                f = va_arg(arg, int);

                switch(p[1])
                {
                    case 'c':      // %fc set foreground color
                        win_set_fg(win, f % 16);
                        p++;
                        break;
                    case 's':      // set foreground grayscale
                        win_set_gray_fg(win, f % 21);
                        p++;
                        break;
                    default:
                        return;
                }
                break;

            case 'b':
                b = va_arg(arg, int);

                switch(p[1])
                {
                    case 'c':       // %bc set background color
                        win_set_bg(win, b % 16);
                        p++;
                        break;
                    case 's':       // %bs set background gray scale
                        win_set_bg(win, b % 21);
                        p++;
                        break;
                    default:
                        return;
                }
                break;

            case '@':    // set cursor X/Y
                x = va_arg(arg, int);
                y = va_arg(arg, int);

                win_cup(win, x, y);
                break;

            case 'x':
                x = va_arg(arg, int);
                win_set_cx(win, x);
                break;

            case 'y':
                y = va_arg(arg, int);
                win_set_cy(win, y);
                break;

            case '^':
                y = va_arg(arg, int);
                while(0 != y)
                {
                    win_scroll_up(win);
                }
                break;
            case 'v':
                y = va_arg(arg, int);
                while(0 != y)
                {
                    win_scroll_dn(win);
                }
                break;
            case '<':
                x = va_arg(arg, int);
                while(0 != x)
                {
                    win_scroll_lt(win);
                }
                break;
            case '>':
                x = va_arg(arg, int);
                while(0 != x)
                {
                    win_scroll_rt(win);
                }
                break;
            case '0':
                win_clear(win);
                break;
            case 'c':
                switch(p[1])
                {
                    case '^':
                        win_crsr_up(win);
                        p++;
                        break;
                    case 'v':
                        win_crsr_dn(win);
                        p++;
                        break;
                    case '<':
                        win_crsr_lt(win);
                        p++;
                        break;
                    case '>':
                        win_crsr_rt(win);
                        p++;
                        break;
                }
        }
    }

    va_end(arg);
}

// =======================================================================
