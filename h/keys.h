// keys.h
// -----------------------------------------------------------------------

#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>

// -----------------------------------------------------------------------

typedef enum
{
    K_ENT,  K_CUU1, K_CUD1, K_CUB1, K_CUF1, K_BS, K_BS2, K_DCH1,
    K_ICH1, K_HOME, K_END,  K_KNP,  K_KPP,  K_F1, K_F2,  K_F3,
    K_F4,   K_F5,   K_F6,   K_F7,   K_F8,   K_F9, K_F10, K_F11,
    K_f12
} key_index_t;

#define KEY_BUFF_SZ (32)

// -----------------------------------------------------------------------

typedef void key_handler_t(void);

// -----------------------------------------------------------------------
// visibility hidden

void read_keys(void);
int16_t match_key(void);
void init_key_handlers(void);

// -----------------------------------------------------------------------

API int8_t uC_test_keys(void);
API key_handler_t *uC_set_key_action(key_index_t index, key_handler_t *action);
API uint8_t uC_key(void);
API void uC_stuff_key(int8_t c);

// -----------------------------------------------------------------------

#endif // KEYS_H

// =======================================================================
