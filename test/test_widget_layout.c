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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_view_group_rejects_oversized_view);
    RUN_TEST(test_view_group_attach_rejects_offscreen_group);
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
