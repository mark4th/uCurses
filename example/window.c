// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>


#include "demo.h"

// -----------------------------------------------------------------------

#define SLEEP 15000000

// -----------------------------------------------------------------------

char *window1_name = "window 1";
char *window2_name = "window 2";

uC_window_t *win1;
uC_window_t *win2;
uC_screen_t *scr;

uC_window_t *status_win;

char status[33];

// -----------------------------------------------------------------------
// global variables considered harmful unless you restrict yourself
// to one thread kthxbai

uint16_t first = 1;
int16_t x1, y1;
int16_t x2, y2;
int16_t x1i, x2i;
int16_t y1i, y2i;

// -----------------------------------------------------------------------

uint8_t sintab[] =
{
  0x80, 0x83, 0x86, 0x89, 0x8C, 0x90, 0x93, 0x96,
  0x99, 0x9C, 0x9F, 0xA2, 0xA5, 0xA8, 0xAB, 0xAE,
  0xB1, 0xB3, 0xB6, 0xB9, 0xBC, 0xBF, 0xC1, 0xC4,
  0xC7, 0xC9, 0xCC, 0xCE, 0xD1, 0xD3, 0xD5, 0xD8,
  0xDA, 0xDC, 0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8,
  0xEA, 0xEB, 0xED, 0xEF, 0xF0, 0xF1, 0xF3, 0xF4,
  0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC,
  0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,

  0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD,
  0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6,
  0xF5, 0xF4, 0xF3, 0xF1, 0xF0, 0xEF, 0xED, 0xEB,
  0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
  0xDA, 0xD8, 0xD5, 0xD3, 0xD1, 0xCE, 0xCC, 0xC9,
  0xC7, 0xC4, 0xC1, 0xBF, 0xBC, 0xB9, 0xB6, 0xB3,
  0xB1, 0xAE, 0xAB, 0xA8, 0xA5, 0xA2, 0x9F, 0x9C,
  0x99, 0x96, 0x93, 0x90, 0x8C, 0x89, 0x86, 0x83,

  0x80, 0x7D, 0x7A, 0x77, 0x74, 0x70, 0x6D, 0x6A,
  0x67, 0x64, 0x61, 0x5E, 0x5B, 0x58, 0x55, 0x52,
  0x4F, 0x4D, 0x4A, 0x47, 0x44, 0x41, 0x3F, 0x3C,
  0x39, 0x37, 0x34, 0x32, 0x2F, 0x2D, 0x2B, 0x28,
  0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18,
  0x16, 0x15, 0x13, 0x11, 0x10, 0x0F, 0x0D, 0x0C,
  0x0B, 0x0A, 0x08, 0x07, 0x06, 0x06, 0x05, 0x04,
  0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,

  0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03,
  0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x0A,
  0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x13, 0x15,
  0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24,
  0x26, 0x28, 0x2B, 0x2D, 0x2F, 0x32, 0x34, 0x37,
  0x39, 0x3C, 0x3F, 0x41, 0x44, 0x47, 0x4A, 0x4D,
  0x4F, 0x52, 0x55, 0x58, 0x5B, 0x5E, 0x61, 0x64,
  0x67, 0x6A, 0x6D, 0x70, 0x74, 0x77, 0x7A, 0x7D
};

// -----------------------------------------------------------------------

char lorem[69][14] =
{
    "Lorem ", "ipsum ", "dolor ", "sit ", "amet, ", "consectetur ",
    "adipiscing ", "elit, ", "sed ", "do ", "eiusmod ", "tempor ",
    "incididunt ", "ut ", "labore ", "et ", "dolore ", "magna ",
    "aliqua. ", "Ut ", "enim ", "ad ", "minim ", "veniam, ", "quis ",
    "nostrud ", "exercitation ", "ullamco ", "laboris ",
    "nisi ", "ut ", "aliquip ", "ex ", "ea ", "commodo ", "consequat. ",
    "Duis ", "aute ", "irure ", "dolor ", "in ", "reprehenderit ",
    "in ", "voluptate ", "velit ", "esse ", "cillum ", "dolore ", "eu ",
    "fugiat ", "nulla ", "pariatur. ", "Excepteur ", "sint ", "occaecat ",
    "cupidatat ", "non ", "proident, ", "sunt ", "in ", "culpa ",
    "qui ", "officia ", "deserunt ", "mollit ", "anim ", "id ",
    "est ", "laborum."
};

// -----------------------------------------------------------------------
// i have no idea what this says :)

char *chinese[] =
{
    "我住长江头，君住长江尾。",    "日日思君不见君，共饮长江水。",
    "此水几时休，此恨何时已。",    "只愿君心似我心，定不负相思意。",  " ",
    "击鼓其镗，踊跃用兵。",       "土国城漕，我独南行。",          " ",
    "从孙子仲，平陈与宋。",       "不我以归，忧心有忡。",          " ",
    "爰居爰处，爰丧其马，",       "于以求之，于林之下。",          " ",
    "死生契阔，与子成说。",       "执子之手，与子偕老。",          " ",
    "于嗟阔兮，不我活兮。",       "于嗟洵兮，不我信兮。",          " ",
    "我欲与君相知， 长命无绝衰。", "山无陵，江水为竭，冬雷震震，",
    "夏雨雪，天地合，乃敢与君绝！",                              " ",
};

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

