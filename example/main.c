// test main
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// not an example of how to do thins properly, just test/debug code
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "../h/uCurses.h"

// -----------------------------------------------------------------------

#define SLEEP 15000000

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

char *chinese[] =
{
    "我住长江头，", "君住长江尾。",
    "日日思君不见君，", "共饮长江水。",
    "此水几时休，", "此恨何时已。",
    "只愿君心似我心，", "定不负相思意。",
    " ",
    "击鼓其镗，踊跃用兵。", "土国城漕，我独南行。",
    " ",
    "从孙子仲，平陈与宋。", "不我以归，忧心有忡。",
    " ",
    "爰居爰处，爰丧其马，", "于以求之，于林之下。",
    " ",
    "死生契阔，与子成说。", "执子之手，与子偕老。",
    " ",
    "于嗟阔兮，不我活兮。", "于嗟洵兮，不我信兮。",
    " ",
    "我欲与君相知，", "长命无绝衰。",
    "山无陵，", "江水为竭，", "冬雷震震，",
    "夏雨雪 ，", "天地合，", "乃敢与君绝！",
    " ",
};

// -----------------------------------------------------------------------

void print_lorem(window_t *win)
{
    static uint16_t i = 0;
    static uint8_t r1 = 105, g1 = 100, b1 = 45;
    static uint8_t r2, g2, b2;

    uint8_t len;
    i = (i == 69) ? 0 : i;

    // how many character cells will this string use.
    // this accounts for double width characters
    len = utf8_width(&lorem[i][0]);

    if((len + win->cx) >= win->width)
    {
        win_el(win);   // this also effects a win_cr()
        r2 = r1 + 128;
        g2 = g1 + 128;
        b2 = b1 + 128;

        win_set_rgb_fg(win, sintab[r1], sintab[g1], sintab[b1]);
        win_set_rgb_bg(win, sintab[r2], sintab[g2], sintab[b2]);

        r1 += 7; g1 += 5; b1 += 6;
    }
    win_puts(win, &lorem[i][0]);
    i++;
}

// -----------------------------------------------------------------------

static void print_chinese(window_t *win)
{
    static uint8_t inc = 1;
    static uint8_t line = 0;
    static uint8_t gray = 0;

    win_set_gray_fg(win, gray);
    win_set_gray_bg(win, abs(20 - (gray + 10)));

    gray += inc;

    if((gray == 23) || (gray == 0x0))
    {
        inc = -inc;
    }

    win_puts(win, chinese[line++]);
    win_el(win);

    if(line > 32) { line = 0; }
}

// -----------------------------------------------------------------------

void clock_sleep(uint32_t when)
{
    struct timespec tv;
    struct timespec remain;
    uint32_t rv;

    do
    {
        tv.tv_sec  = 0;
        tv.tv_nsec = when;
        rv = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &remain);
        tv = remain;
    } while(EINTR == rv);
}

// -----------------------------------------------------------------------

#define X_END(win) (scr->width  - win->width  - 2)
#define Y_END(win) (scr->height - win->height - 2)

uint8_t test_keys(void);
uint16_t first = 1;

// -----------------------------------------------------------------------
// switch which window is on top and which is behind

static void flip_flop(window_t *win1, window_t *win2)
{
    (0 == first)
        ? win_pop(win2)
        : win_pop(win1);

    first ^= 1;
}

// -----------------------------------------------------------------------

uint16_t x1, y1;
uint16_t x2, y2;
uint16_t x1i, x2i;
uint16_t y1i, y2i;

