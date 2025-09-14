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

// -----------------------------------------------------------------------

#define STAT_SIZE (33)
#define STATUS_X  (55)
#define STATUS_Y  (0)

#define WIN_WIDTH  (40)
#define WIN_HEIGHT (15)

extern uC_screen_t *active_screen;

char status[STAT_SIZE];
uC_window_t *status_win;

uint32_t radio;
uint32_t check;

uint32_t radio_final;
uint32_t check_final;

// data buffers for textbox widgets

#define NUMERICAL_WIDTH (16)
#define ALPHA_WIDTH (32)

char hex_data[NUMERICAL_WIDTH];
char oct_data[NUMERICAL_WIDTH];
char dec_data[NUMERICAL_WIDTH];
char bin_data[NUMERICAL_WIDTH];
char alpha_data[ALPHA_WIDTH];

// -----------------------------------------------------------------------

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

static uC_widget_vg_t *init_vg(uint16_t xco, uint16_t yco)
{
    uC_widget_vg_t *vg = uC_widget_vg_create("View Group 1",
        WIN_WIDTH, WIN_HEIGHT, xco, yco, vg_attrs);

    uC_ASSERT(vg != NULL, "Cannot create view group");

    // if it has a border its name will be displayed if it has one
    // even with a name, if there is no border it will not be displayed
    // this may be changed in the future

    uC_widget_vg_add_border(vg, BDR_SINGLE, box_attrs,
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
// so this is not an optional parameter.

// If the button name contains this character then the first instance
// of that character will be displayed with an underline on the button

#define BUTTON_OK_KEY     ('O')
#define BUTTON_CANCEL_KEY ('C')

static void view_add_buttons(uC_widget_view_t *view, uint16_t sequence)
{
    uC_widget_t *button1;
    uC_widget_t *button2;

    button1 = uC_widget_button_create(
        sequence, &which_button, "Ok", BUTTON_OK_KEY,
        BUTTON_WIDTH, BUTTON_XCO_1, BUTTON_YCO,
        btn_attrs, btn_focus_attrs);

    button2 = uC_widget_button_create(
        sequence + 1, &which_button, "Cancel", BUTTON_CANCEL_KEY,
        BUTTON_WIDTH, BUTTON_XCO_2, BUTTON_YCO,
        btn_attrs, btn_focus_attrs);

    uC_ASSERT(button1 != NULL, "Cannot create button");
    uC_ASSERT(button2 != NULL, "Cannot create button");

    uC_widget_view_add_widget(view, button1);
    uC_widget_view_add_widget(view, button2);
}

// -----------------------------------------------------------------------
// helper function

#define RADIO_WIDTH (10)
#define NO_XCO      (0)    // the radio buttons are part of a scrollable
#define NO_YCO      (0)    // view, their coordinates are view controlled

static uC_widget_t *create_radio_button(char *name,
    uint16_t sequence, uint16_t radio_bit)
{
    uC_widget_t *w = uC_widget_radio_create(
        sequence, &radio, radio_bit,
        name, uC_RADIO_DIAMOND,
        RADIO_WIDTH, NO_XCO, NO_YCO,
        radio_attrs, radio_focus_attrs);

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

static void view_add_radio_buttons(uC_widget_view_t *view,
    uint16_t sequence)
{
    uint16_t i;
    uC_widget_t *r;

    for (i = 0; i < NUM_RADIO; i++)
    {
        // while this call will apply the same sequence number to every
        // widget within this view it is only the first widget which
        // matters.  the entire view gets the tab sequence value of the
        // first widget added to it.

        // the i parameter here is the bit number within the "radio"
        // varible that each widget will toggle.

        r = create_radio_button(radio_names[i], sequence, i);
        uC_widget_view_add_widget(view, r);
    }
}

// -----------------------------------------------------------------------
// helper function

static uC_widget_t *create_check_button(char *name,
    uint16_t sequence, uint16_t check_bit)
{
    uC_widget_t *w = uC_widget_check_create(
        sequence, &check, check_bit,
        // or we can have uC_RADIO_BOX uC_RADIO_CHECKBOX etc
        name, uC_RADIO_XBOX,
        RADIO_WIDTH, NO_XCO, NO_YCO,
        radio_attrs, radio_focus_attrs);

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

static void view_add_check_buttons(uC_widget_view_t *view,
    uint16_t sequence)
{
    uint16_t i;
    uC_widget_t *r;

    for (i = 0; i < NUM_CHECK; i++)
    {
        r = create_check_button(check_names[i], sequence, i);
        uC_widget_view_add_widget(view, r);
    }
}

// -----------------------------------------------------------------------
// this is a minor todo item



// API uC_widget_t *uC_widget_text_create(
//     uint16_t sequence, char *data, char *name,
//     uint16_t size, uC_textbox_radix_t radix,
//     uint16_t width, uint8_t xco, uint8_t yco,
//     uC_attribs_t attrs, uC_attribs_t focus)

#define NUMERICAL_WIDGET_SIZE  (16)
#define NUMERICAL_WIDGET_WIDTH (8)
#define ALPHA_WIDGET_SIZE      (32)
#define ALPHA_WIDGET_WIDTH     (16)

#define HEX_X (0)
#define HEX_Y (0)
#define OCT_Y (0)
#define OCT_X (18)
#define DEC_X (0)
#define DEC_Y (1)
#define BIN_X (18)
#define BIN_Y (1)

static void view_add_text_boxes(uC_widget_view_t *view, uint16_t sequence)
{
    uC_widget_t *t1 = uC_widget_text_create(
        sequence++, (char *)&hex_data, "Hex    :",
        NUMERICAL_WIDGET_SIZE, INPUT_HEX,
        NUMERICAL_WIDGET_WIDTH, HEX_X, HEX_Y,
        text_attrs, text_focus_attrs);

    uC_widget_t *t2 = uC_widget_text_create(
        sequence++, (char *)&oct_data, "Octal  :",
        NUMERICAL_WIDGET_SIZE, INPUT_OCTAL,
        NUMERICAL_WIDGET_WIDTH, OCT_X, OCT_Y,
        text_attrs, text_focus_attrs);

    uC_widget_t *t3 = uC_widget_text_create(
        sequence++, (char *)&dec_data, "Decimal:",
        NUMERICAL_WIDGET_SIZE, INPUT_DECIMAL,
        NUMERICAL_WIDGET_WIDTH, DEC_X, DEC_Y,
        text_attrs, text_focus_attrs);

    uC_widget_t *t4 = uC_widget_text_create(
        sequence++, (char *)&bin_data, "Binary :",
        NUMERICAL_WIDGET_SIZE, INPUT_BINARY,
        NUMERICAL_WIDGET_WIDTH, BIN_X, BIN_Y,
        text_attrs, text_focus_attrs);

    uC_widget_t *t5 = uC_widget_text_create(
        sequence++, (char *)&alpha_data, "Text   :",
        ALPHA_WIDGET_SIZE, INPUT_ALPHA,
        NUMERICAL_WIDGET_WIDTH, 0, 3,
        text_attrs, text_focus_attrs);

    uC_widget_view_add_widget(view, t1);
    uC_widget_view_add_widget(view, t2);
    uC_widget_view_add_widget(view, t3);
    uC_widget_view_add_widget(view, t4);
    uC_widget_view_add_widget(view, t5);
}

// -----------------------------------------------------------------------

static void init_widgets(void)
{
    uC_window_t *win;
    uC_widget_view_t *view;

    vg1 = init_vg(5, 3);

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

    uC_widget_view_add_border(view, BDR_CURVED, box_attrs);
    uC_widget_vg_add_view(vg1, view);
    view_add_radio_buttons(view, RADIO_SEQUENCE);

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

    uC_widget_view_add_border(view, BDR_CURVED, box_attrs);
    uC_widget_vg_add_view(vg1, view);
    view_add_check_buttons(view, CHECK_SEQUENCE);

    // -------------------------------------------------------------------
    // create a view for text edit boxes with a tab sequence starting at 3

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

    uC_widget_view_add_border(view, BDR_CURVED, box_attrs);
    uC_widget_vg_add_view(vg1, view);

    view_add_text_boxes(view, TEXT_SEQUENCE);

    // -------------------------------------------------------------------
    // crate anonymous view within view group 1 for buttons

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

    // add buttons to this view and associate it with the view group
    // the magic number is the tab sequence for the buttons which here
    // will be 8 and 9

    view_add_buttons(view, 8);
    uC_widget_vg_add_view(vg1, view);
}

// -----------------------------------------------------------------------

void widget_loop(void)
{
    uint8_t k;

    do
    {
        k = uC_widget_main();

        switch (k)
        {
            case 'w':
                uC_widget_vg_attach(active_screen, vg1);
                break;

            case 'o':  case 'O':
                check_final = check;
                radio_final = radio;
                uC_widget_vg_detach(active_screen, vg1);
                break;

            case 'c':  case 'C':
                radio = radio_final;
                check = check_final;
                uC_widget_vg_detach(active_screen, vg1);
                break;
        }

        sprintf(status, "K:%c R:%02x C:%02x",
            k, radio_final, check_final);

        uC_set_status(status_win, status);
        uC_scr_draw_screen(active_screen);

    } while (k != 0x1b);
}

// -----------------------------------------------------------------------

static void exit_prog(void)
{
    uC_set_key(0x1b);
}

// -----------------------------------------------------------------------

static uC_switch_t menu_vectors[] =
{
    { 0x8d9c616c, exit_prog }
};

#define VCOUNT sizeof(menu_vectors) / sizeof(menu_vectors[0])

// -----------------------------------------------------------------------

opt_t menu_address_cb(int32_t hash)
{
    int16_t i;
    uC_switch_t *s = menu_vectors;

    for(i = 0; i < VCOUNT; i++)
    {
        if(hash == s->option)
        {
            return s->vector;
        }
        s++;
    }

    return NULL;
}

// -----------------------------------------------------------------------

int main(void)
{
    int y;
    uint8_t k;
    uC_window_t *win;

    uCurses_init("json/wtest.json", NULL, menu_address_cb);

    status_win = uC_add_status(active_screen, STAT_SIZE,
        STATUS_X, STATUS_Y);

    // set the foreground and background attributes for the
    // status window

    // %fs   set window forground color to a gray scale
    // %bs   set window background color to a gray scale
    // %0    clear window

    uC_win_printf(status_win, "%fs%bs%0", uC_GRAY_09, uC_GRAY_03);

    init_widgets();         // initialize all widgets
    uC_smkx();              // make cursor up and down actually work

    win = active_screen->backdrop;

    uC_win_printf(win, "%@%fc%bs Press W to activate   ",
        10, 3, uC_COLOR_CYAN, uC_GRAY_04);

    uC_win_printf(win, "%@ Press TAB to select   ", 10, 4);
    uC_win_printf(win, "%@ Press Cursor to move  ", 10, 5);
    uC_win_printf(win, "%@ Press SPACE to toggle ", 10, 6);
    uC_win_printf(win, "%@ Press ENTER on button ", 10, 7);
    uC_win_printf(win, "%@ -- or hit O or C key  ", 10, 8);

    widget_loop();

    // where to put the cursor on exit

    y = active_screen->height - 3;

    // put the console in a normal state on exit

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(y, 0);

    // close the screen, all windows within it, all menus
    // and all active
    uC_scr_close(active_screen);

    // this will deallocate everything that is still allocated
    // so valgrind wont complain
    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================

