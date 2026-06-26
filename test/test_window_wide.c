// test_window_wide.c  - tests for wide-character window buffer handling
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE 700

#include <locale.h>
#include <wchar.h>

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_alloc.h"
#include "uC_window.h"

void setUp(void)    {}
void tearDown(void) { uC_mem_purge(uC_MEM_ZONE_UI); }

// -----------------------------------------------------------------------

#define WIDE_A 0x4e00      // 一
#define WIDE_B 0x4e8c      // 二

static void require_wide_locale(void)
{
    if ((wcwidth((wchar_t)WIDE_A) != 2) ||
        (wcwidth((wchar_t)WIDE_B) != 2))
    {
        TEST_IGNORE_MESSAGE("test locale does not treat CJK codepoints as wide");
    }
}

// -----------------------------------------------------------------------

void test_overwriting_wide_owner_blanks_old_continuation(void)
{
    uC_window_t *win;

    require_wide_locale();

    win = uC_win_open(4, 1);
    TEST_ASSERT_NOT_NULL(win);

    uC_win_emit(win, WIDE_A);
    uC_win_cup(win, 0, 0);
    uC_win_emit(win, 'A');

    TEST_ASSERT_EQUAL_UINT32('A', win->buffer[0].code);
    TEST_ASSERT_EQUAL_UINT32(' ', win->buffer[1].code);

    uC_win_close(win);
}

// -----------------------------------------------------------------------

void test_overwriting_wide_continuation_blanks_old_owner(void)
{
    uC_window_t *win;

    require_wide_locale();

    win = uC_win_open(5, 1);
    TEST_ASSERT_NOT_NULL(win);

    uC_win_emit(win, WIDE_A);
    uC_win_cup(win, 1, 0);
    uC_win_emit(win, WIDE_B);

    TEST_ASSERT_EQUAL_UINT32(' ', win->buffer[0].code);
    TEST_ASSERT_EQUAL_UINT32(WIDE_B, win->buffer[1].code);
    TEST_ASSERT_EQUAL_UINT32(DEADC0DE, win->buffer[2].code);

    uC_win_close(win);
}

// -----------------------------------------------------------------------

void test_single_char_over_continuation_blanks_old_owner(void)
{
    uC_window_t *win;

    require_wide_locale();

    win = uC_win_open(4, 1);
    TEST_ASSERT_NOT_NULL(win);

    uC_win_emit(win, WIDE_A);
    uC_win_cup(win, 1, 0);
    uC_win_emit(win, 'B');

    TEST_ASSERT_EQUAL_UINT32(' ', win->buffer[0].code);
    TEST_ASSERT_EQUAL_UINT32('B', win->buffer[1].code);
    TEST_ASSERT_NOT_EQUAL(DEADC0DE, win->buffer[0].code);

    uC_win_close(win);
}

// -----------------------------------------------------------------------

void test_scroll_left_blanks_clipped_continuation(void)
{
    uC_window_t *win;

    require_wide_locale();

    win = uC_win_open(4, 1);
    TEST_ASSERT_NOT_NULL(win);

    uC_win_emit(win, WIDE_A);
    uC_win_scroll_lt(win);

    TEST_ASSERT_EQUAL_UINT32(' ', win->buffer[0].code);

    uC_win_close(win);
}

// -----------------------------------------------------------------------

void test_scroll_left_preserves_complete_wide_glyph(void)
{
    uC_window_t *win;

    require_wide_locale();

    win = uC_win_open(5, 1);
    TEST_ASSERT_NOT_NULL(win);

    uC_win_emit(win, 'A');
    uC_win_emit(win, WIDE_A);
    uC_win_scroll_lt(win);

    TEST_ASSERT_EQUAL_UINT32(WIDE_A, win->buffer[0].code);
    TEST_ASSERT_EQUAL_UINT32(DEADC0DE, win->buffer[1].code);

    uC_win_close(win);
}

// -----------------------------------------------------------------------

void test_scroll_right_blanks_clipped_wide_owner(void)
{
    uC_window_t *win;

    require_wide_locale();

    win = uC_win_open(4, 1);
    TEST_ASSERT_NOT_NULL(win);

    uC_win_cup(win, 2, 0);
    uC_win_emit(win, WIDE_A);
    uC_win_scroll_rt(win);

    TEST_ASSERT_EQUAL_UINT32(' ', win->buffer[3].code);

    uC_win_close(win);
}

// -----------------------------------------------------------------------

int main(void)
{
    setlocale(LC_ALL, "C.UTF-8");

    UNITY_BEGIN();
    RUN_TEST(test_overwriting_wide_owner_blanks_old_continuation);
    RUN_TEST(test_overwriting_wide_continuation_blanks_old_owner);
    RUN_TEST(test_single_char_over_continuation_blanks_old_owner);
    RUN_TEST(test_scroll_left_blanks_clipped_continuation);
    RUN_TEST(test_scroll_left_preserves_complete_wide_glyph);
    RUN_TEST(test_scroll_right_blanks_clipped_wide_owner);
    return UNITY_END();
}

// =======================================================================
