bin_PROGRAMS = u
u_SOURCES = \
  src/border.c  src/keys.c  src/main.c   src/parse.c  \
  src/tifile.c  src/util.c  src/win_printf.c          \
  src/color.c   src/list.c  src/screen.c src/utf8.c   \
  src/window.c                                        \
  h/border.h    h/color.h   h/list.h     h/tui.h      \
  h/uCurses.h   h/util.h

AM_CFLAGS = -Os -fno-inline -fPIC -fdelete-null-pointer-checks \
  -pedantic -Wall -Werror -std=gnu17 -pipe -march=native -g3 -xc

