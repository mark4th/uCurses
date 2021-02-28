// test main
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>

#include "h/color.h"
#include "h/uCurses.h"
#include "h/util.h"

// -----------------------------------------------------------------------

int main(void)
{
    uint16_t i, j, k;
    setbuf(stdout, NULL);
    uCurses_init();
    printf("\r\n\r\n");

    for(i = 0; i < 20; i++)
    {
        set_gray_fg(i);
        printf("XXXXX\r\n");
    }

    curoff();

    for(i = 0; i < 255; i += 32)
    {
        for(j = 0; j < 255; j += 32)
        {
            for(k = 0; k < 255; k += 32)
            {
                set_rgb_fg(i, j, k);
                printf("X");
            }
        }
    }
    printf("\r\n");
    curon();
    set_fg(WHITE);
    printf("This is normal text\r\n");
    set_fg(BLUE);
    printf("This is blue text\r\n");
    set_fg(WHITE);
    set_bold();
    printf("This is bold text\r\n");
    set_fg(BLUE);
    printf("This is bold blue text\r\n");
    set_fg(WHITE);
    clr_bold();
    set_rev();
    printf("This is reverse text\r\n");
    clr_rev();
    set_ul();
    printf("This is underlined text\r\n");
    clr_ul();
    set_norm();
    set_fg(WHITE);

    return 0;
}

// =======================================================================
