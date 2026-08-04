// test_key_sm.c  - focused tests for the keyboard state machine sm_parse()
// -----------------------------------------------------------------------
// uC_key_sm.c is compiled straight into this executable (it is a hidden,
// non-API symbol, so it is not exported from the shared library).  Its only
// external is ti_vars, which we define and drive ourselves; key_mods lives
// in that same translation unit, so we can read it back directly.

#include <string.h>

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------
// backing store for the sequence under test.  uC_key_sm.c references ti_vars
// as an extern pointer; point it at our own struct.

static ti_vars_t tv;
ti_vars_t *ti_vars = &tv;

extern uint8_t key_mods;        // set by sm_parse(), same TU we link in

void setUp(void)
{
    memset(&tv, 0, sizeof(tv));
    key_mods = 0;
}

void tearDown(void) {}

// -----------------------------------------------------------------------
// load a raw byte sequence into keybuff and run the state machine

static int16_t parse(const char *seq, uint8_t len)
{
    memcpy(tv.keybuff, seq, len);
    tv.num_k = len;
    return sm_parse();
}

// =======================================================================
// the point of the exercise: a long digit run must not overflow the int32
// param accumulator (signed overflow is UB).  the clamp holds param >= the
// cap so it can never alias a valid tilde/modifier value.

void test_csi_param_overflow_is_clamped_not_wrapped(void)
{
    // 26 nines then '~' — without the clamp param[0]*10 overflows int32
    int16_t r = parse("\x1b[99999999999999999999999999~", 29);
    TEST_ASSERT_EQUAL_INT16(SM_UNHANDLED, r);   // not a valid tilde number
}

void test_csi_modifier_overflow_is_clamped(void)
{
    // huge modifier field: final 'A' must still decode; key_mods is just the
    // low 3 bits of (clamped - 1), the guarantee is only that we don't overflow
    int16_t r = parse("\x1b[1;99999999999999999999A", 25);
    TEST_ASSERT_EQUAL_INT16(K_CUU1, r);
}

void test_csi_all_digits_no_final_is_incomplete(void)
{
    // runs off the end of the buffer mid-number: no final byte, no UB
    int16_t r = parse("\x1b[12345678901234567890", 22);
    TEST_ASSERT_EQUAL_INT16(SM_UNHANDLED, r);
}

// =======================================================================
// plain cursor keys (no modifiers)

void test_csi_up(void)
{
    TEST_ASSERT_EQUAL_INT16(K_CUU1, parse("\x1b[A", 3));
    TEST_ASSERT_EQUAL_UINT8(0, key_mods);
}

void test_ss3_up_application_mode(void)
{
    TEST_ASSERT_EQUAL_INT16(K_CUU1, parse("\x1bOA", 3));
}

void test_ss3_f1(void)
{
    TEST_ASSERT_EQUAL_INT16(K_F1, parse("\x1bOP", 3));
}

// -----------------------------------------------------------------------
// modified cursor keys: ESC [ 1 ; <mod> <final>, mod = mask + 1

void test_csi_ctrl_right(void)
{
    TEST_ASSERT_EQUAL_INT16(K_CUF1, parse("\x1b[1;5C", 6));  // 5-1 = 4 = ctrl
    TEST_ASSERT_EQUAL_UINT8(KMOD_CTRL, key_mods);
}

void test_csi_shift_right_has_own_slot(void)
{
    TEST_ASSERT_EQUAL_INT16(K_SRIT, parse("\x1b[1;2C", 6));  // 2-1 = 1 = shift
    TEST_ASSERT_EQUAL_UINT8(KMOD_SHIFT, key_mods);
}

void test_csi_shift_left_has_own_slot(void)
{
    TEST_ASSERT_EQUAL_INT16(K_SLFT, parse("\x1b[1;2D", 6));
}

// -----------------------------------------------------------------------
// tilde ("~") numbered keys

void test_csi_delete(void)
{
    TEST_ASSERT_EQUAL_INT16(K_DCH1, parse("\x1b[3~", 4));
}

void test_csi_f5(void)
{
    TEST_ASSERT_EQUAL_INT16(K_F5, parse("\x1b[15~", 5));
}

void test_csi_unknown_tilde_is_unhandled(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_UNHANDLED, parse("\x1b[99~", 5));
}

// -----------------------------------------------------------------------
// direct / modifier-prefixed forms

