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
// these are used by the optional JSON parser to build the optional menus

typedef void (*menu_fp_t)(void);
typedef menu_fp_t (*fp_finder_t)(int32_t hash);

// -----------------------------------------------------------------------

typedef struct
{
    char *json;
    int len;
} json_mem_t;

// -----------------------------------------------------------------------

API void uCurses_init(char *file, json_mem_t *json, fp_finder_t fp);
API void uCurses_deInit(void);
API void entry(void);
API void make_contrast(uint8_t *r, uint8_t *g, uint8_t *b);
API void uC_register_winch(user_winch_t handler);
API void uC_deregister_winch(user_winch_t handler);

// -----------------------------------------------------------------------

#endif // UCURSES_H

// =======================================================================
