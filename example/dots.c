// dots.c
// -----------------------------------------------------------------------

#include <sys/time.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "demo.h"


// -----------------------------------------------------------------------

#define MAX_POINTS 125          // max # pels in an object

static uC_screen_t *scr;
static uC_window_t *win;
static uC_window_t *status_win;

extern uC_screen_t *active_screen;
extern int16_t dots_sin_tab[512];
extern int point_counts[];             // number of points in each object
extern xyz *obj_list[];                // pointers to each object

static int16_t dots_cos(int16_t angle);
static int16_t dots_sin(int16_t angle);

static int frame;

static int32_t pel;

static int zmin;
static int zmax;

char status[33];

static int32_t pels[] =
{
    0x1390, 0x1390, 0x1390, 0x1390,    // ᎐
    0x1428, 0x1428, 0x1428, 0x1428,    // ᐩ
    0x2022, 0x2022, 0x2022, 0x2022,    // •
    0x2981, 0x2981, 0x2981, 0x2981,    // ⦁
    0x25C6, 0x25C6, 0x25C6, 0x25C6,    // ◆
    0x25CF, 0x25CF, 0x25CF, 0x25CF,    // ●
};

// -----------------------------------------------------------------------

static int x_angle = 0;            // angles of rotation in each axis
static int y_angle = 20;
static int z_angle = 45;

static short cos_x = 0;            // trig stuff
static short cos_y = 0;
static short cos_z = 0;
static short sin_x = 0;
static short sin_y = 0;
static short sin_z = 0;

static int x_off = 30;             // world space position of object
static int y_off = 30;
static int z_off = 200;

static int delta_x = 1;            // rotational speed of object
static int delta_y = 1;
static int delta_z = 1;

static int num_points = 0;         // number of points in object
static int obj_number = 0;

static xyz *object = NULL;         // pointer to current object

// -----------------------------------------------------------------------

static modifier w1 =               // changes x rotation speed
{
    30,                     // count down counter
    34,                     // reset value for countdown counter
    &delta_x,               // item to modify on count = 0
    1,                      // ammount to add to item
    5,                      // upper limit for item
    1,                      // lower limit for item
};

static modifier w2 =                   // changes y rotation speed
{
    20, 20, &delta_y, 1, 5, 1
};

static modifier w3 =                   // changes z rotation speed
{
    40, 30, &delta_z, 1, 5, 1
};

static modifier w4 =                   // zooms object in / out of window
{
    4, 4, &z_off, -1, 200, 50
};

// -----------------------------------------------------------------------
// draw a point at x/y in specified colour

static int px, py, pz;         // 3d coordinates to rotate

static void draw_point(int16_t x, int16_t y, int8_t c)
{
    cell_t *p;

    if (x != -1)
    {
        uC_win_cup(win, x, y);

        p = uC_win_peek(win);

        if (pel > p->code)
        {
            uC_win_set_rgb_fg(win,
                (z_off - px),
                (z_off - py),
                (z_off - pz));
            uC_win_emit(win, pel);
        }
    }
}

// -----------------------------------------------------------------------
// pre calculate sin and cosine values for x y and z angles of rotation

static void dots_sincos(void)
{
    sin_x = dots_sin(x_angle);
    sin_y = dots_sin(y_angle);
    sin_z = dots_sin(z_angle);

    cos_x = dots_cos(x_angle);
    cos_y = dots_cos(y_angle);
    cos_z = dots_cos(z_angle);
}

// -----------------------------------------------------------------------
// roatate object about x y and z axis (in object space)

static void rotate(int *px, int *py, int *pz)
{
    int tx, ty, tz;        // temp store

    if (x_angle != 0)      // rotate point about x axis...
    {
        ty = (*py * cos_x) - (*pz * sin_x);
        tz = (*py * sin_x) + (*pz * cos_x);

        *py = (ty >> 14);   // sin table is scaled up so we
        *pz = (tz >> 14);   // must re scale all results down
    }

    if (y_angle)            // rotate point about y axis
    {
        tx = (*px * cos_y) - (*pz * sin_y);
        tz = (*px * sin_y) + (*pz * cos_y);

        *px = (tx >> 14);
        *pz = (tz >> 14);
    }

    if (z_angle)            // rotate point about z axis
    {
        tx = (*px * cos_z) - (*py * sin_z);
        ty = (*px * sin_z) + (*py * cos_z);

        *px = (tx >> 14);
        *py = (ty >> 14);
    }
}

// -----------------------------------------------------------------------
// project point in 3d space onto plane in 2d space

