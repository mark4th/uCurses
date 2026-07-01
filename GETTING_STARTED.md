# Getting Started With uCurses

This is the short path from a blank C file to a working uCurses program.
It intentionally leaves out most details.  Use this when you want the shape
of an application; use `USING-uCurses.md` and `uCurses.md` when you need the
full explanation.

## 1. Build The Library

From the uCurses checkout:

```sh
make
```

This creates the library under `build/`.  The examples can be rebuilt with:

```sh
make examples
```

## 2. Compile Your Program

Applications should compile with the feature macros for the library features
they use.  A full-featured build usually looks like this:

```sh
cc -I/path/to/uCurses/h \
   -DUC_JSON -DUC_MENUS -DUC_STATUS -DUC_WIDGETS -DUC_POPUPS \
   app.c -L/path/to/uCurses/build -Wl,-rpath,/path/to/uCurses/build \
   -luCurses -o app
```

Strip feature macros for code that does not use those subsystems.

## 3. Minimal Screen And Window

uCurses uses retained window buffers.  Draw into windows first, then ask the
screen to compose and emit one frame.

```c
#include <stdbool.h>
#include <stdint.h>

#include "uCurses.h"
#include "uC_attribs.h"
#include "uC_borders.h"
#include "uC_keys.h"
#include "uC_screen.h"
#include "uC_win_printf.h"
#include "uC_window.h"

static const uC_attribs_t panel_attrs =
{
    .flags.bits = uC_ATTR_FLAG_GRAY_BG,
    .fg = uC_COLOR_WHITE,
    .bg_gray = uC_GRAY_03,
};

static const uC_attribs_t border_attrs =
{
    .flags.bits = uC_ATTR_FLAG_GRAY_FG | uC_ATTR_FLAG_GRAY_BG,
    .fg_gray = uC_GRAY_12,
    .bg_gray = uC_GRAY_02,
};

static const uC_attribs_t focus_attrs =
{
    .flags.bits = uC_ATTR_FLAG_BOLD | uC_ATTR_FLAG_GRAY_BG,
    .fg = uC_COLOR_LT_CYAN,
    .bg_gray = uC_GRAY_04,
};

int main(void)
{
    bool done = false;
    uC_screen_t *screen;
    uC_window_t *win;

    screen = uCurses_init(NULL, NULL, NULL);
    if (screen == NULL)
    {
        return 1;
    }

    win = uC_win_open(48, 8);
    if (win == NULL)
    {
        uC_scr_close(screen);
        uCurses_deInit();
        return 1;
    }

    win->attrs = panel_attrs;
    uC_win_set_border(win, uC_BDR_SINGLE, border_attrs, focus_attrs);
    uC_win_set_name(win, "First uCurses App");
    win->xco = 4;
    win->yco = 3;
    uC_scr_win_attach(screen, win);

    while (!done)
    {
        uC_win_clear(win);
        uC_win_printf(win, "%@Press q to quit", UC_XY(2, 2));
        uC_win_printf(win, "%@Resize handling belongs in your app.",
            UC_XY(2, 4));
        uC_scr_draw_screen(screen);

        if (uC_winch_pending())
        {
            uC_scr_resize_hold(screen);
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

## 4. Main Loop Shape

Most applications follow this pattern:

```c
while (!quit)
{
    while ((uC_test_keys() == 0) && !uC_winch_pending())
    {
        do_background_work_or_sleep();
    }

    if (uC_winch_pending())
    {
        close_application_popups();
        uC_scr_resize_hold(screen);
        rebuild_layout();
        draw_frame();
        continue;
    }

    key = uC_key();
    handle_key_and_update_state(key);
    draw_frame();
}
```

Keep input handling and drawing separate.  Key handlers should change
application state.  Drawing should rebuild the visible windows from that
state.

## 5. Windows

Use windows for rectangular regions you draw yourself:

```c
uC_window_t *log = uC_win_open(60, 10);
log->attrs = panel_attrs;
uC_win_set_border(log, uC_BDR_SINGLE, border_attrs, focus_attrs);
uC_win_set_name(log, "Log");
log->xco = 2;
log->yco = 12;
uC_scr_win_attach(screen, log);
```

Redraw them from state each frame:

```c
uC_win_clear(log);
uC_win_printf(log, "%@%s", UC_XY(1, 1), latest_message);
```

## 6. Widgets

Widgets live in views.  Views live in view groups.

Important rule: one widget type per view.  A view may contain many widgets,
but all widgets in that view must be the same type.  If one visual panel
needs textboxes and checkboxes, use one view group with multiple views.

```c
uint32_t selected = 0;
uC_widget_vg_t *vg;
uC_widget_view_t *view;

vg = uC_widget_vg_create("Tests", 30, 10, 2, 2, panel_attrs);
uC_widget_vg_add_border(vg, uC_BDR_SINGLE, border_attrs, focus_attrs);

view = uC_widget_view_create("List", 24, 6, 2, 2, panel_attrs, true);
uC_widget_view_add_border(view, uC_BDR_SINGLE, border_attrs);
uC_widget_vg_add_view(vg, view, 1);

uC_widget_view_add_widget(view,
    uC_widget_check_create(&selected, 0, "GPIO", uC_RADIO_BOX,
        22, panel_attrs, focus_attrs),
    0);
uC_widget_view_add_widget(view,
    uC_widget_check_create(&selected, 1, "TWI", uC_RADIO_BOX,
        22, panel_attrs, focus_attrs),
    0);

uC_widget_vg_attach(screen, vg);
uC_widget_select_widget(1);
```

For a scrollable view, the whole view has one tab stop and the view owns the
row cursor.  Use `uC_widget_view_index(view)` to ask which row is selected.

## 7. Menus And Shortcuts

Menus and widgets are handled through `uC_key()`.  Do not build separate key
loops for menus and widgets unless you have a specific reason.

```c
static void menu_quit(void)
{
    uC_set_key('q');
}

uC_menu_bar_open(screen);
uC_menu_new_pd(screen, "File");
uC_menu_new_item(screen, "Quit", menu_quit, UC_SHORTCUT('Q'));
```

`uC_key()` gives active menus, popup widgets, normal widgets, and screen
shortcuts a chance to consume the key.  If nobody handles it, the key is
returned to the application.

## 8. Popups

Use a raw popup for information:

```c
uC_scr_popup_attach(screen, popup_window);
```

Use a widget popup for modal editing:

```c
uC_widget_popup_attach(screen, popup_vg);
```

Only one raw popup and one widget popup are attached to a screen at a time.
Widget popups are modal: background menus, shortcuts, and widgets do not
receive input until the popup is detached.

On resize, treat popups as invalid and rebuild them if needed.

## 9. Resize

Do not try to stretch every existing object in place.  Treat resize as a
layout invalidation:

1. Clear your app's popup-visible state.
2. Call `uC_scr_resize_hold(screen)`.
3. Recompute geometry.
4. Rebuild windows and view groups that depend on geometry.
5. Draw the next frame.

## 10. What To Read Next

- `USING-uCurses.md`: practical application patterns.
- `uCurses.md`: full reference and design notes.
- `example/widgets.c`: widget behavior.
- `example/window.c`: overlapping windows.
- `example/menus.c`: menu construction and menu shortcuts.
