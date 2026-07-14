// test_widget_layout.c  - widget clipping and containment rules
// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

#include "unity/unity.h"
#include "uC_alloc.h"
#include "uC_borders.h"
#include "uC_screen.h"
#include "uC_widgets.h"

void setUp(void)    {}
void tearDown(void) { uC_mem_purge(uC_MEM_ZONE_UI); }

// -----------------------------------------------------------------------

static uC_attribs_t test_attrs(void)
{
    uC_attribs_t attrs = { 0 };

    return attrs;
}

// -----------------------------------------------------------------------

void test_view_group_rejects_oversized_view(void)
{
    uC_attribs_t attrs = test_attrs();
    uC_widget_vg_t *vg;
    uC_widget_view_t *view;

    vg = uC_widget_vg_create(NULL, 10, 5, 0, 0, attrs);
    TEST_ASSERT_NOT_NULL(vg);

    view = uC_widget_view_create("bad", 10, 4, 1, 1, attrs, true);
    TEST_ASSERT_NOT_NULL(view);
    uC_widget_view_add_border(view, uC_BDR_SINGLE, attrs);

    TEST_ASSERT_FALSE(uC_widget_vg_add_view(vg, view, 1));
    TEST_ASSERT_EQUAL_UINT32(0, vg->views.count);

    uC_widget_vg_close(vg);
}

// -----------------------------------------------------------------------

void test_view_group_attach_rejects_offscreen_group(void)
{
    uC_attribs_t attrs = test_attrs();
    uC_screen_t *scr;
    uC_widget_vg_t *vg;

    scr = uC_scr_open(10, 5);
    TEST_ASSERT_NOT_NULL(scr);

    vg = uC_widget_vg_create(NULL, 8, 3, 5, 1, attrs);
    TEST_ASSERT_NOT_NULL(vg);

    uC_widget_vg_attach(scr, vg);
    TEST_ASSERT_NULL(vg->window.screen);
    TEST_ASSERT_EQUAL_UINT32(0, scr->view_groups.count);

    uC_widget_vg_close(vg);
    uC_scr_close(scr);
}

// -----------------------------------------------------------------------

void test_selecting_visible_widget_preserves_scroll_position(void)
{
    uC_attribs_t attrs = test_attrs();
    uint32_t selected = 0;
    uC_widget_t *widget;
    uC_widget_vg_t *vg;
    uC_widget_view_t *view;
    uint16_t i;

    vg = uC_widget_vg_create(NULL, 10, 4, 0, 0, attrs);
    TEST_ASSERT_NOT_NULL(vg);

    view = uC_widget_view_create(NULL, 10, 4, 0, 0, attrs, true);
    TEST_ASSERT_NOT_NULL(view);
    TEST_ASSERT_TRUE(uC_widget_vg_add_view(vg, view, 1));

    for (i = 0; i < 8; i++)
    {
        widget = uC_widget_check_create(&selected, i, "item",
            uC_RADIO_CHECKBOX, 10, attrs, attrs);
        TEST_ASSERT_NOT_NULL(widget);
        TEST_ASSERT_TRUE(uC_widget_view_add_widget(view, widget, 0));
    }

    // Four rows show indices 3-6; index 5 is one row above the bottom.

    uC_widget_to_view_index(view, 5);
    view->top = 3;
    view->cy  = 2;

    uC_widget_to_view_index(view, 5);

    TEST_ASSERT_EQUAL_UINT16(3, view->top);
    TEST_ASSERT_EQUAL_UINT16(2, view->cy);
    TEST_ASSERT_EQUAL_UINT16(5, uC_widget_view_index(view));

    uC_widget_vg_close(vg);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_view_group_rejects_oversized_view);
    RUN_TEST(test_view_group_attach_rejects_offscreen_group);
    RUN_TEST(test_selecting_visible_widget_preserves_scroll_position);
    return UNITY_END();
}

#else

#include <stdio.h>

void setUp(void)    {}
void tearDown(void) {}

int main(void)
{
    puts("UC_WIDGETS is disabled; widget layout test is expected to fail.");
    return 1;
}

#endif

// =======================================================================
