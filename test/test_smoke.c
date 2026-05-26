// test_smoke.c  verify Unity framework is wired up correctly
// -----------------------------------------------------------------------

#include "unity/unity.h"

void setUp(void)    {}
void tearDown(void) {}

// -----------------------------------------------------------------------

void test_framework_is_operational(void)
{
    TEST_ASSERT_EQUAL_INT(1, 1);
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_framework_is_operational);
    return UNITY_END();
}

// =======================================================================
