# Keyboard input: char-by-char state machine

Living plan — updated as implementation proceeds. Branch: `keyboard-state-machine`.

## Goal

Replace the "buffer a whole escape sequence, then hash-compare it against
terminfo-generated sequences" input path with a **byte-at-a-time state machine**
that decodes keys incrementally and reports **modifiers** (Alt first — the
sim51 breakpoint-browser Alt-B trigger — then Ctrl/Shift where cheap).

Terminfo cannot describe modified keys: Ctrl/Shift/Alt+arrow arrive as CSI
parameters (`ESC [ 1 ; 5 A`) or an ESC prefix (`ESC b` = Alt-b). So a generic
CSI/SS3 parser is required regardless of terminfo; that parser is the win.

## What we're replacing

- `src/keys/uC_key_read.c` — `uC_read_keys()` buffers bytes into `keybuff`
  until a 25ms poll gap after ESC. `uC_test_keys()` / `read_key()` stay (byte
  source).
- `src/keys/key_sequence.c` — `match_key()` regenerates each key's terminfo
  sequence and FNV-hash-compares the whole buffer. This is what the SM replaces.
- `src/keys/uC_key_table.c` — `uC_key_raw()` orchestrates read → match →
  handler dispatch, plus a *provisional* Alt+char hack (ESC + printable) that
  the SM supersedes. Handler-table dispatch (`key_index_t`, `uC_set_key`) stays.

## Public contract (KEEP)

- `uC_key()` / `uC_key_raw()` return `uint8_t` — `UC_KEY_*` (h/uC_keys.h).
- `uC_alt()` — side-channel bool, valid right after a key returns.
- Key-handler tables (`uC_alloc_kh` / `uC_set_key_action`, `key_index_t` order).
- Backspace 0x7f→0x08 normalization; ENTER=0x0a; ESC=0x1b.

## New design

A small state machine fed one byte at a time from `read_key()`:

```
GROUND: 0x1b        -> ESC
        else        -> emit byte (printable / control); done
ESC:    poll 25ms:
          no byte   -> emit bare ESC (0x1b); done
          '['       -> CSI
          'O'       -> SS3
          0x1b      -> stay ESC (double-ESC); Alt+ESC edge
          else b    -> emit b with ALT set (Alt+char); done
CSI:    collect [0-9;] into params[]; on final 0x40..0x7e -> decode(final,params)
SS3:    one final byte -> decode SS3 (P/Q/R/S = F1..F4; A..D = cursor)
```

`decode()` maps final byte + params to a `UC_KEY_*` and a modifier mask.
Modifier param (CSI param #2, or `1;<m>`): `m-1` is a bitmask
`bit0=Shift, bit1=Alt, bit2=Ctrl` (xterm convention).

Final-byte map (CSI): `A/B/C/D`=Up/Down/Right/Left, `H`=Home, `F`=End,
`Z`=BackTab. `~`-forms by param#1: 1=Home 2=Insert 3=Delete 4=End 5=PgUp
6=PgDn 15/17..=F5.. . SS3: `P/Q/R/S`=F1..F4, `A..D`=cursor (app mode).

Output of the SM per call: resolved `keycode` (into `keybuff[0]`, `num_k=1`)
plus a `mods` byte stored on `ti_vars` (Alt exposed via existing `uC_alt()`;
Ctrl/Shift reserved for a future `uC_key_mods()` — see Open decisions).

## Files

- NEW `src/keys/uC_key_sm.c` — the state machine (`uC_read_keys()` reimplemented
  to run it, producing keycode+mods). Add to `src/keys/meson.build`.
- EDIT `uC_key_table.c` `uC_key_raw()` — consume SM result; drop the provisional
  ESC+printable Alt hack (SM sets alt now).
- `key_sequence.c` / `match_key()` — retire (or keep terminfo fallback behind a
  flag for exotic terminals; leaning retire — CSI/SS3 covers the standard set).
- `h/uC_keys.h` — add `mods` field / accessor if we expose Ctrl/Shift.

## Testing

Mark drives the running TUI himself (do NOT build pty harnesses). Verify at
minimum: arrows, Home/End/PgUp/PgDn, Del/Ins, F-keys, ENTER/BS/TAB, bare ESC
(no hang, ~25ms), **Alt-b** (sim51 trigger), Ctrl/Shift+arrow. I only
compile-check the library here.

## Open decisions (update as we go)

1. Ctrl/Shift reporting: side-channel `uC_key_mods()` vs fold into shortcut
   system only. Alt is already `uC_alt()`. — LEANING: add a `mods` byte on
   ti_vars + `uC_alt()` keeps working; expose `uC_key_mods()` if Mark wants it.
2. Keep terminfo fallback for non-CSI terminals? — LEANING: retire; revisit if a
   target terminal misbehaves.
3. ESC timeout: keep 25ms `ESC_SEQUENCE_POLL_MS`. Fine for local; may need bump
   over slow links.

## Implementation notes (as built)

- First cut runs the SM over the **buffered** sequence (`uC_read_keys()` is
  unchanged — it still reads a whole ESC sequence delimited by the 25ms poll
  gap; `sm_parse()` walks that buffer char-by-char). This keeps the mouse path,
  handler dispatch and backspace-norm untouched → low regression risk. A truly
  *streaming* read (one byte → one state transition, no whole-buffer pre-read)
  is a later refinement if wanted; the decode logic already lives in one place.
- `sm_parse()` returns a `key_index_t` (caller runs `user_key_actions[]`),
  `SM_DIRECT` (keycode in `keybuff[0]`: bare ESC, Alt+char), or `SM_UNHANDLED`
  (mouse `ESC [ M` / `ESC [ <`, or unknown → caller's mouse parser then drop).
- Modifiers land in `uC_key_mods` (KMOD_SHIFT/ALT/CTRL). `uC_alt()` now reads
  its Alt bit — and Alt now works for **special** keys too (Alt+Up etc.), not
  just Alt+char. Ctrl/Shift are captured but not yet surfaced (see decision 1).
- `match_key()` / `key_sequence.c` are now **unused** by `uC_key_raw` but left
  in place (terminfo fallback, retire later per decision 2).
- Shift+Left/Right map to the existing `K_SLFT`/`K_SRIT` slots; other cursor
  keys report the base index + mods.

## Status

- [x] Branch + pull (origin was 1 ahead: grid/resize commit, unrelated).
- [x] Plan saved.
- [x] State machine core (`src/keys/uC_key_sm.c`) — CSI + SS3 + ESC-prefix.
- [x] Wire into `uC_key_raw` (replaces `match_key`; `uC_alt` reads `uC_key_mods`).
- [x] Build (glob picks up the file) — compiles clean, no warnings.
- [~] **Mark live-test** — CONFIRMED 2026-08-03 vs sim51 (no rebuild, .so only):
      **Alt-b** launches the bp browser, **F10** quits. Alt+char + CSI ~-form F-key
      paths good. Still to spot-check: arrows, Home/End/PgUp/PgDn, Del/Ins, bare
      ESC (no hang), Ctrl/Shift+arrow.
- [ ] Optional: `uC_key_mods()` accessor to expose Ctrl/Shift to apps.
- [ ] Optional: retire `match_key`/`key_sequence.c`; streaming read.
- [ ] Not committed yet (WIP on branch).
