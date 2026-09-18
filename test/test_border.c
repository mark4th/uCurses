// test_border.c  - tests for boxes and grids drawn into window buffers
// -----------------------------------------------------------------------

#include <locale.h>

#include "unity/unity.h"
#include "uC_alloc.h"
#include "uC_borders.h"

void setUp(void)    {}
void tearDown(void) { uC_mem_purge(uC_MEM_ZONE_UI); }

// -----------------------------------------------------------------------

static uint32_t code_at(uC_window_t *win, int16_t x, int16_t y)
{
    return win->buffer[(y * win->width) + x].code;
}

// -----------------------------------------------------------------------

void test_grid_vertical_reaches_both_borders(void)
{
    const uint16_t verticals[] = { 2 };
    const uint16_t horizontals[] = { 2 };
    uC_attribs_t attrs = { 0 };
    uC_window_t *win;

    win = uC_win_open(8, 7);
    TEST_ASSERT_NOT_NULL(win);

    uC_win_draw_grid(win, 1, 1, 4, 3,
        verticals, 1, horizontals, 1, uC_BDR_SINGLE, attrs);

    TEST_ASSERT_EQUAL_HEX32(0x250f, code_at(win, 1, 1));
    TEST_ASSERT_EQUAL_HEX32(0x2533, code_at(win, 3, 1));
    TEST_ASSERT_EQUAL_HEX32(0x2503, code_at(win, 3, 2));
    TEST_ASSERT_EQUAL_HEX32(0x254b, code_at(win, 3, 3));
    TEST_ASSERT_EQUAL_HEX32(0x2503, code_at(win, 3, 4));
    TEST_ASSERT_EQUAL_HEX32(0x253b, code_at(win, 3, 5));

    uC_win_close(win);
}

// -----------------------------------------------------------------------

int main(void)
{
    setlocale(LC_ALL, "C.UTF-8");

    UNITY_BEGIN();
    RUN_TEST(test_grid_vertical_reaches_both_borders);
    return UNITY_END();
}

// =======================================================================
