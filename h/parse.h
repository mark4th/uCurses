// parse.h    terminfo format string parsing
// -----------------------------------------------------------------------

#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>

// -----------------------------------------------------------------------

#define MAX_PARAM (9)

// -----------------------------------------------------------------------

typedef struct
{
    char *esc_buff;             // format string compilation output buffer
    uint16_t num_esc;           // max of 64k of compiled escape seq bytes
    uint16_t max_esc;           // for profiling
    int64_t params[MAX_PARAM];  // format string parametesr
    const char *f_str;          // pointer to next char of format string
} ti_parse_t;

// -----------------------------------------------------------------------
// visibility hidden

void flush(void);
void c_emit(char c1);
void uC_parse_format(void);
void uC_format(int16_t i);
void alloc_parse(void);
void free_parse(void);

// -----------------------------------------------------------------------

#endif // PARSE_H

// =======================================================================
