# uCurses — micro-sized curses for Linux virtual terminals

uCurses is a compact C library for building terminal user interfaces on
Linux.  It is not a replacement for ncurses — it is a focused alternative
for developers who need a capable TUI on systems where size matters, such
as embedded Linux targets.

At the time of writing, the full library builds in about two seconds and
weighs in around 87 kB.  ncurses is roughly 4.3 MB and takes several
minutes to compile.  On a resource-constrained target the difference is
significant.

---

## Features

- **Moveable, overlapping windows** — windows are independent objects that
  can be repositioned at runtime and drawn in any order.
- **Full-colour text** — 256-colour palette, 24-bit RGB, and greyscale
  attributes per cell, plus bold, underline, and reverse.
- **UTF-8 and Braille** — native UTF-8 codepoint support throughout;
  Braille rendering utilities included.
- **Keyboard and mouse input** — escape-sequence matching via the terminfo
  database; mouse events delivered to focused windows and widgets.
- **Menu system** — menu bar with pull-down menus, keyboard shortcuts, and
  enable/disable per item.
- **Widget system** — buttons, radio buttons, checkboxes, and editable
  textboxes (decimal, octal, hex, binary, or alphanumeric input); widgets
  are grouped into scrollable views.
- **JSON UI description** — screens, windows, and menus can be described in
  a JSON file loaded at startup (or compiled into the executable).  This
  separates layout from application logic and allows end-user theming.
- **Optional components** — menus, widgets, and the JSON parser are
  independent build options.  Stripping any of them reduces binary size for
  applications that don't need them.

---

## Building

```
make
```

Both `libuCurses.so` and `libuCurses.a` are built by default.  The build
uses Meson/Ninja under the hood — the Makefile is a thin wrapper.  Menus,
widgets, and the JSON parser each have their own build flags in
`meson.build` and can be stripped from the build independently.

---

## Concepts

### Screen → Window → Draw → Update

uCurses follows a retained-mode model:

1. **Screen** — created by `uCurses_init()`.  Owns the terminal and holds
   a list of windows.
2. **Window** — created with `uC_win_open(width, height)` and attached to a
   screen.  Has its own cell buffer; drawing happens here.
3. **Draw** — write into windows using `uC_win_emit()`, `uC_win_printf()`,
   border helpers, and attribute setters.  Nothing is sent to the terminal
   yet.
4. **Update** — call the screen draw function to flush the composed frame to
   the terminal in a single pass.

### Attributes

Each cell carries its own colour and style.  Set attributes on a window
before writing; everything written afterward inherits them.

```c
uC_win_set_rgb_fg(win, UC_RGB(0xff, 0x80, 0x00));
uC_win_set_bold(win);
uC_win_printf(win, "hello");
```

The `UC_XY`, `UC_WH`, and `UC_RGB` macros clarify multi-argument calls at
the use site.

### Keyboard input

```c
if (uC_test_keys())
{
    uC_read_keys();
    int16_t key = match_key();   // returns key index or -1
}
```

Escape sequences are matched against the terminfo database at runtime, so
the same binary works correctly across terminal emulators.

---

## Quick start

```c
#include "uCurses.h"
#include "uC_window.h"
#include "uC_win_printf.h"

int main(void)
{
    uC_screen_t *scr = uCurses_init(NULL, NULL, NULL);

    uC_window_t *win = uC_win_open(40, 10);
    uC_win_set_pos(win, 5, 5);
    uC_win_printf(win, "Hello from uCurses!");

    // attach win to scr, then call screen draw

    uCurses_deInit();
    return 0;
}
```

See the `example/` directory for complete working programs covering
windows, borders, menus, widgets, mouse handling, and more.

---

## Documentation

Full documentation — including the terminfo parser internals, attribute
system, keyboard matching, screen/window architecture, menu system, widget
system, and JSON format — is in [`uCurses.txt`](uCurses.txt).

---

## License

MIT — see [`COPYING`](COPYING).
