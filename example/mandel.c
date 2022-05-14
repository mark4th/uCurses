
#include <inttypes.h>

#include "uCurses.h"
#include "uC_menus.h"
#include "uC_keys.h"
#include "uC_json.h"

#include "demo.h"

#define max 1000

extern screen_t *active_screen;

// -----------------------------------------------------------------------

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb;

rgb palette[max];

// -----------------------------------------------------------------------

int dots_sin(int16_t angle);

static void make_palette(void)
{
    int8_t r, g, b;
    int i;
    double q;

    for (int i = 0; i < max; i++)
    {
        q = 3.1415 * (i + 10);

        r = (dots_sin(q / 16));
        g = (dots_sin(q / 19));
        b = (dots_sin(q / 21));

        palette[i].r = r;
        palette[i].g = g;
        palette[i].b = b;
    }
}

// -----------------------------------------------------------------------

static unsigned mandel_sqrt(int val)
{
    int a, b;

    if(val < 2)
    {
        return val;
    }

    // starting point is relatively unimportant
    a = 1255;

    b = val / a;    a = (a + b) / 2;
    b = val / a;    a = (a + b) / 2;
    b = val / a;    a = (a + b) / 2;
    b = val / a;    a = (a + b) / 2;

    return a;
}

// -----------------------------------------------------------------------

static void mandel(window_t *win)
{
    int32_t c, r, i;
    int16_t w = win->width;
    int16_t h = win->height;
    uint8_t R, G, B;

    double x, y, xn;
    double re, im;
    int cc;

    for (r = 0; r < h; r++)
    {
        for (c = 0; c < w; c++)
        {
            re = (c - w / 2.0) * 5.2 / w;
            im = (r - h / 2.0) * 8.3 / w;

            x = 0;
            y = 0;
            i = 0;

            while (((x * x + y * y) <= 4) && (i < max))
            {
                xn = x * x - y * y + re;
                y  = 2 * x * y + im;
                x  = xn;
                i++;
            }

            uC_win_cup(win, c, r);

            R = palette[i].r;
            G = palette[i].g;
            B = palette[i].b;

            uC_win_set_rgb_fg(win, R, G, B);

            uC_win_emit(win, (i < max) ? 0x2981 : ' ');
        }
    }
}

// -----------------------------------------------------------------------

void mandel_demo(void)
{
    int i;
    screen_t *scr;
    window_t *win;
    node_t *n;

    char status[MAX_STATUS];

    uC_scr_close(active_screen);
    uC_json_create_ui("dots.json", menu_address_cb);
    make_palette();
    uC_alloc_status();
    uC_bar_clr_status();
    uC_menu_init();

    scr = active_screen;
    n   = scr->windows.head;
    win = n->payload;

    mandel(win);
    uC_scr_draw_screen(scr);
    do
    {
        while(uC_test_keys() == 0)
            ;
    } while((uC_key() != 0x1b));

    uC_scr_close(active_screen);
    main_screen();
}

// =======================================================================

