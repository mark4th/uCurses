// uC_attribs.c  - uCurses colors and attributes setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_attribs.h"
#include "uC_terminfo.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;

uC_attribs_t attrs =
{
    .flags.bits = 0,
    .fg         = uC_DEFAULT_FG,
    .bg         = uC_DEFAULT_BG,
};

uC_attribs_t old_attrs;

// -----------------------------------------------------------------------
// normal attribs for menus and widgets (maybe)

API uC_attribs_t uC_attrs_normal =
{
    .flags.bits = uC_ATTR_FLAG_GRAY_BG,
    .fg         = uC_COLOR_BROWN,
    .bg_gray    = uC_GRAY_05,
};

// -----------------------------------------------------------------------
// seleccted attribs for menus and widgets (maybe)

API uC_attribs_t uC_attrs_selected =
{
    .flags.bits = uC_ATTR_FLAG_GRAY_BG,
    .fg         = uC_COLOR_CYAN,
    .bg_gray    = uC_GRAY_08,
};

// -----------------------------------------------------------------------
// disabled attribs for menus and widgets (maybe)

API uC_attribs_t uC_attrs_disabled =
{
    .flags.bits = (uC_ATTR_FLAG_GRAY_BG | uC_ATTR_FLAG_GRAY_FG),
    .fg_gray    = uC_GRAY_08,
    .bg_gray    = uC_GRAY_04,
};

// -----------------------------------------------------------------------

// there is no format string for this in the terminfo string section
// there should be

static void rgb_fg(void)
{
    const char *const rgb_seq = "\x1b[38;2;%p1%3d;%p2%3d;%p3%3dm";

    // copy the RGB elements of the current attribute set into the
    // terminfo parameter array

    ti_vars->params[0] = attrs.fg_r;
    ti_vars->params[1] = attrs.fg_g;
    ti_vars->params[2] = attrs.fg_b;

    uC_parse_format((uint8_t *)rgb_seq);
}

// -----------------------------------------------------------------------

static void rgb_bg(void)
{
    const char *const rgb_seq = "\x1b[48;2;%p1%3d;%p2%3d;%p3%3dm";

    ti_vars->params[0] = attrs.bg_r;
    ti_vars->params[1] = attrs.bg_g;
    ti_vars->params[2] = attrs.bg_b;

    uC_parse_format((uint8_t *)rgb_seq);
}

// -----------------------------------------------------------------------

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

    ti_vars->params[0] += 232;
    uC_parse_format((uint8_t *)fg_seq);
}

// -----------------------------------------------------------------------

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

    ti_vars->params[0] += 232;
    uC_parse_format((uint8_t *)bg_seq);
}

// -----------------------------------------------------------------------
// fg can be a normal color, a gray scale or an RGB value

static void apply_fg(void)
{
    ti_vars->params[0] = attrs.fg;

    if (attrs.flags.rgb_fg)
    {
        rgb_fg();
        return;
    }

    if (attrs.flags.gray_fg)
    {
        gray_fg();
        return;
    }

    // would be nice if the people creating terminal emulators understood
    // the difference between a setaf and a setf.  tho i should probably
    // be outputting a setf here based on xterms infocmp

    // the magic number is the terminfo strings section offset to the
    // format string for setaf.  if it is 0xffff we need to use setf
    // instead

    (ti_vars->ti_file.ti_strings[359] != -1)
        ? ti_setaf()
        : ti_setf();
}

// -----------------------------------------------------------------------
// bg can be a normal color, a gray scale or an RGB value

static void apply_bg(void)
{
    ti_vars->params[0] = attrs.bg;

    if (attrs.flags.rgb_bg)
    {
        rgb_bg();
        return;
    }

    if (attrs.flags.gray_bg)
    {
        gray_bg();
        return;
    }

    // the magic number is the terminfo strings section offset to the
    // format string for setab.  if it is 0xffff we need to use setb
    // instead.

    // im not actually sure of the correct way to handle this, are
    // there any terminals that support both satab and setb and if so
    // which one should be called?

    (ti_vars->ti_file.ti_strings[360] != -1)
        ? ti_setab()
        : ti_setb();
}

// -----------------------------------------------------------------------
// conditionally apply foreground changes

static inline void if_apply_fg(uint16_t changes)
{
    uint8_t diff =
        (attrs.fg   ^ old_attrs.fg)   |
        (attrs.fg_r ^ old_attrs.fg_r) |
        (attrs.fg_g ^ old_attrs.fg_g) |
        (attrs.fg_b ^ old_attrs.fg_b);

    if ((changes | diff) != 0)
    {
        apply_fg();
    }
}

