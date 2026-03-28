// screen.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE 700  // needed to make wcwidth work

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_menus.h"
#include "uC_borders.h"
#include "uC_terminfo.h"
#include "uC_utf8.h"
#include "uC_widgets.h"

extern bool winch;

// -----------------------------------------------------------------------

API uC_screen_t *active_screen;

extern border_t *const borders[];
extern uC_attribs_t attrs;
extern uC_attribs_t old_attrs;

void terminfo_purge(void);
void scr_update_menus(uC_screen_t *scr);
void draw_view_groups(uC_screen_t *scr);

// -----------------------------------------------------------------------
// draw name of window in its border - must have border

static void draw_win_name(uC_window_t *win)
{
    uint8_t *p1;
    cell_t *p2;
    border_t *b;
    uC_screen_t *scr;
    cell_t cell;
    uint16_t index, x, y;

    scr = win->screen;

    // point x/y at top left of window inside the border

    x = win->xco;
    y = win->yco - 1;

    index = (y * scr->width) + x;

    p1 = win->display_name;
    if (p1 == NULL)
    {
        return;
    }

    p2 = &scr->buffer1[index];

    cell.attrs = (win->flags & WIN_FOCUS)
        ? win->focus_attrs
        : win->bdr_attrs;

    b = borders[win->border_type];

    // draw ┤┫╣ border char to left of name

    cell.code = b[BDR_RIGHT_T];
    *p2++ = cell;
    cell.code = 0x20;
    *p2++ = cell;

    while (*p1)
    {
        cell.code = (int32_t)*p1;
        *p2++ = cell;
        p1++;
    }

    // draw ├┣╠ border char to right of name

    cell.code = 0x20;
    *p2++ = cell;
    cell.code = b[BDR_LEFT_T];
    *p2 = cell;
}

// -----------------------------------------------------------------------
// draw window into its parent screen with borders if it has them

void scr_draw_win(uC_window_t *win)
{
    int16_t i;
    int16_t width;
    int16_t index;

    cell_t *src, *dst;
    uC_screen_t *scr;

    if ((win != NULL) && (win->screen != NULL))
    {
        scr = win->screen;

        // draw windows border if it has one
        if ((win->flags & WIN_BOXED) && (win->border_type != BDR_NONE))
        {
            win_draw_borders(win);

            // window name only drawn if window has a border

            if ((win->display_name != NULL) && (win->flags & WIN_NAMED))
            {
               draw_win_name(win);
            }
        }

        index = (scr->width * win->yco);
        dst = &scr->buffer1[index];
        dst += win->xco;
        src = win->buffer;

        width = (win->width * sizeof(cell_t));

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
        if (winch) { break; }

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
        if ((scr->cx != x) || (scr->cy != y))
        {
            uC_cup(y, x);
            scr->cx = x;
            scr->cy = y;
        }
    }
}

// -----------------------------------------------------------------------
// return true if cell at specified index needs to be redrawn

static bool scr_is_modified(uC_screen_t *scr, uint16_t index)
{
    cell_t *p1 = &scr->buffer1[index];
    cell_t *p2 = &scr->buffer2[index];

    // if attrs of this cell in buffer1 are different from the attrs
    // in buffer2 or if the characters in those cells are different
    // then this cell needs updating

    return ((p1->attrs.blob != p2->attrs.blob) ||
            (p1->code       != p2->code));
}

// -----------------------------------------------------------------------

static void new_attrs(uC_attribs_t a)
{
    attrs = a;
    apply_attribs();
}

// -----------------------------------------------------------------------

