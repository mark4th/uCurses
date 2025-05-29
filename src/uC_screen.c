// screen.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE // needed to make wcwidth work

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_menus.h"
#include "uC_borders.h"
#include "uC_terminfo.h"
#include "uC_utf8.h"

// -----------------------------------------------------------------------

API uC_screen_t *active_screen;

extern border_t *const borders[];
extern uC_attribs_t attrs;
extern uC_attribs_t old_attrs;

// -----------------------------------------------------------------------

int16_t scr_alloc(uC_screen_t *scr)
{
    cell_t *p1, *p2;

    // allocate buffers 1 and 2 for screen
    p1 = calloc((scr->width * scr->height), sizeof(*p1));
    p2 = calloc((scr->width * scr->height), sizeof(*p2));

    if ((p1 == NULL) || (p2 == NULL))
    {
        free(p1);           // calling free on which ever one of these
        free(p2);           // returned as null performs no operation
        return -1;          // so is safe
    }

    scr->buffer1 = p1;
    scr->buffer2 = p2;

    return 0;
}

// -----------------------------------------------------------------------

API uC_screen_t *uC_scr_open(int16_t width, int16_t height)
{
    uC_screen_t *scr = calloc(1, sizeof(*scr));

    if (scr != NULL)
    {
        scr->width  = width;
        scr->height = height;

        // allocate screen buffer based on width / height

        if (scr_alloc(scr) != 0)
        {
            free(scr);
            scr = NULL;
        }
    }

    return scr;
}

// -----------------------------------------------------------------------
// deallocate all structures attached to screen

API void uC_scr_close(uC_screen_t *scr)
{
    uC_window_t *win;

    if (scr != NULL)
    {
        free(scr->buffer1);
        free(scr->buffer2);

        scr->buffer1 = NULL;
        scr->buffer2 = NULL;

        // safe to call this on a null window
        uC_win_close(scr->backdrop);

        do
        {
            win = uC_list_pop_head(&scr->windows);
            uC_win_close(win);
        } while (win != NULL);

        // do
        // {
        //     close all widget views
        // } while(...);

        uC_bar_close(scr);

        do
        {
            win = uC_list_pop_head(&scr->status);
            uC_win_close(win);
        }
        while (win != NULL);

        free(scr);
    }
}

// -----------------------------------------------------------------------

static cell_t *scr_line_addr(uC_screen_t *scr, int16_t line)
{
    int16_t index = (scr->width) * line;
    return &scr->buffer1[index];
}

// -----------------------------------------------------------------------

static void draw_win_name(uC_window_t *win)
{
    char *p1;
    border_t *b;
    cell_t cell;
    cell_t *p2;
    uC_screen_t *scr;
    uint16_t index, x, y;

    scr = win->screen;

    x = win->xco;
    y = win->yco - 1;

    index  = (y * scr->width) + x;

    p1 = win->display_name;
    p2 = &scr->buffer1[index];

    cell.attrs.chunk = (win->flags & WIN_FOCUS)
        ? win->focus_attrs.chunk
        : win->bdr_attrs.chunk;

    b = borders[win->border_type];

    // draw ┤┫╣ border char to left of name

    cell.code = b[BDR_RIGHT_T];
    *p2++ = cell;

    while (*p1 != '\0')
    {
        cell.code = (int32_t)*p1;
        *p2++  = cell;
        p1++;
    }

    // draw ├┣╠ border char to right of name

    cell.code = b[BDR_LEFT_T];
    *p2 = cell;
}

// -----------------------------------------------------------------------
// draw window into its parent screen with borders if it has them

void scr_draw_win(uC_window_t *win)
{
    int16_t i;
    int16_t width;
    cell_t *src, *dst;
    uC_screen_t *scr;

    if (win != NULL)
    {
        scr = win->screen;

        // draw windows border if it has one
        if (win->flags & WIN_BOXED)
        {
            win_draw_borders(win);

            // window name only drawn if window has a border

            if (win->display_name != NULL)
            {
               draw_win_name(win);
            }
        }

        dst = scr_line_addr(scr, win->yco);
        dst += win->xco;
        src = win->buffer;

        width = win->width * sizeof(cell_t);

        for (i = 0; i < win->height; i++)
        {
            memcpy(dst, src, width);
            src += win->width;
            dst += scr->width;
        }
    }
}

