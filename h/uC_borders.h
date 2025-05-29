// borders.h
// -----------------------------------------------------------------------

#ifndef UC_BORDERS_H
#define UC_BORDERS_H

#include "uC_window.h"

// -----------------------------------------------------------------------

typedef enum __attribute__((__packed__))
{
    BDR_SINGLE,
    BDR_DOUBLE,
    BDR_CURVED
} uC_border_type_t;

// -----------------------------------------------------------------------

typedef enum
{
    BDR_TOP_LEFT,    BDR_TOP_RIGHT,
    BDR_BOTTOM_LEFT, BDR_BOTTOM_RIGHT,
    BDR_HORIZONTAL,  BDR_VERTICAL,
    BDR_LEFT_T,      BDR_RIGHT_T,
    BDR_TOP_T,       BDR_BOTTOM_T,
    BDR_CROSS,
} border_t;

// -----------------------------------------------------------------------
// visibility hidden

void win_draw_borders(uC_window_t *win);

// -----------------------------------------------------------------------

#endif // UC_BORDERS_H

// =======================================================================
