// utils.h
// -----------------------------------------------------------------------

#ifndef UTILS_H
#define UTILS_H

// -----------------------------------------------------------------------

#define FNV_PRIME (0x01000193)
#define FNV_BASIS (0x811c9dc5)

// -----------------------------------------------------------------------
// visibility hidden

int32_t fnv_hash(char *s);

__attribute__((noreturn)) void xabort(char *msg);

// -----------------------------------------------------------------------

API void uC_noop(void);
API void uC_clock_sleep(int32_t when);
API void uC_restore_terminal(void);
API void uC_init_terminal(void);

// -----------------------------------------------------------------------

#endif // UTILS_H

// =======================================================================
