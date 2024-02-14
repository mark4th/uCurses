
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_menus.h"
#include "uC_keys.h"
#include "uC_json.h"
#include "uC_braille.h"
#include "uC_win_printf.h"
#include "uC_braille.h"

#include "demo.h"

// -----------------------------------------------------------------------

#define max 1000

extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------

double MinRe = -3.0;
double MaxRe =  4.0;
double MinIm = -2.2;

double MaxIm;
double Re_factor;
double Im_factor;

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

    for (i = 0; i < max; i++)
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

// static unsigned mandel_sqrt(int val)
// {
//     int a, b;
//
//     if(val < 2)
//     {
//         return val;
//     }
//
//     // starting point is relatively unimportant
//     a = 1255;
//
//     b = val / a;    a = (a + b) / 2;
//     b = val / a;    a = (a + b) / 2;
//     b = val / a;    a = (a + b) / 2;
//     b = val / a;    a = (a + b) / 2;
//
//     return a;
// }

// -----------------------------------------------------------------------

uint8_t peek(rgb* buffer, rgb *fg, int x, int y, int width)
{
    rgb *p;
    int xx, yy;
    uint8_t mask = 1;
    uint8_t c = 0;
    int r, g, b;

    for (xx = 0; xx < 2; xx++)
    {
        for (yy = 0; yy < 4; yy++)
        {
            p = buffer + (xx + x) + ((yy + y) * width);

            r += p->r;  g += p->g;   b += p->b;

            if ((p->r != 0) || (p->g != 0) || (p->b != 0))
            {
                c |= mask;
            }
            mask <<= 1;
        }
    }
    fg->r = (r / 8);
    fg->g = (g / 8);
    fg->b = (b / 8);

    return c;
}

// -----------------------------------------------------------------------

static void draw_braille(uC_window_t *win, rgb *buffer, int width, int height)
{
    int x, y;
    uint8_t c = 0;
    uint16_t cc;

    rgb fg;

    for (y = 0; y < height; y += 4)
    {
        for (x = 0; x < width; x += 2)
        {
            c = peek(buffer, &fg, x, y, width);
            cc = uC_braille_xlat(c);

            uC_win_printf(win, "%@%rf", x / 2, y / 4, fg.r, fg.g, fg.b);
            uC_win_emit(win, cc);
        }
    }
}

// -----------------------------------------------------------------------

static void mandel(uC_window_t *win, double x_off, double y_off, double z_off)
{
    int width  = win->width  * 2;
    int height = win->height * 4;

    double h2 = height / 2;
    double w2 = width / 2;

    Re_factor = (MaxRe - MinRe) / (width  - 1) * z_off;
    Im_factor = (MaxIm - MinIm) / (height - 1) * z_off;

    MaxIm = MinIm + (MaxRe - MinRe) * height / width;

    rgb *buffer = calloc(width * height, sizeof(rgb));

    int r, c, n;

    for (r = 0; r < height; r++)
    {
        double c_im = MaxIm - ((r + -h2) * Im_factor) + y_off;

        for (c = 0; c < width; c++)
        {
            double c_re = MinRe + ((c + -w2) * Re_factor) + x_off;

            double Z_re = c_re;
            double Z_im = c_im;

            bool isInside = true;

            for (n = 0; n < max; n++)
            {
                double Z_re2 = Z_re * Z_re;
                double Z_im2 = Z_im * Z_im;

                if (Z_re2 + Z_im2 > 4)
                {
                    isInside = false;
                    break;
                }
                Z_im = (2 * Z_re * Z_im) + c_im;
                Z_re = Z_re2 - Z_im2 + c_re;
            }

            if (!isInside)
            {
                buffer[c + (r * width)] = palette[n];
            }
        }
    }

    draw_braille(win, buffer, width, height);
    free(buffer);
}

// -----------------------------------------------------------------------

void mandel_demo(void)
{
    int i;
    uC_screen_t *scr;
    uC_window_t *win;
    uC_list_node_t *n;

    double x_off = 2.5;
    double y_off = -2;
    double z_off = 1.0;

    double scale_factor = 0;
    double sf = .0002;

    uint8_t k;

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

    do
    {
        snprintf(status, MAX_STATUS,
             "X:%1.9f Y:%1.9f Z:%2.2f",
              x_off, y_off, scale_factor);

        uC_bar_set_status(status);

        mandel(win, x_off, y_off, z_off);

        uC_scr_draw_screen(scr);
        while(uC_test_keys() == 0)
            ;
        k = uC_key();

        if (k == 'd') { x_off += 1 / pow(2, scale_factor) * 0.03; }
        if (k == 'a') { x_off -= 1 / pow(2, scale_factor) * 0.03; }
        if (k == 'w') { y_off += 1 / pow(2, scale_factor) * 0.03; }
        if (k == 's') { y_off -= 1 / pow(2, scale_factor) * 0.03; }

        if (k == '=')
        {
            scale_factor += 0.03;
            z_off = 1 / pow(2, scale_factor);
        }
        if (k == '-')
        {
            scale_factor -= 0.03;
            z_off = 1 / pow(2, scale_factor);
        }
    } while((k != 0x1b));

    uC_scr_close(active_screen);
    main_screen();
}

// =======================================================================

