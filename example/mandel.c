
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_keys.h"
#include "uC_braille.h"
#include "uC_win_printf.h"
#include "uC_braille.h"
#include "uC_status.h"
#include "uC_alloc.h"

#include "demo.h"

// -----------------------------------------------------------------------

#define IMAX 1000


uC_window_t *status_win = NULL;
uC_screen_t *scr        = NULL;
uC_window_t *win        = NULL;

// -----------------------------------------------------------------------

long double MinRe = -3.0;
long double MaxRe =  4.0;
long double MinIm = -2.2;

long double MaxIm;
long double Re_factor;
long double Im_factor;

long double x_off = 2.5;
long double y_off = -2;
long double z_off = 1.0;

long double scale_factor = 0;

// -----------------------------------------------------------------------

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb;

rgb palette[IMAX];

uint16_t r, g, b;

// -----------------------------------------------------------------------

static void make_palette(void)
{
    int i;
    long double q;
    long double PI = 3.1415;

    for (i = 0; i < IMAX; i++)
    {
        q = PI * (i + 10);

        r = (uint16_t)(128.0L + 127.0L * sinl(q / 16.0L));
        g = (uint16_t)(128.0L + 127.0L * sinl(q / 19.0L));
        b = (uint16_t)(128.0L + 127.0L * sinl(q / 21.0L));

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
//     if (val < 2)
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

uint8_t peek(rgb* buffer, /*rgb *fg, */int x, int y, int width)
{
    rgb *p;
    int xx, yy;
    uint8_t mask = 1;
    uint8_t c = 0;

    r = g = b = 0;

    // super sample
    for (xx = 0; xx < 2; xx++)
    {
        for (yy = 0; yy < 4; yy++)
        {
            p = buffer + (xx + x) + ((yy + y) * width);

            r += p->r;
            g += p->g;
            b += p->b;

            if ((p->r != 0) || (p->g != 0) || (p->b != 0))
            {
                c |= mask;
            }
            mask <<= 1;
        }
    }

    r = (r - 7) / 8;
    g = (g - 7) / 8;
    b = (b - 7) / 8;

    // fg->r = ((r - 7 ) / 8);
    // fg->g = ((g - 7 ) / 8);
    // fg->b = ((b - 7 ) / 8);

    return c;
}

// -----------------------------------------------------------------------

static void draw_braille(uC_window_t *win, rgb *buffer,
    int width, int height)
{
    int x, y;
    uint8_t c = 0;
    uint16_t cc;

    rgb fg;

    for (y = 0; y < height; y += 4)
    {
        for (x = 0; x < width; x += 2)
        {
            c = peek(buffer, /*&fg, */x, y, width);
            cc = uC_braille_xlat(c);

            // %@ set cursor location within window
            // %rf set window foreground to a 24 bit rgb color

            uC_win_printf(win, "%@%rf",
                UC_XY(x / 2, y / 4),
                UC_RGB(r, g, b));
                //fg.r, fg.g, fg.b);

            uC_win_emit(win, cc);
        }
    }
}

// -----------------------------------------------------------------------

static void mandel(uC_window_t *win, long double x_off,
    long double y_off,
    long double z_off)
{
    int width  = win->width  * 2;
    int height = win->height * 4;

    long double h2 = height / 2;
    long double w2 = width  / 2;

    int r, c, n;

    long double c_re;
    long double c_im;
    long double Z_re;
    long double Z_im;
    long double Z_re2;
    long double Z_im2;

    bool isInside;

    Re_factor = (MaxRe - MinRe) / (width  - 1) * z_off;
    Im_factor = (MaxIm - MinIm) / (height - 1) * z_off;

    MaxIm = MinIm + (MaxRe - MinRe) * height / width;

    rgb *buffer = uC_alloc(uC_MEM_ZONE_DEFAULT,
        width * height * sizeof(rgb));

    for (r = 0; r < height; r++)
    {
        c_im = MaxIm - ((r + -h2) * Im_factor) + y_off;

        for (c = 0; c < width; c++)
        {
            c_re = MinRe + ((c + -w2) * Re_factor) + x_off;

            Z_re = c_re;
            Z_im = c_im;

            isInside = true;

            for (n = 0; n < IMAX; n++)
            {
                Z_re2 = Z_re * Z_re;
                Z_im2 = Z_im * Z_im;

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
    uC_free(uC_MEM_ZONE_DEFAULT, buffer);
}

// -----------------------------------------------------------------------

static void lt(void)
{
    x_off += 1 / powl(2, scale_factor) * 0.02;
}

static void rt(void)
{
    x_off -= 1 / powl(2, scale_factor) * 0.02;
}

static void up(void)
{
    y_off += 1 / powl(2, scale_factor) * 0.02;
}

static void dn(void)
{
    y_off -= 1 / powl(2, scale_factor) * 0.02;
}

static void zi(void)
{
    scale_factor += 0.02;
    if (scale_factor > 54.0L) { scale_factor = 54.0L; }
    z_off = 1 / powl(2, scale_factor);
}

static void zo(void)
{
    scale_factor -= 0.02;
    z_off = 1 / powl(2, scale_factor);
}

// -----------------------------------------------------------------------

uC_switch_t mandel_keys[] =
{
     { 'd', lt },  { 'a', rt },  { 'w', up },
     { 's', dn },  { '=', zi },  { '-', zo },
};

// -----------------------------------------------------------------------

static void update_status(void)
{
    char xy[STAT_SIZE];
    char z[16];

    snprintf(xy, sizeof(xy), "X:%1.8Lf  Y:%1.8Lf  ", x_off, y_off);
    snprintf(z,  sizeof(z),  "Z:%3.2Lf", scale_factor);

    if (scale_factor >= 54.0L)
    {
        uC_win_printf(status_win, "%0%fs%s%rf%s", uC_GRAY_09, xy, UC_RGB(255, 140, 0), z);
    }
    else
    {
        uC_win_printf(status_win, "%0%fs%s%s", uC_GRAY_09, xy, z);
    }
}

// -----------------------------------------------------------------------

void mandel_demo(void)
{
    uint8_t k;

    mandel(win, x_off, y_off, z_off);

    do
    {
        update_status();

        mandel(win, x_off, y_off, z_off);

        uC_scr_draw_screen(scr);

        while(uC_test_keys() == 0)
            ;
        k = uC_key();

        uC_switch(mandel_keys, 6, k);

    } while((k != 0x1b));
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
    uC_list_node_t *n;

    scr = uCurses_init("json/dots.json", NULL, menu_address_cb);
    n   = scr->windows.head;
    win = n->payload;

    make_palette();

    status_win = uC_add_status(scr, 40, 55, 0);
    uC_win_printf(status_win, "%fs%bs%0", uC_GRAY_09, uC_GRAY_03);

    mandel_demo();

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(10, 0);

    uC_scr_close(scr);
    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}


// =======================================================================

