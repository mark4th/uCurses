// wtest.c  - widget test code
// -----------------------------------------------------------------------

// not yet added to the main demo application as it is still a work in
// progress.   just run make wtest then ./wtest.  there will be
// instructions added to the screen but to view the widgets press w to
// activate them.   tab will bounce between the widgets and for the check
// box and radio button views the space bar will toggle the value.

// at any time pressing either O or o will act like pressing the Ok button
// and pressing C or c will act like pressing the Cancel button.

// while the text boxes will accept focus and will show this visually the
// text editing for those widgets is not yet implemented.  incoming!

// the escape key quits the demo

// -----------------------------------------------------------------------

#include <stdio.h>

#include "uCurses.h"

#include "uC_screen.h"
#include "uC_widgets.h"
#include "uC_utils.h"
#include "uC_borders.h"
#include "uC_keys.h"
#include "uC_status.h"
#include "uC_win_printf.h"
#include "uC_terminfo.h"
#include "uC_utils.h"
#include "uC_switch.h"
#include "uC_menus.h"

// -----------------------------------------------------------------------

#define STAT_SIZE (33)
#define STATUS_X  (55)
#define STATUS_Y  (0)

#define WIN_WIDTH  (40)
#define WIN_HEIGHT (15)

uC_screen_t *active_screen;

char status[STAT_SIZE];
uC_window_t *status_win;

uint32_t radio;
uint32_t check;

uint32_t radio_final;
uint32_t check_final;

// data buffers for textbox widgets

#define NUMERICAL_WIDTH (16)
#define ALPHA_WIDTH (32)

char hex_data[NUMERICAL_WIDTH + 1];
char oct_data[NUMERICAL_WIDTH + 1];
char dec_data[NUMERICAL_WIDTH + 1];
char bin_data[NUMERICAL_WIDTH + 1];
char alpha_data[ALPHA_WIDTH + 1];

// -----------------------------------------------------------------------

// if your buttons do not have a defined keyboard shortcut you can tell
// which one was pressed by which widget sequence value is written to here

uint16_t which_button;

uC_widget_vg_t *vg1;
uC_widget_vg_t *vg2;

// -----------------------------------------------------------------------

#include "attr_defs.c"

// -----------------------------------------------------------------------
// a view group is a window and you can have as many view groups on the
// screen as you can fit.  A view group can have as many widget views
// within it as you can fit.

// a view is a container for one type of widget.  it can either have a
// border and a name or be unbounded and anonymous.  The button view in
// this demo application is both unbounded and anonymous but both buttons
// are contained within it.

// tab sequences are yours to define.  you can give each widget any tab
// focus sequence as long as those values start at ONE and there are no
// gaps or repeats in the sequence.   for a scrollable view such as the
// ones used for the radio buttons and check boxes in this demo it is the
// first widget within that view that defines its tab sequence.   the
// entire view gets that tab sequence value.   subsequent widgets within
// that view can either have the same sequence number or a sequence of
// zero.

static uC_widget_vg_t *init_vg(char *name, uint16_t xco, uint16_t yco)
{
    uC_widget_vg_t *vg = uC_widget_vg_create(name,
        WIN_WIDTH, WIN_HEIGHT, xco, yco, vg_attrs);

    uC_ASSERT(vg != NULL, "Cannot create view group");

    // if it has a border its name will be displayed if it has one
    // even with a name, if there is no border it will not be displayed
    // this may be changed in the future

    uC_widget_vg_add_border(vg, uC_BDR_SINGLE, box_attrs,
        box_focus_attrs);

    return vg;
}

// -----------------------------------------------------------------------
// add button widgets to button view

#define BUTTON_WIDTH (8)
#define BUTTON_XCO_1 (0)
#define BUTTON_XCO_2 (30)
#define BUTTON_YCO   (0)

// the key character is used as a visual indication of which key press
// will act like a button press on that button.  It is also the value
// returned by the widget button handler when that button is pressed
// so this is not an optional parameter.  If no button key is defined
// the button handler will write the buttons sequence value into the
// widgets *select address for the application to interpret
//
// If the button name contains this character then the first instance
// of that character will be displayed with an underline on the button

