// color.c  - uCurses colors and attributes setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

attribs_t attrs;
attribs_t old_attrs; // to test for changes

extern const char *f_str; // terminfo format string pointer

// -----------------------------------------------------------------------
// users can change their default fg and bg (make black on white? ICK!!)

int8_t default_bg = BLACK;
int8_t default_fg = WHITE;

// -----------------------------------------------------------------------
// fg can be a normal color, a gray scale or an RGB value

static INLINE void do_set_fg(void)
{
    // terminfo format string for setting RGB colors.  these are not
    // supported by any current terminfo files that I know of and may
    // not be supported by all terminal types

    const char *const rgb_seq = "\x1b[38;2;%p1%3d;%p2%3d;%p3%3dm";

    // format string to set gray scales which again might not be
    // supported by all terminal types.  this has been hard coded
    // here because for some terminals this is how they encode
    // the setaf and for others it is how they encide the setf
    // (e.g. xterm)

    const char *const fg_seq =
        "\x1b["
        "%?%p1%{8}%<"    // if p1 < 8
        "%t3%p1%d"       //    output '3' followed by p1
        "%e%p1%{16}%<"   // else  if p1 < 16
        "%t9%p1%{8}%-%d" //    output '9' followed by p1 - 8
        "%e38;5;%p1%d"   // else output '38;5;' followed by p1
        "%;m";           // last char output is always the m

    // the params array is how we pass parameters to the terminfo
    // parsing functions for each format string.  this converts the
    // given format string into an escape sequence for the terminal

    params[0] = attrs.bytes[FG];

    if(attrs.bytes[ATTR] & FG_RGB)
    {
        params[0] = attrs.bytes[FG_R];
        params[1] = attrs.bytes[FG_G];
        params[2] = attrs.bytes[FG_B];

        // there is no format string for this within the terminfo
        // string section

        f_str = rgb_seq;
        parse_format();
        return;
    }

    if(attrs.bytes[ATTR] & FG_GRAY)
    {
        f_str = fg_seq;

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

static INLINE void do_set_bg(void)
{
    // terminfo format string for setting RGB colors.  these are not
    // supported by any current terminfo files that I know of and may
    // not be supported by all terminal types

    const char *const rgb_seq = "\x1b[48;2;%p1%3d;%p2%3d;%p3%3dm";

    const char *const bg_seq =
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

    params[0] = attrs.bytes[BG];

    // are we setting a rgb background?
    if(attrs.bytes[ATTR] & BG_RGB)
    {
        params[0] = attrs.bytes[BG_R];
        params[1] = attrs.bytes[BG_G];
        params[2] = attrs.bytes[BG_B];

        f_str = &rgb_seq[0];
        parse_format();
        return;
    }

    // are we setting a gray scale foreground?

    if(attrs.bytes[ATTR] & BG_GRAY)
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

    changes = attrs.bytes[ATTR] ^ old_attrs.bytes[ATTR];

    if((changes & BOLD) || (changes & REVERSE))
    {
        ti_sgr0();

        if((attrs.bytes[ATTR] & BOLD) != 0)
        {
            ti_bold();
        }
        if((attrs.bytes[ATTR] & REVERSE) != 0)
        {
            ti_rev();
        }
    }

    // if underline changed we need to set it.  if it was not changed
    // we might need to restore it because of the above sgr0
    if((changes & UNDERLINE) ||
       (!(changes & UNDERLINE) && (attrs.bytes[ATTR] & UNDERLINE)))
    {
        (attrs.bytes[ATTR] & UNDERLINE) ? ti_smul() : ti_rmul();
    }

    if((attrs.bytes[BG] != old_attrs.bytes[BG]) ||
       (attrs.bytes[BG_R] != old_attrs.bytes[BG_R]) ||
       (attrs.bytes[BG_G] != old_attrs.bytes[BG_G]) ||
       (attrs.bytes[BG_B] != old_attrs.bytes[BG_B]) ||
       (changes != 0))
    {
        do_set_bg();
    }

    if((attrs.bytes[FG] != old_attrs.bytes[FG]) ||
       (attrs.bytes[FG_R] != old_attrs.bytes[FG_R]) ||
       (attrs.bytes[FG_G] != old_attrs.bytes[FG_G]) ||
       (attrs.bytes[FG_B] != old_attrs.bytes[FG_B]) ||
       (changes != 0))
    {
        do_set_fg();
    }

    old_attrs.chunk = attrs.chunk;
}

// -----------------------------------------------------------------------

ti_attrib_t add_attr(uint8_t a, ti_attrib_t attr)
{
    a |= attr;

    if(FG_RGB & attr)
    {
        a &= ~FG_GRAY;
    }
    if(BG_RGB & attr)
    {
        a &= ~BG_GRAY;
    }
    if(FG_GRAY & attr)
    {
        a &= ~FG_RGB;
    }
    if(BG_GRAY & attr)
    {
        a &= ~BG_RGB;
    }
    return a;
}

// -----------------------------------------------------------------------

static void set_attr(ti_attrib_t attr)
{
    attrs.bytes[ATTR] = add_attr(attrs.bytes[ATTR], attr);
    apply_attribs();
}

// -----------------------------------------------------------------------

static void clr_attr(ti_attrib_t attr)
{
    attrs.bytes[ATTR] &= ~attr;
    apply_attribs();
}

// -----------------------------------------------------------------------
// set individual attribs

void set_ul(void)
{
    set_attr(UNDERLINE);
}

void set_rev(void)
{
    set_attr(REVERSE);
}

void set_bold(void)
{
    set_attr(BOLD);
}

// -----------------------------------------------------------------------
// clear individual attribs

void clr_ul(void)
{
    clr_attr(UNDERLINE);
}

void clr_rev(void)
{
    clr_attr(REVERSE);
}

void clr_bold(void)
{
    clr_attr(BOLD);
}

// -----------------------------------------------------------------------

void set_gray_fg(int8_t c)
{
    attrs.bytes[FG] = c;
    set_attr(FG_GRAY);
}

// -----------------------------------------------------------------------

void set_gray_bg(int8_t c)
{
    attrs.bytes[BG] = c;
    set_attr(BG_GRAY);
}

// -----------------------------------------------------------------------

void set_rgb_fg(int8_t r, int8_t g, int8_t b)
{
    attrs.bytes[FG_R] = r;
    attrs.bytes[FG_G] = g;
    attrs.bytes[FG_B] = b;

    set_attr(FG_RGB);
}

// -----------------------------------------------------------------------

void set_rgb_bg(int8_t r, int8_t g, int8_t b)
{
    attrs.bytes[BG_R] = r;
    attrs.bytes[BG_G] = g;
    attrs.bytes[BG_B] = b;

    set_attr(BG_RGB);
}

// -----------------------------------------------------------------------
// calling this resets the fg to the usual 16 pre-set color palette

void set_fg(int8_t c)
{
    attrs.bytes[FG] = c;
    clr_attr(FG_RGB | FG_GRAY);
}

// -----------------------------------------------------------------------
// calling this resets the bg to the usual 16 pre-set color palette

void set_bg(int8_t c)
{
    attrs.bytes[BG] = c;
    clr_attr(BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------
// clear all attributes, reset colors to defaults

void set_norm(void)
{
    attrs.bytes[FG] = default_fg;
    attrs.bytes[BG] = default_bg;

    attrs.bytes[ATTR] = 0;
    apply_attribs();
}

// =======================================================================
