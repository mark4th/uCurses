// color.c  - uCurses colors and attributes setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "h/color.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

uint8_t attrs[8];
static uint8_t old_attrs[8];

extern uint8_t *f_str;

// -----------------------------------------------------------------------

uint8_t default_bg = BLACK;
uint8_t default_fg = WHITE;

// -----------------------------------------------------------------------
// fg can be a normal color, a gray scale or an RGB value

static void do_set_fg(void)
{
    uint8_t gray_seq[] =
    {
        0x1b, '[', '3', '8', ';', '5', ';',
       '%', 'p', '1', '%', '3', 'd', 'm', 0,
    };

    uint8_t rgb_seq[] =
    {
       0x1b, '[', '3', '8', ';', '2', ';',
       '%', 'p', '1', '%', 'd', ';',
       '%', 'p', '2', '%', 'd', ';',
       '%', 'p', '3', '%', 'd', 'm', 0
    };

    // for normal colors and gray scales
    params[0] = attrs[FG];

    // are we setting a rgb foreground?
    if(attrs[ATTR] & FG_RGB)
    {
        params[0] = attrs[FG_R];
        params[1] = attrs[FG_G];
        params[2] = attrs[FG_B];
        f_str = &rgb_seq[0];
        do_parse_format();
    }
    // are we setting a gray scale foreground?
    else if(attrs[ATTR] & FG_GRAY)
    {
        params[0] += 232;
        f_str = &gray_seq[0];
        do_parse_format();
    }
    // normal fg color
    else
    {
        ti_setaf();
    }
}

// -----------------------------------------------------------------------
// bg can be a normal color, a gray scale or an RGB value

static void do_set_bg(void)
{
    uint8_t gray_seq[] =
    {
       0x1b, '[', '4', '8', ';', '5', ';',
       '%', 'p', '1', '%', 'd', 'm', 0
    };
    uint8_t rgb_seq[] =
    {
       0x1b, '[', '4', '8', ';', '2', ';',
       '%', 'p', '1', '%', 'd', ';',
       '%', 'p', '2', '%', 'd', ';',
       '%', 'p', '3', '%', 'd', 'm', 0
    };

    // for normal colors and gray scales
    params[0] = attrs[BG];

    // are we setting a rgb background?
    if(attrs[ATTR] & BG_RGB)
    {
        params[0] = attrs[BG_R];
        params[1] = attrs[BG_G];
        params[2] = attrs[BG_B];
        f_str = &rgb_seq[0];
        do_parse_format();
    }
    // are we setting a gray scale foreground?
    else if(attrs[ATTR] & BG_GRAY)
    {
        params[0] += 232;
        f_str = &gray_seq[0];
        do_parse_format();
    }
    else
    {
        ti_setab();
    }
}

// -----------------------------------------------------------------------

static void fix_bbr(void)
{
    ti_sgr0();

    if(attrs[ATTR] & BLINK)    { ti_blink(); }
    if(attrs[ATTR] & BOLD)     { ti_bold();  }
    if(attrs[ATTR] & REVERSE)   { ti_rev();  }

    // ti_sgr0() removed this

    if(attrs[ATTR] & UNDERLINE) { ti_smul(); }
}

// -----------------------------------------------------------------------

static void fix_ul(uint8_t changes)
{
    // if underline changed
    if(changes & UNDERLINE)
    {
        (attrs[ATTR] & UNDERLINE) ? ti_smul() : ti_rmul();
    }
}

// -----------------------------------------------------------------------

void set_attribs(void)
{
    uint8_t changes;

    changes = attrs[ATTR] ^ old_attrs[ATTR];
    if(0 != changes)
    {
        ((changes & BLINK) || (changes & BOLD) || (changes & REVERSE))
            ? fix_bbr()
            : fix_ul(changes);
    }

    if((attrs[BG]   != old_attrs[BG]) ||
       (attrs[BG_R] != old_attrs[BG_R]) ||
       (attrs[BG_G] != old_attrs[BG_G]) ||
       (attrs[BG_B] != old_attrs[BG_B]) ||
       (changes & BG_GRAY) || (changes & BG_RGB))
    {
        do_set_bg();
    }

    if((attrs[FG]   != old_attrs[FG]) ||
       (attrs[FG_R] != old_attrs[FG_R]) ||
       (attrs[FG_G] != old_attrs[FG_G]) ||
       (attrs[FG_B] != old_attrs[FG_B]) ||
       (changes & FG_GRAY) || (changes & FG_RGB))
    {
        do_set_fg();
    }

    memcpy(old_attrs, attrs, 8);
}

// -----------------------------------------------------------------------

static void set_attr(ti_attrib_t attr)
{
    attrs[ATTR] |= attr;

    // gray scales and rgb are mutually exclusive
    if(FG_RGB == attr)   { attrs[ATTR] &= ~FG_GRAY; }
    if(BG_RGB == attr)   { attrs[ATTR] &= ~BG_GRAY; }
    if(FG_GRAY == attr)  { attrs[ATTR] &= ~FG_RGB;  }
    if(BG_GRAY == attr)  { attrs[ATTR] &= ~BG_RGB;  }

    set_attribs();
}

// -----------------------------------------------------------------------

static void clr_attr(ti_attrib_t attr)
{
    attrs[ATTR] &= ~attr;
    set_attribs();
}

// -----------------------------------------------------------------------
// set individual attribs

void set_ul(void)    { set_attr(UNDERLINE); }
void set_rev(void)   { set_attr(REVERSE);   }
void set_bold(void)  { set_attr(BOLD);      }
void set_blink(void) { set_attr(BLINK);     }

// -----------------------------------------------------------------------
// clear individual attribs

void clr_ul(void)    { clr_attr(UNDERLINE); }
void clr_rev(void)   { clr_attr(REVERSE);   }
void clr_bold(void)  { clr_attr(BOLD);      }
void clr_blink(void) { clr_attr(BLINK);     }

// -----------------------------------------------------------------------

void set_gray_fg(uint8_t c)
{
    attrs[FG] = c;
    set_attr(FG_GRAY);
}

// -----------------------------------------------------------------------

void set_gray_bg(uint8_t c)
{
    attrs[BG] = c;
    set_attr(BG_GRAY);
}

// -----------------------------------------------------------------------

void set_rgb_fg(uint8_t r, uint8_t g, uint8_t b)
{
   attrs[FG_R] = r;
   attrs[FG_G] = g;
   attrs[FG_B] = b;

   set_attr(FG_RGB);
}

// -----------------------------------------------------------------------

void clr_rgb_fg(uint8_t c)
{
    attrs[FG] = c;
    clr_attr(FG_RGB);
}

// -----------------------------------------------------------------------

void set_rgb_bg(uint8_t r, uint8_t g, uint8_t b)
{
   attrs[BG_R] = r;
   attrs[BG_G] = g;
   attrs[BG_B] = b;

   set_attr(BG_RGB);
}

// -----------------------------------------------------------------------
// calling this resets the fg to the usual 16 pre-set color palette

void set_fg(uint8_t c)
{
    attrs[FG] = c;
    clr_attr(FG_RGB | FG_GRAY);
}

// -----------------------------------------------------------------------
// calling this resets the bg to the usual 16 pre-set color palette

void set_bg(uint8_t c)
{
    attrs[BG] = c;
    clr_attr(BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------
// clear all attributes, reset colors to defaults

void set_norm(void)
{
    attrs[FG] = default_fg;
    attrs[BG] = default_bg;

    attrs[ATTR] = 0;
    set_attribs();
}

// =======================================================================
