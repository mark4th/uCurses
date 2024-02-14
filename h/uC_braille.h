// uC_braille.h
// -----------------------------------------------------------------------

#ifndef UC_BRAILLE_H
#define UC_BRAILLE_H

// -----------------------------------------------------------------------

API int16_t uC_braille_xlat(uint8_t chr);
API void uC_braille_8(uC_window_t *win, uint16_t *braille_data, uint8_t *map,
    uint16_t width);
API void uC_braille_1(uC_window_t *win, uint16_t *braille_data, uint8_t *map,
    uint16_t width, uint16_t height);
API void uC_draw_braille(uC_window_t *win, uint16_t *braille_data);

// -----------------------------------------------------------------------

#endif // UC_BRAILLE_H

// =======================================================================
