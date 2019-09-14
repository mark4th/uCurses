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
// clear screen

void clear(void)
{
  cx = cy = 0;              // remember position of cursor
  ti_clear();               // wipe display
}

// -----------------------------------------------------------------------
// set cursor position on current line

void hpa(uint16_t x)
{
  params[0] = cx = x;
  ti_hpa();
}

// -----------------------------------------------------------------------
// set cursor position in console

void cup(uint16_t x, uint16_t y)
{
  params[0] = cx = x;
  params[1] = cy = y;
  ti_cup();
}

// -----------------------------------------------------------------------

void cud1(void)
{
  cy++;
  ti_cud1();
}

// -----------------------------------------------------------------------

void home(void)
{
  cx = cy = 0;
  ti_home();
}

// -----------------------------------------------------------------------

void cub1(void)
{
  if(0 != cx)
  {
    cx--;
    ti_cub1();
  }
}

// -----------------------------------------------------------------------

void cuf1(void)
{
  if(cx == width)
  {
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

void cuu1(void)
{
  if(0 != cy)
  {
    cy--;
  }
  cup(cx, cy);
}

// -----------------------------------------------------------------------

void dch1(void)
{
  if(0 != cx)
  {
    cx--;
    ti_dch();
  }
}

// -----------------------------------------------------------------------

void cud(uint16_t n1)
{
  while(n1)
  {
    cud1();
    n1--;
  }
}

// -----------------------------------------------------------------------

void ich(void)
{
  cx++;
  ti_ich();
}

// -----------------------------------------------------------------------

void cub(uint16_t n1)
{
  while(n1)
  {
    cub1();
    n1--;
  }
}

// -----------------------------------------------------------------------

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

void vpa(uint16_t y1)
{
  params[0] = cy = y1;
  ti_vpa();
}

// -----------------------------------------------------------------------

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

void smkx(void) { ti_smkx(); }
void rmkx(void) { ti_rmkx(); }

// =======================================================================
