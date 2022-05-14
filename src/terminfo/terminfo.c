// terminfo.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------

extern ti_parse_t *uC_ti_parse;

// -----------------------------------------------------------------------

int16_t cx;                 // current cursor position in screen
int16_t cy;
int16_t width;              // width and height of screen
int16_t height;

// -----------------------------------------------------------------------

API void uC_curoff(void) { ti_civis(); }   // turn cursor off
API void uC_curon(void)  { ti_cnorm(); }   // turn cursor on

// -----------------------------------------------------------------------
// clear terminal

API void uC_clear(void)
{
    cx = cy = 0;            // remember position of cursor
    ti_clear();             // wipe display
}

// -----------------------------------------------------------------------
// set cursor position on current line

API void uC_hpa(int16_t x)
{
    // horizontal position absolute
    uC_ti_parse->params[0] = cx = x;
    ti_hpa();
}

// -----------------------------------------------------------------------
// set cursor x/y position in console

API void uC_cup(int16_t x, int16_t y)
{
    uC_ti_parse->params[0] = cx = x;
    uC_ti_parse->params[1] = cy = y;
    ti_cup();
}

// -----------------------------------------------------------------------
// cursor down

API void uC_cud1(void)
{
    cy++;
    ti_cud1();
}

// -----------------------------------------------------------------------
// cursor to home location

API void uC_home(void)
{
    cx = cy = 0;
    ti_home();
}

// -----------------------------------------------------------------------
// cursor back one on line

API void uC_cub1(void)
{
    if (cx != 0)
    {
        cx--;
        ti_cub1();
    }
}

// -----------------------------------------------------------------------
// cursor forward one on line

API void uC_cuf1(void)
{
    if (cx == width)
    {
        cx = 0;
        if (cy != height)
        {
            cy++;
        }
    }
    else
    {
        cx++;
    }
    uC_cup(cx, cy);
}

// -----------------------------------------------------------------------
// cursor up one line

API void uC_cuu1(void)
{
    if (cy != 0)
    {
        cy--;
    }
    uC_cup(cx, cy);
}

// -----------------------------------------------------------------------
// delete one character on line

API void uC_dch1(void)
{
    if (cx != 0)
    {
        cx--;
        ti_dch();
    }
}

// -----------------------------------------------------------------------
// cursor down multiple lines (this can be done better)

API void uC_cud(int16_t n1)
{
    while (n1)
    {
        uC_cud1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// insert space on line

API void uC_ich(void)
{
    cx++;
    ti_ich();
}

// -----------------------------------------------------------------------
// cursor back multiple (this can be done better)

API void uC_cub(int16_t n1)
{
    while (n1)
    {
        uC_cub1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// cursor forward (this can be done better)

API void uC_cuf(int16_t n1)
{
    while (n1)
    {
        uC_cuf1();
        n1--;
    }
}

// -----------------------------------------------------------------------

API void uC_cuu(int16_t n1)
{
    while (n1)
    {
        uC_cuu1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// vertical position absolute

API void uC_vpa(int16_t y1)
{
    uC_ti_parse->params[0] = cy = y1;
    ti_vpa();
}

// -----------------------------------------------------------------------
// look ma, not using the ti_cr() define!

API void uC_cr(void)
{
    cx = 0;
    if (cy != height)
    {
        cy++;
    }
    uC_cup(cx, cy);
}

// -----------------------------------------------------------------------
// enable cursor keys

API void uC_smkx(void) { ti_smkx(); }
API void uC_rmkx(void) { ti_rmkx(); }

// =======================================================================
