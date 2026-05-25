// uC_keys.h
// -----------------------------------------------------------------------

#ifndef UC_KEYS_H
#define UC_KEYS_H

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

// -----------------------------------------------------------------------
// Key handler table system
// -----------------------------------------------------------------------
// The library maintains a current key handler table — an array of function
// pointers, one per named key (see key_index_t below).  When uC_key()
// matches an escape sequence it calls the corresponding handler, which may
// stuff a synthetic key code via uC_set_key() so the caller sees it as a
// normal return value.  Unhandled keys default to uC_noop and cause
// uC_key() to return 0.
//
// Cursor keys (K_CUU1 / K_CUD1 / K_CUB1 / K_CUF1) default to uC_noop.
// Any app that needs cursor key input must install its own handlers:
//
//   uC_kh_t saved = uC_alloc_kh();          // push new table (copy of defaults)
//   uC_set_key_action(K_CUU1, my_up);       // override individual keys
//   uC_set_key_action(K_CUD1, my_dn);
//   ...
//   uC_release_kh(saved);                   // pop table and restore previous
//
// uC_alloc_kh() returns the PREVIOUS table handle, not the new one.
// Pass that handle to uC_release_kh() on cleanup — not saving it leaks
// the allocated table.  The widget system manages its own table internally;
// apps that bypass the widget system must manage it themselves.
//
// this enumeration defines the order in which keyboard handlers are
// defined for each key press. they all correspond to an index into the
// key-action arrays (which can be user defined but the order cant)

typedef enum
{
    K_ENT,  K_CUU1, K_CUD1, K_CUB1, K_CUF1, K_BS,  K_DCH1,
    K_ICH1, K_HOME, K_END,  K_KNP,  K_KPP,  K_BT,
    K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   K_F6,
    K_F7,   K_F8,   K_F9,   K_F10,  K_F11,  K_f12
} __attribute__((__packed__)) key_index_t;

// -----------------------------------------------------------------------

typedef void uC_key_handler_t(void);
typedef void (**uC_kh_t)(void);

// -----------------------------------------------------------------------
// visibility hidden

void uC_read_keys(void);
int16_t match_key(void);

// -----------------------------------------------------------------------

API uC_kh_t uC_alloc_kh(void);
API void uC_release_kh(uC_kh_t saved);
API int8_t uC_test_keys(void);
API uC_key_handler_t *uC_set_key_action(key_index_t index,
    uC_key_handler_t *action);
API uint8_t uC_key(void);
API void uC_set_key(uint8_t c);
API void uC_flush_keys(void);
API bool uC_push_key_handler(uC_kh_t user_actions);
API bool uC_pop_key_handler(void);
void init_key_handlers(void);

// -----------------------------------------------------------------------

#endif // UC_KEYS_H

// =======================================================================
