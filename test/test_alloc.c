// test_alloc.c  - tests for uC_alloc tracked allocation helpers
// -----------------------------------------------------------------------

#include <string.h>

#include "unity/unity.h"
#include "uC_alloc.h"

#define TEST_ZONE uC_MEM_ZONE_USER5

void setUp(void)    {}
void tearDown(void) { uC_mem_purge(TEST_ZONE); }

// -----------------------------------------------------------------------

void test_realloc_null_allocates_tracked_buffer(void)
{
    char *p;

    p = uC_realloc(TEST_ZONE, NULL, 16);

    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_UINT32(1, uC_zone_query(TEST_ZONE));
}

// -----------------------------------------------------------------------

void test_realloc_preserves_contents_and_tracking(void)
{
    char *p;
    char *q;

    p = uC_alloc(TEST_ZONE, 8);
    TEST_ASSERT_NOT_NULL(p);
    strcpy(p, "abc");

    q = uC_realloc(TEST_ZONE, p, 32);

    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("abc", q);
    TEST_ASSERT_EQUAL_UINT32(1, uC_zone_query(TEST_ZONE));
}

// -----------------------------------------------------------------------

void test_realloc_zero_frees_tracked_buffer(void)
{
    char *p;
    char *q;

    p = uC_alloc(TEST_ZONE, 8);
    TEST_ASSERT_NOT_NULL(p);

    q = uC_realloc(TEST_ZONE, p, 0);

    TEST_ASSERT_NULL(q);
    TEST_ASSERT_EQUAL_UINT32(0, uC_zone_query(TEST_ZONE));
}

// -----------------------------------------------------------------------

void test_free_unknown_pointer_is_ignored(void)
{
    int stack_value = 7;

    uC_free(TEST_ZONE, &stack_value);

    TEST_ASSERT_EQUAL_UINT32(0, uC_zone_query(TEST_ZONE));
}

// -----------------------------------------------------------------------

void test_realloc_unknown_pointer_returns_null(void)
{
    int stack_value = 7;

    TEST_ASSERT_NULL(uC_realloc(TEST_ZONE, &stack_value, 16));
    TEST_ASSERT_EQUAL_UINT32(0, uC_zone_query(TEST_ZONE));
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_realloc_null_allocates_tracked_buffer);
    RUN_TEST(test_realloc_preserves_contents_and_tracking);
    RUN_TEST(test_realloc_zero_frees_tracked_buffer);
    RUN_TEST(test_free_unknown_pointer_is_ignored);
    RUN_TEST(test_realloc_unknown_pointer_returns_null);
    return UNITY_END();
}

// =======================================================================
