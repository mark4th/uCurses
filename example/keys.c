// keys.c -- keyboard state machine test bench
// -----------------------------------------------------------------------
// echoes every keypress exactly as the LIBRARY resolves it: the UC_KEY_*
// code uC_key() returns and the modifier mask from uC_key_mods().  a
// checklist tracks which keys have been seen, so a full keyboard pass is
// one sitting - press keys until every entry is green.
//
// this is the one keyboard test that cannot be automated.  test_key_sm
// drives the decoder over an array and test_key_stream drives it over a
// pipe, but neither presses a key on a real terminal, and neither covers
// handler dispatch, key stuffing or a real mouse report.
//
// Ctrl-X quits.  it is the only key that does, because every other key
// worth quitting on is a key under test - ESC especially, whose job here
// is to arrive on its own after the ~25ms window WITHOUT hanging.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_win_printf.h"
#include "uC_attribs.h"
#include "uC_borders.h"
#include "uC_terminfo.h"

#ifdef UC_MOUSE
#include "uC_mouse.h"
#endif

// -----------------------------------------------------------------------

#define QUIT_KEY    (0x18)          // Ctrl-X

#define MIN_WIDTH   (80)
#define MIN_HEIGHT  (20)

#define CHK_H       (9)             // checklist window interior height
#define CHK_COL_W   (15)            // one checklist entry
#define LOG_MAX     (64)            // ring of remembered log lines
#define LINE_MAX    (96)

// the F keys all default to uC_noop, so uC_key() returns UC_KEY_NONE for
// every one of them and the state machine's work is invisible.  install a
// handler per slot that stuffs a private code instead - F1 through F12
// become KF(1)..KF(12) and the decode can be seen.

#define KF_BASE     (0xe0)
#define KF(n)       ((uint8_t)(KF_BASE + (n) - 1))

// -----------------------------------------------------------------------

static uC_screen_t *screen;
static uC_window_t *log_win;
static uC_window_t *chk_win;
static uC_kh_t      saved_kh;

static char     log_line[LOG_MAX][LINE_MAX];
static int      log_count;          // total keys seen, not lines held
static int16_t  log_h;

// -----------------------------------------------------------------------

static uC_attribs_t win_attrs =
{
    .flags.bits = (uC_ATTR_FLAG_GRAY_FG | uC_ATTR_FLAG_GRAY_BG),
    .fg_gray    = uC_GRAY_14,
    .bg_gray    = uC_GRAY_02,
};

static uC_attribs_t bdr_attrs =
{
    .flags.bits = (uC_ATTR_FLAG_GRAY_FG | uC_ATTR_FLAG_GRAY_BG),
    .fg_gray    = uC_GRAY_10,
    .bg_gray    = uC_GRAY_02,
};

// -----------------------------------------------------------------------
// the checklist.  an entry is satisfied either by a specific key code, by
// a modifier bit appearing in the mask, or by the class of key.

enum
{
    CHK_CODE,               // val is a UC_KEY_* / KF() code
    CHK_ALTC,               // val is a character, and Alt must be held
    CHK_MOD,                // val is a KMOD_* bit
    CHK_PRINT,              // any printable character
};

typedef struct
{
    const char *name;
    uint8_t     tag;
    uint8_t     val;
} chk_t;

