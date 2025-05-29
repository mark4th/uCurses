// color.c  - uCurses colors and attributes setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "uCurses.h"
#include "uC_attribs.h"
#include "uC_terminfo.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

extern ti_parse_t *uC_ti_parse;

uC_attribs_t attrs =
{
    .bytes[FG] = DEFAULT_FG,
    .bytes[BG] = DEFAULT_BG,
};

uC_attribs_t old_attrs;

// -----------------------------------------------------------------------
// hard coded format string to set a rgb fg                            380

// there is no format string for this within the terminfo
// string section

static void rgb_fg(void)
{
    const char *const rgb_seq = "\x1b[38;2;%p1%3d;%p2%3d;%p3%3dm";

    // copy the RGB elements of the current attribute set into the
    // terminfo parameter array

    uC_ti_parse->params[0] = attrs.bytes[FG_R];
    uC_ti_parse->params[1] = attrs.bytes[FG_G];
    uC_ti_parse->params[2] = attrs.bytes[FG_B];

    uC_parse_format(rgb_seq);
}

// -----------------------------------------------------------------------
//                                                                     382

static void rgb_bg(void)
{
    const char *const rgb_seq = "\x1b[48;2;%p1%3d;%p2%3d;%p3%3dm";

    uC_ti_parse->params[0] = attrs.bytes[BG_R];
    uC_ti_parse->params[1] = attrs.bytes[BG_G];
    uC_ti_parse->params[2] = attrs.bytes[BG_B];

    uC_parse_format(rgb_seq);
}

// -----------------------------------------------------------------------
// hard coded format string to set a gray fg                           384

static void gray_fg(void)
{
    const char *const fg_seq =
        "\x1b["
        "%?%p1%{8}%<"    // if p1 < 8
        "%t3%p1%d"       //    output '3' followed by p1
        "%e%p1%{16}%<"   // else  if p1 < 16
        "%t9%p1%{8}%-%d" //    output '9' followed by p1 - 8
        "%e38;5;%p1%d"   // else output '38;5;' followed by p1
        "%;m";           // last char output is always the m

    // gray scales are specified as values from 0 to 23 but
    // the escape seaueces use values from 232 to 255

    uC_ti_parse->params[0] += 232;
    uC_parse_format(fg_seq);
}

// -----------------------------------------------------------------------
//                                                                     386

static void gray_bg(void)
{
    const char *const bg_seq =
        "\x1b["
        "%?%p1%{8}%<"
        "%t4%p1%d"
        "%e%p1%{16}%<"
        "%t10%p1%{8}%-%d"
        "%e48;5;%p1%d"
        "%;m";

    // gray scales are specified as values from 0 to 23 but
    // the escape seaueces use values from 232 to 255

    uC_ti_parse->params[0] += 232;
    uC_parse_format(bg_seq);
}

// -----------------------------------------------------------------------
// fg can be a normal color, a gray scale or an RGB value              389

static void apply_fg(void)
{
    uC_ti_parse->params[0] = attrs.bytes[FG];

    if (attrs.bytes[ATTR] & FG_RGB)
    {
        rgb_fg();
        return;
    }

    if (attrs.bytes[ATTR] & FG_GRAY)
    {
        gray_fg();
        return;
    }

    // would be nice if the people creating terminal emulators understood
    // the difference between a setaf and a setf.  tho i should probably
    // be outputting a setf here based on xterms infocmp

    ti_setaf();
}

// -----------------------------------------------------------------------
// bg can be a normal color, a gray scale or an RGB value              392

static void apply_bg(void)
{
    // the params array is how we pass parameters to the terminfo
    // parsing functions for each format string.

    uC_ti_parse->params[0] = attrs.bytes[BG];

    // are we setting a rgb background?
    if (attrs.bytes[ATTR] & BG_RGB)
    {
        rgb_bg();
        return;
    }

    // are we setting a gray scale foreground?

    if (attrs.bytes[ATTR] & BG_GRAY)
    {
        gray_bg();
        return;
    }

    ti_setab();
}

// -----------------------------------------------------------------------
// conditionally apply background changes

static inline void if_apply_bg(uint8_t changes)
{
    uint8_t diff =
        (attrs.bytes[BG]   ^ old_attrs.bytes[BG])   |
        (attrs.bytes[BG_R] ^ old_attrs.bytes[BG_R]) |
        (attrs.bytes[BG_G] ^ old_attrs.bytes[BG_G]) |
        (attrs.bytes[BG_B] ^ old_attrs.bytes[BG_B]);

    if ((changes | diff) != 0) { apply_bg(); }
}

// -----------------------------------------------------------------------
// conditionally apply foreground changes

static inline void if_apply_fg(uint8_t changes)
{
    uint8_t diff =
        (attrs.bytes[FG]   ^ old_attrs.bytes[FG])   |
        (attrs.bytes[FG_R] ^ old_attrs.bytes[FG_R]) |
        (attrs.bytes[FG_G] ^ old_attrs.bytes[FG_G]) |
        (attrs.bytes[FG_B] ^ old_attrs.bytes[FG_B]);

    if ((changes | diff) != 0) { apply_fg(); }
}

// -----------------------------------------------------------------------
// apply various attribute changes                                     396

