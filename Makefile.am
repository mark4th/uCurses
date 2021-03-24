ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libuCurses_0.01.la

libuCurses_0_01_la_SOURCES =  \
  h/uCurses.h      \
  src/border.c     \
  src/color.c      \
  src/keys.c       \
  src/list.c       \
  src/menus.c      \
  src/parse.c      \
  src/reswitch.c   \
  src/screen.c     \
  src/tifile.c     \
  src/utf8.c       \
  src/util.c       \
  src/window.c     \
  src/win_printf.c

AM_CFLAGS = -Os -fpic -fno-inline -pedantic -Wall -Werror \
   -Wextra -std=gnu17 -pipe -march=native -g3 \
   -fno-align-functions

AM_LDFLAGS = -fuse-ld=lld -shared

## -----------------------------------------------------------------------
## custom build targets i use during devel

strip: uCurses
	strip -R .comment uCurses

## -----------------------------------------------------------------------

scan:
	make clean
	scan-build --use-cc=clang make

## =======================================================================
