// uC_screen_draw.c   - uCurses text user interface screen handling
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
#include "uC_alloc.h"
#include "uC_utils.h"
#include "uC_utf8.h"
#include "uC_widgets.h"
#include "uC_win_printf.h"


// -----------------------------------------------------------------------

uC_screen_t *active_screen;

extern border_t *const borders[];
extern uC_attribs_t attrs;
extern uC_attribs_t old_attrs;
extern uC_attribs_t uC_attrs_normal;
extern uC_attribs_t uC_attrs_selected;
extern uC_attribs_t uC_attrs_disabled;

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

#ifdef UC_POPUPS
static bool scr_is_too_small(uC_screen_t *scr)
{
    return ((scr->min_width > 0) && (scr->width < scr->min_width)) ||
        ((scr->min_height > 0) && (scr->height < scr->min_height));
}

// -----------------------------------------------------------------------

static void too_small_write(uC_window_t *win, int16_t x, int16_t y,
    const char *text, uC_attribs_t attrs)
{
    int16_t i;
    uC_attribs_t save;

    if (!win || !text || x < 0 || y < 0 ||
        x >= win->width || y >= win->height)
    {
        return;
    }

    save = win->attrs;
    win->attrs = attrs;
    uC_win_cup(win, x, y);

    for (i = 0; text[i] && (x + i) < win->width; i++)
    {
        uC_win_emit(win, (uint8_t)text[i]);
    }

    win->attrs = save;
}

// -----------------------------------------------------------------------

static int16_t text_len_i16(const char *text)
{
    int16_t len = 0;

    while (text[len] != '\0')
    {
        len++;
    }

    return len;
}

// -----------------------------------------------------------------------

static bool too_small_popup_prepare(uC_screen_t *scr)
{
    const char *message = "Too small";
    int16_t width = 14;
    int16_t height = 3;
    int16_t x;
    uC_window_t *win;

    if (scr->width < width + 2 || scr->height < height + 2)
    {
        return false;
    }

    win = scr->too_small_popup;
    if ((win != NULL) && ((win->width != width) || (win->height != height)))
    {
        uC_win_close(win);
        win = NULL;
        scr->too_small_popup = NULL;
    }

    if (win == NULL)
    {
        win = uC_win_open(width, height);
        if (win == NULL)
        {
            return false;
        }

        win->xco = (scr->width - width) / 2;
        win->yco = (scr->height - height) / 2;
        win->flags = uC_WIN_LOCKED;
        win->blank = 0x20;
        win->attrs = uC_attrs_normal;
        uC_win_set_border(win, uC_BDR_CURVED, uC_attrs_selected,
            uC_attrs_selected);
        win->screen = scr;
        scr->too_small_popup = win;
    }

    uC_win_clear(win);
    x = (width - text_len_i16(message)) / 2;
    too_small_write(win, x, 1, message, uC_attrs_selected);

    return true;
}

// -----------------------------------------------------------------------

static void scr_draw_too_small_popup(uC_screen_t *scr)
{
    if (!scr_is_too_small(scr))
    {
        uC_win_close(scr->too_small_popup);
        scr->too_small_popup = NULL;
        return;
    }

    if (too_small_popup_prepare(scr))
    {
        scr_draw_win(scr->too_small_popup);
    }
}
#endif // UC_POPUPS

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
// DEADC0DE is only valid as the continuation of a wide glyph in the same
// row.  Direct overlays such as borders and menus can cover only part of
// an underlying wide glyph, so normalize the composed screen before diffing
// it against the previous frame.

static int16_t scr_cell_width(cell_t *cell)
{
    int32_t width;

    if (cell->code == (uint32_t)DEADC0DE)
    {
        return 0;
    }

    width = wcwidth((wchar_t)cell->code);
    return (width > 1) ? (int16_t)width : 1;
}

// -----------------------------------------------------------------------

static void scr_blank_cell(cell_t *cell)
{
    cell->code = 0x20;
}

