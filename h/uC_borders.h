// uC_borders.h
// -----------------------------------------------------------------------

#ifndef UC_BORDERS_H
#define UC_BORDERS_H

// -----------------------------------------------------------------------

#include "uC_window.h"

// -----------------------------------------------------------------------

typedef enum
{
    uC_BDR_NONE,
    uC_BDR_SINGLE,
    uC_BDR_DOUBLE,
    uC_BDR_CURVED
} __attribute__((__packed__)) uC_border_type_t;

// -----------------------------------------------------------------------

typedef enum
{
    BDR_TOP_LEFT,    BDR_TOP_RIGHT,
    BDR_BOTTOM_LEFT, BDR_BOTTOM_RIGHT,
    BDR_HORIZONTAL,  BDR_VERTICAL,
    BDR_LEFT_T,      BDR_RIGHT_T,
    BDR_TOP_T,       BDR_BOTTOM_T,
    BDR_CROSS,
    zzz = 0xffff,      // bs item so it packs to 16 bits
} __attribute__((__packed__)) border_t;

// -----------------------------------------------------------------------
// visibility hidden

// borders are automagically drawn into a screen around the outide of any
// window that is defined to have a border.

void win_draw_borders(uC_window_t *win);

// -----------------------------------------------------------------------
// visibility normal   (do these belong in window_draw.c ?)

// a box is like a border but is drawn directly into a window but must fit
// entirely within that window.   the x and y coordinates here are within
// the window not the screen.

API void uC_window_clear_box(uC_window_t *win,
    uint16_t xco, uint16_t yco,
    uint16_t width, uint16_t height);

API void uC_win_draw_box(uC_window_t *win,
    uint16_t xco, uint16_t yco,
    uint16_t width, uint16_t height,
    uC_border_type_t bdr_type,
    uC_attribs_t box_attrs);

// -----------------------------------------------------------------------

#endif // UC_BORDERS_H

// =======================================================================
