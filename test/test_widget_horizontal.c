// test_widget_horizontal.c - horizontal scrollable widget views
// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

#include <string.h>

#include "unity/unity.h"
#include "uC_alloc.h"
#include "uC_screen.h"
#include "uC_widgets.h"

extern widget_state_t widget_state;

void setUp(void)
{
    memset(&widget_state, 0, sizeof(widget_state));
    uC_widget_reset_sequence();
}

void tearDown(void)
{
    memset(&widget_state, 0, sizeof(widget_state));
    uC_mem_purge(uC_MEM_ZONE_UI);
}

// -----------------------------------------------------------------------

static uC_attribs_t test_attrs(void)
{
    uC_attribs_t attrs = { 0 };

    return attrs;
}

// -----------------------------------------------------------------------

static uC_widget_view_t *horizontal_buttons(uint16_t count,
    uint16_t view_width, uint16_t widget_width)
{
    uC_attribs_t attrs = test_attrs();
    uC_widget_view_t *view;
    uC_widget_t *widget;
    uint16_t i;

    view = uC_widget_view_create(NULL, view_width, 1, 0, 0,
        attrs, true);
    TEST_ASSERT_NOT_NULL(view);
    TEST_ASSERT_TRUE(uC_widget_view_set_orientation(view,
        uC_VIEW_HORIZONTAL));

    for (i = 0; i < count; i++)
    {
        widget = uC_widget_button_create(NULL, "tab", '\0',
            widget_width, attrs, attrs);
        TEST_ASSERT_NOT_NULL(widget);
        TEST_ASSERT_TRUE(uC_widget_view_add_widget(view, widget, 0));
    }

    return view;
}

// -----------------------------------------------------------------------

void test_horizontal_view_requires_fixed_width_widgets(void)
{
    uC_attribs_t attrs = test_attrs();
    uC_widget_view_t *view = horizontal_buttons(1, 12, 4);
    uC_widget_t *widget;

    widget = uC_widget_button_create(NULL, "bad", '\0', 3,
        attrs, attrs);
    TEST_ASSERT_NOT_NULL(widget);
    TEST_ASSERT_FALSE(uC_widget_view_add_widget(view, widget, 0));
    uC_widget_close_widget(widget);

}

// -----------------------------------------------------------------------

void test_horizontal_view_scrolls_to_reveal_selected_index(void)
{
    uC_widget_view_t *view = horizontal_buttons(6, 12, 4);

    uC_widget_to_view_index(view, 0);
    TEST_ASSERT_EQUAL_UINT16(0, view->top);
    TEST_ASSERT_EQUAL_UINT16(0, view->cy);

    uC_widget_to_view_index(view, 2);
    TEST_ASSERT_EQUAL_UINT16(0, view->top);
    TEST_ASSERT_EQUAL_UINT16(2, view->cy);
    TEST_ASSERT_EQUAL_UINT16(2, uC_widget_view_index(view));

    uC_widget_to_view_index(view, 3);
    TEST_ASSERT_EQUAL_UINT16(1, view->top);
    TEST_ASSERT_EQUAL_UINT16(2, view->cy);
    TEST_ASSERT_EQUAL_UINT16(3, uC_widget_view_index(view));

    uC_widget_to_view_index(view, 2);
    TEST_ASSERT_EQUAL_UINT16(1, view->top);
    TEST_ASSERT_EQUAL_UINT16(1, view->cy);
    TEST_ASSERT_EQUAL_UINT16(2, uC_widget_view_index(view));
}

// -----------------------------------------------------------------------

void test_horizontal_index_keeps_visible_viewport(void)
{
    uC_widget_view_t *view = horizontal_buttons(6, 12, 4);

    uC_widget_to_view_index(view, 5);
    TEST_ASSERT_EQUAL_UINT16(3, view->top);
    TEST_ASSERT_EQUAL_UINT16(2, view->cy);

    uC_widget_to_view_index(view, 4);
    TEST_ASSERT_EQUAL_UINT16(3, view->top);
    TEST_ASSERT_EQUAL_UINT16(1, view->cy);
    TEST_ASSERT_EQUAL_UINT16(4, uC_widget_view_index(view));

    uC_widget_to_view_index(view, 1);
    TEST_ASSERT_EQUAL_UINT16(1, view->top);
    TEST_ASSERT_EQUAL_UINT16(0, view->cy);

}

// -----------------------------------------------------------------------