void apply_attribs(void)
{
    uint8_t changes;        // detects alterations to attributes

    changes = attrs.bytes[ATTR] ^ old_attrs.bytes[ATTR];

    // in order to make changes to bold we need to clear ALL
    // attributes because there is no way to clear just bold

    if ((changes & BOLD) || (changes & REVERSE))
    {
        ti_sgr0();

        if ((attrs.bytes[ATTR] & BOLD)    != 0)  { ti_bold(); }
        if ((attrs.bytes[ATTR] & REVERSE) != 0)  { ti_rev();  }
    }

    // if underline changed we need to set it.  if it was not changed
    // we might need to restore it because of the above sgr0

    if ((changes & UNDERLINE) ||
       (!(changes & UNDERLINE) && (attrs.bytes[ATTR] & UNDERLINE)))
    {
        (attrs.bytes[ATTR] & UNDERLINE)
            ? ti_smul()
            : ti_rmul();
    }

    // now we can apply any alterations made to the fg / bg colors

    if_apply_bg(changes);
    if_apply_fg(changes);

    old_attrs.chunk = attrs.chunk;
}

// -----------------------------------------------------------------------
// disable attribes that are mutually exclusive with the one being set 399

ti_attrib_t add_attr(uint8_t a, ti_attrib_t attr)
{
    a |= attr;

    if (FG_RGB  & attr)   { a &= ~FG_GRAY; }
    if (BG_RGB  & attr)   { a &= ~BG_GRAY; }
    if (FG_GRAY & attr)   { a &= ~FG_RGB;  }
    if (BG_GRAY & attr)   { a &= ~BG_RGB;  }

    return a;
}

// -----------------------------------------------------------------------
//                                                                     267

API void uC_attr_set_attr(uC_attribs_t *attribs, ti_attrib_t attr)
{
    attribs->bytes[ATTR] = add_attr(attribs->bytes[ATTR], attr);
}

// -----------------------------------------------------------------------
//                                                                     272

API void uC_attr_clr_attr(uC_attribs_t *attribs, ti_attrib_t attr)
{
    attribs->bytes[ATTR] &= ~attr;
}

// -----------------------------------------------------------------------
//                                                                     283

API void uC_attr_set_bytes(uC_attribs_t *attribs, uC_attr_index_t which,
    uC_color_t color)
{
    attribs->bytes[which] = color;
}

// -----------------------------------------------------------------------
// clear all attributes, reset colors to defaults                      289

API void uC_console_reset_attrs(void)
{
    attrs.bytes[FG] = DEFAULT_FG;
    attrs.bytes[BG] = DEFAULT_BG;

    attrs.bytes[ATTR] = 0;
    apply_attribs();
}

// -----------------------------------------------------------------------
//                                                                     293

API void uC_console_set_fg(uC_color_t color)
{
    uC_attr_set_bytes(&attrs, FG, color);
    uC_attr_clr_attr(&attrs, FG_RGB | FG_GRAY);
}

// -----------------------------------------------------------------------
//                                                                     422

API void uC_console_set_bg(uC_color_t color)
{
    uC_attr_set_bytes(&attrs, BG, color);
    uC_attr_clr_attr(&attrs, BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------
//                                                                     43

API void uC_console_set_gray_fg(uC_color_t color)
{
    uC_attr_set_bytes(&attrs, FG, color);
    uC_attr_set_attr(&attrs, FG_GRAY);
}

// -----------------------------------------------------------------------
//                                                                     436

API void console_set_gray_bg(uC_color_t color)
{
    uC_attr_set_bytes(&attrs, BG, color);
    uC_attr_set_attr(&attrs, BG_GRAY);
}

// -----------------------------------------------------------------------
//                                                                     452

API void uC_console_set_rgb_fg(uC_color_t r, uC_color_t g, uC_color_t b)
{
    uC_attr_set_bytes(&attrs, FG_R, r);
    uC_attr_set_bytes(&attrs, FG_G, g);
    uC_attr_set_bytes(&attrs, FG_B, b);
    uC_attr_set_attr(&attrs, FG_RGB);
}

// -----------------------------------------------------------------------
//                                                                     461

API void uC_console_set_rgb_bg(uC_color_t r, uC_color_t g, uC_color_t b)
{
    uC_attr_set_bytes(&attrs, BG_R, r);
    uC_attr_set_bytes(&attrs, BG_G, g);
    uC_attr_set_bytes(&attrs, BG_B, b);
    uC_attr_set_attr(&attrs, BG_RGB);
}

// -----------------------------------------------------------------------
//                                                                     465

API void uC_console_clr_attr(ti_attrib_t attr)
{
    uC_attr_clr_attr(&attrs, attr);
}

// -----------------------------------------------------------------------
//                                                                     468

API void uC_console_set_bytes(uC_attr_index_t which, uC_color_t color)
{
    uC_attr_set_bytes(&attrs, which, color);
}

// -----------------------------------------------------------------------
// 473 : 475 : 378 : 481 : 484 :

#define uC_console_set_attr(attr) \
    uC_attr_set_attr(&attrs, attr)

API void uC_console_set_ul(void)    { uC_console_set_attr(UNDERLINE); }
API void uC_console_set_rev(void)   { uC_console_set_attr(REVERSE);   }
API void uC_console_set_bold(void)  { uC_console_set_attr(BOLD);      }
API void uC_console_clr_ul(void)    { uC_console_clr_attr(UNDERLINE); }
API void uC_console_clr_rev(void)   { uC_console_clr_attr(REVERSE);   }
API void uC_console_clr_bold(void)  { uC_console_clr_attr(BOLD);      }

// =======================================================================
