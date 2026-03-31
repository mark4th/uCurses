// uC_console_attribs.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_attribs.h"

// -----------------------------------------------------------------------

extern uC_attribs_t attrs;

// -----------------------------------------------------------------------

API void uC_console_reset_attrs(void)
{
    attrs.fg = uC_DEFAULT_FG;
    attrs.bg = uC_DEFAULT_BG;

    attrs.flags.bits = 0;

    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_fg(uC_color_t color)
{
    uC_set_fg(&attrs, color);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_bg(uC_color_t color)
{
    uC_set_bg(&attrs, color);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_gray_fg(uC_colors_gray_t color)
{
    uC_set_gray_fg(&attrs, color);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void console_set_gray_bg(uC_colors_gray_t color)
{
    uC_set_gray_bg(&attrs, color);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_rgb_fg(uC_color_t r, uC_color_t g, uC_color_t b)
{
    uC_set_rgb_fg(&attrs, r, g, b);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_rgb_bg(uC_color_t r, uC_color_t g, uC_color_t b)
{
    uC_set_rgb_bg(&attrs, r, g, b);
    apply_attribs();
}

// -----------------------------------------------------------------------

void console_clr_flags(uint16_t flags)
{
    uC_attr_clr_flags(&attrs, flags);
    apply_attribs();
}

// -----------------------------------------------------------------------

void console_set_flags(uint16_t flags)
{
    uC_attr_set_flags(&attrs, flags);
    apply_attribs();
}

// -----------------------------------------------------------------------

API void uC_console_set_ul(void)   { console_set_flags(ATTR_FLAG_UL);   }
API void uC_console_set_rev(void)  { console_set_flags(ATTR_FLAG_REV);  }
API void uC_console_set_bold(void) { console_set_flags(ATTR_FLAG_BOLD); }
API void uC_console_clr_ul(void)   { console_clr_flags(ATTR_FLAG_UL);   }
API void uC_console_clr_rev(void)  { console_clr_flags(ATTR_FLAG_REV);  }
API void uC_console_clr_bold(void) { console_clr_flags(ATTR_FLAG_BOLD); }

// =======================================================================
