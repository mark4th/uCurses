// test main
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#include "h/color.h"
#include "h/tui.h"
#include "h/uCurses.h"
#include "h/util.h"

// -----------------------------------------------------------------------

struct termios term_save;
struct termios term;

// -----------------------------------------------------------------------

void clock_sleep(uint32_t clocks)
{
    clock_t time = clock();

    // looping till required time is not achieved
    while (clock() < time + clocks)
        ;
}

// -----------------------------------------------------------------------

void restore_term(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &term_save);
}

// -----------------------------------------------------------------------

#define X_END(win) (scr->width  - win->width  - 2)
#define Y_END(win) (scr->height - win->height - 2)

uint8_t test_keys(void);
uint16_t first = 0;

// -----------------------------------------------------------------------

static void flip_flop(window_t *win1, window_t *win2)
{
    if(0 == first)
    {
        win_pop(win2);
        first = 1;
    }
    else
    {
        win_pop(win1);
        first = 0;
    }
}

// -----------------------------------------------------------------------

void run_demo(screen_t *scr, window_t *win1, window_t *win2)
{
    uint16_t x1 = 2, y1 = 2;
    uint16_t x2 = X_END(win2), y2 = Y_END(win2);


    for(;;)
    {
        while (x1 != X_END(win1))
        {
            if(0 != test_keys())
            {
                return;
            }
            x1++;  x2--;
            win_set_pos(win1, x1, y1);
            win_set_pos(win2, x2, y2);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };

        while(y1 != Y_END(win1))
        {
            if(0 != test_keys())
            {
                return;
            }
            y1++; y2--;
            win_set_pos(win1, x1, y1);
            win_set_pos(win2, x2, y2);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };

        flip_flop(win1, win2);

        while(x1 != 2)
        {
            if(0 != test_keys())
            {
                return;
            }
            x1--; x2++;
            win_set_pos(win1, x1, y1);
            win_set_pos(win2, x2, y2);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };

        while(y1 != 2)
        {
            if(0 != test_keys())
            {
                return;
            }
            y1--; y2++;
            win_set_pos(win1, x1, y1);
            win_set_pos(win2, x2, y2);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };
        flip_flop(win1, win2);
    }
}

// -----------------------------------------------------------------------

void test(void);

int main(void)
{
// test();
// return 0;
    struct winsize w;
    screen_t *scr;
    window_t *win1, *win2;
    uint8_t c;

    uCurses_init();

    curoff();
    atexit(restore_term);
    tcgetattr(STDIN_FILENO, &term_save);
    term = term_save;
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    ioctl(0, TIOCGWINSZ, &w);

    scr = scr_open(w.ws_col, w.ws_row);
    scr_add_backdrop(scr);

    win1 = win_open(30, 15);
    win2 = win_open(30, 15);
    win1->xco = 2;
    win1->yco = 2;
    win2->xco = X_END(win2);
    win2->yco = Y_END(win2);

    win_set_fg(win1, WHITE);
    win_set_bg(win1, BLUE);
    win_set_fg(win2, GREEN);
    win_set_bg(win2, MAGENTA);

    win1->flags |= WIN_BOXED;
    win2->flags |= WIN_BOXED;

    win1->bdr_attrs[ATTR] = BG_GRAY;
    win1->bdr_attrs[FG]   = 10;
    win1->bdr_attrs[BG]   = 6;
    win1->bdr_type        = BDR_SINGLE;

    win2->bdr_attrs[ATTR] = BG_GRAY;
    win2->bdr_attrs[FG]   = 10;
    win2->bdr_attrs[BG]   = 6;
    win2->bdr_type        = BDR_SINGLE;

    win_clear(win1);
    win_clear(win2);

    scr_win_attach(scr, win1);
    scr_win_attach(scr, win2);
    scr_do_draw_screen(scr);
    cup(20, 0);

    run_demo(scr, win1, win2);

    set_fg(WHITE);
    set_bg(BLACK);
    read(STDIN_FILENO, &c, 1);
    clear();
    cup(10, 0);
    printf("bye!\n");
    return 0;
}

// =======================================================================
