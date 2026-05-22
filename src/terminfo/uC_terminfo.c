// uC_terminfo.c
// -----------------------------------------------------------------------

#include <string.h>
#include <unistd.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_screen.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;

static uC_screen_t *ti_screen;

void ti_set_screen(uC_screen_t *scr)
{
    ti_screen = scr;
}

// -----------------------------------------------------------------------

API void uC_curoff(void) { ti_civis(); }   // turn cursor off
API void uC_curon(void)  { ti_cnorm(); }   // turn cursor on

// -----------------------------------------------------------------------
// clear terminal

API void uC_clear(void)
{
    int size;

    if (ti_screen != NULL)
    {
        // this forces a complete redraw of the screen on the next
        // call to uC_scr_draw_screen() (i.e. all cells have changed)

        ti_screen->cx = 0;   // reset position of cursor
        ti_screen->cy = 0;

        size = (ti_screen->width * ti_screen->height *
            sizeof(cell_t));

        memset(ti_screen->buffer2, 0, size);
    }

    ti_clear();                  // wipe display
}

// -----------------------------------------------------------------------
// set cursor position on current line

API void uC_hpa(uint16_t x)  // horizontal position absolute
{
    if (ti_screen != NULL)
    {
        ti_screen->cx = x;
    }

    ti_vars->params[0] = x;
    ti_hpa();
}

// -----------------------------------------------------------------------
// set cursor x/y position in console

API void uC_cup(uint16_t x, uint16_t y)
{
    if (ti_screen != NULL)
    {
        if ((x < ti_screen->width) && (y < ti_screen->height))
        {
            ti_screen->cx = x;
            ti_screen->cy = y;
        }
    }

    ti_vars->params[0] = x;
    ti_vars->params[1] = y;
    ti_cup();
}

// -----------------------------------------------------------------------
// cursor down

API void uC_cud1(void)
{
    if (ti_screen != NULL)
    {
        if (ti_screen->cy != ti_screen->height - 1)
        {
            ti_screen->cy++;
        }
    }
    ti_cud1();
}

// -----------------------------------------------------------------------
// cursor to home location

API void uC_home(void)
{
    if (ti_screen != NULL)
    {
        ti_screen->cx = 0;
        ti_screen->cy = 0;
    }
    ti_home();
}

// -----------------------------------------------------------------------
// cursor back one on line

API void uC_cub1(void)
{
    if (ti_screen != NULL)
    {
        if (ti_screen->cx != 0)
        {
            ti_screen->cx--;
        }
        else if (ti_screen->cy != 0)
        {
           ti_screen->cy--;
           ti_screen->cx = ti_screen->width - 1;
        }
    }
    ti_cub1();
}

// -----------------------------------------------------------------------
// cursor forward one on line

API void uC_cuf1(void)
{
    if (ti_screen != NULL)
    {
        if (ti_screen->cx != ti_screen->width - 1)
        {
            ti_screen->cx++;
        }
        else if (ti_screen->cy != ti_screen->height - 1)
        {
            ti_screen->cx = 0;
            ti_screen->cy++;
        }
    }
    uC_cup(ti_screen->cx, ti_screen->cy);
}

// -----------------------------------------------------------------------
// cursor up one line

API void uC_cuu1(void)
{
    if (ti_screen != NULL)
    {
        if (ti_screen->cy != 0)
        {
            ti_screen->cy--;
        }
    }
    uC_cup(ti_screen->cx, ti_screen->cy);
}

// -----------------------------------------------------------------------
// delete one character on line

API void uC_dch1(void)
{
    if (ti_screen != NULL)
    {
        if (ti_screen->cx != 0)
        {
            ti_screen->cx--;
        }
    }
    ti_dch();
}

// -----------------------------------------------------------------------
// cursor down multiple lines (this can be done better)

API void uC_cud(uint16_t n1)
{
    int i;

    if (ti_screen != NULL)
    {
        for (i = 0; i != n1; i++)
        {
            if (ti_screen->cy == ti_screen->height - 1)
            {
                break;
            }
            ti_screen->cy++;
        }
    }
    while (n1)
    {
        uC_cud1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// insert space on line

API void uC_ich(void)
{
    ti_ich();
}

// -----------------------------------------------------------------------
// cursor back multiple (this can be done better)

API void uC_cub(uint16_t n1)
{
    if (ti_screen != NULL)
    {
        if (ti_screen->cx != 0)
        {
            ti_screen->cx--;
        }
        else if (ti_screen->cy != 0)
        {
            ti_screen->cx = ti_screen->width - 1;
            ti_screen->cy--;
        }
    }
    while (n1)
    {
        uC_cub1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// cursor forward (this can be done better)

API void uC_cuf(uint16_t n1)
{
    int i;

    if (ti_screen != NULL)
    {
        for (i = 0; i != n1; i++)
        {
            if (ti_screen->cx != ti_screen->width - 1)
            {
                ti_screen->cx++;
            }
            else
            {

                if (ti_screen->cy == ti_screen->height - 1)
                {
                    break;
                }
                ti_screen->cy++;
                ti_screen->cx = 0;
            }
        }
    }

    while (n1)
    {
        uC_cuf1();
        n1--;
    }
}

// -----------------------------------------------------------------------

API void uC_cuu(uint16_t n1)
{
    int i;

    for (i = 0; i != n1; i++)
    {
        if (ti_screen->cy == 0)
        {
            break;
        }
        ti_screen->cy--;
    }
    while (n1)
    {
        uC_cuu1();
        n1--;
    }
}

// -----------------------------------------------------------------------
// vertical position absolute

API void uC_vpa(uint16_t y)
{
    if ((ti_screen != NULL) && (y < ti_screen->height))
    {
        ti_screen->cy  = y;
    }
    ti_vars->params[0] = y;
    ti_vpa();
}

// -----------------------------------------------------------------------

API void uC_cr(void)
{
    char *cr = "\r\n";
    size_t n;

    if (ti_screen != NULL)
    {
        ti_screen->cx = 0;
        if (ti_screen->cy != ti_screen->height)
        {
            ti_screen->cy++;
        }
    }

    n = write(1, cr, 2);
    if (n != 2)
    {
        ;
    }
}

// -----------------------------------------------------------------------
// enable / disable cursor keys

API void uC_smkx(void)
{
    ti_smkx();
}

// -----------------------------------------------------------------------

API void uC_rmkx(void)
{
    ti_rmkx();
}

// =======================================================================
