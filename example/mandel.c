
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

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
#define MANDEL_MAX_THREADS 32


uC_window_t *status_win = NULL;
uC_screen_t *scr        = NULL;
uC_window_t *win        = NULL;

// -----------------------------------------------------------------------

long double MinRe = -3.0L;
long double MaxRe =  4.0L;
long double MinIm = -2.2L;

long double MaxIm;
long double Re_factor;
long double Im_factor;

long double x_off = 2.5L;
long double y_off = -2.0L;
long double z_off = 1.0L;

long double scale_factor = 0.0L;

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
    const long double pi = 3.141592653589793238462643383279502884197L;

    for (i = 0; i < IMAX; i++)
    {
        q = pi * (i + 10);

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

typedef struct
{
    rgb *buffer;
    int width;
    int first_row;
    int last_row;
    long double h2;
    long double w2;
    long double x_offset;
    long double y_offset;
} mandel_worker_t;

static void *render_mandel_rows(void *argument)
{
    mandel_worker_t *worker = argument;

    for (int row = worker->first_row; row < worker->last_row; row++)
    {
        long double c_im = MaxIm - ((row - worker->h2) * Im_factor) + worker->y_offset;

        for (int column = 0; column < worker->width; column++)
        {
            long double c_re = MinRe + ((column - worker->w2) * Re_factor) + worker->x_offset;
            long double z_re = c_re;
            long double z_im = c_im;
            int iteration;

            for (iteration = 0; iteration < IMAX; iteration++)
            {
                const long double z_re2 = z_re * z_re;
                const long double z_im2 = z_im * z_im;

                if (z_re2 + z_im2 > 4.0L)
                {
                    break;
                }
                z_im = (2.0L * z_re * z_im) + c_im;
                z_re = z_re2 - z_im2 + c_re;
            }

            if (iteration < IMAX)
            {
                worker->buffer[column + (row * worker->width)] = palette[iteration];
            }
        }
    }

    return NULL;
}

static void mandel(uC_window_t *win, long double x_offset,
    long double y_offset,
    long double zoom)
{
    const int width = win->width * 2;
    const int height = win->height * 4;
    const long double h2 = (long double)height / 2.0L;
    const long double w2 = (long double)width / 2.0L;
    long available_threads = sysconf(_SC_NPROCESSORS_ONLN);
    int thread_count = (available_threads > 0) ? (int)available_threads : 1;

    if (thread_count > MANDEL_MAX_THREADS)
        thread_count = MANDEL_MAX_THREADS;
    if (thread_count > height)
        thread_count = height;

    MaxIm = MinIm + (MaxRe - MinRe) * (long double)height / (long double)width;
    Re_factor = (MaxRe - MinRe) / (long double)(width - 1) * zoom;
    Im_factor = (MaxIm - MinIm) / (long double)(height - 1) * zoom;

    rgb *buffer = uC_alloc(uC_MEM_ZONE_DEFAULT, width * height * sizeof(*buffer));
    pthread_t threads[MANDEL_MAX_THREADS];
    mandel_worker_t workers[MANDEL_MAX_THREADS];

    for (int thread = 0; thread < thread_count; thread++)
    {
        workers[thread] = (mandel_worker_t){
            .buffer = buffer,
            .width = width,
            .first_row = (height * thread) / thread_count,
            .last_row = (height * (thread + 1)) / thread_count,
            .h2 = h2,
            .w2 = w2,
            .x_offset = x_offset,
            .y_offset = y_offset,
        };
        pthread_create(&threads[thread], NULL, render_mandel_rows, &workers[thread]);
    }

    for (int thread = 0; thread < thread_count; thread++)
        pthread_join(threads[thread], NULL);

    draw_braille(win, buffer, width, height);
    uC_free(uC_MEM_ZONE_DEFAULT, buffer);
}

// -----------------------------------------------------------------------

static void lt(void)
{
    x_off += 1.0L / powl(2.0L, scale_factor) * 0.02L;
}

static void rt(void)
{
    x_off -= 1.0L / powl(2.0L, scale_factor) * 0.02L;
}

static void up(void)
{
    y_off += 1.0L / powl(2.0L, scale_factor) * 0.02L;
}

static void dn(void)
{
    y_off -= 1.0L / powl(2.0L, scale_factor) * 0.02L;
}

static void zi(void)
{
    scale_factor += 0.02L;
    if (scale_factor > 54.0L) { scale_factor = 54.0L; }
    z_off = 1.0L / powl(2.0L, scale_factor);
}

static void zo(void)
{
    scale_factor -= 0.02L;
    z_off = 1.0L / powl(2.0L, scale_factor);
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
    snprintf(z, sizeof(z), "Z:%3.2Lf", scale_factor);

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

int main(void)
{
    uC_list_node_t *n;

    scr = uCurses_init("json/dots.json", NULL, NULL);
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
