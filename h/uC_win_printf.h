// uC_win_printf.h
// -----------------------------------------------------------------------

#ifndef UC_WIN_PRINTF_H
#define UC_WIN_PRINTF_H

// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_window.h"

// -----------------------------------------------------------------------

API void uC_win_puts(uC_window_t *win, const char *p);

// format specifiers (args in parens where required):
//   cursor:   %@(x,y)  %x(x)  %y(y)  %cu  %cd  %cl  %cr
//   window:   %P(x,y)  %0
//   scroll:   %up(n)  %dn(n)  %lt(n)  %rt(n)
//   fg color: %fc(c)  %fs(gray)  %rf(r,g,b)
//   bg color: %bc(c)  %bs(gray)  %rb(r,g,b)
//   attribs:  %B+/-  %U+/-  %R+/-
//   output:   %s(str)  %8(codepoint)  %e  %*(n,c)
API void uC_win_printf(uC_window_t *win, const char *format, ...);

// -----------------------------------------------------------------------

#endif // UC_WIN_PRINTF_H

// =======================================================================
