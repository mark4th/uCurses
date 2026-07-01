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
  database, stable internal key codes, screen-level shortcuts, and mouse
  events delivered to focused windows and widgets.
- **Menu system** — menu bar with pull-down menus, modifier-capable
  screen-level accelerators, and application-controlled enable/disable
  state per pulldown or item.
- **Widget system** — buttons, radio buttons, checkboxes, and editable
  textboxes (decimal, octal, hex, binary, or alphanumeric input); widgets
  are grouped into scrollable views, and button shortcuts use the same
  screen-level shortcut registry as menu accelerators.
- **JSON UI description** — screens, windows, and menus can be described in
  a JSON file loaded at startup (or compiled into the executable).  This
  separates layout construction from application logic.
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

See the `example/` directory for complete working programs covering
windows, borders, menus, widgets, mouse handling, and more.

For the shortest "write your first app" path, start with
[`GETTING_STARTED.md`](GETTING_STARTED.md).  For a task-oriented guide to
writing a real application with the library, see
[`USING-uCurses.md`](USING-uCurses.md).  The large `uCurses.md` file remains
the reference/manual material for what the library contains and how it works.

---

## Menu dispatch code generator

The shared library doubles as a code-generation tool.  Because the ELF
`.interp` section is embedded in the `.so` and the linker entry point is
set to a custom `entry()` symbol, `libuCurses.so` can be executed directly
from the shell like a regular program:

```
./build/libuCurses.so --help
./build/libuCurses.so my_menu_vectors.txt
```

### The problem it solves

JSON-described menus carry a `"vector"` string on each menu item — the name
of the C function that should be called when that item is selected.  At
runtime the JSON parser needs to turn that string into a C function pointer.
Using `strcmp` for every lookup on every keypress works, but it keeps
function-name strings in the binary and touches them repeatedly at runtime.

uCurses uses FNV hashing instead.  The library hashes the vector string once
when the JSON is loaded, then compares a single `uint32_t` against a
pre-built dispatch table.  The strings never appear in the final binary.

### How to use it

1. Create a plain text file with one vector name per line — the names must
   exactly match the `"vector"` values in your JSON:

   ```
   file_new
   file_open
   file_save
   file_quit
   ```

2. Run the library against that file:

   ```
   ./build/libuCurses.so my_vectors.txt
   ```

3. The library prints ready-to-compile C code — a `switch_t` array with
   pre-computed hashes and a callback function — which you paste into your
   application source:

   ```c
   static switch_t my_vectors.txt
   {
       { 0x1a2b3c4d, file_new  },
       { 0x5e6f7a8b, file_open },
       ...
   };

   static opt_t menu_address_cb(uint32_t hash) { ... }
   ```

4. The JSON parser calls `menu_address_cb` to resolve vector names to
   function pointers when the menu JSON is loaded.

Lines beginning with `/` are treated as comments and skipped.

---

## Documentation

Full documentation — including the terminfo parser internals, attribute
system, keyboard matching, screen/window architecture, menu system, widget
system, and JSON format — is in [`uCurses.md`](uCurses.md).

---

## License

MIT — see [`COPYING`](COPYING).
