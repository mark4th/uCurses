// uC_parse.h    terminfo format string parsing
// -----------------------------------------------------------------------

#ifndef UC_PARSE_H
#define UC_PARSE_H

// -----------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------

#define MAX_PARAM (9)

// -----------------------------------------------------------------------
// visibility hidden

void flush(void);
void c_emit(char c1);
void uC_format(int16_t i);
void alloc_parse(void);
void uC_format(int16_t i);

// -----------------------------------------------------------------------

API void uC_terminfo_flush(void);
API void uC_parse_format(const char *f);

// -----------------------------------------------------------------------

#endif // UC_PARSE_H

// =======================================================================
