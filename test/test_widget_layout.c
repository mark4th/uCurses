// test_widget_layout.c  - widget clipping and containment rules
// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

#include "unity/unity.h"
#include "uC_alloc.h"
#include "uC_borders.h"
#include "uC_screen.h"
#include "uC_list.h"
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

void test_widget_user_pointer_round_trip(void)
{
    uC_attribs_t attrs = test_attrs();
    uC_widget_t *widget;
    uint32_t payload = 0x12345678;

    widget = uC_widget_button_create(NULL, "item", '\0', 10,
        attrs, attrs);
    TEST_ASSERT_NOT_NULL(widget);
    TEST_ASSERT_NULL(widget->user);

    widget->user = &payload;
    TEST_ASSERT_EQUAL_PTR(&payload, widget->user);
    TEST_ASSERT_EQUAL_HEX32(payload, *(uint32_t *)widget->user);

    uC_widget_close_widget(widget);
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

void test_view_index_no_scroll(void)
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

// -----------------------------------------------------------------------
// build a view of `count` checkboxes over bits 0.. of one word

static uC_widget_view_t *check_list(uC_widget_vg_t **out_vg,
    uint32_t *selected, uint16_t count)
{
    uC_attribs_t attrs = test_attrs();
    uC_widget_view_t *view;
    uC_widget_t *widget;
    uint16_t i;

    *out_vg = uC_widget_vg_create(NULL, 12, 8, 0, 0, attrs);
    TEST_ASSERT_NOT_NULL(*out_vg);

    view = uC_widget_view_create(NULL, 12, 8, 0, 0, attrs, true);
    TEST_ASSERT_NOT_NULL(view);
    TEST_ASSERT_TRUE(uC_widget_vg_add_view(*out_vg, view, 1));

    for (i = 0; i < count; i++)
    {
        widget = uC_widget_check_create(selected, i, "item",
            uC_RADIO_CHECKBOX, 12, attrs, attrs);
        TEST_ASSERT_NOT_NULL(widget);
        TEST_ASSERT_TRUE(uC_widget_view_add_widget(view, widget, 0));
    }

    return view;
}

// -----------------------------------------------------------------------

static uC_widget_t *nth_widget(uC_widget_view_t *view, uint16_t index)
{
    uC_list_node_t *n = uC_list_scan(&view->widgets, NULL);
    uint16_t i = 0;

    while ((n != NULL) && (i != index))
    {
        n = uC_list_scan(NULL, n);
        i++;
    }

    TEST_ASSERT_NOT_NULL(n);

    return (uC_widget_t *)n->payload;
}

// -----------------------------------------------------------------------
// the count comes off the BITS, so a caller that writes them itself is
// still counted correctly

void test_view_checks_used_counts_the_bits(void)
{
    uC_widget_vg_t *vg;
    uint32_t selected = 0;
    uC_widget_view_t *view = check_list(&vg, &selected, 4);

    TEST_ASSERT_EQUAL_UINT16(0, uC_widget_view_checks_used(view));

    selected = 0x5;                     // bits 0 and 2, set behind us

    TEST_ASSERT_EQUAL_UINT16(2, uC_widget_view_checks_used(view));

    selected = 0xf;

    TEST_ASSERT_EQUAL_UINT16(4, uC_widget_view_checks_used(view));

    uC_widget_vg_close(vg);
}

// -----------------------------------------------------------------------
// a view starts unbounded, and an unbounded view refuses nothing

void test_check_max_defaults_to_unlimited(void)
{
    uC_widget_vg_t *vg;
    uint32_t selected = 0xf;
    uC_widget_view_t *view = check_list(&vg, &selected, 4);

    TEST_ASSERT_EQUAL_UINT16(0, view->check_max);
    TEST_ASSERT_FALSE(uC_widget_check_refused(nth_widget(view, 0)));

    selected = 0;

    TEST_ASSERT_FALSE(uC_widget_check_refused(nth_widget(view, 0)));

    uC_widget_vg_close(vg);
}

// -----------------------------------------------------------------------
// at the cap the unticked boxes refuse and the ticked ones do not

void test_check_max_refuses_only_the_unticked(void)
{
    uC_widget_vg_t *vg;
    uint32_t selected = 0;
    uC_widget_view_t *view = check_list(&vg, &selected, 4);

    TEST_ASSERT_TRUE(uC_widget_view_set_check_max(view, 2));
    TEST_ASSERT_EQUAL_UINT16(2, view->check_max);

    // under the cap, nothing is refused

    selected = 0x1;

    TEST_ASSERT_FALSE(uC_widget_check_refused(nth_widget(view, 1)));

    // at it, an unticked box is refused and a ticked one is not - there
    // has to be a way back out

    selected = 0x3;

    TEST_ASSERT_TRUE(uC_widget_check_refused(nth_widget(view, 2)));
    TEST_ASSERT_TRUE(uC_widget_check_refused(nth_widget(view, 3)));
    TEST_ASSERT_FALSE(uC_widget_check_refused(nth_widget(view, 0)));
    TEST_ASSERT_FALSE(uC_widget_check_refused(nth_widget(view, 1)));

    // untick one and the room comes back

    selected = 0x1;

    TEST_ASSERT_FALSE(uC_widget_check_refused(nth_widget(view, 2)));

    // over the cap - a caller may set more bits than it asked for - is
    // still refused rather than wrapping round to allowed

    selected = 0xd;

    TEST_ASSERT_TRUE(uC_widget_check_refused(nth_widget(view, 1)));

    uC_widget_vg_close(vg);
}

// -----------------------------------------------------------------------

void test_check_max_rejects_a_null_view(void)
{
    TEST_ASSERT_FALSE(uC_widget_view_set_check_max(NULL, 3));
    TEST_ASSERT_EQUAL_UINT16(0, uC_widget_view_checks_used(NULL));
    TEST_ASSERT_FALSE(uC_widget_check_refused(NULL));
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_widget_user_pointer_round_trip);
    RUN_TEST(test_view_group_rejects_oversized_view);
    RUN_TEST(test_view_group_attach_rejects_offscreen_group);
    RUN_TEST(test_view_index_no_scroll);
    RUN_TEST(test_view_checks_used_counts_the_bits);
    RUN_TEST(test_check_max_defaults_to_unlimited);
    RUN_TEST(test_check_max_refuses_only_the_unticked);
    RUN_TEST(test_check_max_rejects_a_null_view);
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
