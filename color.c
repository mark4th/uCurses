// color.c  - uCurses colors and attributes setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>

#include "h/color.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

uint16_t attr;                // current attribs and fg/bg

static uint16_t old_attr;
// TODO: bit flags
static uint8_t acs_flag = 0;  // 1 = alt charset selected
static uint8_t acs_old  = 0;  // old acs flag (sense changes)

uint8_t default_bg = BLACK;
uint8_t default_fg = WHITE;

// -----------------------------------------------------------------------

static void noop(void){;}

// -----------------------------------------------------------------------

uint8_t fg(void) { return attr & 0x0f; }
uint8_t bg(void) { return (attr & 0xf0) >> 4; }

// -----------------------------------------------------------------------

void set_fg(uint8_t c)
{
  c &= 0x0f;                // ensure color is sane
  attr &= 0xfffffff0;       // set fg in master attribute value
  attr |= c;
  params[0] = c;            // pass parameter directly to setaf
  ti_setaf();
}

// -----------------------------------------------------------------------

void set_bg(uint8_t c)
{
  c &= 0x0f;                // ensure color is sane
  attr &= 0xffffff0f;       // set bg in master attribute value
  attr |= (c << 4);
  params[0] = c;            // pass parameter directly to setab
  ti_setab();
}

// -----------------------------------------------------------------------

static void _set_acs(void)
{
  if(acs_flag ^ acs_old)
  {
    acs_flag ? ti_smacs() : ti_rmacs();
    acs_old  = acs_flag;
  }
}

// -----------------------------------------------------------------------

void set_attribs(void)
{
  uint16_t a;
  uint8_t f, b;

  a = ((attr ^ old_attr) >> 16);

  if(0 != a)
  {
    old_attr = attr;

    // these 3 dont have an rmxxx
    if((a & BLINK) || (a & DIM) | (a & BOLD))
    {
      ti_sgr0();
    }

    f = attr & 0x0f;
    b = (attr >> 4) & 0x0f;

    if(b != bg())    { set_bg(b); }
    if(f != fg())    { set_fg(f); }

    a = attr >> 16;

    (a & STANDOUT)  ? ti_smso()  : ti_rmso();
    (a & UNDERLINE) ? ti_smul()  : ti_rmul();
    (a & REVERSE)   ? ti_rev()   : ti_rum();
    (a & BLINK)     ? ti_blink() : noop();
    (a & DIM)       ? ti_dim()   : noop();
    (a & BOLD)      ? ti_bold()  : noop();
    // blank
    // prot
  }
  _set_acs();
}

// -----------------------------------------------------------------------

static void set_attr(ti_attrib_t a)
{
  attr |= (a << 16);
  set_attribs();
}

// -----------------------------------------------------------------------

static void clr_attr(ti_attrib_t a)
{
  attr &= ~(a << 16);
  set_attribs();
}

// -----------------------------------------------------------------------

void set_acs(void) { acs_flag = 1; set_attribs(); }
void clr_acs(void) { acs_flag = 0; set_attribs(); }

// -----------------------------------------------------------------------
// set individual attribs

void set_so(void)    { set_attr(STANDOUT);  }
void set_ul(void)    { set_attr(UNDERLINE); }
void set_rev(void)   { set_attr(REVERSE);   }
void set_bold(void)  { set_attr(BOLD);      }
void set_blink(void) { set_attr(BLINK);     }
void set_dim(void)   { set_attr(DIM);       }

// -----------------------------------------------------------------------
// clear individual attribs

void clr_so(void)    { clr_attr(STANDOUT);  }
void clr_ul(void)    { clr_attr(UNDERLINE); }
void clr_rev(void)   { clr_attr(REVERSE);   }
void clr_bold(void)  { clr_attr(BOLD);      }
void clr_blink(void) { clr_attr(BLINK);     }
void clr_dim(void)   { clr_attr(DIM);       }

// -----------------------------------------------------------------------
// clear all attributes, reset colors to configurable defaults

void set_norm(void)
{
  acs_flag = 0;
  attr = (default_bg << 4) | default_fg;

  set_attribs();
}

// =======================================================================
