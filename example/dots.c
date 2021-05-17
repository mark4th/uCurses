// dots.c
// -----------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include "../h/uCurses.h"
#include "demo.h"

// -----------------------------------------------------------------------

#define MAX_POINTS 125          // max # pels in an object

static window_t *dots_win;

extern int16_t dots_sin_tab[512];
extern int point_counts[];             // number of points in each object
extern xyz *obj_list[];                // pointers to each object

int16_t dots_cos(int16_t angle);
int16_t dots_sin(int16_t angle);

int frame;

int32_t pel;

int zmin;
int zmax;

int32_t pels[] =
{
    0x1390, 0x1390, 0x1390, 0x1390,    // ᎐
    0x1428, 0x1428, 0x1428, 0x1428,    // ᐩ
    0x1540, 0x1540, 0x1540, 0x1540,    // ᕁ
    0x2981, 0x2981, 0x2981, 0x2981,    // ⦁
    0x29fb, 0x29fb, 0x29fb, 0x29fb,    // ⧻
    0x2a68, 0x2a68, 0x2a68, 0x2a68,    // ⩨
};

// -----------------------------------------------------------------------

int x_angle = 0;            // angles of rotation in each axis
int y_angle = 20;
int z_angle = 45;

short cos_x = 0;                // trig stuff
short cos_y = 0;
short cos_z = 0;
short sin_x = 0;
short sin_y = 0;
short sin_z = 0;

int x_off = 30;             // world space position of object
int y_off = 30;
int z_off = 200;

int delta_x = 1;            // rotational speed of object
int delta_y = 1;
int delta_z = 1;

int num_points = 0;         // number of points in object
int obj_number = 0;

xyz *object = NULL;         // pointer to current object

// -----------------------------------------------------------------------

modifier w1 =               // changes x rotation speed
{
    30,                     // count down counter
    34,                     // reset value for countdown counter
    &delta_x,               // item to modify on count = 0
    1,                      // ammount to add to item
    5,                      // upper limit for item
    1,                      // lower limit for item
};

modifier w2 =                   // changes y rotation speed
{
    20, 20, &delta_y, 1, 5, 1
};

modifier w3 =                   // changes z rotation speed
{
    40, 30, &delta_z, 1, 5, 1
};

modifier w4 =                   // zooms object in / out of window
{
    4, 4, &z_off, -1, 200, 50
};

// -----------------------------------------------------------------------
// draw a point at x/y in specified colour
    int px, py, pz;         // 3d coordinates to rotate

void draw_point(int16_t x, int16_t y, int8_t c)
{
    cell_t *p;

    if(x != -1)
    {
        win_cup(dots_win, x, y);

        p = win_peek(dots_win);

        if(pel > p->code)
        {
            win_set_rgb_fg(dots_win,
                (z_off - px),
                (z_off - py),
                (z_off - pz));
            win_emit(dots_win, pel);
        }
    }
}

// -----------------------------------------------------------------------
// pre calculate sin and cosine values for x y and z angles of rotation

void dots_sincos(void)
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

void rotate(int *px, int *py, int *pz)
{
    int tx, ty, tz;        // temp store

    if(x_angle != 0)      // rotate point about x axis...
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

void project(int px, int py, int pz, int *x, int *y)
{
    int tx, ty;             // temp store...

    *x = *y = -1;           // assume point is clipped

    if((z_off + pz) < 1)
    {
        return;
    }

    ty = ((y_off * py) / (z_off + pz)) + 10;

    if((ty > 0) && (ty < (25)))
    {
        tx = ((x_off * px) / (z_off + pz)) + 40;

        if((tx > 0) && (tx < 96))
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

void do_frame(void)
{
    int x, y, c;            // 2d coordiantes of point and colour
    int i;

    dots_sincos();          // calculate all sin/cos values

    win_clear(dots_win);

    for(i = 0; i < num_points; i++)
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

        if(q == 0) { q = 1; }

        c = ((pz + w) / q) % 24;
        c = 22 - c;

        pel = pels[(c)];

        draw_point(x, y, c);
    }
    frame++;
}

// -----------------------------------------------------------------------
// adjust rotational speeds / distance between min and max for each

void modify(modifier *mod)
{
    mod->counter--;

    if(!mod->counter)
    {
        mod->counter = mod->reset;

        *mod->ptr += mod->delta;

        if(*mod->ptr >= mod->upper || *mod->ptr <= mod->lower)
        {
            mod->delta = -(mod->delta);
        }
    }
}

// -----------------------------------------------------------------------
// do the above on each of the 4 modifiers

void do_deltas(void)
{
    modify(&w1);            // modify x rotational speed
    modify(&w2);            // modify y rotational speed
    modify(&w3);            // modify z rotational speed
    modify(&w4);            // zoom object in and out
}

// -----------------------------------------------------------------------
// adjust x y and z angles of ritation for next frame

void change_angles(void)
{
    x_angle += delta_x;
    y_angle += delta_y;
    z_angle += delta_z;

    x_angle &= 0x1ff;
    y_angle &= 0x1ff;
    z_angle &= 0x1ff;
}

// -----------------------------------------------------------------------

#include <sys/time.h>

void do_dots(void)
{
    screen_t *scr;
    node_t *n;
    char status[MAX_STATUS];
    int seconds;
    time_t start;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    start = tv.tv_sec;

    json_create_ui("dots.json", menu_address_cb);

    alloc_status();
    bar_clr_status();
    menu_init();
    int fps;

    scr = active_screen;
    n = scr->windows.head;
    dots_win = n->payload;

    for(;;)               // only way out is to die
    {
        if(obj_number == NUM_OBJECTS)
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

        while(itters++ != 2000000)
        {
            if(pause == 0)
            {
                gettimeofday(&tv, NULL);
                seconds = (tv.tv_sec - start);

//        snprintf(status, MAX_STATUS,
//            "min: %3d : max: %3d", zmin, zmax);

                snprintf(status, MAX_STATUS,
                    "F: %8dk S: %5d FPS: %2dK",
                    frame / 1000, seconds, fps/1000);

                bar_set_status(status);

                do_frame();        // draw object

                if(seconds != 0)
                {
                    fps = frame / seconds;
                }

                if((itters & 0x3ff) == 0)
                {
                    change_angles();   // adjust angles of rotation
                    do_deltas();       // modify rotation speeds etc
                }

                scr_draw_screen(scr);
            }
            if(test_keys() != 0)
            {
                c = key();
                if(c == 0x20)
                {
                    pause ^= 1;
                    continue;
                }
                if(c == 0x1b)
                {
                    break;
                }
            }
        }
        if(c == 0x1b)
        {
            break;
        }
    }
}

// =======================================================================
