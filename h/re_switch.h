// re_switch.h
// -----------------------------------------------------------------------

#ifndef RE_SWITCH_H
#define RE_SWITCH_H

// -----------------------------------------------------------------------

typedef void (*opt_t)(void);     // re_switch vectors

// -----------------------------------------------------------------------
// c switch statements are FUGLY

typedef struct
{
    int32_t option;
    opt_t vector;
} switch_t;

// -----------------------------------------------------------------------
// visibility hidden

int re_switch(const switch_t *s, size_t size, int32_t option);

// -----------------------------------------------------------------------

#endif // RE_SWITCH_H

// =======================================================================