static void project(int px, int py, int pz, int *x, int *y)
{
    int tx, ty;             // temp store...


    *x = *y = -1;           // assume point is clipped

    if ((z_off + pz) < 1)
    {
        return;
    }

    ty = ((y_off * py) / (z_off + pz)) + (win->height / 2);

    if ((ty > 0) && (ty < (28)))
    {
        tx = ((x_off * px) / (z_off + pz)) + (win->width / 2);

        if ((tx > 0) && (tx < 96))
        {
            *x = tx;
            *y = ty;
        }
    }
}

// -----------------------------------------------------------------------
// draw one frame of object...

#define min(a,b) (a < b) ? a : b;
#define max(a,b) (a > b) ? a : b;

static void do_frame(void)
{
    int x, y, c;            // 2d coordiantes of point and colour
    int i;

    dots_sincos();          // calculate all sin/cos values

    uC_win_clear(win);

    for (i = 0; i < num_points; i++)
    {
        px = object[i].x;   // collect point from object
        py = object[i].y;
        pz = object[i].z;

        // rotate this point about x/y and z axis
        // then project 3d coordinates into 2d space
        rotate(&px, &py, &pz);
        project(px, py, pz, &x, &y);

        zmin = min(zmin, pz);
        zmax = max(zmax, pz);

        int w = abs(zmin);
        int q = (zmax + w) / 15;

        if (q == 0) { q = 1; }

        c = ((pz + w) / q) % 24;

        pel = pels[(24 - c)];

        draw_point(x, y, c);
    }
    frame++;
}

// -----------------------------------------------------------------------
// adjust rotational speeds / distance between min and max for each

static void modify(modifier *mod)
{
    mod->counter--;

    if (!mod->counter)
    {
        mod->counter = mod->reset;

        *mod->ptr += mod->delta;

        if (*mod->ptr >= mod->upper || *mod->ptr <= mod->lower)
        {
            mod->delta = -(mod->delta);
        }
    }
}

// -----------------------------------------------------------------------
// do the above on each of the 4 modifiers

static void do_deltas(void)
{
    modify(&w1);            // modify x rotational speed
    modify(&w2);            // modify y rotational speed
    modify(&w3);            // modify z rotational speed
    modify(&w4);            // zoom object in and out
}

// -----------------------------------------------------------------------
// adjust x y and z angles of ritation for next frame

static void change_angles(void)
{
    x_angle += delta_x;
    y_angle += delta_y;
    z_angle += delta_z;

    x_angle &= 0x1ff;
    y_angle &= 0x1ff;
    z_angle &= 0x1ff;
}

// -----------------------------------------------------------------------

void do_dots(void)
{
    int seconds;
    time_t start;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    start = tv.tv_sec;

    int fps;

    for (;;)               // only way out is to die
    {
        if (obj_number == NUM_OBJECTS)
        {
            obj_number = 0;
        }

        object     = obj_list[obj_number];
        num_points = point_counts[obj_number++];

        int itters = 0;
        char c;
        int pause = 0;

        zmin = 1000;
        zmax = -1000;

        while (itters++ != 2000000)
        {
            if (pause == 0)
            {
                gettimeofday(&tv, NULL);
                seconds = (tv.tv_sec - start);

                snprintf(status, 32,
                    "F: %8dk S: %5d FPS: %2dK",
                    frame / 1000, seconds, fps/1000);

                uC_set_status(status_win, status);

                do_frame();        // draw object

                if (seconds != 0)
                {
                    fps = frame / seconds;
                }

                if ((itters & 0x3ff) == 0)
                {
                    change_angles();   // adjust angles of rotation
                    do_deltas();       // modify rotation speeds etc
                }

                uC_scr_draw_screen(scr);
            }
            if (uC_test_keys() != 0)
            {
                c = uC_key();
                if (c == 0x20)
                {
                    pause ^= 1;
                    continue;
                }
                if (c == 0x1b)
                {
                    break;
                }
            }
        }
        if (c == 0x1b)
        {
            break;
        }
    }
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

    uCurses_init();
    uC_json_file_create_ui("json/dots.json", menu_address_cb);
    uC_menu_init();

    scr = active_screen;
    n   = scr->windows.head;
    win = n->payload;

    status_win = uC_add_status(scr, 32, 55, 0);

    uC_win_printf(status_win, "%fs%bs%0", 9, 3);

    uC_set_status(status_win, status);
    uC_clr_status(status_win);

    do_dots();

    uC_scr_close(active_screen);

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(10, 0);

    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================
