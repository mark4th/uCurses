#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libuCurses], [0.01],,[github.com/mark4th/uCurses])
AC_CONFIG_SRCDIR([src/screen.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC
AM_PROG_LIBTOOL
AC_CONFIG_MACRO_DIRS([m4])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h locale.h stdint.h stdlib.h string.h sys/ioctl.h termios.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([memmove memset setlocale wcwidth])

AC_OUTPUT
