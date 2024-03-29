// uC_switch.h
// -----------------------------------------------------------------------

#ifndef UC_SWITCH_H
#define UC_SWITCH_H

// -----------------------------------------------------------------------

#include "uCurses.h"

// -----------------------------------------------------------------------

typedef void (*opt_t)(void);     // re_switch vectors

// -----------------------------------------------------------------------
// c switch statements are FUGLY

typedef struct
{
    int32_t option;
    opt_t vector;
} uC_switch_t;

// -----------------------------------------------------------------------
// visibility hidden

int uC_switch(const uC_switch_t *s, int size, int32_t option);

// -----------------------------------------------------------------------

#else
#endif // UC_SWITCH_H

// =======================================================================