static const chk_t checks[] =
{
    { "Up",          CHK_CODE,  UC_KEY_UP      },
    { "Down",        CHK_CODE,  UC_KEY_DOWN    },
    { "Left",        CHK_CODE,  UC_KEY_LEFT    },
    { "Right",       CHK_CODE,  UC_KEY_RIGHT   },
    { "Home",        CHK_CODE,  UC_KEY_HOME    },
    { "End",         CHK_CODE,  UC_KEY_END     },
    { "PgUp",        CHK_CODE,  UC_KEY_PGUP    },
    { "PgDn",        CHK_CODE,  UC_KEY_PGDN    },
    { "Insert",      CHK_CODE,  UC_KEY_INSERT  },
    { "Delete",      CHK_CODE,  UC_KEY_DELETE  },
    { "Backspace",   CHK_CODE,  UC_KEY_BS      },
    { "Tab",         CHK_CODE,  UC_KEY_TAB     },
    { "BackTab",     CHK_CODE,  UC_KEY_BACKTAB },
    { "Enter",       CHK_CODE,  UC_KEY_ENTER   },
    { "Esc (bare)",  CHK_CODE,  UC_KEY_ESC     },
    { "Shift-Left",  CHK_CODE,  UC_KEY_SLEFT   },
    { "Shift-Right", CHK_CODE,  UC_KEY_SRIGHT  },
    { "F1",          CHK_CODE,  KF(1)          },
    { "F2",          CHK_CODE,  KF(2)          },
    { "F3",          CHK_CODE,  KF(3)          },
    { "F4",          CHK_CODE,  KF(4)          },
    { "F5",          CHK_CODE,  KF(5)          },
    { "F6",          CHK_CODE,  KF(6)          },
    { "F7",          CHK_CODE,  KF(7)          },
    { "F8",          CHK_CODE,  KF(8)          },
    { "F9",          CHK_CODE,  KF(9)          },
    { "F10",         CHK_CODE,  KF(10)         },
    { "F11",         CHK_CODE,  KF(11)         },
    { "F12",         CHK_CODE,  KF(12)         },
    { "Shift+key",   CHK_MOD,   KMOD_SHIFT     },
    { "Alt+key",     CHK_MOD,   KMOD_ALT       },
    { "Ctrl+key",    CHK_MOD,   KMOD_CTRL      },

    // O and [ are the state machine's own introducers, so Alt+O and Alt+[
    // are the only two letters whose Alt form is not the same code path as
    // the other 24.  they are here because they were both wrong: Alt+O
    // decoded as a bare ESC and Alt+[ was dropped on the floor.

    { "Alt+O",       CHK_ALTC,  'O'            },
    { "Alt+[",       CHK_ALTC,  '['            },
    { "printable",   CHK_PRINT, 0              },
#ifdef UC_MOUSE
    { "Mouse",       CHK_CODE,  UC_KEY_MOUSE   },
#endif
};

#define NUM_CHECKS (sizeof(checks) / sizeof(checks[0]))

static bool seen[NUM_CHECKS];

// -----------------------------------------------------------------------
// one handler per F key slot.  each stuffs its private code, which
// uC_key_raw() then returns to us as if the terminal had sent it

#define F_HANDLER(n) static void kf##n(void) { uC_set_key(KF(n)); }

F_HANDLER(1)  F_HANDLER(2)  F_HANDLER(3)  F_HANDLER(4)
F_HANDLER(5)  F_HANDLER(6)  F_HANDLER(7)  F_HANDLER(8)
F_HANDLER(9)  F_HANDLER(10) F_HANDLER(11) F_HANDLER(12)

// -----------------------------------------------------------------------
// K_f12 is spelled with a lower case f in key_index_t - not a typo here

static void install_key_handlers(void)
{
    saved_kh = uC_alloc_kh();

    uC_set_key_action(K_F1,  kf1);   uC_set_key_action(K_F2,  kf2);
    uC_set_key_action(K_F3,  kf3);   uC_set_key_action(K_F4,  kf4);
    uC_set_key_action(K_F5,  kf5);   uC_set_key_action(K_F6,  kf6);
    uC_set_key_action(K_F7,  kf7);   uC_set_key_action(K_F8,  kf8);
    uC_set_key_action(K_F9,  kf9);   uC_set_key_action(K_F10, kf10);
    uC_set_key_action(K_F11, kf11);  uC_set_key_action(K_f12, kf12);
}

// -----------------------------------------------------------------------

static const char *key_name(uint8_t k)
{
    static char buf[24];

    switch (k)
    {
        case UC_KEY_NONE:    return "NONE (no handler)";
        case UC_KEY_BS:      return "BS";
        case UC_KEY_TAB:     return "TAB";
        case UC_KEY_ENTER:   return "ENTER";
        case UC_KEY_ESC:     return "ESC";
        case UC_KEY_UP:      return "UP";
        case UC_KEY_DOWN:    return "DOWN";
        case UC_KEY_LEFT:    return "LEFT";
        case UC_KEY_RIGHT:   return "RIGHT";
        case UC_KEY_INSERT:  return "INSERT";
        case UC_KEY_DELETE:  return "DELETE";
        case UC_KEY_HOME:    return "HOME";
        case UC_KEY_END:     return "END";
        case UC_KEY_MOUSE:   return "MOUSE";
        case UC_KEY_BACKTAB: return "BACKTAB";
        case UC_KEY_PGDN:    return "PGDN";
        case UC_KEY_PGUP:    return "PGUP";
        case UC_KEY_SLEFT:   return "SHIFT-LEFT";
        case UC_KEY_SRIGHT:  return "SHIFT-RIGHT";
        default:             break;
    }

    if ((k >= KF(1)) && (k <= KF(12)))
    {
        snprintf(buf, sizeof(buf), "F%d", (k - KF_BASE) + 1);
        return buf;
    }

    if ((k >= 0x20) && (k < 0x7f))
    {
        snprintf(buf, sizeof(buf), "'%c'", (char)k);
        return buf;
    }

    if (k < 0x20)
    {
        snprintf(buf, sizeof(buf), "Ctrl-%c", (char)('@' + k));
        return buf;
    }

    snprintf(buf, sizeof(buf), "unnamed");
    return buf;
}

