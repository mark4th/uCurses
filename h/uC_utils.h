// utils.h
// -----------------------------------------------------------------------

#ifndef UC_UTILS_H
#define UC_UTILS_H

#include <stdbool.h>

// -----------------------------------------------------------------------

#define FNV_PRIME (0x01000193)
#define FNV_BASIS (0x811c9dc5)

// -----------------------------------------------------------------------
// visibility hidden

API int32_t fnv_hash(char *s);
void uC_assert(bool f, char *file, int line, char *msg);

__attribute__((noreturn)) void xabort(char *msg);

// -----------------------------------------------------------------------

#define uC_ASSERT(f, msg) uC_assert(f, __FILE__, __LINE__, msg)

// -----------------------------------------------------------------------

API void uC_noop(void);
API void uC_clock_sleep(int32_t when);
API void uC_restore_terminal(void);
API void uC_init_terminal(void);

// -----------------------------------------------------------------------

#endif // UC_UTILS_H

// =======================================================================
