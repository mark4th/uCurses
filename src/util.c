// util.c
// -----------------------------------------------------------------------

#include <errno.h>
#include <inttypes.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

int16_t cx; // current cursor position in screen
int16_t cy;
int16_t width; // width and height of screen
int16_t height;

extern struct termios term_save;

// -----------------------------------------------------------------------

void clock_sleep(int32_t when)
{
    struct timespec tv;
    struct timespec remain;
    int rv;

    do
    {
        tv.tv_sec = 0;
        tv.tv_nsec = when;
        rv = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &remain);
        tv = remain;
    } while(EINTR == rv);
}

// -----------------------------------------------------------------------
// turn cursor off

void curoff(void)
{
    ti_civis(); // what you cant read this ?
}

// -----------------------------------------------------------------------
// turn cursor on

void curon(void) { ti_cnorm(); }

// -----------------------------------------------------------------------
// clear screen

void clear(void)
{
    cx = cy = 0; // remember position of cursor
    ti_clear();  // wipe display
}

// -----------------------------------------------------------------------
// set cursor position on current line

void hpa(int16_t x)
{
    params[0] = cx = x; // horizontal position absolute
    ti_hpa();
}

// -----------------------------------------------------------------------
// set cursor x/y position in console

void cup(int16_t x, int16_t y)
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

void cud(int16_t n1)
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

void cub(int16_t n1)
{
    while(n1)
    {
        cub1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// cursor forward (this can be done better)

void cuf(int16_t n1)
{
    while(n1)
    {
        cuf1();
        n1--;
    }
}

// -----------------------------------------------------------------------

void cuu(int16_t n1)
{
    while(n1)
    {
        cuu1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// vertical position absolute

void vpa(int16_t y1)
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

// -----------------------------------------------------------------------

void restore_term(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &term_save);
    curon();
    flush();
}

// =======================================================================