#define BUTTON_OK_KEY     ('O')
#define BUTTON_CANCEL_KEY ('C')

static void view_add_buttons(uC_widget_view_t *view, uint16_t sequence)
{
    uC_widget_t *button1;
    uC_widget_t *button2;

    button1 = uC_widget_button_create(
        &which_button, "Ok", BUTTON_OK_KEY,
        BUTTON_WIDTH, btn_attrs, btn_focus_attrs);

    button2 = uC_widget_button_create(
        &which_button, "Cancel", BUTTON_CANCEL_KEY,
        BUTTON_WIDTH, btn_attrs, btn_focus_attrs);

    uC_ASSERT(button1 != NULL, "Cannot create button");
    uC_ASSERT(button2 != NULL, "Cannot create button");

    uC_widget_set_position(button1, BUTTON_XCO_1, BUTTON_YCO);
    uC_widget_set_position(button2, BUTTON_XCO_2, BUTTON_YCO);

    uC_widget_view_add_widget(view, button1, sequence);
    uC_widget_view_add_widget(view, button2, sequence + 1);
}

// -----------------------------------------------------------------------
// helper function used to create each radio button in the view

#define RADIO_WIDTH (10)
#define NO_XCO      (0)    // the radio buttons are part of a scrollable
#define NO_YCO      (0)    // view, their coordinates are view controlled

static uC_widget_t *create_radio_button(char *name, uint16_t radio_bit)
{
    uC_widget_t *w = uC_widget_radio_create(
        &radio, radio_bit, name, uC_RADIO_DIAMOND,
        RADIO_WIDTH, radio_attrs, radio_focus_attrs);

    uC_ASSERT(w != NULL, "Cannot allocate radio widget");

    return w;
}

// -----------------------------------------------------------------------

char *radio_names[] =
{
    "Radio 1",    "Radio 2",    "Radio 3",    "Radio 4",
    "Radio 5",    "Radio 6",    "Radio 7",    "Radio 7",
    "Radio 9",    "Radio 10",   "Radio 11",   "Radio 12",
    "Radio 13",   "Radio 14",   "Radio 15",   "Radio 16",
};

#define NUM_RADIO (sizeof(radio_names) / sizeof(radio_names[0]))

static void view_add_radio_buttons(uC_widget_view_t *view)
{
    uint16_t i;
    uC_widget_t *r;

    for (i = 0; i < NUM_RADIO; i++)
    {
        r = create_radio_button(radio_names[i], i);
        uC_widget_view_add_widget(view, r, 0);
    }
}

// -----------------------------------------------------------------------
// helper function

static uC_widget_t *create_check_button(char *name, uint16_t check_bit)
{
    uC_widget_t *w = uC_widget_check_create(
        &check, check_bit, name, uC_RADIO_BOX,
        RADIO_WIDTH, radio_attrs, radio_focus_attrs);

    uC_ASSERT(w != NULL, "Cannot allocate check widget");

    return w;
}

// -----------------------------------------------------------------------
// for a scrollable view it is the view itself that gets the tab sequence
// but it is still set within the first widget in that view.  which widget
// within the view gets the actual focus on tab selection is controlled by
// the view handler itself.

char *check_names[] =
{
    "Check 1",    "Check 2",    "Check 3",    "Check 4",
    "Check 5",    "Check 6",    "Check 7",    "Check 7",
    "Check 9",    "Check 10",   "Check 11",   "Check 12",
    "Check 13",   "Check 14",   "Check 15",   "Check 16",
};

#define NUM_CHECK (sizeof(check_names) / sizeof(check_names[0]))

static void view_add_check_buttons(uC_widget_view_t *view)
{
    uint16_t i;
    uC_widget_t *r;

    for (i = 0; i < NUM_CHECK; i++)
    {
        r = create_check_button(check_names[i], i);
        uC_widget_view_add_widget(view, r, 0);
    }
}

