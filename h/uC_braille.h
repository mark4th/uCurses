// uC_braille.h
// -----------------------------------------------------------------------

#ifndef BRAILLE_H
#define BRAILLE_H

// -----------------------------------------------------------------------

API int16_t uC_braille_xlat(uint8_t chr);
API void uC_braille_8(window_t *win, uint16_t *braille_data, uint8_t *map,
    uint16_t width);
API void uC_braille_1(window_t *win, uint16_t *braille_data, uint8_t *map,
    uint16_t width, uint16_t height);
API void uC_draw_braille(window_t *win, uint16_t *braille_data);

// -----------------------------------------------------------------------

#endif // BRAILLE_H

// =======================================================================
