// test_fnv.c  - tests for uC_fnv_hash() and eval()
// -----------------------------------------------------------------------

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_utils.h"

void setUp(void)    {}
void tearDown(void) {}

// -----------------------------------------------------------------------
// uC_fnv_hash

void test_fnv_null_returns_zero(void)
{
    TEST_ASSERT_EQUAL_INT32(0, uC_fnv_hash(NULL));
}

void test_fnv_consistent(void)
{
    TEST_ASSERT_EQUAL_INT32(
        uC_fnv_hash((uint8_t *)"hello"),
        uC_fnv_hash((uint8_t *)"hello"));
}

void test_fnv_different_strings_differ(void)
{
    TEST_ASSERT_NOT_EQUAL(
        uC_fnv_hash((uint8_t *)"hello"),
        uC_fnv_hash((uint8_t *)"world"));
}

void test_fnv_case_sensitive(void)
{
    TEST_ASSERT_NOT_EQUAL(
        uC_fnv_hash((uint8_t *)"Hello"),
        uC_fnv_hash((uint8_t *)"hello"));
}

void test_fnv_single_char(void)
{
    TEST_ASSERT_NOT_EQUAL(
        uC_fnv_hash((uint8_t *)"a"),
        uC_fnv_hash((uint8_t *)"b"));
}

// -----------------------------------------------------------------------
// eval

void test_eval_decimal(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(1, eval((uint8_t *)"12345", &result, 10));
    TEST_ASSERT_EQUAL_UINT32(12345, result);
}

void test_eval_hex_uppercase(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(1, eval((uint8_t *)"FF", &result, 16));
    TEST_ASSERT_EQUAL_UINT32(255, result);
}

void test_eval_hex_lowercase(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(1, eval((uint8_t *)"ff", &result, 16));
    TEST_ASSERT_EQUAL_UINT32(255, result);
}

void test_eval_binary(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(1, eval((uint8_t *)"1010", &result, 2));
    TEST_ASSERT_EQUAL_UINT32(10, result);
}

void test_eval_octal(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(1, eval((uint8_t *)"77", &result, 8));
    TEST_ASSERT_EQUAL_UINT32(63, result);
}

void test_eval_zero(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(1, eval((uint8_t *)"0", &result, 10));
    TEST_ASSERT_EQUAL_UINT32(0, result);
}

void test_eval_invalid_char_fails(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(0, eval((uint8_t *)"12G3", &result, 10));
}

void test_eval_digit_out_of_radix_fails(void)
{
    uint32_t result;
    TEST_ASSERT_EQUAL_UINT8(0, eval((uint8_t *)"9", &result, 8));
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_fnv_null_returns_zero);
    RUN_TEST(test_fnv_consistent);
    RUN_TEST(test_fnv_different_strings_differ);
    RUN_TEST(test_fnv_case_sensitive);
    RUN_TEST(test_fnv_single_char);
    RUN_TEST(test_eval_decimal);
    RUN_TEST(test_eval_hex_uppercase);
    RUN_TEST(test_eval_hex_lowercase);
    RUN_TEST(test_eval_binary);
    RUN_TEST(test_eval_octal);
    RUN_TEST(test_eval_zero);
    RUN_TEST(test_eval_invalid_char_fails);
    RUN_TEST(test_eval_digit_out_of_radix_fails);
    return UNITY_END();
}

// =======================================================================
