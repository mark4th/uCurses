// test_utf8.c  - tests for UTF-8 utility functions
// -----------------------------------------------------------------------

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_utf8.h"

void setUp(void)    {}
void tearDown(void) {}

// -----------------------------------------------------------------------
// uC_utf8_char_length

void test_char_length_ascii(void)
{
    TEST_ASSERT_EQUAL_UINT8(1, uC_utf8_char_length((uint8_t *)"A"));
}

void test_char_length_2byte(void)
{
    uint8_t s[] = { 0xc3, 0xa9, 0 };   // é  U+00E9
    TEST_ASSERT_EQUAL_UINT8(2, uC_utf8_char_length(s));
}

void test_char_length_3byte(void)
{
    uint8_t s[] = { 0xe2, 0x82, 0xac, 0 };   // €  U+20AC
    TEST_ASSERT_EQUAL_UINT8(3, uC_utf8_char_length(s));
}

void test_char_length_4byte(void)
{
    uint8_t s[] = { 0xf0, 0x9d, 0x84, 0x9e, 0 };   // 𝄞  U+1D11E
    TEST_ASSERT_EQUAL_UINT8(4, uC_utf8_char_length(s));
}

// -----------------------------------------------------------------------
// uC_utf8_decode

void test_decode_ascii(void)
{
    uint32_t cp;
    TEST_ASSERT_EQUAL_UINT8(1, uC_utf8_decode(&cp, (uint8_t *)"A"));
    TEST_ASSERT_EQUAL_UINT32('A', cp);
}

void test_decode_2byte(void)
{
    uint32_t cp;
    uint8_t s[] = { 0xc3, 0xa9, 0 };   // é  U+00E9
    TEST_ASSERT_EQUAL_UINT8(2, uC_utf8_decode(&cp, s));
    TEST_ASSERT_EQUAL_UINT32(0x00e9, cp);
}

void test_decode_3byte(void)
{
    uint32_t cp;
    uint8_t s[] = { 0xe2, 0x82, 0xac, 0 };   // €  U+20AC
    TEST_ASSERT_EQUAL_UINT8(3, uC_utf8_decode(&cp, s));
    TEST_ASSERT_EQUAL_UINT32(0x20ac, cp);
}

void test_decode_4byte(void)
{
    uint32_t cp;
    uint8_t s[] = { 0xf0, 0x9d, 0x84, 0x9e, 0 };   // 𝄞  U+1D11E
    TEST_ASSERT_EQUAL_UINT8(4, uC_utf8_decode(&cp, s));
    TEST_ASSERT_EQUAL_UINT32(0x1d11e, cp);
}

// -----------------------------------------------------------------------
// uC_utf8_strlen

void test_strlen_empty(void)
{
    TEST_ASSERT_EQUAL_INT16(0, uC_utf8_strlen((uint8_t *)""));
}

void test_strlen_ascii(void)
{
    TEST_ASSERT_EQUAL_INT16(5, uC_utf8_strlen((uint8_t *)"hello"));
}

void test_strlen_2byte_chars(void)
{
    // "éñ"  =  0xc3 0xa9  0xc3 0xb1  — 2 chars, 4 bytes
    uint8_t s[] = { 0xc3, 0xa9, 0xc3, 0xb1, 0 };
    TEST_ASSERT_EQUAL_INT16(2, uC_utf8_strlen(s));
}

void test_strlen_3byte_chars(void)
{
    // "€™"  =  0xe2 0x82 0xac  0xe2 0x84 0xa2  — 2 chars, 6 bytes
    uint8_t s[] = { 0xe2, 0x82, 0xac, 0xe2, 0x84, 0xa2, 0 };
    TEST_ASSERT_EQUAL_INT16(2, uC_utf8_strlen(s));
}

void test_strlen_mixed(void)
{
    // "hé"  =  'h'  0xc3 0xa9  — 2 chars, 3 bytes
    uint8_t s[] = { 'h', 0xc3, 0xa9, 0 };
    TEST_ASSERT_EQUAL_INT16(2, uC_utf8_strlen(s));
}

// -----------------------------------------------------------------------
// uC_utf8_strncmp

void test_strncmp_equal(void)
{
    TEST_ASSERT_EQUAL_INT16(
        0, uC_utf8_strncmp((uint8_t *)"hello", (uint8_t *)"hello", 5));
}

void test_strncmp_differ(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0, uC_utf8_strncmp((uint8_t *)"hello", (uint8_t *)"world", 5));
}

void test_strncmp_partial_match(void)
{
    // first 3 chars are the same
    TEST_ASSERT_EQUAL_INT16(
        0, uC_utf8_strncmp((uint8_t *)"hello", (uint8_t *)"helxx", 3));
}

void test_strncmp_zero_len(void)
{
    TEST_ASSERT_EQUAL_INT16(
        0, uC_utf8_strncmp((uint8_t *)"hello", (uint8_t *)"world", 0));
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_char_length_ascii);
    RUN_TEST(test_char_length_2byte);
    RUN_TEST(test_char_length_3byte);
    RUN_TEST(test_char_length_4byte);
    RUN_TEST(test_decode_ascii);
    RUN_TEST(test_decode_2byte);
    RUN_TEST(test_decode_3byte);
    RUN_TEST(test_decode_4byte);
    RUN_TEST(test_strlen_empty);
    RUN_TEST(test_strlen_ascii);
    RUN_TEST(test_strlen_2byte_chars);
    RUN_TEST(test_strlen_3byte_chars);
    RUN_TEST(test_strlen_mixed);
    RUN_TEST(test_strncmp_equal);
    RUN_TEST(test_strncmp_differ);
    RUN_TEST(test_strncmp_partial_match);
    RUN_TEST(test_strncmp_zero_len);
    return UNITY_END();
}

// =======================================================================
