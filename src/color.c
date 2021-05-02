// color.c  - uCurses colors and attributes setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

uint8_t attrs[8];
uint8_t old_attrs[8];        // to test for changes

extern const char *f_str;    // terminfo format string pointer

// -----------------------------------------------------------------------
// users can change their default fg and bg (make black on white? ICK!!)

uint8_t default_bg = BLACK;
uint8_t default_fg = WHITE;

// -----------------------------------------------------------------------
// fg can be a normal color, a gray scale or an RGB value

static void do_set_fg(void)
{
    // terminfo format string for setting RGB colors.  these are not
    // supported by any current terminfo files that I know of and may
    // not be supported by all terminal types

    const char * const rgb_seq  =
        "\x1b[38;2;%p1%3d;%p2%3d;%p3%3dm";

    // format string to set gray scales which again might not be
    // supported by all terminal types.  this has been hard coded
    // here because for some terminals this is how they encode
    // their setaf and for others it is their setf (e.g. xterm)

    const char * const fg_seq   =
        "\x1b["
        "%?%p1%{8}%<"       // if p1 < 8
        "%t3%p1%d"          //    output '3' followed by p1
        "%e%p1%{16}%<"      // else  if p1 < 16
        "%t9%p1%{8}%-%d"    //    output '9' followed by p1 - 8
        "%e38;5;%p1%d"      // else output '38;5;' followed by p1
        "%;m";              // last char output is always the m

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
        parse_format();
        return;
    }
    if(attrs[ATTR] & FG_GRAY)
    {
        f_str = &fg_seq[0];
        // gray scales are specified as values from 0 to 23 but
        // the escape seaueces use values from 232 to 255
        params[0] += 232;
        parse_format();
        return;
    }
    // would be nice if the people creating terminal emulators understood
    // the difference between a setaf and a setf.  tho i should probably
    // be outputting a setf here based on xterms infocmp
    ti_setaf();
}

// -----------------------------------------------------------------------
// bg can be a normal color, a gray scale or an RGB value

static void do_set_bg(void)
{
    // terminfo format string for setting RGB colors.  these are not
    // supported by any current terminfo files that I know of and may
    // not be supported by all terminal types

    const char * const rgb_seq  =
        "\x1b[48;2;%p1%3d;%p2%3d;%p3%3dm";

    const char * const bg_seq   =
        "\x1b["
        "%?%p1%{8}%<"
        "%t4%p1%d"
        "%e%p1%{16}%<"
        "%t10%p1%{8}%-%d"
        "%e48;5;%p1%d"
        "%;m";

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

        f_str = &rgb_seq[0];
        parse_format();
        return;
    }
    // are we setting a gray scale foreground?
    if(attrs[ATTR] & BG_GRAY)
    {
        f_str = &bg_seq[0];
        // gray scales are specified as values from 0 to 23 but
        // the escape seaueces use values from 232 to 255
        params[0] += 232;
        parse_format();
        return;
    }
    ti_setab();
}

// -----------------------------------------------------------------------
// apply various attribute changes

void apply_attribs(void)
{
    uint8_t changes;

    changes = attrs[ATTR] ^ old_attrs[ATTR];

    if((changes & BLINK) || (changes & BOLD) || (changes & REVERSE))
    {
        ti_sgr0();

        if(attrs[ATTR] & BLINK)    { ti_blink(); }
        if(attrs[ATTR] & BOLD)     { ti_bold();  }
        if(attrs[ATTR] & REVERSE)  { ti_rev();   }
    }
    // if underline changed we need to set it.  if it was not changed
    // we might need to restore it because of the above sgr0
    if((changes & UNDERLINE) ||
       (!(changes & UNDERLINE) && (attrs[ATTR] & UNDERLINE)))
    {
        (attrs[ATTR] & UNDERLINE) ? ti_smul() : ti_rmul();
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

    *(uint64_t *)&old_attrs = *(uint64_t *)&attrs;
}

// -----------------------------------------------------------------------

static void set_attr(ti_attrib_t attr)
{
    attrs[ATTR] |= attr;

    // gray scale and rgb color settngs are mutually exclusive
    if(FG_RGB  & attr)  { attrs[ATTR] &= ~FG_GRAY; }
    if(BG_RGB  & attr)  { attrs[ATTR] &= ~BG_GRAY; }
    if(FG_GRAY & attr)  { attrs[ATTR] &= ~FG_RGB;  }
    if(BG_GRAY & attr)  { attrs[ATTR] &= ~BG_RGB;  }

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