static void _scr_emit(uC_screen_t *scr, int16_t index,
    cell_t *p1, cell_t *p2)
{
    int16_t x, y;
    int16_t wide;
    int16_t force;

    force = 0;

    if (index != (scr->width * scr->height) - 1)
    {
        wide = wcwidth(p1->code);

        // if the character we are about to write is double width but there
        // is a single width character overlapping it to the right then
        // force an update of the overlapping single width char

        if (wide != 1)
        {
            if (p1[1].code != (uint32_t)DEADC0DE)
            {
                force = 1;
            }
        }

        // if we are about to overwrite the left edge of a double wide
        // character with a single width char then we need to output a
        // blank over the associated DEADC0DE slot (see below)

        else if (p1[1].code == (uint32_t)DEADC0DE)
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

    if ((force != 0) && (scr->cx != scr->width - 1))
    {
        // dont output attribute change escape sequences if
        // the attributes of p1[1] are the same as we already
        // have set

        if (p1->attrs.blob != p1[1].attrs.blob)
        {
            new_attrs(p1[1].attrs);
        }

        uC_utf8_emit((force == 1)
            ? p1[1].code : 0x20);

        // restore working attributes after the above detour

        if (p1->attrs.blob != p1[1].attrs.blob)
        {
            new_attrs(p1->attrs);
        }
        p2[1] = p1[1];
    }
}

// -----------------------------------------------------------------------
// emits charcter from screen buffer1 to the escape buffer

void scr_emit(uC_screen_t *scr, int16_t index)
{
    cell_t *p1, *p2;

    p1 = &scr->buffer1[index];
    p2 = &scr->buffer2[index];

    // are we about to write a wide character here..

    if (p1->code != (uint32_t)DEADC0DE)
    {
        _scr_emit(scr, index, p1, p2);
    }

    scr->cx++;

    if (scr->cx == scr->width)
    {
        scr->cx = 0;
        scr->cy++;
    }

    *p2 = *p1;              // mark cell as updated
}

// -----------------------------------------------------------------------
// inner loop of screen update.  write chars with same attrib to escape
// buffer

static int16_t inner_update(uC_screen_t *scr, int16_t index, int16_t end)
{
    cell_t *p1;
    int indx;
    bool f;

    uC_attribs_t a;

    indx = 0;

    // select current attributes and write out every character in
    // the screen buffer that has these atrributes, no matter where
    // they are located.   it is significantly faster to repeatedly
    // bounce the cursor all over the map than to constantly have to
    // activate new attrbute values.

    p1     = &scr->buffer1[index];
    a.blob = p1->attrs.blob;

    new_attrs(a);

    do
    {
        // if (winch) { break; }

        f = scr_is_modified(scr, index);

        // this removed condition seems to have been causing screen
        // update problems when single width characters are drawn
        // over the top of what were previously multi width characters

        if (f == true) // && (p1->code != DEADC0DE))
        {
            if (a.blob == p1->attrs.blob)
            {
                scr_emit(scr, index);
            }
            else if (indx == 0)
            {
                // we will later return the index of this character which
                // is the first modified character that has different
                // attributes to those we are currently updating.  this
                // means that the outer loop does not need to scan over
                // all the unmofified characters we have alredy scanned
                // past within this loop.

                indx = index;
            }
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
    bool f;

    int16_t index = 0;
    int16_t end   = (scr->width * scr->height);

    scr->cx = scr->cy = -1; // force a screen cursor position update

    do
    {
        // if (winch)          // abort mission?
        // {
        //     break;
        // }

        // if char at index is modified then output every char in the
        // screen that shares its attributes that has also been modified.

        f = scr_is_modified(scr, index);

        if (f == true)
        {
            index = inner_update(scr, index, end);

            // the return value is the index of the first character
            // that update found that had different atributes to the
            // ones it was setting.  this is our new scan point.
            // if index is zero then update scanned to the end of the
            // screen and found no characters that that needed to be
            // updated so we can exit the loop.

            if (index == 0)
            {
                break;
            }
            continue;       // skip the ++
        }

        // this part of the loop is litreally only executed when scanning
        // the screen state for the first modified character.  The indicies
        // of subesquent modified characters are returned to us by the
        // inner loop.

        // would splitting this into two separate loops make it faster?
        index++;
    } while (index != end);
}

// -----------------------------------------------------------------------

API void uC_scr_draw_screen(uC_screen_t *scr)
{
    // this sgr0 fixes a bug where the most recent write to the console
    // was of an underlined character and the underline attribute is still
    // set (for reasons unknown)

    ti_sgr0();

    // move this to uCurses initialization?
    setlocale(LC_CTYPE, "");

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

        // draw backdrop window then all other non special
        // case windows

        scr_draw_win(scr->backdrop);
        scr_draw_windows(&scr->windows);

        // special case windows ->

#ifdef UC_STATUS
        scr_draw_windows(&scr->status);
#endif // UC_STATUS

// i hate embedding cluster fuck conditional compilation all over!
// if this gets out of hand im reverting to ZERO conditionals buried
// in these soures

#ifdef UC_MENUS
        scr_update_menus(scr);
#endif // UC_MENUS


// widgets, if active are always drawn over the top of any other
// entity in view including menus -- should I reverse the order?

#ifdef UC_WIDGETS
        draw_view_groups(scr);
#endif // UC_WIDGETS

        // at this point everything that should be displayed has been
        // drawn into the screen buffers.   we can now compile all the
        // escape sequeces needed to physically draw the screen state
        // out to the terminal.

        outer_update(scr);

        // winch not working yet

        // if we got a window change notification while updating the
        // screen then purge all output without writing it out to the
        // system... otherwise go ahead and write it all out

        // todo: rename flush because it sounds like its doing the same
        // thing as purge.

        // (winch)
            // ? terminfo_purge()
            // : uC_terminfo_flush();

        // write all compiled escape sequences out to the terminal.

        uC_terminfo_flush();
    }
}

// =======================================================================
