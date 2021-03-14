// util.c
// -----------------------------------------------------------------------

#include <inttypes.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

uint16_t cx;                // current cursor position in screen
uint16_t cy;
uint16_t width;             // width and height of screen
uint16_t height;

extern uint64_t params[MAX_PARAM];

// -----------------------------------------------------------------------
// turn cursor off

void curoff(void)
{
    ti_civis();             // what you cant read this ?
}

// -----------------------------------------------------------------------
// turn cursor on

void curon(void)
{
    ti_cnorm();
}

// -----------------------------------------------------------------------
// clear screen

void clear(void)
{
    cx = cy = 0;            // remember position of cursor
    ti_clear();             // wipe display
}

// -----------------------------------------------------------------------
// set cursor position on current line

void hpa(uint16_t x)
{
    params[0] = cx = x;     // horizontal position absolute
    ti_hpa();
}

// -----------------------------------------------------------------------
// set cursor x/y position in console

void cup(uint16_t x, uint16_t y)
{
    params[0] = cx = x;
    params[1] = cy = y;
    ti_cup();
}

// -----------------------------------------------------------------------
// cursor down

void cud1(void)
{
    cy++;
    ti_cud1();
}

// -----------------------------------------------------------------------
// cursor to home location

void home(void)
{
    cx = cy = 0;
    ti_home();
}

// -----------------------------------------------------------------------
// cursor back one on line

void cub1(void)
{
    if(cx != 0)
    {
        cx--;
        ti_cub1();
    }
}

// -----------------------------------------------------------------------
// cursor forward one on line

void cuf1(void)
{
    if(cx == width)
    {
        cx = 0;
        if(cy != height)
        {
            cy++;
        }
    }
    else
    {
       cx++;
    }
    cup(cx, cy);
}

// -----------------------------------------------------------------------
// cursor up one line

void cuu1(void)
{
    if(cy != 0)
    {
        cy--;
    }
    cup(cx, cy);
}

// -----------------------------------------------------------------------
// delete one character on line

void dch1(void)
{
    if(cx != 0)
    {
        cx--;
        ti_dch();
    }
}

// -----------------------------------------------------------------------
// cursor down multiple lines (this can be done better)

void cud(uint16_t n1)
{
    while(n1)
    {
        cud1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// insert space on line

void ich(void)
{
    cx++;
    ti_ich();
}

// -----------------------------------------------------------------------
// cursor back multiple (this can be done better)

void cub(uint16_t n1)
{
    while(n1)
    {
        cub1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// cursor forward (this can be done better)

void cuf(uint16_t n1)
{
    while(n1)
    {
        cuf1();
        n1--;
    }
}

// -----------------------------------------------------------------------

void cuu(uint16_t n1)
{
    while(n1)
    {
        cuu1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// vertical position absolute

void vpa(uint16_t y1)
{
    params[0] = cy = y1;
    ti_vpa();
}

// -----------------------------------------------------------------------
// look ma, not using the ti_cr() define!

void cr(void)
{
    cx = 0;
    if(cy != height)
    {
        cy++;
    }
    cup(cx, cy);
}

// -----------------------------------------------------------------------
// enable cursor keys

void smkx(void) { ti_smkx(); }
void rmkx(void) { ti_rmkx(); }

// =======================================================================