// -----------------------------------------------------------------------

static void scr_normalize_wide_row(uC_screen_t *scr, cell_t *row)
{
    int16_t width;
    int16_t x;
    int16_t i;
    bool valid;

    x = 0;

    while (x < scr->width)
    {
        if (row[x].code == (uint32_t)DEADC0DE)
        {
            scr_blank_cell(&row[x++]);
            continue;
        }

        width = scr_cell_width(&row[x]);

        if (width <= 1)
        {
            x++;
            continue;
        }

        if ((x + width) > scr->width)
        {
            scr_blank_cell(&row[x++]);
            continue;
        }

        valid = true;

        for (i = 1; i < width; i++)
        {
            if (row[x + i].code != (uint32_t)DEADC0DE)
            {
                valid = false;
                break;
            }
        }

        if (!valid)
        {
            scr_blank_cell(&row[x++]);
            continue;
        }

        x += width;
    }
}

// -----------------------------------------------------------------------

void scr_normalize_wide_buffer(uC_screen_t *scr)
{
    int16_t y;
    cell_t *row;

    if (!scr || !scr->buffer1)
    {
        return;
    }

    for (y = 0; y < scr->height; y++)
    {
        row = &scr->buffer1[y * scr->width];
        scr_normalize_wide_row(scr, row);
    }
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

        if (wide > 1)
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
// outer loop of screen update

void scr_outer_update(uC_screen_t *scr)
{
    int16_t index;
    int16_t end;
    bool have_attrs = false;
    cell_t *cell;
    uC_attribs_t active_attrs;

    if (scr == NULL)
    {
        return;
    }

    end   = (scr->width * scr->height);
    index = 0;

    scr->cx = scr->cy = -1; // force a screen cursor position update

    do
    {
        // if char at index is modified then output every char in the
        // order the composed screen describes.  Updating by attribute
        // bucket is faster but can make overlays visibly blink because
        // blanking cells and text cells may be emitted in separate passes.

        if (scr_is_modified(scr, index))
        {
            cell = &scr->buffer1[index];

            if (!have_attrs || (active_attrs.blob != cell->attrs.blob))
            {
                active_attrs = cell->attrs;
                new_attrs(active_attrs);
                have_attrs = true;
            }

            scr_emit(scr, index);
        }

        // would splitting this into two separate loops make it faster?
        index++;
    } while (index != end);
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

// Widgets draw over normal windows. Menus draw after widgets so pulldowns
// are not covered by widget view groups. Status windows draw after menus so
// they can intentionally occupy unused menu/status row space.

#ifdef UC_WIDGETS
        draw_view_groups(scr);
#endif // UC_WIDGETS

#ifdef UC_MENUS
        scr_update_menus(scr);
#endif // UC_MENUS

#ifdef UC_STATUS
        scr_draw_windows(&scr->status);
#endif // UC_STATUS

#ifdef UC_POPUPS
        // Popup windows are modal overlays. Draw them after widgets so
        // they are not covered by view groups.

        scr_draw_win(scr->popup);
#ifdef UC_WIDGETS
        draw_popup_view_group((uC_widget_vg_t *)scr->popup_vg);
#endif
        scr_draw_too_small_popup(scr);
#endif

        // at this point everything that should be displayed has been
        // drawn into the screen buffers.   we can now compile all the
        // escape sequeces needed to physically draw the screen state
        // out to the terminal.

        scr_normalize_wide_buffer(scr);
        scr_outer_update(scr);

        // If the terminal resized while this frame was being composed,
        // discard the partial update.  The application can then rebuild
        // from its layout source after noticing uC_winch_pending().

        if (uC_winch_pending())
        {
#ifdef UC_POPUPS
            uC_scr_popup_cancel(scr);
#endif
            terminfo_purge();
        }
        else
        {
            uC_terminfo_flush();
        }
    }
}

// =======================================================================