static void do_run_demo(screen_t *scr, window_t *win1, window_t *win2)
{
    // write text into each window
    print_lorem(win1);
    print_chinese(win2);

    // move each window and refrseh the screen display
    win_set_pos(win1, x1, y1);
    win_set_pos(win2, x2, y2);
    scr_draw_screen(scr);

    // add respective increments to x1, y1, x2, y2
    x1 += x1i;    y1 += y1i;
    x2 += x2i;    y2 += y2i;

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

    if(y1i == 0)    // if we are moving hrizontally...
    {
        if((x1 == X_END(win1)) || (x1 == 2))
        {
            flip_flop(win1, win2);

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

    if(x1i == 0)  // if we are moving vertically
    {
        if((y1 == Y_END(win1)) || (y1 == 2))
        {
            x1i = -y1i;    y1i = 0;
            x2i = -y2i;    y2i = 0;
        }
    }
}

// -----------------------------------------------------------------------
// rotates two windows around the screen in opposite directions
// can this even be done wtih ncurses?

void run_demo(screen_t *scr, window_t *win1, window_t *win2)
{
    uint8_t pause = 0;
    uint8_t c;

    x1 = 2;
    y1 = 2;
    x2 = X_END(win2);
    y2 = Y_END(win2);

    x1i = 1;
    x2i = -1;
    y1i = y2i = 0;

    for(;;)
    {
        // check for key presses. space pauses anything else quits
        if(test_keys() != 0)
        {
            c = key();

            if((c == ' ') || (c == 0x0a))
            {
                pause ^= 1;
                continue;
            }
            if(c == 0x1b)
            {
                return;
            }
        }

        if(pause == 0)        // if not paused...
        {
            do_run_demo(scr, win1, win2);
        }
        // if we did not do this the display would look like its
        // freaking out!
        clock_sleep(SLEEP);
    }
}

// -----------------------------------------------------------------------
// just flipflops the border color of which ever window is on top

static void open_file(void)
{
    bar_set_status("You Selected Open File");
    screen_t *scr = active_screen;
    window_t *win = list_scan(&scr->windows);
    win->bdr_attrs[FG] ^= 0x55;
}

// dummy menu functions for now will add proper demos and proper
// working menu functions

static void close_file(void)
{
    bar_set_status("You Selected Close File");
}
static void delete_internet(void)
{
    bar_set_status("You Selected Delete File");
}
static void copy_nothing(void)
{
    bar_set_status("You Selected Copy Cat");
}
static void insert_mode(void)
{
    bar_set_status("You Selected Insert Mode");
}
static void overwrite_mode(void)
{
    bar_set_status("You Selected Overwrite Mode");
}
static void delete_line(void)
{
    bar_set_status("You Selected Delete Line");
}
static void insert_line(void)
{
    bar_set_status("You Selected Insert Line");
}
static void view_point(void)
{
    bar_set_status("You Selected View Point");
}
static void view_to_a_kill(void)
{
    bar_set_status("You Selected View To a Kill");
}
static void review(void)
{
    bar_set_status("You Selected Review");
}
static void manchester_screwdriver(void)
{
    bar_set_status("You Selected Manchester Screwdriver");
}
static void dentists_drill(void)
{
    bar_set_status("You Selected Dentists Drill");
}
static void diamond_file(void)
{
    bar_set_status("You Selected Diamond File");
}
static void shovel(void)
{
    bar_set_status("You Selected Shovel");
}
static void self_help(void)
{
    bar_set_status("You Selected Self help");
}
static void helping_hand(void)
{
    bar_set_status("You Selected Helping hand");
}
static void helpless(void)
{
    bar_set_status("You Selected Helpless");
}

// -----------------------------------------------------------------------

static switch_t menu_vectors[] =
{
    { 0x89839cf6,  open_file              },
    { 0x77d55838,  close_file             },
    { 0x3b0b4410,  delete_internet        },
    { 0xf5976bec,  copy_nothing           },
    { 0x2f36a87c,  insert_mode            },
    { 0x880a72b2,  overwrite_mode         },
    { 0xc498a8b1,  delete_line            },
    { 0x9f754b9b,  insert_line            },
    { 0xf91bf85d,  view_point             },
    { 0x3d62b4a5,  view_to_a_kill         },
    { 0x01d7329d,  review                 },
    { 0xf67c2cc8,  manchester_screwdriver },
    { 0x42776537,  dentists_drill         },
    { 0x5429e330,  diamond_file           },
    { 0x3e8dbf4c,  shovel                 },
    { 0x935ec77f,  self_help              },
    { 0x5293e60c,  helping_hand           },
    { 0x6131c777,  helpless               }
};

#define VCOUNT sizeof(menu_vectors) / sizeof(menu_vectors[0])

// -----------------------------------------------------------------------
// json parsing calls this to get the address of a specified menu function

static opt_t menu_address_cb(uint32_t hash)
{
    uint16_t i;
    switch_t *s = menu_vectors;

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
    screen_t *scr;
    window_t *win1, *win2;

    alloc_status();

    uCurses_init();

    // initialize menu key handling
    menu_init();

    json_create_ui("example.json", menu_address_cb);
    scr = active_screen;

    scr_draw_screen(scr);

    node_t *n = scr->windows.head;
    win1 = n->payload;
    n = n->next;
    win2 = n->payload;

    run_demo(scr, win1, win2);

    scr_close(scr);

    set_fg(WHITE);
    set_bg(BLACK);

    clear();
    cup(10, 0);
    restore_term();

    uCurses_deInit();

    printf("bye!\n");
    return 0;
}

// =======================================================================
