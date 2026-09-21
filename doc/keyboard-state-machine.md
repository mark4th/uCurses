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

`example/keys.c` is the bench for that pass (`make examples`, then
`example/keys`).  It echoes every keypress as the library resolves it — the
`UC_KEY_*` code `uC_key()` returns plus the `uC_key_mods()` mask and
`uC_alt()` — and keeps a checklist of the 33 keys/classes under test, so the
pass is one sitting: press until every entry is green.  **Ctrl-X quits**, and
nothing else does, because every other candidate is a key under test (ESC
especially — its job is to arrive on its own after the ~25ms window without
hanging).

Two things it has to do that a normal app does not:

- the 12 F-key slots all default to `uC_noop`, so `uC_key()` returns
  `UC_KEY_NONE` for every one of them and the SM's work is invisible.  the
  bench installs its own table (`uC_alloc_kh` + `uC_set_key_action`) whose
  handlers stuff private codes `0xe0`..`0xeb`, which also exercises the
  handler-dispatch and key-stuffing path the pipe test cannot reach.
- the mouse is a **build option, default off** (`meson_options.txt`).  the
  Mouse checklist entry and the click/drag test only exist under
  `-Dmouse=true`; a stock build tests 33 entries, not 34.

Still not covered by the bench: menu/widget/shortcut re-entrancy (that path is
mdv's and sim51's to exercise) and Alt+UTF-8 (unimplemented, see below).

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
- **Modifiers exist only on the CSI path.** `key_mods` is assembled from CSI
  param #1 in `sm_csi()`; the only other write is the hardcoded `KMOD_ALT` in
  the ESC-prefix branch.  So for *letters* there is no modifier mask at all:
  Shift+x is the uppercase byte, Ctrl+x is a control byte 0x01-0x1a — neither
  reaches the SM (no ESC, GROUND emits them) — and `uC_key_mods()` reports 0
  for both.  Ctrl-ness lives in the byte VALUE, not the mask.  Meta is Alt;
  there is no `KMOD_META` (only the shortcut system has `UC_SHORTCUT_MOD_META`).
  Ctrl+Alt+x is ESC + control byte: `KMOD_ALT`, Ctrl still in the byte.
  ★ this is why the test matrix is NOT modifiers x 26 letters — 24 of the 26
  are the same two lines of code.  The cross-product only means something on
  the CSI keys (arrows/F-keys/Home/End), which is what the bench checklists.
