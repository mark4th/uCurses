// border.c  -- draw border around window
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_borders.h"
#include "uC_win_printf.h"

// -----------------------------------------------------------------------
// force refresh of specific cell

enum
{
    NO_FORCE,               // this is ultra complex to explain and may
    FORCE                   // have a better solution (maybe?)
};

// -----------------------------------------------------------------------
// these three borders give the outer edge characters for a window
// border plus the characters that you would use to segment up that window
// the entries in these arrays are the utf8 codepoints for each char

border_t bdr_single[] =
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

border_t bdr_double[] =
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

border_t bdr_curved[] =
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

border_t *const borders[] =
{
    NULL,                   // uC_BDR_NONE
    &bdr_single[0],
    &bdr_double[0],
    &bdr_curved[0]
};

// -----------------------------------------------------------------------
// draw border character directly into screen buffer 1

static void draw_char(uC_window_t *win, int16_t cx, int16_t cy,
    int32_t code, int16_t force)
{
    uC_screen_t *scr = win->screen;

    int16_t index = (cy * scr->width) + cx;

    cell_t *p1 = &scr->buffer1[index];
    cell_t *p2 = &scr->buffer2[index];

    p1->attrs = (win->flags & uC_WIN_FOCUS)
        ? win->focus_attrs
        : win->bdr_attrs;

    p1->code = code;

    // when a double width character is drawn underneath a pulled down
    // menu window such that the menu window overwrites the left edge
    // of said double width character we must force an update of the
    // menu character as this will also cause us to write a space into
    // the DEADC0DE slot of the covered double wide char.
    // if we did not force the right edge of the pulldown menus to be
    // updated we would get glitches in the following DEAD slot
    // as it would not be updated.

    if (force == FORCE)
    {
        p2->code = DEADC0DE;
    }
}

// -----------------------------------------------------------------------
// draw one entire row of a border.

static void draw_top_bottom(uC_window_t *win, int32_t c1, int32_t c2,
    int32_t c3, int16_t cy)
{
    int16_t cx = win->xco;

    draw_char(win, cx - 1, cy, c1, FORCE);

    while (cx < win->xco + win->width)
    {
        draw_char(win, cx, cy, c2, NO_FORCE);
        cx++;
    }

    draw_char(win, cx, cy, c3, FORCE);
}

// -----------------------------------------------------------------------

static void draw_mid_row(uC_window_t *win, int32_t c1, int32_t c3,
    int16_t cy)
{
    draw_char(win, win->xco - 1, cy, c1, NO_FORCE);
    draw_char(win, win->xco + win->width, cy, c3, FORCE);
}

// -----------------------------------------------------------------------
// window borders are not drawn into windows but directly into the screen

void win_draw_borders(uC_window_t *win)
{
    int16_t height;
    int16_t cy;

    border_t *bdr;

    if (win->border_type == uC_BDR_NONE)
    {
        return;
    }

    bdr = borders[win->border_type];

    height = win->height;

    // a windows position is not allowed to put its border outside the
    // bounds of its parent screen

    cy = win->yco - 1;

    // draw top row of windows border which is always a signle line border
    // i may offer the choice of using either a single or double line
    // border

    draw_top_bottom(win,
        bdr[BDR_TOP_LEFT],
        bdr[BDR_HORIZONTAL],
        bdr[BDR_TOP_RIGHT], cy++);

    // draw all rows between the top and bottom row

    while (height != 0)
    {
        draw_mid_row(win,
            bdr[BDR_VERTICAL],
            bdr[BDR_VERTICAL], cy++);
        height--;
    }

    // draw bottom row of the border

    draw_top_bottom(win,
        bdr[BDR_BOTTOM_LEFT],
        bdr[BDR_HORIZONTAL],
        bdr[BDR_BOTTOM_RIGHT], cy);
}

// -----------------------------------------------------------------------
// clear a specified rectangular area within a window

API void uC_window_clear_box(uC_window_t *win,
    uint16_t xco, uint16_t yco, uint16_t width, uint16_t height)
{
    while (height--)
    {
        // %@ set cursor x / y location within window
        // %* print multiple repetitions of char

        uC_win_printf(win, "%@%*", xco, yco++, width, 0x20);
    }
}

// -----------------------------------------------------------------------

// a box has the same visual appearance of a border but is drawn directly
// into the windows buffer not the screen.   there is no concept here of
// forcing an update to any characters that are covered by a box border
// so this code is simpler than the functions above

API void uC_win_draw_box(uC_window_t *win,
    uint16_t x, uint16_t y,
    uint16_t width, uint16_t height,
    uC_border_type_t bdr_type,
    uC_attribs_t box_attrs)
{
    border_t *bdr;

    if (bdr_type == uC_BDR_NONE)        { return; }
    if ((x + width  + 1) > win->width)  { return; }
    if ((y + height + 1) > win->height) { return; }

    bdr = borders[bdr_type];

    win->attrs = box_attrs;

    // %@ set cursor location in window
    // %8 print utf8 char (top left)
    // %* repeat print char  (horizontal)
    // %8 print utf8 char (top right)

    uC_win_printf(win, "%@%8%*%8",
        x, y++, bdr[BDR_TOP_LEFT],
        width,  bdr[BDR_HORIZONTAL],
        bdr[BDR_TOP_RIGHT]);

    // draw all rows between the top and bottom row

    // %@ set cursor location within window
    // %8 emit a utf8 character to window
    // %x set the cursor x locaton on the window line
    // %8 emit a utf8 character to window

    while (height != 0)
    {
        uC_win_printf(win, "%@%8%x%8",
            x, y++,    bdr[BDR_VERTICAL],
            x + width + 1, bdr[BDR_VERTICAL]);
        height--;
    }

    // draw bottom row of the border

    uC_win_printf(win, "%@%8%*%8",
        x, y,  bdr[BDR_BOTTOM_LEFT],
        width, bdr[BDR_HORIZONTAL],
        bdr[BDR_BOTTOM_RIGHT]);
}

// =======================================================================