void print_lorem(uC_window_t *win)
{
    int16_t len;

    static int16_t i = 0;
    static uint8_t r1 = 105, g1 = 100, b1 = 45;
    static uint8_t r2, g2, b2;

    if (i == 70) { i = 0; }

    // how many character cells will this string use.
    // this accounts for double width characters

    len = uC_utf8_width(lorem[i]);

    if ((len + win->cx) > win->width)
    {
        r2 = sintab[r1];
        g2 = sintab[g1];
        b2 = sintab[b1];

        uC_win_el(win);     // this also does a win_cr()
        uC_win_set_rgb_bg(win, r2, g2, b2);
        make_contrast(&r2, &g2, &b2);
        uC_win_set_rgb_fg(win, r2, g2, b2);

        r1+= 4; g1 += 3; b1 += 2;
    }
    uC_win_puts(win, lorem[i]);
    i++;
}

// -----------------------------------------------------------------------

static void print_chinese(uC_window_t *win)
{
    static int8_t inc = 1;
    static int8_t line = 0;
    static int8_t gray = 1;

    uC_win_set_gray_fg(win, gray);
    uC_win_set_gray_bg(win, abs(23 - gray));

    gray += inc;

    if ((gray == 23) || (gray == 0x1))
    {
        inc = -inc;
    }

    uC_win_puts(win, chinese[line++]);
    uC_win_el(win);

    if (line > 23) { line = 0; }
}

// -----------------------------------------------------------------------
// furthest right and down a window can go and not move off screen

#define X_MAX(win) (active_screen->width  - win->width  - 2)
#define Y_MAX(win) (active_screen->height - win->height - 2)

// -----------------------------------------------------------------------
// switch which window is on top and which is behind

static void flip_flop(void)
{
    (0 == first)
        ? uC_win_pop(win2)
        : uC_win_pop(win1);

    first ^= 1;
}

// -----------------------------------------------------------------------

static void do_win_demo(void)
{
    // write text into each window
    print_lorem(win1);
    print_chinese(win2);

    // move each window and refrseh the screen display
    uC_win_set_pos(win1, x1, y1);
    uC_win_set_pos(win2, x2, y2);

    // add respective increments to x1, y1, x2, y2
    x1 += x1i;   y1 += y1i;
    x2 += x2i;   y2 += y2i;

    // this if / and / but looping makes each window
    // move around the edge of the screen in one direction
    // or the other

    // for example initially the x1 increment is 1 and the y1
    // increment is zero and the window start at top left.
    // that window will move along the top edge of the screen.
    // when it gets to its max x the y increment is set to
    // the x increment and the x increment is set to zero.
    // the window will now traverse down the right edge of
    // the screen...

    if (y1i == 0)    // if we are moving horizontally...
    {
        if ((x1 == X_MAX(win1)) || (x1 == 2))
        {
            flip_flop();

            y1i = x1i;    x1i = 0;
            y2i = x2i;    x2i = 0;

            return;
        }
    }

    // ... once the window reaches its the bottom right of the
    // window the y increment is set to zero and the x
    // increment is set to the negative of the y increment.
    // the window will now move right to left along the
    // boottom of the window... and so on and so forth

    if (x1i == 0)  // if we are moving vertically
    {
        if ((y1 == Y_MAX(win1)) || (y1 == 2))
        {
            x1i = -y1i;    y1i = 0;
            x2i = -y2i;    y2i = 0;
        }
    }
}

// -----------------------------------------------------------------------
// rotates two windows around the screen in opposite directions
// can this even be done wtih ncurses?

void win_demo(void)
{
    int8_t pause = 0;
    int8_t c;
    uint32_t frames = 0;

    win1->display_name = window1_name;
    win2->display_name = window2_name;

    char status[33];

    x1 = 2;   y1 = 2;
    x2 = X_MAX(win2);
    y2 = Y_MAX(win2);

    x1i = 1;  x2i = -1;
    y1i = 0;  y2i = 0;

    uC_scr_draw_screen(scr);

    for (;;)
    {
        // check for key presses.
        // space pauses anything else quits
        if (uC_test_keys() != 0)
        {
            c = uC_key();

            if ((c == ' ') || (c == 0x0a))
            {
                pause ^= 1;
                continue;
            }
            if(c == 0x1b) { break; }
        }

        if (pause == 0)        // if not paused...
        {
            frames++;

            snprintf(status, 31, "Frame: %d", frames);
            do_win_demo();
            uC_set_status(status_win, status);
            uC_scr_draw_screen(scr);
        }

        // if we did not do this the display would look like itsxz4
        // freaking out!
        uC_clock_sleep(SLEEP);
    }
}

// -----------------------------------------------------------------------

int main(void)
{
    uC_list_node_t *n;
    uC_window_t *win;

    uCurses_init();
    uC_json_file_create_ui("json/window_demo.json", menu_address_cb);
    uC_menu_init();

    scr = active_screen;

    status_win = uC_add_status(scr, 32, 55, 0);
    uC_win_printf(status_win, "%fs%bs%0", 9, 3);

    n = scr->windows.head;
    win1 = n->payload;
    n = n->next;
    win2 = n->payload;

    uC_set_status(status_win, status);
    uC_clr_status(status_win);

    win_demo();

    uC_scr_close(active_screen);
    uC_console_reset_attrs();
    uC_clear();
    uC_cup(10, 0);
    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}


// =======================================================================