void test_bare_esc_is_direct(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_DIRECT, parse("\x1b", 1));
}

void test_esc_esc_reports_single_esc(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_DIRECT, parse("\x1b\x1b", 2));
    TEST_ASSERT_EQUAL_INT16(1, tv.num_k);
}

void test_alt_char(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_DIRECT, parse("\x1b" "b", 2));
    TEST_ASSERT_EQUAL_UINT8(KMOD_ALT, key_mods);
    TEST_ASSERT_EQUAL_UINT8('b', tv.keybuff[0]);    // rewritten in place
    TEST_ASSERT_EQUAL_INT16(1, tv.num_k);
}

// -----------------------------------------------------------------------
// mouse introducers are not keys — left for the caller's mouse parser

void test_x10_mouse_is_unhandled(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_UNHANDLED, parse("\x1b[M", 3));
}

void test_sgr_mouse_is_unhandled(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_UNHANDLED, parse("\x1b[<0;1;1M", 9));
}

// =======================================================================
// drive sm_run() through an independent array source (bytes one at a time,
// -1 when exhausted) — the streaming pull path, decoupled from keybuff.  the
// sequences here are the bytes AFTER the leading ESC (already "consumed").

typedef struct
{
    const uint8_t *p;
    uint8_t        len;
    uint8_t        i;
} arr_src_t;

static int arr_source(void *ctx, int timeout_ms)
{
    arr_src_t *s = ctx;

    (void)timeout_ms;
    return (s->i < s->len) ? s->p[s->i++] : -1;
}

static int16_t run_arr(const char *post_esc, uint8_t len)
{
    arr_src_t s = { (const uint8_t *)post_esc, len, 0 };

    tv.keybuff[0] = 0x1b;               // as if the ESC was already read
    tv.num_k      = 1;
    return sm_run(arr_source, &s);
}

void test_stream_up(void)
{
    TEST_ASSERT_EQUAL_INT16(K_CUU1, run_arr("[A", 2));
    TEST_ASSERT_EQUAL_UINT8(0, key_mods);
}

void test_stream_ctrl_right(void)
{
    TEST_ASSERT_EQUAL_INT16(K_CUF1, run_arr("[1;5C", 5));
    TEST_ASSERT_EQUAL_UINT8(KMOD_CTRL, key_mods);
}

void test_stream_bare_esc_source_empty(void)
{
    // source yields nothing after ESC -> bare ESC (the timeout branch)
    TEST_ASSERT_EQUAL_INT16(SM_DIRECT, run_arr("", 0));
}

void test_stream_incomplete_csi(void)
{
    // digit then the stream ends mid-sequence -> incomplete, not UB
    TEST_ASSERT_EQUAL_INT16(SM_UNHANDLED, run_arr("[3", 2));
}

void test_stream_alt_char(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_DIRECT, run_arr("b", 1));
    TEST_ASSERT_EQUAL_UINT8(KMOD_ALT, key_mods);
    TEST_ASSERT_EQUAL_UINT8('b', tv.keybuff[0]);
}

// =======================================================================

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_csi_param_overflow_is_clamped_not_wrapped);
    RUN_TEST(test_csi_modifier_overflow_is_clamped);
    RUN_TEST(test_csi_all_digits_no_final_is_incomplete);
    RUN_TEST(test_csi_up);
    RUN_TEST(test_ss3_up_application_mode);
    RUN_TEST(test_ss3_f1);
    RUN_TEST(test_csi_ctrl_right);
    RUN_TEST(test_csi_shift_right_has_own_slot);
    RUN_TEST(test_csi_shift_left_has_own_slot);
    RUN_TEST(test_csi_delete);
    RUN_TEST(test_csi_f5);
    RUN_TEST(test_csi_unknown_tilde_is_unhandled);
    RUN_TEST(test_bare_esc_is_direct);
    RUN_TEST(test_esc_esc_reports_single_esc);
    RUN_TEST(test_alt_char);
    RUN_TEST(test_x10_mouse_is_unhandled);
    RUN_TEST(test_sgr_mouse_is_unhandled);
    RUN_TEST(test_stream_up);
    RUN_TEST(test_stream_ctrl_right);
    RUN_TEST(test_stream_bare_esc_source_empty);
    RUN_TEST(test_stream_incomplete_csi);
    RUN_TEST(test_stream_alt_char);
    return UNITY_END();
}

// =======================================================================
