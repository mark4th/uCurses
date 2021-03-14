bin_PROGRAMS = u
u_SOURCES = \
  src/border.c     \
  src/color.c      \
  src/keys.c       \
  src/list.c       \
  src/main.c       \
  src/parse.c      \
  src/screen.c     \
  src/tifile.c     \
  src/utf8.c       \
  src/util.c       \
  src/window.c     \
  src/win_printf.c \
  h/border.h       \
  h/color.h        \
  h/list.h         \
  h/tui.h          \
  h/uCurses.h      \
  h/util.h

AM_CFLAGS = -Os -fno-inline -pedantic -Wall -Werror \
  -std=gnu17 -pipe -march=native -g3

## -----------------------------------------------------------------------

strip: u
	strip -R .comment u

## -----------------------------------------------------------------------

scan:
	make clean
	scan-build make

## =======================================================================