// -----------------------------------------------------------------------
// conditionally apply background changes

static inline void if_apply_bg(uint8_t changes)
{
    uint8_t diff =
        (attrs.bg   ^ old_attrs.bg)   |
        (attrs.bg_r ^ old_attrs.bg_r) |
        (attrs.bg_g ^ old_attrs.bg_g) |
        (attrs.bg_b ^ old_attrs.bg_b);

    if ((changes | diff) != 0)
    {
        apply_bg();
    }
}

// -----------------------------------------------------------------------
// apply various attribute changes

void apply_attribs(void)
{
    uC_ti_attr_flags_t changes;

    changes.bits = (attrs.flags.bits ^ old_attrs.flags.bits);

    // reset if bold/rev clearing OR if color mode is changing
    bool color_mode_change =
        (changes.rgb_fg  || changes.gray_fg ||
         changes.rgb_bg  || changes.gray_bg);

    if ((changes.bold) || (changes.rev) || color_mode_change)
    {
        // if we are clearing either of these we must clear
        // everything then reinstate the attributes we did not
        // want to clear if amy

        if ((attrs.flags.bold == 0) || (attrs.flags.rev == 0))
        {
            ti_sgr0();
            old_attrs.blob = 0;
        }

        // this may needlessly set one or other of these to
        // the state they are currently in

        if (attrs.flags.bold)  { ti_bold(); }
        if (attrs.flags.rev)   { ti_rev();  }
    }

    if (changes.ul)
    {
        (attrs.flags.ul)
            ? ti_smul()
            : ti_rmul();
    }

    // now we can apply any alterations made to the fg / bg colors

    if_apply_bg(changes.bits);
    if_apply_fg(changes.bits);

    old_attrs = attrs;
}

// -----------------------------------------------------------------------
// disable attribes that are mutually exclusive with the one being set

static uC_ti_attr_flags_t attr_add_flags(uC_ti_attr_flags_t flags,
    uint16_t bits)
{
    uC_ti_attr_flags_t f;  // exists so we can test

    flags.bits |= bits;
    f.bits      = bits;    // convert bits to flags

    if (f.rgb_fg)    { flags.gray_fg = 0; }
    if (f.rgb_bg)    { flags.gray_bg = 0; }
    if (f.gray_fg)   { flags.rgb_fg  = 0; }
    if (f.gray_bg)   { flags.rgb_bg  = 0; }

    return flags;
}

// -----------------------------------------------------------------------

API void uC_attr_set_flags(uC_attribs_t *attribs, uint16_t bits)
{
    attribs->flags = attr_add_flags(attribs->flags, bits);
}

// -----------------------------------------------------------------------

API void uC_attr_clr_flags(uC_attribs_t *attribs, uint16_t bits)
{
    attribs->flags.bits &= ~bits;
}

// -----------------------------------------------------------------------

API void uC_set_fg(uC_attribs_t *attribs, uC_color_t color)
{
    attribs->fg = color;
    uC_attr_clr_flags(attribs, (uC_ATTR_FLAG_RGB_FG | uC_ATTR_FLAG_GRAY_FG));
}

// -----------------------------------------------------------------------

API void uC_set_bg(uC_attribs_t *attribs, uC_color_t color)
{
    attribs->bg = color;
    uC_attr_clr_flags(attribs, (uC_ATTR_FLAG_RGB_BG | uC_ATTR_FLAG_GRAY_BG));
}

// -----------------------------------------------------------------------

API void uC_set_gray_fg(uC_attribs_t *attr, uC_colors_gray_t color)
{
    attr->fg_gray = color;
    uC_attr_set_flags(attr, uC_ATTR_FLAG_GRAY_FG);
}

// -----------------------------------------------------------------------

API void uC_set_gray_bg(uC_attribs_t *attr, uC_colors_gray_t color)
{
    attr->bg_gray = color;
    uC_attr_set_flags(attr, uC_ATTR_FLAG_GRAY_BG);
}

// -----------------------------------------------------------------------

API void uC_set_rgb_fg(uC_attribs_t *attr, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    attr->fg_r = r;
    attr->fg_g = g;
    attr->fg_b = b;
    uC_attr_set_flags(attr, uC_ATTR_FLAG_RGB_FG);
}

// -----------------------------------------------------------------------

API void uC_set_rgb_bg(uC_attribs_t *attr, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    attr->bg_r = r;
    attr->bg_g = g;
    attr->bg_b = b;
    uC_attr_set_flags(attr, uC_ATTR_FLAG_RGB_BG);
}

// =======================================================================
