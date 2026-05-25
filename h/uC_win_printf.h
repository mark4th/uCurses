// uC_win_printf.h
// -----------------------------------------------------------------------

#ifndef UC_WIN_PRINTF_H
#define UC_WIN_PRINTF_H

// -----------------------------------------------------------------------

#include <stdarg.h>

#include "uCurses.h"
#include "uC_window.h"

// UC_XY, UC_WH, UC_RGB grouping macros are defined in uCurses.h

// -----------------------------------------------------------------------

API void uC_win_puts(uC_window_t *win, const char *p);

// Format specifiers (args in parens where required):
//   cursor:   %@(x,y)  %x(x)  %y(y)  %cu  %cd  %cl  %cr
//   window:   %P(x,y)  %0(clear+home)  %e(crlf)
//   scroll:   %up(n)  %dn(n)  %lt(n)  %rt(n)
//   fg color: %fc(c)  %fs(gray)  %rf(r,g,b)
//   bg color: %bc(c)  %bs(gray)  %rb(r,g,b)
//   attribs:  %B+/-  %U+/-  %R+/-
//   output:   %s(str)  %8(codepoint)  %*(n,c)
// WARNING: %0 clears the window — it is NOT printf zero-padding.

API void uC_win_vprintf(uC_window_t *win, const char *format, va_list args);
API void uC_win_printf(uC_window_t *win, const char *format, ...);

// -----------------------------------------------------------------------

#endif // UC_WIN_PRINTF_H

// =======================================================================