// -----------------------------------------------------------------------

static void scr_draw_windows(uC_list_t *list)
{
    uC_window_t *win;
    uC_list_node_t *n1;

    n1 = uC_list_scan(list, NULL);

    while (n1 != NULL)
    {
        win = (uC_window_t *)n1->payload;
        scr_draw_win(win);

        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------
// set terminal cursor location to same location as screens cursor

// unless it is already there :)

static void scr_cup(uC_screen_t *scr, int16_t x, int16_t y)
{
    if ((x >= 0) && (y >= 0) &&
        (x < scr->width) && (y < scr->height))
    {
        uC_cup(y, x);
        scr->cx = x;
        scr->cy = y;
    }
}

// -----------------------------------------------------------------------

void init_backdrop(uC_screen_t *scr, uC_window_t *win)
{
    if ((win != NULL) && (scr != NULL))
    {
        win->xco    = 1;
        win->yco    = 1;
        win->width  = scr->width - 2;
        win->height = scr->height - 2;
        win->flags  = WIN_BOXED | WIN_LOCKED;
        win->blank  = SOLID;
        win->screen = scr;
    }
}

// -----------------------------------------------------------------------
// add a backdrop window to the screen

API void uC_scr_add_backdrop(uC_screen_t *scr)
{
    uC_window_t *win;

    if (scr != NULL)
    {
        win = uC_win_open(scr->width - 2, scr->height - 2);

        if (win != NULL)
        {
            // TODO: magic numbers should be user config items
            win->bdr_attrs.bytes[ATTR] =
                (FG_GRAY | BG_GRAY | BOLD);
            win->bdr_attrs.bytes[FG] = 12;
            win->bdr_attrs.bytes[BG] = 0;

            win->border_type = BDR_SINGLE;

            uC_win_set_gray_fg(win, 12);

            init_backdrop(scr, win);
            scr->backdrop = win;
            uC_win_clear(win);
        }
    }
}

// -----------------------------------------------------------------------

static bool scr_is_modified(uC_screen_t *scr, uint16_t index)
{
    cell_t *p1 = &scr->buffer1[index];
    cell_t *p2 = &scr->buffer2[index];

    // if attrs of this cell in buffer1 are different from the attrs
    // in buffer2 or if the characters in those cells are different
    // then this cell needs updating

    return (p1->attrs.chunk != p2->attrs.chunk) || (p1->code != p2->code);
}

// -----------------------------------------------------------------------

static void new_attrs(int64_t a)
{
    attrs.chunk = a;
    apply_attribs();
}

// -----------------------------------------------------------------------
// emits charcter from screen buffer1 to the console

void scr_emit(uC_screen_t *scr, int16_t index)
{
    cell_t *p1, *p2;

    int16_t x, y;
    int16_t wide;
    int16_t force = 0;

    p1 = &scr->buffer1[index];
    p2 = &scr->buffer2[index];

    // are we about to write a wide character here..

    // mental note to self.  If you are about to read p1[1] make sure
    // p1 is not pointing to the last element of the array because p2
    // will be beyond the end of the array :/

    if (index != (scr->width * scr->height) - 1)
    {
        wide = wcwidth(p1->code);

        // if the character we are about to write is double width but there
        // is a single width character overlapping it to the right then
        // force an update of the overlapping single width char

        if (wide != 1)
        {
            if (p1[1].code != (int32_t)DEADC0DE)
            {
                force = 1;
            }
        }

        // if we are about to overwrite the left edge of a double wide
        // character with a single width char then we need to output a
        // blank over the associated DEADC0DE slot (see below)

        else if (p1[1].code == (int32_t)DEADC0DE)
        {
            force = 2;
        }
    }

    // convert index to coordinates and reposition the cursor in the
    // terminal unless it is already there

    y = index / scr->width;
    x = index % scr->width;
    scr_cup(scr, x, y);

    uC_utf8_emit(p1->code);

    // are we overlaying either the left or right edge of a double
    // width char with a single width char?

    if (force != 0)
    {
        new_attrs(p1[1].attrs.chunk);

        if (force == 1)         // right?
        {
            scr_cup(scr, x + 1, y);
            uC_utf8_emit(p1[1].code);
        }
        else if (force == 2)    // left?
        {
            uC_utf8_emit(0x20);
        }
    }

    // restore working attributes after the above detour
    new_attrs(p1->attrs.chunk);

    scr->cx++;

    if (scr->cx == scr->width)
    {
        scr->cx = 0;
        scr->cy++;
    }

    *p2 = *p1;              // mark cell as updated
}

// -----------------------------------------------------------------------
// inner loop of screen update

static int16_t inner_update(uC_screen_t *scr, int16_t index, int16_t end)
{
    cell_t *p1;
    int indx = 0;
    bool f;

    p1 = &scr->buffer1[index];
    new_attrs(p1->attrs.chunk);

    do
    {
        if (attrs.chunk == p1->attrs.chunk)
        {
            f = scr_is_modified(scr, index);

            if (f)
            {
                scr_emit(scr, index);
            }
        }
        else if (indx == 0)
        {
            indx = index;
        }
        index++;
        p1++;
    } while (index != end);

    // return index of first char that had different attributes to the
    // ones we were updating

    return indx;
}

// -----------------------------------------------------------------------
// outer loop of screen update

static void outer_update(uC_screen_t *scr)
{
    int16_t index = 0;
    int16_t end = scr->width * scr->height;
    bool f;

    // force a screen cursor position update
    scr->cx = scr->cy = -1;

    do
    {
        // if char at index is modified then output everey char in the
        // screen that shares its attributes.

        f = scr_is_modified(scr, index);

        if (f)
        {
            index = inner_update(scr, index, end);

            // the return value is the index of the first character
            // that update found that had different atributes to the
            // ones it was setting.  this is our new scan point
            // if index is zero then update scanned to the end of the
            // screen and found no characters that that it did not
            // already update so we can exit early.

            if (index == 0)
            {
                break;
            }
            continue;       // skip the ++
        }
        index++;
    } while (index != end);
}

// -----------------------------------------------------------------------

static void scr_update_menus(uC_screen_t *scr)
{
    menu_bar_t *bar;
    pulldown_t *pd;

    if (scr->menu_bar != NULL)
    {
        bar = scr->menu_bar;

        // draw all text into memu bar window then write that to
        // the screen buffer
        uC_bar_draw_text(scr);
        scr_draw_win(bar->window);

        if (bar->active != 0)
        {
            pd = bar->items[bar->which];

            // draw all text inside pulldown memus window
            bar_populdate_pd(pd);
            // draw pulldown window into screen
            scr_draw_win((uC_window_t *)pd->window);
        }
    }
}

// -----------------------------------------------------------------------

// void scr_draw_vgs(uC_screen_t *scr);

API void uC_scr_draw_screen(uC_screen_t *scr)
{
    old_attrs.chunk = 0;

    if (scr != NULL)
    {
        active_screen = scr;

        // the backdrop if it exists is always the first window
        // to be drawn into the screen. its main purpose is to
        // allow for moveable windows which would leave trails
        // behind if there was no backdrop.
        // it also gives you the ability to set the screen
        // background color which is not a screen attribute
        // normally

        scr_draw_win(scr->backdrop);

        // next draw all normal windows
        scr_draw_windows(&scr->windows);
        // then the menu whicl is always on top
        scr_update_menus(scr);
        // and finally the status windows which are also always
        // on top and can overlay the menu bar window
        scr_draw_windows(&scr->status);

        // the status bar is a 1 high window that can be used
        // by applications to show status info.

        outer_update(scr);

        // draw widget view groups
        // scr_draw_vgs(scr);

        uC_terminfo_flush();
    }
}

// -----------------------------------------------------------------------
// attach a window to a screen

API void uC_scr_win_attach(uC_screen_t *scr, uC_window_t *win)
{
    uC_screen_t *scr2;
    win->screen = scr;
    bool f;

    if (win->screen != NULL)
    {
        scr2 = win->screen;
        uC_list_remove_node(&scr2->windows, win);
    }

    f = uC_list_push_tail(&scr->windows, win);

    if (f != true)
    {
        // log error here?
        // insert more ram to continue!
    }
}

// -----------------------------------------------------------------------
// detach window from its parent screen

API void uC_scr_win_detach(uC_window_t *win)
{
    uC_screen_t *scr = win->screen;

    if (scr != NULL)
    {
        uC_list_remove_node(&scr->windows, win);
        win->screen = NULL;
    }
}

// =======================================================================
