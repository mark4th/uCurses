// border.c  -- draw border round window
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/border.h"
#include "h/tui.h"

// -----------------------------------------------------------------------
// draw border character directly into screen buffer 1

static void draw_char(window_t *win, uint16_t cx,
    uint16_t cy, uint32_t code)
{
    screen_t *scr = win->screen;
    cell_t *p1 = &scr->buffer1[(cy * scr->width) + cx];

    memcpy(&p1->attrs, &win->bdr_attrs, 8);

    p1->code = code;
}

// -----------------------------------------------------------------------
// draw one entire row of a border.

// this fills the window content with the blank character for rows that
// are not the top or bottom rows.  this is why drawing of a border must
// happen before drawing of a windows contents

static void draw_top_bottom(window_t *win, uint32_t c1,
    uint32_t c2, uint32_t c3, uint16_t cy)
{
    uint16_t cx = win->xco - 1;

    draw_char(win, cx, cy, c1);

    for(cx = win->xco; cx < win->xco + win->width; cx++)
    {
        draw_char(win, cx, cy, c2);
    }

    draw_char(win, cx, cy, c3);
}

// -----------------------------------------------------------------------

static void draw_mid_row(window_t *win, uint32_t c1, uint32_t c3, uint16_t cy)
{
    draw_char(win, win->xco - 1 , cy, c1);
    draw_char(win, win->xco + win->width , cy, c3);
}

// -----------------------------------------------------------------------
// window borders are not drawn into windows but directly into the screen

void win_draw_borders(window_t *win)
{
    uint16_t width, height;
    uint16_t cy;

    height = win->height;
    width  = win->width;

    // a windows position is not allowed to put its border outside the bounds
    // of its parent screen

    cy = win->yco - 1;

    // draw top row of windows border which is always a signle line border
    // i may offer the choice of using either a single or double line border

    draw_top_bottom(win,
        bdr_singles[BDR_TOP_LEFT],
        bdr_singles[BDR_HORIZONTAL],
        bdr_singles[BDR_TOP_RIGHT], cy++);

    // draw all rows between the top and bottom row

    while(0 != height)
    {
        draw_mid_row(win,
            bdr_singles[BDR_VERTICAL],
            bdr_singles[BDR_VERTICAL], cy++);
        height--;
    }

    // draw bottom row of the border

    draw_top_bottom(win,
        bdr_singles[BDR_BOTTOM_LEFT],
        bdr_singles[BDR_HORIZONTAL],
        bdr_singles[BDR_BOTTOM_RIGHT], cy);
}

// =======================================================================
