// uCurses.h
// -----------------------------------------------------------------------

#ifndef UCURSES_H
#define UCURSES_H

// -----------------------------------------------------------------------

#include <stdint.h>

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
API void make_contrast(uint8_t *r, uint8_t *g, uint8_t *b);

// -----------------------------------------------------------------------

#endif // UCURSES_H

// =======================================================================
