// uC_keys.h
// -----------------------------------------------------------------------

#ifndef UC_KEYS_H
#define UC_KEYS_H

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "uCurses.h"

// -----------------------------------------------------------------------
// Key handler table system
// -----------------------------------------------------------------------
// The library maintains a current key handler table which is an array of
// function pointers, one per named key (see key_index_t below).  When
// uC_key() matches an escape sequence it calls the corresponding handler,
// which may stuff a synthetic key code via uC_set_key() so the caller
// sees it as a normal return value.  Unhandled keys default to uC_noop
// and cause uC_key() to return 0.
//
// Common non-printing keys default to the UC_KEY_* values below. Apps can
// still override individual handlers when they need custom key translation:
//
//   uC_kh_t saved = uC_alloc_kh();      // get new table with defaults
//   uC_set_key_action(K_CUU1, my_up);   // override individual keys
//   uC_set_key_action(K_CUD1, my_dn);
//   ...
//   uC_release_kh(saved);               // pop table and restore previous
//
// uC_alloc_kh() returns the PREVIOUS table handle, not the new one.  The
// new one becomes the active keyboard handler used by the system until it
// is released and the previous one is restored.
// Passing the value returned by uC_alloc_kh() to uC_release_kh() restores
// the previous handler. not saving and restoring it leaks the allocated
// table.
// The widget system manages its own table internally. apps that bypass
// the widget system must manage it themselves.
//
enum
{
    UC_KEY_NONE    = 0x00,
    UC_KEY_BS      = 0x08,
    UC_KEY_TAB     = 0x09,
    UC_KEY_ENTER   = 0x0a,
    UC_KEY_ESC     = 0x1b,
    UC_KEY_UP      = 0x81,
    UC_KEY_DOWN    = 0x82,
    UC_KEY_LEFT    = 0x83,
    UC_KEY_RIGHT   = 0x84,
    UC_KEY_INSERT  = 0x85,
    UC_KEY_DELETE  = 0x86,
    UC_KEY_HOME    = 0x87,
    UC_KEY_END     = 0x88,
    UC_KEY_MOUSE   = 0x89,
    UC_KEY_F10     = 0x8a,
    UC_KEY_BACKTAB = 0x8b,
    UC_KEY_PGDN    = 0x8c,
    UC_KEY_PGUP    = 0x8d,
    UC_KEY_SLEFT   = 0x8e,
    UC_KEY_SRIGHT  = 0x8f,
};

// this enumeration defines the order in which keyboard handlers are
// defined for each key press. they all correspond to an index into the
// key-action arrays (which can be user defined but the order cant)

typedef enum
{
    K_ENT,  K_CUU1, K_CUD1, K_CUB1, K_CUF1, K_BS,  K_DCH1,
    K_ICH1, K_HOME, K_END,  K_KNP,  K_KPP,  K_BT,
    K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   K_F6,
    K_F7,   K_F8,   K_F9,   K_F10,  K_F11,  K_f12,
    K_SLFT, K_SRIT
} __attribute__((__packed__)) key_index_t;

// -----------------------------------------------------------------------

typedef void uC_key_handler_t(void);
typedef void (**uC_kh_t)(void);

typedef uint16_t uC_shortcut_t;
typedef void uC_shortcut_action_t(void *context);

// -----------------------------------------------------------------------

enum
{
    UC_SHORTCUT_KEY_MASK = 0x00ff,
    UC_SHORTCUT_MOD_CTRL = 0x0100,
    UC_SHORTCUT_MOD_ALT  = 0x0200,
    UC_SHORTCUT_MOD_META = 0x0400,
};

// -----------------------------------------------------------------------

#define UC_SHORTCUT(k)       ((uC_shortcut_t)((uint8_t)(k)))
#define UC_SHORTCUT_CTRL(k)  ((uC_shortcut_t)(UC_SHORTCUT_MOD_CTRL | \
                                             (uint8_t)(k)))
#define UC_SHORTCUT_ALT(k)   ((uC_shortcut_t)(UC_SHORTCUT_MOD_ALT | \
                                             (uint8_t)(k)))
#define UC_SHORTCUT_META(k)  ((uC_shortcut_t)(UC_SHORTCUT_MOD_META | \
                                             (uint8_t)(k)))

// -----------------------------------------------------------------------
// visibility hidden

void uC_read_keys(void);
int16_t match_key(void);
bool uC_shortcut_register(uC_screen_t *scr,
    uC_shortcut_t shortcut, uC_shortcut_action_t *action, void *context,
    void *owner);
bool uC_shortcut_run(uC_screen_t *scr, uint8_t key);
bool uC_shortcut_run_popup(uC_screen_t *scr, uint8_t key);
void uC_shortcut_remove_owner(uC_screen_t *scr, void *owner);
void uC_shortcut_clear(uC_screen_t *scr);
bool uC_shortcut_matches(uC_shortcut_t shortcut, uint8_t key);

// -----------------------------------------------------------------------

API uC_kh_t uC_alloc_kh(void);
API void uC_release_kh(uC_kh_t saved);
API int8_t uC_test_keys(void);
API uint8_t uC_key_raw(void);
uC_key_handler_t *uC_set_default_key_action(key_index_t index,
    uC_key_handler_t *action);
bool uC_restore_default_key_action(key_index_t index,
    uC_key_handler_t *expected, uC_key_handler_t *action);
API uC_key_handler_t *uC_set_key_action(key_index_t index,
    uC_key_handler_t *action);
bool uC_restore_key_action(key_index_t index,
    uC_key_handler_t *expected, uC_key_handler_t *action);
API uint8_t uC_key(void);
API void uC_set_key(uint8_t c);
API void uC_flush_keys(void);
void init_key_handlers(void);

// -----------------------------------------------------------------------

#endif // UC_KEYS_H

// =======================================================================
