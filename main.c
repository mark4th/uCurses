// test main
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <locale.h>

#include "h/color.h"
#include "h/tui.h"
#include "h/uCurses.h"
#include "h/util.h"

// -----------------------------------------------------------------------

struct termios term_save;
struct termios term;

// -----------------------------------------------------------------------

#define SLEEP 10000000
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

void restore_term(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &term_save);
    curon();
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
// rotates two windows around the screen in opposite directions
// can this even be done wtih ncurses?

void run_demo(screen_t *scr, window_t *win1, window_t *win2)
{
    uint8_t c;
    uint8_t pause = 0;
    uint16_t x1 = 2, y1 = 2;
    uint16_t x2 = X_END(win2), y2 = Y_END(win2);

    uint16_t x1i, x2i;
    uint16_t y1i, y2i;

    x1i = 1;
    x2i = -1;

    y1i = y2i = 0;

    for(;;)
    {
        if(test_keys() != 0)
        {
            read(STDIN_FILENO, &c, 1);
            if(c == ' ')
            {
                pause ^= 1;
                continue;
            }
            return;
        }

        if(pause == 0)
        {
            win_set_pos(win1, x1, y1);
            win_set_pos(win2, x2, y2);
            scr_do_draw_screen(scr);

            x1 += x1i;    y1 += y1i;
            x2 += x2i;    y2 += y2i;

            if(y1i == 0)
            {
                if((x1 == X_END(win1)) || (x1 == 2))
                {
                    flip_flop(win1, win2);
                    y1i = x1i;    x1i = 0;
                    y2i = x2i;    x2i = 0;
                    continue;
                }
            }

            if(x1i == 0)
            {
                if((y1 == Y_END(win1)) || (y1 == 2))
                {
                    x1i = -y1i;    y1i = 0;
                    x2i = -y2i;    y2i = 0;
                }
            }

        }
        clock_sleep(SLEEP);
    }
}

// -----------------------------------------------------------------------

// void test(void);

int main(void)
{
//    test();
//    return 0;
    struct winsize w;
    screen_t *scr;
    window_t *win1, *win2;

    uCurses_init();
    setlocale(LC_ALL, "C.UTF-8");
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
    win_set_fg(win2, LT_CYAN);
    win_set_bg(win2, GRAY);

    win1->flags |= WIN_BOXED;
    win2->flags |= WIN_BOXED;

    win1->bdr_attrs[ATTR] = BG_GRAY;
    win1->bdr_attrs[FG]   = 10;
    win1->bdr_attrs[BG]   = 6;
    win1->bdr_type        = BDR_SINGLE;

    win2->bdr_attrs[ATTR] = BG_GRAY;
    win2->bdr_attrs[FG]   = 11;
    win2->bdr_attrs[BG]   = 6;
    win2->bdr_type        = BDR_SINGLE;

    win_clear(win1);
    win_clear(win2);

    scr_win_attach(scr, win1);
    scr_win_attach(scr, win2);

    cup(20, 0);

    char str2[] = "中文鍵盤中文键盘";

    win_printf(win1, "This is a test\r%fcI hope it works!", 9);
    win_puts(win2, str2);
    scr_do_draw_screen(scr);

    run_demo(scr, win1, win2);

    set_fg(WHITE);
    set_bg(BLACK);

    clear();
    cup(10, 0);
    printf("bye!\n");
    return 0;
}


// =======================================================================
