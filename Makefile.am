
bin_PROGRAMS = uCurses

uCurses_SOURCES = \
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


CFLAGS = -Oz -fpic -fno-inline -pedantic -Wall -Werror \
  -Wextra -std=gnu17 -pipe -march=native -g3 \
  -fno-align-functions -flto
LDFLAGS = -fuse-ld=lld -shared -flto

## -----------------------------------------------------------------------
## custom build targets i use during devel

strip: uCurses
	strip -R .comment uCurses

## -----------------------------------------------------------------------

scan:
	make clean
	scan-build make

## =======================================================================