// -----------------------------------------------------------------------
// SAC - the three modifier bits in a fixed set of columns, so a wrong one
// is spotted by position rather than by reading

static void mods_str(uint8_t m, char *out)
{
    out[0] = (m & KMOD_SHIFT) ? 'S' : '-';
    out[1] = (m & KMOD_ALT)   ? 'A' : '-';
    out[2] = (m & KMOD_CTRL)  ? 'C' : '-';
    out[3] = '\0';
}

// -----------------------------------------------------------------------

static void tick(uint8_t key, uint8_t mods)
{
    size_t i;

    for (i = 0; i < NUM_CHECKS; i++)
    {
        switch (checks[i].tag)
        {
            case CHK_CODE:
                seen[i] |= (key == checks[i].val);
                break;

            case CHK_ALTC:
                seen[i] |= ((key == checks[i].val) &&
                            ((mods & KMOD_ALT) != 0));
                break;

            case CHK_MOD:
                seen[i] |= ((mods & checks[i].val) != 0);
                break;

            case CHK_PRINT:
                seen[i] |= ((key >= 0x20) && (key < 0x7f));
                break;

            default:
                break;
        }
    }
}

// -----------------------------------------------------------------------
// build the log line for one key.  the modifier mask is read through both
// uC_key_mods() and uC_alt() so the shorthand can be seen to agree

static void log_key(uint8_t key)
{
    char  mods[4];
    char *line = log_line[log_count % LOG_MAX];
    uint8_t m  = uC_key_mods();

    mods_str(m, mods);

    snprintf(line, LINE_MAX, " %4d  0x%02x  %-18s mods %s  alt=%d",
        log_count + 1, key, key_name(key), mods, uC_alt() ? 1 : 0);

#ifdef UC_MOUSE
    if (key == UC_KEY_MOUSE)
    {
        snprintf(line, LINE_MAX,
            " %4d  0x%02x  %-18s btn %d %s at %d,%d",
            log_count + 1, key, key_name(key), uC_mouse_event.button,
            uC_mouse_event.pressed ? "down" : "up  ",
            uC_mouse_event.x, uC_mouse_event.y);
    }
#endif

    log_count++;
    tick(key, m);
}

// -----------------------------------------------------------------------

static void draw_log(void)
{
    int16_t row;
    int     first;
    int     held;

    uC_win_clear(log_win);

    uC_win_printf(log_win, "%@%fcuCurses keyboard state machine - "
        "press keys, Ctrl-X quits  (-a = alternate screen)",
        UC_XY(0, 0), uC_COLOR_LT_CYAN);

    // the two checklist entries nobody can guess at: the collision is with
    // the SS3 introducer ESC O (0x4f), so it is the CAPITAL letter - Alt+o
    // is 0x6f and was never on the special path at all

    uC_win_printf(log_win, "%@%fcAlt+O means Alt+SHIFT+o (capital O).  "
        "Alt+[ is the plain bracket.", UC_XY(0, 1), uC_COLOR_GRAY);

    held  = (log_count < LOG_MAX) ? log_count : LOG_MAX;
    first = log_count - held;

    // the newest line sits on the bottom row.  only as many as fit are
    // drawn, oldest first, so the log reads downwards like a terminal

    if (held > (log_h - 3))
    {
        first = log_count - (log_h - 3);
        held  = log_h - 3;
    }

    for (row = 0; row < held; row++)
    {
        uC_win_printf(log_win, "%@%fc%s", UC_XY(0, row + 3),
            uC_COLOR_LT_WHITE, log_line[(first + row) % LOG_MAX]);
    }
}

// -----------------------------------------------------------------------

