// utf8.h
// -----------------------------------------------------------------------

#ifndef UC_UTF8_H
#define UC_UTF8_H

// --------------------------------------------------------------------------

#include <stdint.h>

// --------------------------------------------------------------------------

typedef struct
{
    int8_t len;
    int8_t str[4];
    int width;
} utf8_encode_t;

// --------------------------------------------------------------------------
// visibility hidden

utf8_encode_t *utf8_encode(int32_t cp);
int8_t utf8_decode(int32_t *cp, char *s);

// --------------------------------------------------------------------------

API int16_t uC_utf8_is_wide(int32_t code);
API void uC_utf8_emit(int32_t cp);
API int8_t utf8_decode(int32_t *cp, char *s);
API uint8_t uC_utf8_char_length(char *s);
API int16_t uC_utf8_width(char *s);
API int16_t uC_utf8_strlen(char *s);
API int16_t uC_utf8_strncmp(char *s1, char *s2, int16_t len);

// --------------------------------------------------------------------------

#endif // UC_UTF8_H

// =======================================================================
