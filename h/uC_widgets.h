// uC_widgets.h
// -----------------------------------------------------------------------

#ifndef UC_WIDGETS_H
#define UC_WIDGETS_H

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "uCurses.h"
#include "uC_list.h"
#include "uC_borders.h"
#include "uC_screen.h"
#include "uC_borders.h"
#include "uC_window.h"

// -----------------------------------------------------------------------

#define MAX_WIDGET_NAME_LEN (64)

// -----------------------------------------------------------------------

typedef enum
{
    INPUT_BINARY  = 0,        // 0-1
    INPUT_OCTAL   = 1,        // 0-7
    INPUT_DECIMAL = 2,        // 0-9
    INPUT_HEX     = 3,        // 0-9 a-f A-F
    INPUT_ALPHA   = 4,        // 0-9 a-z A-Z _
} __attribute__((__packed__)) uC_textbox_radix_t;

// -----------------------------------------------------------------------

typedef enum
{
    uC_WIDGET_NONE,
    uC_WIDGET_BUTTON,       //
    uC_WIDGET_RADIO,        // radio buttons, only one can be selected
    uC_WIDGET_CHECK,        // checkbox buttons, any number can be selected
    uC_WIDGET_TEXTBOX,      // editzble text, decimal, octal, hex, binary
} __attribute__((__packed__)) uC_widget_type_t;

// -----------------------------------------------------------------------

typedef enum
{
    VIEW_BOXED,        // view has a border
    VIEW_NAMED,        // view has a name
    VIEW_SCROLL,
} __attribute__((__packed__)) uC_view_flags_t;

// -----------------------------------------------------------------------
// visual chars for radio buttons and checkbox's

// i could add triangles to this too

typedef enum
{
    uC_RADIO_CHECKBOX,    // ☐  ☑
    uC_RADIO_XBOX,        // ☐  ☒
    uC_RADIO_BOX,         // □  ▣
    uC_RADIO_BIGBOX,      // □  ■
    uC_RADIO_SMALLBOX,    // ▫  ▪
    uC_RADIO_DIAMOND,     // ◇  ◈
} __attribute__((__packed__)) uC_radio_type_t;

// -----------------------------------------------------------------------

// view groups are containers for views.  A view can only have one type
// of widget associated with it but a view group can have many different
// associated views.   The view group is the window into which all views
// associated with it are drawn.

typedef struct
{
    uC_window_t window;     // allocated when attached to screen
    uC_list_t views;        // views to be drawn into above window
} uC_widget_vg_t;

// -----------------------------------------------------------------------

typedef struct
{
    uC_view_flags_t flags;
    uC_list_t widgets;      // list of widgets attached to this view

    uC_list_node_t *view_node;

    char *name;             // view name drawn in top border

    int16_t width;
    int16_t height;
    int16_t xco;            // view x/y coords within parent window
    int16_t yco;

    // if there are more widgets associated with this view than can fit in
    // the vertical space defined above then that view can be scrollable,
    // views can only scroll up and down not left or right

    uint16_t sequence;      // whole view gets same sequence number
    uint16_t top;           // index to widget at top of view
    uint16_t cy;            // view cursor y position to widget with focus

    uC_attribs_t attrs;
    uC_attribs_t box_attrs; // attribs for border
    uC_border_type_t box_type;
} uC_widget_view_t;

// -----------------------------------------------------------------------

typedef struct
{
    char letter;            // keyboard shortcut
    uint16_t *select;       // widget sequence of butten written here
} __attribute__((__packed__)) uC_widget_button_t;

// -----------------------------------------------------------------------

typedef struct
{
    uint16_t bit;           // which bit of *select to toggle
    uint32_t *select;       // only one bit can be active
    uC_radio_type_t type;
} __attribute__((__packed__)) uC_widget_radio_t;

// -----------------------------------------------------------------------

typedef struct
{
    uint16_t bit;           // which bit of *select to toggle
    uint32_t *select;       // any can be active
    uC_radio_type_t type;   // which graphc to display for on / off state
} __attribute__((__packed__)) uC_widget_check_t;

// -----------------------------------------------------------------------

typedef struct
{
    uC_textbox_radix_t radix;

    // width is defined in the widget structure, size can be wider

    bool insert;
    uint8_t size;           // size of text edit buffer
    uint8_t count;          // how many chars are in the edit buffer
    uint8_t cx;             // cursor position within edit buffer
    uint8_t offset;         // offset to char at start of widget display
    char *data;             // where to store the string
} __attribute__((__packed__)) uC_widget_textbox_t;

// -----------------------------------------------------------------------

