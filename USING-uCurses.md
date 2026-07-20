# Using uCurses

This is the practical guide for building an application with uCurses.  The
large `uCurses.md` file is the reference manual: it describes what exists,
how the pieces work internally, and why they are shaped that way.  This file
answers the different question: "what do I write in my program?"

## Mental Model

A uCurses application usually has this shape:

1. Initialize the library and get a screen.
2. Create windows, status bars, menus, and widget view groups.
3. Enter a loop:
   - wait for a key or resize event,
   - update application state,
   - redraw window buffers,
   - ask uCurses to compose the screen.
4. Close the screen and deinitialize the library.

The important rule is that windows are retained buffers.  You draw into
windows first.  The terminal is updated only when you call
`uC_scr_draw_screen()`.

## Minimal Program

```c
#include <stdbool.h>

#include "uCurses.h"
#include "uC_borders.h"
#include "uC_keys.h"
#include "uC_screen.h"
#include "uC_win_printf.h"
#include "uC_window.h"

int main(void)
{
    bool done = false;
    uC_screen_t *screen;
    uC_window_t *main_win;

    screen = uCurses_init(NULL, NULL, NULL);
    if (screen == NULL)
    {
        return 1;
    }

    main_win = uC_win_open(50, 8);
    if (main_win == NULL)
    {
        uCurses_deInit();
        return 1;
    }

    uC_win_set_pos(main_win, 4, 3);
    uC_win_set_border(main_win, uC_BDR_SINGLE,
        main_win->bdr_attrs, main_win->focus_attrs);
    uC_win_set_name(main_win, "Example");
    uC_scr_win_attach(screen, main_win);

    while (!done)
    {
        uC_win_clear(main_win);
        uC_win_printf(main_win, "%@Press q to quit", UC_XY(2, 2));
        uC_scr_draw_screen(screen);

        if (uC_winch_pending())
        {
            uC_scr_resize_hold(screen);
            done = true; /* Real apps usually rebuild their layout here. */
            continue;
        }

        if (uC_test_keys())
        {
            done = (uC_key() == 'q');
        }
    }

    uC_scr_close(screen);
    uCurses_deInit();
    return 0;
}
```

Compile against the headers and library:

```sh
cc -I/path/to/uCurses/h app.c -L/path/to/uCurses/build -luCurses -o app
```

If the shared library is not installed in a system library path, either run
with `LD_LIBRARY_PATH=/path/to/uCurses/build` or add an rpath in your build.

## Windows

Use a normal window for any rectangular region that your application redraws.

```c
uC_window_t *win = uC_win_open(width, height);
uC_win_set_pos(win, x, y);
uC_win_set_border(win, uC_BDR_SINGLE, border_attrs, focus_attrs);
uC_win_set_name(win, "Log");
uC_scr_win_attach(screen, win);
```

Typical redraw code clears the window and writes the current state:

```c
uC_win_clear(win);
uC_win_printf(win, "%@state=%s", UC_XY(1, 1), state_name);
```

Use `uC_win_emit(win, codepoint)` for individual Unicode codepoints.  Do not
put multi-byte UTF-8 drawing characters into ordinary C strings unless the
API you are calling explicitly parses UTF-8.  For single drawing characters,
emit the codepoint directly.

## Status Bars

Status bars are one-line windows attached to the screen status list.  They
are useful for hints, transient notices, and compact runtime state.

```c
uC_window_t *status = uC_add_status(screen, 40, 0, 0);
uC_set_status(status, "F10 menu  q quit");
```

Status windows are drawn after normal windows, menus, and normal widget view
groups.  They are good for intentional overlays such as a helper strip.

## Menus

Menus are screen-owned.  Create a menu bar, add pulldowns, then add items.
Each item can have an action function and an optional shortcut.

```c
static void menu_quit(void)
{
    uC_set_key('q');
}

uC_menu_bar_open(screen);
uC_menu_new_pd(screen, "File");
uC_menu_new_item(screen, "Quit", menu_quit, UC_SHORTCUT('Q'));
```

In the input loop, call `uC_key()`.  It gives menus, shortcuts, and widgets
the correct chance to handle input before returning an unhandled key to the
application.

```c
uint8_t key = uC_key();
if (key == 'q')
{
    done = true;
}
```

Use F10 to activate the menu bar unless the application deliberately changes
the key table.

## Widgets And Views

Widgets live inside views.  Views live inside view groups.  A scrollable
view is the right way to present a list of similar controls that may be
taller than the space available.

