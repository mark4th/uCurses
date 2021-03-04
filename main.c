// test main
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "h/color.h"
#include "h/tui.h"
#include "h/uCurses.h"
#include "h/util.h"

// -----------------------------------------------------------------------

struct termios term_save;
struct termios term;

// -----------------------------------------------------------------------

void restore_term(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &term_save);
}

// -----------------------------------------------------------------------

int main(void)
{
    struct winsize w;
    screen_t *scr;
    window_t *win1;

    uCurses_init();

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
    win1->attrs[ATTR] |= WIN_BOXED;
    win1->
    win1->bdr_attrs[ATTR] = FG_GRAY | BG_GRAY | BOLD;
    win1->bdr_attrs[FG] = 3;
    win1->bdr_attrs[BG] = 0;

    scr_win_attach(scr, win1);

    scr_do_draw_screen(scr);
    cup(10, 0);
    return 0;
}

// =======================================================================
