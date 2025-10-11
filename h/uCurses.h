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

#define UCURSES_NAN (0x7fff)  // not a number (honest!)
#define DEADC0DE (0xdeadc0de)

// -----------------------------------------------------------------------

typedef void (*fp_t)(void *x);
typedef void (*user_winch_t)(void);

// -----------------------------------------------------------------------

typedef struct
{
    char *json;
    int len;
} __attribute__((__packed__)) json_mem_t;

// -----------------------------------------------------------------------

API void uCurses_init(char *file, json_mem_t *json, void *fp);
API void uCurses_deInit(void);
API void entry(void);
API void make_contrast(uint8_t *r, uint8_t *g, uint8_t *b);
API void uC_register_winch(user_winch_t handler);
API void uC_deregister_winch(user_winch_t handler);

// -----------------------------------------------------------------------

#endif // UCURSES_H

// =======================================================================
