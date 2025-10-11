// attr_defs.c  definitions for widget attributes
// -----------------------------------------------------------------------

// this c file is included directly so all the macros and what have you
// are already known, no need to #incldue anything here.

// -----------------------------------------------------------------------
// attributes for box around wiget view

uC_attribs_t box_attrs =
{
    .flags.bits = (ATTR_FLAG_GRAY_FG | ATTR_FLAG_GRAY_BG),
    .fg         = uC_GRAY_07,
    .bg_gray    = uC_GRAY_03,
};

// -----------------------------------------------------------------------

API uC_attribs_t box_focus_attrs =
{
    .flags.bits = ATTR_FLAG_GRAY_BG,
    .fg         = uC_COLOR_CYAN,
    .bg_gray    = uC_GRAY_03,
};

// -----------------------------------------------------------------------
// attributes for box around wiget view

uC_attribs_t view_attrs =
{
    .flags.bits = (ATTR_FLAG_GRAY_FG | ATTR_FLAG_GRAY_BG),
    .fg         = uC_GRAY_07,
    .bg_gray    = uC_GRAY_03,
};

// -----------------------------------------------------------------------
// attributes for view group

uC_attribs_t vg_attrs =
{
    .flags.bits = (ATTR_FLAG_GRAY_FG | ATTR_FLAG_GRAY_BG),
    .fg         = uC_GRAY_05,
    .bg_gray    = uC_GRAY_03,
};

// -----------------------------------------------------------------------
// attributes for buttons

uC_attribs_t btn_attrs =
{
    .flags.bits = (ATTR_FLAG_BOLD | ATTR_FLAG_GRAY_BG),
    .fg         = uC_COLOR_CYAN,
    .bg_gray    = uC_GRAY_06,
};

// -----------------------------------------------------------------------
// attributes for buttons that have focus

uC_attribs_t btn_focus_attrs =
{
    .flags.bits = (ATTR_FLAG_BOLD),
    .fg         = uC_COLOR_GREEN,
    .bg         = uC_COLOR_BROWN,
};

// -----------------------------------------------------------------------
// attributes for radio buttons

uC_attribs_t radio_attrs =
{
    .flags.bits = (ATTR_FLAG_GRAY_BG),
    .fg         = uC_COLOR_BROWN,
    .bg_gray    = uC_GRAY_03,
};

// -----------------------------------------------------------------------
// attributes for radio buttons that have focus

uC_attribs_t radio_focus_attrs =
{
    .flags.bits = (ATTR_FLAG_GRAY_BG),
    .fg         = uC_COLOR_LT_GREEN,
    .bg_gray    = uC_GRAY_09,
};

// -----------------------------------------------------------------------
// attributes for radio buttons

uC_attribs_t text_attrs =
{
    .flags.bits = (ATTR_FLAG_GRAY_BG),
    .fg         = uC_COLOR_BROWN,
    .bg_gray    = uC_GRAY_04,
};

// -----------------------------------------------------------------------
// attributes for radio buttons that have focus

uC_attribs_t text_focus_attrs =
{
    .flags.bits = (ATTR_FLAG_GRAY_BG),
    .fg         = uC_COLOR_BROWN,
    .bg         = uC_GRAY_06,
};

// =======================================================================