- **Alt+O and Alt+[ are the two exceptions** (fixed 2026-08-22).  `O` and `[`
  are the SM's own introducers, so those two Alt forms arrive as a sequence
  head whose body never comes.  Before: Alt+O fell out of `case 'O'` as
  `SM_DIRECT` with `keybuff[0]` still 0x1b — **a bare ESC** — and Alt+[ fell
  out of `sm_csi()` as `SM_UNHANDLED`, so `uC_key_raw()` **dropped it** and
  blocked for another key.  Both now route through the new `sm_alt_char()`
  helper, which the `default:` branch shares: the poll window closing is
  already proof the sequence ended, and that is exactly the Alt+char
  condition.  A body that DOES arrive still wins (`ESC O P` = F1,
  `ESC [ A` = Up) — locked in by four unit tests plus two streaming ones.
  ⚠ the trade-off: over a slow enough link a real `ESC O P` whose `P` misses
  the window now degrades to **Alt+O** rather than to a bare ESC — a plausible
  keystroke instead of an obviously-wrong one.  `SM_INTRA_MS` is the lever
  (open decision 3).
- **Modified F1–F4 were dropped** (fixed 2026-08-22).  F1–F4 have no `~` form:
  unmodified they arrive as SS3 `ESC O P..S`, and SS3 cannot carry a
  parameter — so the *modified* forms arrive as CSI `ESC [ 1 ; <m> P..S` and
  fell out of `final_letter()` as `SM_UNHANDLED`.  Ctrl+F1 and Shift+F1
  vanished exactly the way Alt+[ did.  F5–F12 were never affected: they go
  through `tilde_number()`, which is modifier-agnostic.  New `csi_fkey()`
  maps P/Q/R/S.
  ⚠⚠ **it is gated on `param[0] == 1`, and that gate is load-bearing.**  CSI
  `R` is *also* the cursor position report — the reply to the `ESC [ 6 n`
  that `uC_get_console_size()` (`uC_utils.c:150`) writes, and that runs from
  `uC_screen_resize.c:134`, i.e. **during the key loop, on SIGWINCH**.  An
  ungated `R` → F3 makes a window resize look like an F3 keypress.  The probe
  parks the cursor on the bottom row (`ESC [ 9999 d`) before asking, so a
  report's `param[0]` is the last row and never 1, while a modified F key's
  `param[0]` always is.  `test_cursor_position_report_is_not_f3` pins it.
- **The whole numeric keypad was dead** (fixed 2026-08-22, found live).
  `uC_smkx()` sends terminfo's smkx, which on xterm is `\E[?1h\E=` — DECCKM
  *and* **DECKPAM**.  In application keypad mode the keypad stops sending
  plain characters and sends SS3: `ESC O o` = `/`, `ESC O j` = `*`,
  `ESC O k/m/n` = `+ - .`, `ESC O p`..`y` = `0`..`9`, `ESC O M` = keypad
  Enter.  `decode_ss3()` knew only `P/Q/R/S` and fell through to
  `final_letter()`, so every one of them was `SM_UNHANDLED` and dropped.
  New `keypad_char()` maps them; they are **emitted as the plain character**
  (`sm_emit(c, 0)`) rather than given table slots, because the character is
  what the app would have received with the mode off.  Keypad Enter returns
  `K_ENT` so it stuffs `UC_KEY_ENTER` like the main one.
  ★ note `mods` is 0, not `KMOD_ALT` — these arrive via SS3, not the ESC
  prefix, and a keypad `*` must be indistinguishable from a typed `*`.
  (With NumLock **off** the pad sends the CSI navigation forms instead,
  which `final_letter()`/`tilde_number()` already handled — which is why
  this went unnoticed.)
- `sm_alt_char()` generalised to `sm_emit(b, mods)` and lifted above
  `decode_ss3()` so both the Alt path and the keypad path share it.

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
- [x] Committed b9c761b (first cut) + bbd72ab (Codex review: param-overflow
      clamp, `uC_key_mods()` accessor, `test/test_key_sm.c`). Pushed.
- [x] `uC_key_mods()` accessor (full KMOD mask) — DONE (bbd72ab).
- [ ] Streaming read — Stage 1 DONE (see below); Stage 2/3 pending.
- [ ] Optional: retire `match_key`/`key_sequence.c` (Stage 3).

## Streaming reader — staged plan (2026-08-04)

Goal: replace the fragile *whole-sequence* 25 ms poll (`uC_read_keys()` reads
greedily into `keybuff`, then `sm_parse()` scans it) with a **byte-at-a-time**
read where the SM pulls each post-ESC byte on its own 25 ms window. Robust to
slow links / split sequences, and the natural home for Alt+UTF-8 grouping.

Staged so each commit is green and low-risk; the risky I/O flip (Stage 2)
is isolated and TUI-test-gated (only Mark can drive the live terminal).

### Stage 1 — pull-based SM behind a byte-source seam ✅ DONE (this session, uncommitted→commit next)

`sm_parse()` no longer indexes `keybuff` directly. The SM pulls bytes through
a source: `typedef int (*sm_source_t)(void *ctx, int timeout_ms)` returning the
next raw byte or -1 ("none within window"). New entry `sm_run(src, ctx)` drives
GROUND/CSI/SS3 by pulling; `sm_parse()` is now a thin wrapper that runs it over
a **buffer source** (`buf_source` walks the already-filled `keybuff` from [1]).
Behaviour is byte-identical — proven by the existing 17 asserts passing
unchanged. Added 5 more asserts driving `sm_run()` through an independent array
source (`test/test_key_sm.c`), incl. bare-ESC-via-empty-source and incomplete
CSI. All in `src/keys/uC_key_sm.c` + `h/uC_keys.h` (seam decls). No change to
`uC_key_raw` or the read path yet → zero runtime regression risk.

### Stage 2 — live fd source + one-byte read ✅ DONE (this session)

Implemented as specced below. `uC_key_fd_source()` + `uC_read_key()` in
`uC_key_read.c`; `uC_key_raw()` now reads one byte (`uC_read_key`) and, on ESC,
calls `sm_run(uC_key_fd_source, NULL)` instead of `uC_read_keys()`+`sm_parse()`.
`uC_read_keys()` is left in place, now unused. NEW integration test
`test/test_key_stream.c` drives the REAL fd path over a pipe (read end duped to
stdin): arrows, ctrl+right, tilde-del, SS3-F1, **bare-ESC-via-25ms-timeout**,
**Alt-b-via-timeout**, and X10-mouse-drain (verifies keybuff is filled from
index 3 for `uC_mouse_parse`). All green. Unity reports on fd 2 (`unity_putc`)
so its output is separate from the fake stdin. STILL NEEDS Mark's live TUI
pass (dispatch/stuffed/
shortcut re-entrancy/real mouse aren't covered by the pipe test): arrows,
Home/End, PgUp/PgDn, Del/Ins, F-keys, ENTER/BS/TAB, bare ESC (no hang), Alt-b,
Ctrl/Shift+arrow, a mouse click/drag.

Original spec (as implemented):

The only new code is a source that reads the tty. Sketch, in
`src/keys/uC_key_read.c` (it already owns `pfd` + `read_key()`):

```c
// returns next stdin byte within timeout_ms, or -1; appends it to keybuff so
// the mouse parser (reads keybuff/num_k) and SM_DIRECT still see the bytes.
int uC_key_fd_source(void *ctx, int timeout_ms)
{
    (void)ctx;
    if (poll(&pfd, 1, timeout_ms) <= 0) return -1;   // no byte in window
    if (ti_vars->num_k >= KEY_BUFF_SZ)  return -1;    // overflow guard
    uint8_t b;
    if (read(0, &b, 1) != 1)            return -1;
    ti_vars->keybuff[ti_vars->num_k++] = b;
    return b;
}
```

Then `uC_key_raw()` (uC_key_table.c): read ONE byte (blocking, honoring
`stuffed`); if it isn't 0x1b it's a complete key (`num_k=1`, done); if it is
0x1b, set `keybuff[0]=0x1b; num_k=1;` and call
`c = sm_run(uC_key_fd_source, NULL);` instead of `uC_read_keys(); sm_parse();`.
The rest of `uC_key_raw` (handler dispatch, SM_DIRECT, mouse fall-through,
0x7f→0x08) is unchanged: on SM_UNHANDLED the mouse bytes are already appended
to keybuff by the source (the `sm_csi` mouse-drain loop pulls them), so
`uC_mouse_parse()` works as before.

Watch-outs:
- `num_esc`/`stuffed` and `uC_set_key()` stuffing path — keep working (a
  stuffed key must still return without touching the tty).
- Mouse drain currently ends on a 25 ms gap; that matches today's whole-buffer
  poll, so no worse. SGR/X10 both handled by the `'M'`/`'<'` branch.
- `uC_read_keys()` becomes unused by `uC_key_raw` — leave it (or delete once
  nothing else calls it; grep first).
- CANNOT unit-test the fd source here (needs a real fd); CANNOT TUI-test
  (Mark drives it). So Stage 2 must be validated live: arrows, Home/End,
  PgUp/PgDn, Del/Ins, F-keys, ENTER/BS/TAB, bare ESC (no hang ~25 ms),
  Alt-b, Ctrl/Shift+arrow, and a mouse click/drag (mouse parser still fed).

### Stage 3 — retire `match_key`/`key_sequence.c` ✅ DONE (committed local, unpushed); Alt+UTF-8 still open

Deleted `src/keys/key_sequence.c` (held `match_key`, `k_table`, and the
`ti_k*` sequence generators — all only ever fed `match_key`). Removed the
`match_key` decl from `uC_keys.h` and the stale `extern k_table` from
`uC_key_table.c`. Build globs `src/keys/*.c` so the file just drops out.
Behaviour-neutral: `match_key` had no callers since Stage 0 replaced it with
the SM, so this can't affect the pending live test — committed as its own
commit, left UNPUSHED so it can be held/dropped independently. Builds clean,
11/11 tests pass.

`uC_read_keys()` — the greedy whole-sequence reader Stage 2 replaced — is now
gone too (2026-08-22), along with its `ESC_SEQUENCE_POLL_MS`; the only poll
window left is `SM_INTRA_MS` in `uC_key_sm.c`, which is the one the streaming
reader actually uses.  Nothing called it.

Alt+UTF-8 remains:
Alt+UTF-8: extend the `sm_run` `default:` branch to, on `b` being a UTF-8 lead
byte, pull its continuation bytes as one Alt+char — but the return path is a
single `uint8_t` (`keybuff[0]`), so a >255 codepoint needs an API decision
first (widen the key carrier or expose the codepoint via a side-channel). Not
started; design call, not a mechanical change.