static void draw_checks(void)
{
    size_t  i;
    int16_t cols;
    int16_t rows;
    int     done = 0;
    char    buf[LINE_MAX];

    uC_win_clear(chk_win);

    cols = chk_win->width / CHK_COL_W;

    if (cols < 1)
    {
        cols = 1;
    }

    rows = (int16_t)((NUM_CHECKS + cols - 1) / cols);

    for (i = 0; i < NUM_CHECKS; i++)
    {
        int16_t x = (int16_t)((i / (size_t)rows) * CHK_COL_W);
        int16_t y = (int16_t)(i % (size_t)rows);

        snprintf(buf, sizeof(buf), "%s %s", seen[i] ? "[x]" : "[ ]",
            checks[i].name);

        uC_win_printf(chk_win, "%@%fc%s", UC_XY(x, y),
            seen[i] ? uC_COLOR_LT_GREEN : uC_COLOR_GRAY, buf);

        done += seen[i] ? 1 : 0;
    }

    snprintf(buf, sizeof(buf), "%d of %d keys seen", done, (int)NUM_CHECKS);

    uC_win_printf(chk_win, "%@%fc%s", UC_XY(0, chk_win->height - 1),
        (done == (int)NUM_CHECKS) ? uC_COLOR_LT_GREEN : uC_COLOR_YELLOW,
        buf);
}

// -----------------------------------------------------------------------
// the position has to be set before the attach and directly rather than
// through uC_win_set_pos(): attach validates the position it finds in the
// window, and uC_win_set_pos() refuses to work on an unattached window

static uC_window_t *open_win(int16_t x, int16_t y, int16_t w, int16_t h)
{
    uC_window_t *win = uC_win_open(w, h);

    if (win == NULL)
    {
        return NULL;
    }

    win->attrs = win_attrs;

    uC_win_set_flag(win, uC_WIN_LOCKED);
    uC_win_set_border(win, uC_BDR_SINGLE, bdr_attrs, bdr_attrs);

    win->xco = x;
    win->yco = y;

    uC_scr_win_attach(screen, win);

    return win;
}

// -----------------------------------------------------------------------
// two stacked windows inside the backdrop, sharing the border row between
// them.  interior width is width - 4 so both side borders land inside the
// backdrop's own

static bool build_ui(void)
{
    int16_t w = (int16_t)(screen->width  - 4);

    log_h = (int16_t)(screen->height - 5 - CHK_H);

    log_win = open_win(2, 2, w, log_h);
    chk_win = open_win(2, (int16_t)(log_h + 3), w, CHK_H);

    return (log_win != NULL) && (chk_win != NULL);
}

// -----------------------------------------------------------------------

int main(int argc, char **argv)
{
    uint8_t key;
    bool    alt = (argc > 1) && (argv[1][0] == '-') && (argv[1][1] == 'a');

    // no json - a screen the size of the console, with a backdrop

    screen = uCurses_init(NULL, NULL, NULL);

    if (screen == NULL)
    {
        printf("keys: cannot open screen\n");
        return 1;
    }

    // -a runs on the alternate screen.  worth having on the bench because
    // it is what decides where a mouse WHEEL event goes: on the primary
    // screen the terminal has scrollback of its own to scroll and eats the
    // event, on the alternate screen there is none, so it reaches us.
    // uC_restore_terminal() turns it back off on the way out.

    if (alt)
    {
        uC_altscreen_on();
    }

    if ((screen->width < MIN_WIDTH) || (screen->height < MIN_HEIGHT))
    {
        uCurses_deInit();
        printf("keys: terminal must be at least %d x %d\n",
            MIN_WIDTH, MIN_HEIGHT);
        return 1;
    }

    if (!build_ui())
    {
        uCurses_deInit();
        printf("keys: cannot open windows\n");
        return 1;
    }

    install_key_handlers();

#ifdef UC_MOUSE
    uC_mouse_enable();
#endif

    draw_log();
    draw_checks();
    uC_scr_draw_screen(screen);

    // uC_key() blocks in read(), so there is no poll loop here and no
    // reason for one - every iteration is exactly one keypress

    do
    {
        key = uC_key();

        log_key(key);
        draw_log();
        draw_checks();
        uC_scr_draw_screen(screen);
    } while (key != QUIT_KEY);

#ifdef UC_MOUSE
    uC_mouse_disable();
#endif

    uC_release_kh(saved_kh);

    // leave the console in a sane state

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(0, 0);

    uC_scr_close(screen);
    uCurses_deInit();

    return 0;
}

// =======================================================================
