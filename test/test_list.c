// test_list.c  - tests for uC_list doubly-linked list
// -----------------------------------------------------------------------

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_list.h"
#include "uC_alloc.h"

void setUp(void)    {}
void tearDown(void) { uC_mem_purge(uC_MEM_ZONE_DEFAULT); }

// -----------------------------------------------------------------------

static uC_list_t make_list(void)
{
    uC_list_t l = { NULL, NULL, 0, uC_MEM_ZONE_DEFAULT };
    return l;
}

// -----------------------------------------------------------------------
// push / pop head (stack)

void test_stack_lifo(void)
{
    uC_list_t list = make_list();
    int a = 1, b = 2, c = 3;

    uC_stack_push(&list, &a);
    uC_stack_push(&list, &b);
    uC_stack_push(&list, &c);

    TEST_ASSERT_EQUAL_UINT32(3, list.count);
    TEST_ASSERT_EQUAL_PTR(&c, uC_stack_pop(&list));
    TEST_ASSERT_EQUAL_PTR(&b, uC_stack_pop(&list));
    TEST_ASSERT_EQUAL_PTR(&a, uC_stack_pop(&list));
    TEST_ASSERT_EQUAL_UINT32(0, list.count);
}

void test_pop_empty_returns_null(void)
{
    uC_list_t list = make_list();
    TEST_ASSERT_NULL(uC_list_pop_head(&list));
    TEST_ASSERT_NULL(uC_list_pop_tail(&list));
}

// -----------------------------------------------------------------------
// push / pop tail (queue)

void test_queue_fifo(void)
{
    uC_list_t list = make_list();
    int a = 1, b = 2, c = 3;

    uC_queue_put(&list, &a);
    uC_queue_put(&list, &b);
    uC_queue_put(&list, &c);

    TEST_ASSERT_EQUAL_UINT32(3, list.count);
    TEST_ASSERT_EQUAL_PTR(&a, uC_queue_get(&list));
    TEST_ASSERT_EQUAL_PTR(&b, uC_queue_get(&list));
    TEST_ASSERT_EQUAL_PTR(&c, uC_queue_get(&list));
    TEST_ASSERT_NULL(uC_queue_get(&list));
}

// -----------------------------------------------------------------------
// scan

void test_scan_forward(void)
{
    uC_list_t list = make_list();
    int a = 1, b = 2, c = 3;

    uC_list_push_tail(&list, &a);
    uC_list_push_tail(&list, &b);
    uC_list_push_tail(&list, &c);

    uC_list_node_t *n = uC_list_scan(&list, NULL);
    TEST_ASSERT_EQUAL_PTR(&a, n->payload);
    n = uC_list_scan(NULL, n);
    TEST_ASSERT_EQUAL_PTR(&b, n->payload);
    n = uC_list_scan(NULL, n);
    TEST_ASSERT_EQUAL_PTR(&c, n->payload);
    n = uC_list_scan(NULL, n);
    TEST_ASSERT_NULL(n);
}

void test_scan_empty_list(void)
{
    uC_list_t list = make_list();
    TEST_ASSERT_NULL(uC_list_scan(&list, NULL));
}

// -----------------------------------------------------------------------
// remove

void test_remove_middle(void)
{
    uC_list_t list = make_list();
    int a = 1, b = 2, c = 3;

    uC_list_push_tail(&list, &a);
    uC_list_push_tail(&list, &b);
    uC_list_push_tail(&list, &c);

    uC_list_remove_node(&list, &b);

    TEST_ASSERT_EQUAL_UINT32(2, list.count);
    TEST_ASSERT_EQUAL_PTR(&a, uC_list_pop_head(&list));
    TEST_ASSERT_EQUAL_PTR(&c, uC_list_pop_head(&list));
}

void test_remove_head(void)
{
    uC_list_t list = make_list();
    int a = 1, b = 2;

    uC_list_push_tail(&list, &a);
    uC_list_push_tail(&list, &b);

    uC_list_remove_node(&list, &a);

    TEST_ASSERT_EQUAL_UINT32(1, list.count);
    TEST_ASSERT_EQUAL_PTR(&b, uC_list_pop_head(&list));
}

void test_remove_tail(void)
{
    uC_list_t list = make_list();
    int a = 1, b = 2;

    uC_list_push_tail(&list, &a);
    uC_list_push_tail(&list, &b);

    uC_list_remove_node(&list, &b);

    TEST_ASSERT_EQUAL_UINT32(1, list.count);
    TEST_ASSERT_EQUAL_PTR(&a, uC_list_pop_head(&list));
}

// -----------------------------------------------------------------------
// insert_before

void test_insert_before_middle(void)
{
    uC_list_t list = make_list();
    int a = 1, b = 2, x = 99;

    uC_list_push_tail(&list, &a);
    uC_list_push_tail(&list, &b);

    // find the node for b and insert x before it
    uC_list_node_t *n = uC_list_scan(&list, NULL);  // &a
    n = uC_list_scan(NULL, n);                       // &b
    uC_list_insert_before(n, &x);

    TEST_ASSERT_EQUAL_UINT32(3, list.count);
    TEST_ASSERT_EQUAL_PTR(&a, uC_list_pop_head(&list));
    TEST_ASSERT_EQUAL_PTR(&x, uC_list_pop_head(&list));
    TEST_ASSERT_EQUAL_PTR(&b, uC_list_pop_head(&list));
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_stack_lifo);
    RUN_TEST(test_pop_empty_returns_null);
    RUN_TEST(test_queue_fifo);
    RUN_TEST(test_scan_forward);
    RUN_TEST(test_scan_empty_list);
    RUN_TEST(test_remove_middle);
    RUN_TEST(test_remove_head);
    RUN_TEST(test_remove_tail);
    RUN_TEST(test_insert_before_middle);
    return UNITY_END();
}

// =======================================================================
