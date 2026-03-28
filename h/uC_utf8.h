// uC_utf8.h
// -----------------------------------------------------------------------

#ifndef UC_UTF8_H
#define UC_UTF8_H

// -----------------------------------------------------------------------

#include <stdint.h>

// --------------------------------------------------------------------------

typedef struct
{
    uint8_t len;
    union
    {
        uint8_t str[4];
        uint32_t zero;
    };
    int width;
} __attribute__((__packed__)) utf8_encode_t;

// --------------------------------------------------------------------------
// visibility hidden

utf8_encode_t *utf8_encode(int32_t cp);

// --------------------------------------------------------------------------

API int16_t uC_utf8_is_wide(uint32_t code);
API void uC_utf8_emit(uint32_t cp);
API uint8_t utf8_decode(uint32_t *cp, uint8_t *s);
API uint8_t uC_utf8_char_length(uint8_t *s);
API int16_t uC_utf8_width(uint8_t *s);
API int16_t uC_utf8_strlen(uint8_t *s);
API int16_t uC_utf8_strncmp(uint8_t *s1, uint8_t *s2, int16_t len);

// -----------------------------------------------------------------------

#endif // UC_UTF8_H

// =======================================================================
