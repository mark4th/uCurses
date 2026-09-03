// uC_utils.h
// -----------------------------------------------------------------------

#ifndef UC_UTILS_H
#define UC_UTILS_H

// -----------------------------------------------------------------------

#include <stdbool.h>

// -----------------------------------------------------------------------

#define FNV_PRIME (0x01000193)
#define FNV_BASIS (0x811c9dc5)

// -----------------------------------------------------------------------
// Public helpers used by the library and available to applications.

API int32_t uC_fnv_hash(uint8_t *s);
API void uC_assert(bool f, const char *file, int line, const char *msg);
API void uC_ui_free(void *mem);

// -----------------------------------------------------------------------

#define uC_ASSERT(f, msg) uC_assert(f, __FILE__, __LINE__, msg)

// -----------------------------------------------------------------------

API __attribute__((noreturn)) void uC_abort(const char *msg);
API void uC_set_fatal_handler(void (*fp)(const char *msg));
API void uC_noop(void);
API void uC_clock_sleep(int32_t when);
API void uC_restore_terminal(void);
API void uC_init_terminal(void);

// Ctrl-C, opt out.  by default Ctrl-C raises SIGINT and never reaches
// uC_key().  uC_ctrl_c_off() disables the terminal's INTR character so it
// arrives as key 0x03 - what UC_SHORTCUT_CTRL('C') has always expected -
// and uC_ctrl_c_on() puts it back.  ISIG itself is untouched, so both
// Ctrl-\ (SIGQUIT) and Ctrl-Z (SIGTSTP) still work, and
// uC_restore_terminal() restores the character either way.
API void uC_ctrl_c_off(void);
API void uC_ctrl_c_on(void);
API void uC_get_console_size(uint16_t *width, uint16_t *height);
API uint8_t eval(uint8_t *s, uint32_t *result, uint8_t radix);

// -----------------------------------------------------------------------

#endif // UC_UTILS_H

// =======================================================================