```c
uint32_t selected = 0;
uC_widget_vg_t *vg;
uC_widget_view_t *view;
uC_widget_t *widget;

vg = uC_widget_vg_create("Tests", 30, 10, 2, 2, attrs);
uC_widget_vg_add_border(vg, uC_BDR_SINGLE, border_attrs, focus_attrs);

view = uC_widget_view_create("List", 24, 6, 2, 2, attrs, true);
uC_widget_view_add_border(view, uC_BDR_SINGLE, border_attrs);
uC_widget_vg_add_view(vg, view, 1);

widget = uC_widget_check_create(&selected, 0, "GPIO", uC_RADIO_BOX,
    22, attrs, focus_attrs);
uC_widget_view_add_widget(view, widget, 0);

widget = uC_widget_check_create(&selected, 1, "TWI", uC_RADIO_BOX,
    22, attrs, focus_attrs);
uC_widget_view_add_widget(view, widget, 0);

uC_widget_vg_attach(screen, vg);
uC_widget_select_widget(1);
```

Each widget also has a `void *user` field for application-owned context.
uCurses initializes it to `NULL` and never interprets, owns, or frees the
pointer.

```c
widget->user = document_section;
```

For scrollable views, the view owns the row cursor and scroll offset.  The
application should query the current view index instead of maintaining a
separate row cursor.

```c
uint16_t index = uC_widget_view_index(view);
uC_widget_to_view_index(view, index + 1);
```

Scrollable views are vertical by default.  A horizontal view lays out
equal-width, single-line widgets from left to right and scrolls with the left
and right arrow keys.  This is useful for tab strips and other compact
selectors.

```c
view = uC_widget_view_create(NULL, 36, 1, 0, 0, attrs, true);
if (!uC_widget_view_set_orientation(view, uC_VIEW_HORIZONTAL))
{
    /* invalid dimensions or a non-scrollable view */
}

widget = uC_widget_button_create(NULL, "Document", '\0',
    12, attrs, focus_attrs);
uC_widget_view_add_widget(view, widget, 0);

widget = uC_widget_button_create(NULL, "Summary", '\0',
    12, attrs, focus_attrs);
uC_widget_view_add_widget(view, widget, 0);
```

All widgets in a horizontal view must have the same nonzero width, fit within
the view width, and have a height of one.  Editable text boxes are rejected
because their own left/right editing keys conflict with view navigation.
The view remains one tab stop; query `uC_widget_view_index()` to identify the
focused item when Enter is pressed.

Checkbox and radio widgets store their state in bit masks.  If a logical
list has more than 32 entries, use more than one mask word and point each
widget at the word that owns its bit.

## Popups

There are two popup mechanisms:

- `uC_scr_popup_attach()` for a plain informational popup window.
- `uC_widget_popup_attach()` for a modal widget view group.

The screen has one raw popup pointer and one widget popup pointer.  Attaching
a new popup detaches the previous popup of the same type.  A popup is modal:
normal menus, normal shortcuts, and background widgets do not receive input
while it is active.  A widget popup still allows its own widget shortcuts and
widget navigation.

Plain informational popup:

```c
uC_window_t *popup = uC_win_open(40, 8);
uC_win_set_pos(popup, 20, 6);
uC_win_set_border(popup, uC_BDR_SINGLE, border_attrs, focus_attrs);
uC_win_set_name(popup, "Notice");
uC_win_printf(popup, "%@Any key closes this popup", UC_XY(2, 2));
uC_scr_popup_attach(screen, popup);

/* Later, after a key press: */
uC_scr_popup_detach(popup);
uC_win_close(popup);
```

Widget popup:

```c
uC_widget_popup_attach(screen, popup_vg);

/* Optional: override the automatically selected first popup widget. */
uC_widget_select_widget(sequence);

/* Later: */
uC_widget_popup_detach(popup_vg);
```

You can combine the two when it is useful: draw a normal popup window as the
frame/detail surface, then overlay a borderless popup view group on part of
that window.  Put the scrollable widget view inside that view group and let
the view draw its own border.

## Resize Handling

`SIGWINCH` is not a request to stretch every object in place.  Treat it as a
layout invalidation.

The usual pattern is:

```c
if (uC_winch_pending())
{
    uC_scr_resize_hold(screen);
    close_application_popups();
    rebuild_screen_and_layout();
    continue;
}
```

uCurses cancels active popup attachments when resize handling sees a WINCH.
The application should also clear its own popup-visible state before or
during layout rebuild so the next frame returns to a known main interface.

## Main Loop Pattern

Most applications end up with a loop like this:

```c
while (!quit)
{
    while ((uC_test_keys() == 0) && !uC_winch_pending())
    {
        sleep_or_poll_other_work();
    }

    if (uC_winch_pending())
    {
        uC_scr_resize_hold(screen);
        rebuild_ui();
        draw_ui();
        continue;
    }

    key = uC_key();
    handle_key(key);
    draw_ui();
}
```

Keep drawing code separate from key handling.  Let key handling change state;
let drawing rebuild windows from that state.

## Shutdown

Close or detach application-owned dynamic popups before closing the screen if
you allocated extra storage for them.  Then close the screen and deinitialize
the library.

```c
uC_scr_close(screen);
uCurses_deInit();
```

Do not keep window, view, widget, or screen pointers after the screen has
been closed.