// -----------------------------------------------------------------------

#define NUMERICAL_WIDGET_SIZE  (16)
#define NUMERICAL_WIDGET_WIDTH (8)
#define ALPHA_WIDGET_SIZE      (32)
#define ALPHA_WIDGET_WIDTH     (16)

#define HEX_X   (0)
#define HEX_Y   (0)
#define OCT_Y   (0)
#define OCT_X   (18)
#define DEC_X   (0)
#define DEC_Y   (1)
#define BIN_X   (18)
#define BIN_Y   (1)
#define ALPHA_X (0)
#define ALPHA_Y (3)

// -----------------------------------------------------------------------

static void view_add_text_boxes(uC_widget_view_t *view, uint16_t sequence)
{
    uC_widget_t *t1 = uC_widget_textbox_create(
        (char *)&hex_data, "Hex    :",
        NUMERICAL_WIDGET_SIZE, uC_INPUT_HEX,
        NUMERICAL_WIDGET_WIDTH, text_attrs, text_focus_attrs);

    uC_widget_t *t2 = uC_widget_textbox_create(
        (char *)&oct_data, "Octal  :",
        NUMERICAL_WIDGET_SIZE, uC_INPUT_OCTAL,
        NUMERICAL_WIDGET_WIDTH, text_attrs, text_focus_attrs);

    uC_widget_t *t3 = uC_widget_textbox_create(
        (char *)&dec_data, "Decimal:",
        NUMERICAL_WIDGET_SIZE, uC_INPUT_DECIMAL,
        NUMERICAL_WIDGET_WIDTH, text_attrs, text_focus_attrs);

    uC_widget_t *t4 = uC_widget_textbox_create(
        (char *)&bin_data, "Binary :",
        NUMERICAL_WIDGET_SIZE, uC_INPUT_BINARY,
        NUMERICAL_WIDGET_WIDTH, text_attrs, text_focus_attrs);

    uC_widget_t *t5 = uC_widget_textbox_create(
        (char *)&alpha_data, "Alpha  :",
        ALPHA_WIDGET_SIZE, uC_INPUT_ALPHA,
        NUMERICAL_WIDGET_WIDTH, text_attrs, text_focus_attrs);

    uC_widget_set_position(t1, HEX_X, HEX_Y);
    uC_widget_set_position(t2, OCT_X, OCT_Y);
    uC_widget_set_position(t3, DEC_X, DEC_Y);
    uC_widget_set_position(t4, BIN_X, BIN_Y);
    uC_widget_set_position(t5, ALPHA_X, ALPHA_Y);

    uC_widget_view_add_widget(view, t1, sequence);
    uC_widget_view_add_widget(view, t2, sequence + 1);
    uC_widget_view_add_widget(view, t3, sequence + 2);
    uC_widget_view_add_widget(view, t4, sequence + 3);
    uC_widget_view_add_widget(view, t5, sequence + 4);
}

// -----------------------------------------------------------------------