typedef struct
{
    uint16_t xco;           // position of widget in view
    uint16_t yco;
    uint16_t width;         // width of widget (all have a height of 1)
    uint16_t sequence;      // position in tab focus sequence

    bool focused;           // true if widget has focus
    bool disabled;          // allows for mutual exclusion

    uC_attribs_t attrs;
    uC_attribs_t focus_attrs;

    char *name;             // display name for widget

    uC_widget_type_t type;  // one of the following

    // i hate unions, they make the code look like a cluster fuck
    // and this blob is as large as the largest of is elements which
    // makes this very non space efficicent.

    union
    {
         uC_widget_button_t   button;
         uC_widget_radio_t    radio;
         uC_widget_check_t    check;
         uC_widget_textbox_t  textbox;
    } __attribute__((__packed__));
} __attribute__((__packed__)) uC_widget_t;

// -----------------------------------------------------------------------

typedef struct
{
    uint16_t sequence;      // current tab sequence

    uC_widget_vg_t *vg;
    uC_widget_view_t *view;
    uC_widget_t *widget;    // current widget with focus in view
} __attribute__((__packed__)) widget_state_t;

// -----------------------------------------------------------------------

enum
{
    WIDGET_KEY_BS   = 0x08,

    // keys which return escape sequences are translated into the
    // following values.  no key can actually return any of these
    // so they are safe to use.

    WIDGET_KEY_UP   = 0x81,
    WIDGET_KEY_DOWN,
    WIDGET_KEY_LEFT,
    WIDGET_KEY_RIGHT,
    WIDGET_KEY_INSERT,
    WIDGET_KEY_DELETE,
    WIDGET_KEY_HOME,
    WIDGET_KEY_END,
};

// -----------------------------------------------------------------------
// visibility hidden

uC_widget_t *create_widget(uC_widget_type_t type,
    char *name, uint16_t sequence,
    uint16_t xco, uint16_t yco, uint16_t width,
    uC_attribs_t attrs, uC_attribs_t focus);

uint8_t handle_button(uint8_t k);
uint8_t handle_check(uint8_t k);
uint8_t handle_radio(uint8_t k);
uint8_t handle_text(uint8_t k);

void widget_close_view(uC_widget_view_t *view);
void widget_scroll_view(uint8_t k);
char tab_next_widget(void);

void draw_button(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y);
void draw_check(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y);
void draw_radio(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y);
void draw_textbox(uC_window_t *win, uC_widget_t *widget,
    uint16_t x, uint16_t y);

// -----------------------------------------------------------------------

API uC_widget_vg_t *uC_widget_vg_create(
    char *name,
    uint16_t width, uint16_t height,
    uint16_t xco, uint16_t yco,
    uC_attribs_t attrs);

// -----------------------------------------------------------------------

API void uC_widget_vg_add_border(uC_widget_vg_t *vg,
    uC_border_type_t bdr_type,
    uC_attribs_t bdr_attrs,
    uC_attribs_t focus_attrs);

// -----------------------------------------------------------------------

API void uC_widget_vg_attach(uC_screen_t *scr, uC_widget_vg_t *vg);
API void uC_widget_vg_detach(uC_screen_t *scr, uC_widget_vg_t *vg);
API void uC_widget_vg_close(uC_screen_t *scr, uC_widget_vg_t *vg);
API void uC_widget_vg_add_view(uC_widget_vg_t *vg, uC_widget_view_t *v);

// -----------------------------------------------------------------------
// uC_widget_view.c

API uC_widget_view_t *uC_widget_view_create(char *name,
    uint16_t width, uint16_t height, uint16_t xco, uint16_t yco,
    uC_attribs_t attrs, bool scroll);

API void uC_widget_view_add_border(uC_widget_view_t *view,
    uC_border_type_t bdr_type, uC_attribs_t bdr_attrs);

API bool uC_widget_view_add_widget(uC_widget_view_t *view, uC_widget_t *w);

// -----------------------------------------------------------------------

API uC_widget_t *uC_widget_button_create(
    uint16_t sequence, uint16_t *select,
    char *name, char letter, uint16_t width,
    uint8_t xco, uint8_t yco,
    uC_attribs_t attrs, uC_attribs_t focus);

API uC_widget_t *uC_widget_radio_create(
    uint16_t sequence, uint32_t *select,
    uint16_t bit, char *name, uC_radio_type_t type,
    uint16_t width, uint16_t xco, uint16_t yco,
    uC_attribs_t attrs, uC_attribs_t focus);

API uC_widget_t *uC_widget_check_create(
    uint16_t sequence, uint32_t *select,
    uint16_t bit, char *name, uC_radio_type_t type,
    uint16_t width, uint16_t xco, uint16_t yco,
    uC_attribs_t attrs, uC_attribs_t focus);

API uC_widget_t *uC_widget_text_create(
    uint16_t sequence, char *data, char *name,
    uint16_t size, uC_textbox_radix_t radix,
    uint16_t width, uint8_t xco, uint8_t yco,
    uC_attribs_t attrs, uC_attribs_t focus);

// -----------------------------------------------------------------------

API char uC_widget_main(void);

// -----------------------------------------------------------------------

#endif // UC_WIDGETS_H

// =======================================================================
