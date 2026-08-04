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
fds 0/1): arrows, ctrl+right, tilde-del, SS3-F1, **bare-ESC-via-25ms-timeout**,
**Alt-b-via-timeout**, and X10-mouse-drain (verifies keybuff is filled from
index 3 for `uC_mouse_parse`). All green. Unity reports on fd 2 (`unity_putc`)
since 0/1 are the pipe. STILL NEEDS Mark's live TUI pass (dispatch/stuffed/
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
11/11 tests pass. (`uC_read_keys()` is also unused now but kept as a labelled
legacy reference; drop it too if wanted.)

Alt+UTF-8 remains:
Alt+UTF-8: extend the `sm_run` `default:` branch to, on `b` being a UTF-8 lead
byte, pull its continuation bytes as one Alt+char — but the return path is a
single `uint8_t` (`keybuff[0]`), so a >255 codepoint needs an API decision
first (widen the key carrier or expose the codepoint via a side-channel). Not
started; design call, not a mechanical change.
