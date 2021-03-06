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

#define X_END (scr->width  - win1->width  - 2)
#define Y_END (scr->height - win1->height - 2)

uint8_t test_keys(void);

void run_demo(screen_t *scr, window_t *win1)
{
    uint16_t x = 2, y = 2;

    for(;;)
    {
        while (x != X_END)
        {
            if(0 != test_keys())
            {
                return;
            }
            x++;
            win_set_pos(win1, x, y);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };

        while(y != Y_END)
        {
            if(0 != test_keys())
            {
                return;
            }
            y++;
            win_set_pos(win1, x, y);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };

        while(x != 2)
        {
            if(0 != test_keys())
            {
                return;
            }
            x--;
            win_set_pos(win1, x, y);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };

        while(y != 2)
        {
            if(0 != test_keys())
            {
                return;
            }
            y--;
            win_set_pos(win1, x, y);
            scr_do_draw_screen(scr);
            clock_sleep(15000);
        };

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
    window_t *win1;
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

    win1 = win_open(30, 10);
    win1->xco = 2;
    win1->yco = 2;

    win_set_fg(win1, WHITE);
    win_set_bg(win1, BLUE);

    win1->flags |= WIN_BOXED;

    win1->bdr_attrs[ATTR] = BG_GRAY;
    win1->bdr_attrs[FG]   = 10;
    win1->bdr_attrs[BG]   = 6;
    win1->bdr_type        = BDR_SINGLE;

    win_clear(win1);
    scr_win_attach(scr, win1);

    scr_do_draw_screen(scr);
    cup(20, 0);

    run_demo(scr, win1);
    set_fg(WHITE);
    set_bg(BLACK);
    read(STDIN_FILENO, &c, 1);
    clear();
    cup(10, 0);
    printf("bye!\n");
    return 0;
}

// =======================================================================
