// color.c  - uCurses colors and attributes setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "h/color.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

uint8_t attrs[8];
uint8_t old_attrs[8];  // to test for changes

// terminfo format string pointer within the string section
extern const uint8_t *f_str;

// -----------------------------------------------------------------------
// users can change their default fg and bg (make black on white? ICK!!)

uint8_t default_bg = BLACK;
uint8_t default_fg = WHITE;

// -----------------------------------------------------------------------
// fg can be a normal color, a gray scale or an RGB value

static void do_set_fg(void)
{
    // terminfo format strings for setting gray scales or RGB colors
    // these are not supported by any current terminfo files that I
    // know of and may not be supported by all terminal types

    const uint8_t gray_seq[] = "\x1b[38;5;%p1%3dm";
    const uint8_t rgb_seq[]  = "\x1b[38;2;%p1%3d;%p2%3d;%p3%3dm";

    // the params array is how we pass parameters to the terminfo
    // parsing functions for each format string.  this converts the
    // given format string into an escape sequence for the terminal

    params[0] = attrs[FG];

    if(attrs[ATTR] & FG_RGB)
    {
        params[0] = attrs[FG_R];
        params[1] = attrs[FG_G];
        params[2] = attrs[FG_B];
        // there is no format string for this within the terminfo
        // string section
        f_str = &rgb_seq[0];
        do_parse_format();
    }
    else if(attrs[ATTR] & FG_GRAY)
    {
        // gray scales are specified as values from 0 to 20 but
        // the escape seaueces use values from 232 to 255
        params[0] += 232;
        f_str = &gray_seq[0];
        do_parse_format();
    }
    // oridinay, every day, run of the mill. ho-hum foreground color
    else
    {
        ti_setaf();
    }
}

// -----------------------------------------------------------------------
// bg can be a normal color, a gray scale or an RGB value

static void do_set_bg(void)
{
    // terminfo format strings for setting gray scales or RGB colors
    // these are not supported by any current terminfo files that I
    // know of and may not be supported by all terminal types

    const uint8_t gray_seq[] = "\x1b[48;5;%p1%3dm";
    const uint8_t rgb_seq[]  = "\x1b[48;2;%p1%3d;%p2%3d;%p3%3dm";

    // the params array is how we pass parameters to the terminfo
    // parsing functions for each format string.  this converts the
    // given format string into an escape sequence for the terminal

    params[0] = attrs[BG];

    // are we setting a rgb background?
    if(attrs[ATTR] & BG_RGB)
    {
        params[0] = attrs[BG_R];
        params[1] = attrs[BG_G];
        params[2] = attrs[BG_B];
        // there is no format string for this within the terminfo
        // string section
        f_str = &rgb_seq[0];
        do_parse_format();
    }
    // are we setting a gray scale foreground?
    else if(attrs[ATTR] & BG_GRAY)
    {
        // gray scales are specified as values from 0 to 20 but
        // the escape seaueces use values from 232 to 255
        params[0] += 232;
        f_str = &gray_seq[0];
        do_parse_format();
    }
    // oridinay, every day, run of the mill. ho-hum background color
    else
    {
        ti_setab();
    }
}

// -----------------------------------------------------------------------
// apply varius attribute changes

void apply_attribs(void)
{
    uint8_t changes;

    changes = attrs[ATTR] ^ old_attrs[ATTR];

    if(0 != changes)
    {
        if((changes & BLINK) || (changes & BOLD) || (changes & REVERSE))
        {
            ti_sgr0();

            if(attrs[ATTR] & BLINK)    { ti_blink(); }
            if(attrs[ATTR] & BOLD)     { ti_bold();  }
            if(attrs[ATTR] & REVERSE)  { ti_rev();  }
        }
        // if underline changed we need to set if.  if it was not changed
        // we might need to restore it because of the above sgr0
        if((changes & UNDERLINE) ||
           (!(changes & UNDERLINE) && (attrs[ATTR] & UNDERLINE)))
        {
            (attrs[ATTR] & UNDERLINE) ? ti_smul() : ti_rmul();
        }
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

    // gray scale and rgb color settngs are mutually exclusive
    if(FG_RGB == attr)   { attrs[ATTR] &= ~FG_GRAY; }
    if(BG_RGB == attr)   { attrs[ATTR] &= ~BG_GRAY; }
    if(FG_GRAY == attr)  { attrs[ATTR] &= ~FG_RGB;  }
    if(BG_GRAY == attr)  { attrs[ATTR] &= ~BG_RGB;  }

    apply_attribs();
}

// -----------------------------------------------------------------------

static void clr_attr(ti_attrib_t attr)
{
    attrs[ATTR] &= ~attr;
    apply_attribs();
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
    apply_attribs();
}

// =======================================================================