static void init_widgets(void)
{
    uC_window_t *win;
    uC_widget_view_t *view;

    vg1 = init_vg("View Group 1", 5, 3);
    vg2 = init_vg("View Group 2", 50, 3);

    // -------------------------------------------------------------------
    // create a named scrollable view for radio buttons

    #define RADIO_VIEW_WIDTH  (10)
    #define RADIO_VIEW_HEIGHT (4)
    #define RADIO_VIEW_XCO    (2)
    #define RADIO_VIEW_YCO    (1)
    #define SCROLL            (true)
    #define RADIO_SEQUENCE    (1)

    view = uC_widget_view_create("Radio",
        RADIO_VIEW_WIDTH, RADIO_VIEW_HEIGHT,
        RADIO_VIEW_XCO, RADIO_VIEW_YCO,
        view_attrs, SCROLL);

    uC_ASSERT(view != NULL, "Cannot create view");

    uC_widget_view_add_border(view, uC_BDR_CURVED, box_attrs);
    uC_widget_vg_add_view(vg1, view, RADIO_SEQUENCE);
    view_add_radio_buttons(view);

    // -------------------------------------------------------------------
    // create a named scrollable view for check box buttons

    #define CHECK_VIEW_WIDTH  (10)
    #define CHECK_VIEW_HEIGHT (4)
    #define CHECK_VIEW_XCO    (28)
    #define CHECK_VIEW_YCO    (1)
    #define CHECK_SEQUENCE    (2)

    view = uC_widget_view_create("Check",
        CHECK_VIEW_WIDTH, CHECK_VIEW_HEIGHT,
        CHECK_VIEW_XCO, CHECK_VIEW_YCO,
        view_attrs, SCROLL);

    uC_ASSERT(view != NULL, "Cannot create view");

    uC_widget_view_add_border(view, uC_BDR_CURVED, box_attrs);
    uC_widget_vg_add_view(vg1, view, CHECK_SEQUENCE);
    view_add_check_buttons(view);

    // -------------------------------------------------------------------
    // create a view for text edit boxes

    #define TEXT_VIEW_WIDTH  (36)
    #define TEXT_VIEW_HEIGHT (6)
    #define TEXT_VIEW_XCO    (2)
    #define TEXT_VIEW_YCO    (7)
    #define TEXT_SEQUENCE    (3)

    view = uC_widget_view_create("Text View (todo)",
        TEXT_VIEW_WIDTH, TEXT_VIEW_HEIGHT,
        TEXT_VIEW_XCO, TEXT_VIEW_YCO,
        view_attrs, false);

    uC_ASSERT(view != NULL, "Cannot create view");

    uC_widget_view_add_border(view, uC_BDR_CURVED, box_attrs);
    uC_widget_vg_add_view(vg1, view, TEXT_SEQUENCE);
    view_add_text_boxes(view, TEXT_SEQUENCE);

    // -------------------------------------------------------------------
    // anonymous view within view group 1 for buttons

    #define NO_NAME            (NULL)
    #define BUTTON_VIEW_WIDTH  (WIN_WIDTH - 1)
    #define BUTTON_VIEW_HEIGHT (1)
    #define BUTTON_VIEW_XCO    (1)
    #define BUTTON_VIEW_YCO    (WIN_HEIGHT - 1)
    #define NO_SCROLL          (false)

    view = uC_widget_view_create(NO_NAME, BUTTON_VIEW_WIDTH,
        BUTTON_VIEW_HEIGHT, BUTTON_VIEW_XCO, BUTTON_VIEW_YCO,
        view_attrs, NO_SCROLL);

    uC_ASSERT(view != NULL, "Cannot create view");

    view_add_buttons(view, TEXT_SEQUENCE + 5);
    uC_widget_vg_add_view(vg1, view, 0);
}

// -----------------------------------------------------------------------

static inline void handle_key(char k)
{
    switch (k)
    {
        case 'w':
            // toggle widget activation
            if (active_screen->view_groups.count == 0)
            {
                uC_widget_vg_attach(active_screen, vg1);
                uC_widget_vg_attach(active_screen, vg2);
            }
            else
            {
                uC_widget_vg_detach(active_screen, vg1);
                uC_widget_vg_detach(active_screen, vg2);
            }
            break;
        case 'o':  case 'O':
            // only valid if widgets are active
            if (active_screen->view_groups.count != 0)
            {
                check_final = check;
                radio_final = radio;
                uC_widget_vg_detach(active_screen, vg1);
                uC_widget_vg_detach(active_screen, vg2);
            }
            break;
        case 'c':  case 'C':
            // only valid if widgets are active
            if (active_screen->view_groups.count != 0)
            {
                radio = radio_final;
                check = check_final;
                uC_widget_vg_detach(active_screen, vg1);
                uC_widget_vg_detach(active_screen, vg2);
            }
            break;
    }
}

// -----------------------------------------------------------------------

