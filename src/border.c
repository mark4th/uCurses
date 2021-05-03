// border.c  -- draw border round window
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------
// these three borders give the outer edge characters for a window
// border plus the characters that you would use to segment up that window
// the entries in these arrays are the utf8 codepoint for each char

static border_t bdr_single[] =
{
    0x250f,                 // ┏        ┏━━━┳━━━┓
    0x2513,                 //  ┓       ┃   ┃   ┃
    0x2517,                 // ┗        ┣━━━╋━━━┫
    0x251b,                 //  ┛       ┃   ┃   ┃
    0x2501,                 // ━        ┗━━━┻━━━┛
    0x2503,                 //  ┃
    0x2523,                 // ┣
    0x252b,                 //  ┫
    0x2533,                 // ┳
    0x253b,                 //  ┻
    0x254b,                 // ╋
};

// -----------------------------------------------------------------------

static border_t bdr_double[] =
{
    0x2554,                 // ╔        ╔═══╦═══╗
    0x2557,                 //  ╗       ║   ║   ║
    0x255a,                 // ╚        ╠═══╬═══╣
    0x255d,                 //  ╝       ║   ║   ║
    0x2550,                 // ═        ╚═══╩═══╝
    0x2551,                 //  ║
    0x2560,                 // ╠
    0x2563,                 //  ╣
    0x2566,                 // ╦
    0x2569,                 //  ╩
    0x256c,                 // ╬
};

// -----------------------------------------------------------------------

static border_t bdr_curved[] =
{
    0x256d,                 // ╭        ╭───┬───╮
    0x256e,                 //  ╮       │   │   │
    0x2570,                 // ╰        ├───┼───┤
    0x256f,                 //  ╯       │   │   │
    0x2500,                 // ─        ╰───┴───╯
    0x2502,                 //  │
    0x251c,                 // ├
    0x2524,                 //  ┤
    0x252c,                 // ┬
    0x2534,                 //  ┴
    0x253c,                 // ┼
};

// -----------------------------------------------------------------------
// draw border character directly into screen buffer 1

static void draw_char(window_t *win, uint16_t cx,
    uint16_t cy, uint32_t code)
{
    screen_t *scr = win->screen;
    uint16_t index = (cy * scr->width) + cx;

    cell_t *p1 = &scr->buffer1[index];

    *(uint64_t *)p1->attrs = *(uint64_t *)win->bdr_attrs;
    p1->code = code;
}

// -----------------------------------------------------------------------
// draw one entire row of a border.

static INLINE void draw_top_bottom(window_t *win, uint32_t c1,
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

static INLINE void draw_mid_row(window_t *win,
      uint32_t c1, uint32_t c3, uint16_t cy)
{
    draw_char(win, win->xco - 1, cy, c1);
    draw_char(win, win->xco + win->width, cy, c3);
}

// -----------------------------------------------------------------------
// window borders are not drawn into windows but directly into the screen

void win_draw_borders(window_t *win)
{
    uint16_t height;
    uint16_t cy;

    border_t *borders[] =
    {
        &bdr_single[0],
        &bdr_double[0],
        &bdr_curved[0]
    };

    border_t *b;

    b = borders[win->bdr_type];

    height = win->height;

    // a windows position is not allowed to put its border outside the bounds
    // of its parent screen

    cy = win->yco - 1;

    // draw top row of windows border which is always a signle line border
    // i may offer the choice of using either a single or double line border

    draw_top_bottom(win,
        b[BDR_TOP_LEFT],
        b[BDR_HORIZONTAL],
        b[BDR_TOP_RIGHT], cy++);

    // draw all rows between the top and bottom row

    while(0 != height)
    {
        draw_mid_row(win,
            b[BDR_VERTICAL],
            b[BDR_VERTICAL], cy++);
        height--;
    }

    // draw bottom row of the border

    draw_top_bottom(win,
        b[BDR_BOTTOM_LEFT],
        b[BDR_HORIZONTAL],
        b[BDR_BOTTOM_RIGHT], cy);
}

// =======================================================================