void test_orientation_change_preserves_selected_index(void)
{
    uC_attribs_t attrs = test_attrs();
    uC_widget_view_t *view;
    uC_widget_t *widget;
    uint16_t i;

    view = uC_widget_view_create(NULL, 12, 2, 0, 0, attrs, true);
    TEST_ASSERT_NOT_NULL(view);

    for (i = 0; i < 5; i++)
    {
        widget = uC_widget_button_create(NULL, "tab", '\0', 4,
            attrs, attrs);
        TEST_ASSERT_NOT_NULL(widget);
        TEST_ASSERT_TRUE(uC_widget_view_add_widget(view, widget, 0));
    }

    uC_widget_to_view_index(view, 4);
    TEST_ASSERT_EQUAL_UINT16(4, uC_widget_view_index(view));

    TEST_ASSERT_TRUE(uC_widget_view_set_orientation(view,
        uC_VIEW_HORIZONTAL));
    TEST_ASSERT_EQUAL_UINT16(4, uC_widget_view_index(view));
    TEST_ASSERT_EQUAL_UINT16(2, view->top);
    TEST_ASSERT_EQUAL_UINT16(2, view->cy);

    TEST_ASSERT_TRUE(uC_widget_view_set_orientation(view,
        uC_VIEW_VERTICAL));
    TEST_ASSERT_EQUAL_UINT16(4, uC_widget_view_index(view));
    TEST_ASSERT_EQUAL_UINT16(3, view->top);
    TEST_ASSERT_EQUAL_UINT16(1, view->cy);

}

// -----------------------------------------------------------------------

void test_restoring_view_focus_keeps_horizontal_selection(void)
{
    uC_attribs_t attrs = test_attrs();
    uC_screen_t *screen;
    uC_widget_vg_t *group;
    uC_widget_view_t *view;
    uC_widget_t *tabs[5];
    uint16_t i;

    screen = uC_scr_open(20, 3);
    TEST_ASSERT_NOT_NULL(screen);
    group = uC_widget_vg_create(NULL, 20, 2, 0, 0, attrs);
    TEST_ASSERT_NOT_NULL(group);
    view = uC_widget_view_create(NULL, 12, 1, 0, 0,
        attrs, true);
    TEST_ASSERT_NOT_NULL(view);
    TEST_ASSERT_TRUE(uC_widget_view_set_orientation(view,
        uC_VIEW_HORIZONTAL));
    TEST_ASSERT_TRUE(uC_widget_vg_add_view(group, view, 7));

    for (i = 0; i < 5; i++)
    {
        tabs[i] = uC_widget_button_create(NULL, "tab", '\0', 4,
            attrs, attrs);
        TEST_ASSERT_NOT_NULL(tabs[i]);
        TEST_ASSERT_TRUE(uC_widget_view_add_widget(view, tabs[i], 0));
    }

    uC_widget_vg_attach(screen, group);
    uC_widget_to_view_index(view, 4);

    TEST_ASSERT_FALSE(uC_widget_select_widget(99));
    TEST_ASSERT_TRUE(uC_widget_select_widget(7));
    TEST_ASSERT_EQUAL_PTR(tabs[4], widget_state.widget);
    TEST_ASSERT_EQUAL_PTR(tabs[4], view->view_node->payload);
    TEST_ASSERT_EQUAL_UINT16(4, uC_widget_view_index(view));

    uC_widget_vg_close(group);
    uC_scr_close(screen);
}

// -----------------------------------------------------------------------

void test_orientation_change_does_not_steal_other_view_focus(void)
{
    uC_attribs_t attrs = test_attrs();
    uC_widget_view_t *active;
    uC_widget_view_t *other;
    uC_widget_t *active_widget;
    uC_widget_t *widget;
    uint16_t i;

    active = horizontal_buttons(1, 4, 4);
    uC_widget_to_view_index(active, 0);
    active_widget = widget_state.widget;

    other = uC_widget_view_create(NULL, 12, 2, 0, 0, attrs, true);
    TEST_ASSERT_NOT_NULL(other);
    for (i = 0; i < 4; i++)
    {
        widget = uC_widget_button_create(NULL, "tab", '\0', 4,
            attrs, attrs);
        TEST_ASSERT_NOT_NULL(widget);
        TEST_ASSERT_TRUE(uC_widget_view_add_widget(other, widget, 0));
    }

    TEST_ASSERT_TRUE(uC_widget_view_set_orientation(other,
        uC_VIEW_HORIZONTAL));
    TEST_ASSERT_EQUAL_PTR(active, widget_state.view);
    TEST_ASSERT_EQUAL_PTR(active_widget, widget_state.widget);
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_horizontal_view_requires_fixed_width_widgets);
    RUN_TEST(test_horizontal_view_scrolls_to_reveal_selected_index);
    RUN_TEST(test_horizontal_index_keeps_visible_viewport);
    RUN_TEST(test_orientation_change_preserves_selected_index);
    RUN_TEST(test_restoring_view_focus_keeps_horizontal_selection);
    RUN_TEST(test_orientation_change_does_not_steal_other_view_focus);
    return UNITY_END();
}

#else

#include <stdio.h>

void setUp(void)    {}
void tearDown(void) {}

int main(void)
{
    puts("UC_WIDGETS is disabled; horizontal widget test is expected to fail.");
    return 1;
}

#endif

// =======================================================================