void widget_loop(void)
{
    char k;

    uC_scr_draw_screen(active_screen);

    do
    {
        // the F10 menus are disabled here when widgets are active
        // but they do not need to be, you could change the menu
        // context based on whether or not widgets were active or not

        k = (active_screen->view_groups.count == 0)
            ? uC_key()
            : uC_widget_main();

        handle_key(k);

        // sprintf(status, "R:%04x C:%04x K:%02x",
        //     radio_final, check_final, k);
        // uC_set_status(status_win, status);

        uC_scr_draw_screen(active_screen);
    } while (k != 0x1b);
}

// -----------------------------------------------------------------------

static void exit_prog(void)
{
    uC_set_key(0x1b);
}

// -----------------------------------------------------------------------
// programatically initialize some do nothing eample pulldown menus

static void init_menus(void)
{
    int32_t f;

    // create a menu bar and attach it to the active screen

    f = uC_menu_bar_open(active_screen);

    if (f != -1)
    {
        // create a pulldown menu and attach it to the menu bar

        f = uC_menu_new_pd(active_screen, "File");
        if (f != -1)
        {
            // create menu items in this pulldown
            uC_menu_new_item(active_screen, "Open",  uC_noop, 0);
            uC_menu_new_item(active_screen, "Close", uC_noop, 0);
            uC_menu_new_item(active_screen, "Save",  uC_noop, 0);
            uC_menu_new_item(active_screen, "Exit",  exit_prog, 0);
        }

        f = uC_menu_new_pd(active_screen, "Edit");
        if (f != -1)
        {
            uC_menu_new_item(active_screen, "Cut",       uC_noop, 0);
            uC_menu_new_item(active_screen, "Copy",      uC_noop, 0);
            uC_menu_new_item(active_screen, "Paste",     uC_noop, 0);
            uC_menu_new_item(active_screen, "Delete",    uC_noop, 0);
            uC_menu_new_item(active_screen, "Insert",    uC_noop, 0);
            uC_menu_new_item(active_screen, "Overwrite", uC_noop, 0);
        }

        f = uC_menu_new_pd(active_screen, "Help");
        if (f != -1)
        {
            uC_menu_new_item(active_screen, "Helpful",   uC_noop, 0);
            uC_menu_new_item(active_screen, "Helpless",  uC_noop, 0);
            uC_menu_new_item(active_screen, "Self Help", uC_noop, 0);
        }

        uC_menu_init_keys();
    }
}

// -----------------------------------------------------------------------

int main(void)
{
    int y;
    uint8_t k;
    uC_window_t *win;

    active_screen = uCurses_init("json/wtest.json", NULL, NULL);

    status_win = uC_add_status(active_screen, STAT_SIZE,
        STATUS_X, STATUS_Y);

    init_menus();

    // set the foreground and background attributes for the
    // status window

    // %fs   set window forground color to a gray scale
    // %bs   set window background color to a gray scale
    // %0    clear window

    uC_win_printf(status_win, "%fs%bs%0", uC_GRAY_09, uC_GRAY_03);

    init_widgets();         // initialize all widgets

    // you can actually display things in the backdrop widnow but it
    // is *always* the first window drawn so everything else will
    // overlap it

    win = active_screen->backdrop;

    uC_win_printf(win, "%@%fc%bs Press W to activate   ",
        UC_XY(10, 3), uC_COLOR_CYAN, uC_GRAY_04);

    uC_win_printf(win, "%@ Press TAB to select   ", UC_XY(10, 4));
    uC_win_printf(win, "%@ Press Cursor to move  ", UC_XY(10, 5));
    uC_win_printf(win, "%@ Press SPACE to toggle ", UC_XY(10, 6));
    uC_win_printf(win, "%@ Press ENTER on button ", UC_XY(10, 7));
    uC_win_printf(win, "%@ -- or hit O or C key  ", UC_XY(10, 8));

    widget_loop();

    // where to put the cursor on exit

    y = active_screen->height - 3;

    // put the console in a normal state on exit

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(y, 0);

    // close the screen, all windows within it, all menus
    // and all acti
    uC_scr_close(active_screen);

    // this will deallocate everything that is still allocated
    // so valgrind wont complain
    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================

