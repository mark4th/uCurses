// uCurses.h
// -----------------------------------------------------------------------

#ifndef UCURSES_H
#define UCURSES_H

// -----------------------------------------------------------------------

#include <aio.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>

// -----------------------------------------------------------------------
// macro to make public API functions etc visible

#define API __attribute__ ((visibility ("default")))

// -----------------------------------------------------------------------

typedef void (*fp_t)(void *x);

// -----------------------------------------------------------------------

#define UCURSES_NAN (0x7fff)  // not a number (honest!)
#define DEADC0DE (0xdeadc0de)

// -----------------------------------------------------------------------

API void uCurses_init(void);
API void uCurses_deInit(void);
API void entry(void);

// -----------------------------------------------------------------------

#endif // UCURSES_H

// =======================================================================
