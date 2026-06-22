// uC_screen_draw.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE 700  // needed to make wcwidth work

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_menus.h"
#include "uC_borders.h"
#include "uC_terminfo.h"
#include "uC_alloc.h"
#include "uC_utils.h"
#include "uC_utf8.h"
#include "uC_widgets.h"


// -----------------------------------------------------------------------

uC_screen_t *active_screen;

extern border_t *const borders[];
extern uC_attribs_t attrs;
extern uC_attribs_t old_attrs;

void terminfo_purge(void);
void scr_update_menus(uC_screen_t *scr);
void draw_view_groups(uC_screen_t *scr);
void ti_set_screen(uC_screen_t *scr);

// -----------------------------------------------------------------------
// draw name of window in its border - must have border

static void draw_win_name(uC_window_t *win)
{
    const char *p1;
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
    if (!p1)
    {
        return;
    }

    p2 = &scr->buffer1[index];

    cell.attrs = (win->flags & uC_WIN_FOCUS)
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
        cell.code = (uint8_t)*p1;
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
        if ((win->flags & uC_WIN_BOXED) &&
            (win->border_type != uC_BDR_NONE))
        {
            win_draw_borders(win);

            // window name only drawn if window has a border

            if (win->display_name &&
                (win->flags & uC_WIN_NAMED))
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
        if (uC_winch_pending()) { break; }

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
            uC_cup(x, y);
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
        // if (uC_winch_pending()) { break; }

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

    int16_t index;
    int16_t end;

    if (scr == NULL)
    {
        return;
    }

    end   = (scr->width * scr->height);
    index = 0;

    scr->cx = scr->cy = -1; // force a screen cursor position update

    do
    {
        // if (uC_winch_pending()) // abort mission?
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
// Resize hold support.  On first observed SIGWINCH, snapshot the most
// recently rendered screen, gray it, and keep redrawing that shadow until
// the winch stream is quiet.  The application rebuilds its real UI once
// this function returns true.

#define RESIZE_HOLD_POLL_NS  (25000000)
#define RESIZE_HOLD_QUIET_NS (150000000LL)

static uC_attribs_t resize_shadow_attrs =
{
    .flags.bits = (uC_ATTR_FLAG_GRAY_FG | uC_ATTR_FLAG_GRAY_BG),
    .fg_gray    = uC_GRAY_10,
    .bg_gray    = uC_GRAY_03,
};

static int64_t monotonic_ns(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((int64_t)ts.tv_sec * 1000000000LL) + ts.tv_nsec;
}

static cell_t *resize_snapshot(uC_screen_t *scr)
{
    size_t i;
    size_t count;
    cell_t *shadow;

    if (!scr || !scr->buffer1 || scr->width <= 0 || scr->height <= 0)
    {
        return NULL;
    }

    count = (size_t)scr->width * (size_t)scr->height;
    shadow = uC_alloc(uC_MEM_ZONE_DEFAULT, count * sizeof(*shadow));

    if (!shadow)
    {
        return NULL;
    }

    for (i = 0; i < count; i++)
    {
        shadow[i] = scr->buffer1[i];
        shadow[i].attrs = resize_shadow_attrs;

        if (shadow[i].code == 0)
        {
            shadow[i].code = 0x20;
        }
    }

    return shadow;
}

static void resize_fill_shadow_view(uC_screen_t *view, cell_t *shadow,
    int16_t shadow_w, int16_t shadow_h)
{
    int16_t x;
    int16_t y;
    cell_t blank =
    {
        .attrs = resize_shadow_attrs,
        .code  = 0x20,
    };

    for (y = 0; y < view->height; y++)
    {
        for (x = 0; x < view->width; x++)
        {
            size_t dst = ((size_t)y * (size_t)view->width) + (size_t)x;

            if ((x < shadow_w) && (y < shadow_h))
            {
                size_t src = ((size_t)y * (size_t)shadow_w) + (size_t)x;
                view->buffer1[dst] = shadow[src];
            }
            else
            {
                view->buffer1[dst] = blank;
            }
        }
    }
}

static bool resize_draw_shadow(uC_screen_t *scr, cell_t *shadow)
{
    uint16_t width;
    uint16_t height;
    size_t size;
    uC_screen_t view = { 0 };
    uC_screen_t *restore = active_screen;

    if (!scr || !shadow)
    {
        return false;
    }

    uC_get_console_size(&width, &height);

    if ((width == 0) || (height == 0) ||
        (width > INT16_MAX) || (height > INT16_MAX))
    {
        return false;
    }

    view.width  = (int16_t)width;
    view.height = (int16_t)height;
    view.cx = -1;
    view.cy = -1;

    size = (size_t)view.width * (size_t)view.height * sizeof(cell_t);
    view.buffer1 = uC_alloc(uC_MEM_ZONE_DEFAULT, size);
    view.buffer2 = uC_alloc(uC_MEM_ZONE_DEFAULT, size);

    if (!view.buffer1 || !view.buffer2)
    {
        if (view.buffer1) { uC_free(uC_MEM_ZONE_DEFAULT, view.buffer1); }
        if (view.buffer2) { uC_free(uC_MEM_ZONE_DEFAULT, view.buffer2); }
        return false;
    }

    memset(view.buffer2, 0, size);
    resize_fill_shadow_view(&view, shadow, scr->width, scr->height);

    ti_sgr0();
    old_attrs.blob = 0;
    setlocale(LC_CTYPE, "");

    active_screen = &view;
    ti_set_screen(&view);
    uC_clear();
    outer_update(&view);

    if (uC_winch_pending())
    {
        terminfo_purge();
    }
    else
    {
        uC_terminfo_flush();
    }

    active_screen = restore;
    ti_set_screen(scr);

    uC_free(uC_MEM_ZONE_DEFAULT, view.buffer1);
    uC_free(uC_MEM_ZONE_DEFAULT, view.buffer2);

    return true;
}

API bool uC_scr_resize_hold(uC_screen_t *scr)
{
    int64_t last_winch;
    cell_t *shadow;

    if (!scr || !uC_winch_pending())
    {
        return false;
    }

    shadow = resize_snapshot(scr);
    uC_winch_ack();
    last_winch = monotonic_ns();

    if (shadow)
    {
        resize_draw_shadow(scr, shadow);
    }

    while (true)
    {
        uC_clock_sleep(RESIZE_HOLD_POLL_NS);

        if (uC_winch_pending())
        {
            uC_winch_ack();
            last_winch = monotonic_ns();

            if (shadow)
            {
                resize_draw_shadow(scr, shadow);
            }
            continue;
        }

        if ((monotonic_ns() - last_winch) >= RESIZE_HOLD_QUIET_NS)
        {
            break;
        }
    }

    if (shadow)
    {
        uC_free(uC_MEM_ZONE_DEFAULT, shadow);
    }

    return true;
}


// -----------------------------------------------------------------------

API void uC_scr_draw_screen(uC_screen_t *scr)
{
    if (scr != NULL)
    {
        // this sgr0 fixes a bug where the most recent write to the
        // console was of an underlined character and the underline
        // attribute is still set (for reasons unknown)

        ti_sgr0();
        old_attrs.blob = 0;

        // move this to uCurses initialization?
        setlocale(LC_CTYPE, "");

        active_screen = scr;
        ti_set_screen(scr);

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

// i hate embedding cluster fuck conditional compilation all over!
// if this gets out of hand im reverting to ZERO conditionals buried
// in these soures

#ifdef UC_MENUS
        scr_update_menus(scr);
#endif // UC_MENUS

#ifdef UC_STATUS
        scr_draw_windows(&scr->status);
#endif // UC_STATUS


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

        // If the terminal resized while this frame was being composed,
        // discard the partial update.  The application can then rebuild
        // from its layout source after noticing uC_winch_pending().

        if (uC_winch_pending())
        {
            terminfo_purge();
        }
        else
        {
            uC_terminfo_flush();
        }
    }
}

// =======================================================================
