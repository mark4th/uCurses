ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libuCurses.la

libuCurses_la_SOURCES =  \
  h/uCurses.h      \
  src/border.c     \
  src/color.c      \
  src/json.c       \
  src/json_token.c \
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

CFLAGS = -Os -fpic -fno-inline -pedantic -Wall -Werror \
   -Wextra -std=gnu17 -pipe -march=native -g3 \
   -fno-align-functions -m64

#AM_LDFLAGS = -fuse-ld=lld

libuCurses_la_LDFLAGS = \
  -fuse-ld=lld -Wl,--entry=entry -shared -version-info @PACKAGE_VERSION@

## -----------------------------------------------------------------------
## custom build targets i use during devel

strip: uCurses
	strip -R .comment uCurses

## -----------------------------------------------------------------------

scan:
	make clean
	scan-build --use-cc=clang make

## =======================================================================
