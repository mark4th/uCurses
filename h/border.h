// border.h
// -----------------------------------------------------------------------

    #pragma once

// -----------------------------------------------------------------------

typedef enum
{
    BDR_TOP_LEFT,
    BDR_TOP_RIGHT,
    BDR_BOTTOM_LEFT,
    BDR_BOTTOM_RIGHT,
    BDR_HORIZONTAL,
    BDR_VERTICAL,
    BDR_LEFT_T,
    BDR_RIGHT_T,
    BDR_TOP_T,
    BDR_BOTTOM_T,
    BDR_CROSS,
} border_t;

// -----------------------------------------------------------------------

enum
{
    BDR_SINGLE,
    BDR_DOUBLE,
    BDR_CURVED
};

// =======================================================================
