// uC_console_attribs.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_attribs.h"

// -----------------------------------------------------------------------

extern uC_attribs_t attrs;

// -----------------------------------------------------------------------

API void uC_console_reset_attrs(void)
{
    attrs.fg = DEFAULT_FG;
    attrs.bg = DEFAULT_BG;

    attrs.flags.bits = 0;

    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_fg(uC_color_t color)
{
    attrs.fg = color;

    uC_attr_clr_attr(&attrs, (ATTR_FLAG_RGB_FG | ATTR_FLAG_GRAY_FG));
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_bg(uC_color_t color)
{
    attrs.bg = color;

    uC_attr_clr_attr(&attrs, (ATTR_FLAG_RGB_BG | ATTR_FLAG_GRAY_BG));
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_gray_fg(uC_colors_gray_t color)
{
    attrs.fg_gray = color;

    uC_attr_set_attr(&attrs, ATTR_FLAG_GRAY_FG);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void console_set_gray_bg(uC_colors_gray_t color)
{
    attrs.bg_gray = color;

    uC_attr_set_attr(&attrs, ATTR_FLAG_GRAY_BG);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_rgb_fg(uC_color_t r, uC_color_t g, uC_color_t b)
{
    attrs.fg_r = r;
    attrs.fg_g = g;
    attrs.fg_b = b;

    uC_attr_set_attr(&attrs, ATTR_FLAG_RGB_FG);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_rgb_bg(uC_color_t r, uC_color_t g, uC_color_t b)
{
    attrs.bg_r = r;
    attrs.bg_g = g;
    attrs.bg_b = b;

    uC_attr_set_attr(&attrs, ATTR_FLAG_RGB_BG);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void console_clr_attr(uint16_t flags)
{
    uC_attr_clr_attr(&attrs, flags);
    apply_attribs();
}

// -----------------------------------------------------------------------

void console_set_attr(uint16_t flags)
{
    uC_attr_set_attr(&attrs, flags);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_ul(void)   { console_set_attr(ATTR_FLAG_UL);   }
API void uC_console_set_rev(void)  { console_set_attr(ATTR_FLAG_REV);  }
API void uC_console_set_bold(void) { console_set_attr(ATTR_FLAG_BOLD); }
API void uC_console_clr_ul(void)   { console_clr_attr(ATTR_FLAG_UL);   }
API void uC_console_clr_rev(void)  { console_clr_attr(ATTR_FLAG_REV);  }
API void uC_console_clr_bold(void) { console_clr_attr(ATTR_FLAG_BOLD); }

// =======================================================================
