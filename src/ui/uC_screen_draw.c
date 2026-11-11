// uC_screen_draw.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE 700  // needed to make wcwidth work

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>

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

extern ti_vars_t *ti_vars;
extern uint64_t uC_stat_bytes;
extern uint64_t uC_stat_flushes;
extern uint64_t uC_stat_peak;
extern uint64_t uC_stat_full;
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
    int32_t index;
    uint16_t x, y;

    scr = win->screen;

    // point x/y at top left of window inside the border

    x = win->xco;
    y = win->yco - 1;

    index = ((int32_t)y * (int32_t)scr->width) + x;

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
    size_t  width;
    int32_t index;

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

        index = (int32_t)scr->width * (int32_t)win->yco;
        dst = &scr->buffer1[index];
        dst += win->xco;
        src = win->buffer;

        // ⚠ a BYTE count, so int16_t overflowed at a width of 4096
        // and not at 32767 like the indices

        width = (size_t)win->width * sizeof(cell_t);

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

static bool scr_is_too_small(uC_screen_t *scr)
{
    return ((scr->min_width > 0) && (scr->width < scr->min_width)) ||
        ((scr->min_height > 0) && (scr->height < scr->min_height));
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

static void scr_force_full_redraw(uC_screen_t *scr)
{
    size_t size;

    if ((scr != NULL) && (scr->buffer2 != NULL))
    {
        size = (size_t)scr->width * (size_t)scr->height *
            sizeof(*scr->buffer2);
        memset(scr->buffer2, 0, size);
    }
}

// -----------------------------------------------------------------------

static void scr_draw_too_small_direct(uC_screen_t *scr)
{
    char message[64];
    const char *text = "Too Small!";
    int16_t len;
    int16_t write_len;
    int16_t x;
    int16_t y;
    ssize_t unused;

    if (scr->too_small_active &&
        scr->too_small_width == scr->width &&
        scr->too_small_height == scr->height)
    {
        return;
    }

    if (scr->width >= 28)
    {
        snprintf(message, sizeof(message), "Too Small! %dx%d required",
            scr->min_width, scr->min_height);
        text = message;
    }

    scr->too_small_active = true;
    scr->too_small_width = scr->width;
    scr->too_small_height = scr->height;
    scr_force_full_redraw(scr);

    ti_sgr0();
    uC_clear();
    if (scr->width > 0 && scr->height > 0)
    {
        len = text_len_i16(text);
        write_len = (scr->width > len) ? len : scr->width;
        x = (scr->width > write_len)
            ? (int16_t)((scr->width - write_len) / 2)
            : 0;
        y = scr->height / 2;
        uC_cup((uint16_t)x, (uint16_t)y);
        uC_terminfo_flush();
        unused = write(STDOUT_FILENO, text, (size_t)write_len);
        (void)unused;
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

static bool scr_is_modified(uC_screen_t *scr, int32_t index)
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

static void _scr_emit(uC_screen_t *scr, int32_t index,
    cell_t *p1, cell_t *p2)
{
    int16_t x, y;
    int16_t wide;
    int16_t force;

    force = 0;

    if (index != ((int32_t)scr->width * (int32_t)scr->height) - 1)
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
// runs.  a stretch of one printable ascii character in one set of
// attributes goes out as a single ti_rep() - the character then
// ESC [ count b - instead of one byte per cell.  that is 4 bytes plus the
// digits of the count, so a run under UC_REP_MIN cells is not worth the
// sequence.  a run cannot touch a wide glyph: every cell in it is narrow
// ascii, and scr_normalize_wide_buffer() has already left DEADC0DE only as
// the tail of a whole wide glyph, so nothing after a run is a tail.  only
// cells already marked modified take part: reaching over
// unmodified ones would send bytes the diff exists to avoid.  UC_REP=0 in
// the environment turns this off, to compare the output with and without.

#define UC_REP_MIN (6)

static bool scr_rep_enabled(void)
{
    static int8_t state = -1;
    const char *e;

    if (state < 0)
    {
        e     = getenv("UC_REP");
        state = (uC_ti_has(121) && !(e != NULL && e[0] == '0')) ? 1 : 0;
    }
    return state == 1;
}

// -----------------------------------------------------------------------
// number of cells, starting at index, that can go out as one repeat

static int32_t scr_run_len(uC_screen_t *scr, int32_t index)
{
    cell_t *p1;
    int32_t n, left;

    p1 = &scr->buffer1[index];

    if (p1->code < 0x20)
    {
        return 0;
    }

    // outside ascii only a plain one column glyph repeats.  the terminfo
    // rep string cannot (its %c is one byte) so scr_rep_emit() sends the
    // utf8 once and then CSI count b, which repeats the whole glyph.

    if ((p1->code > 0x7e) &&
        ((p1->code < 0xa0) || (p1->code == (uint32_t)DEADC0DE) ||
         (wcwidth((wchar_t)p1->code) != 1)))
    {
        return 0;
    }

    left = scr->width - (index % scr->width);   // cells to end of row
    n    = 1;

    while ((n < left) && scr_is_modified(scr, index + n) &&
           (p1[n].code == p1->code) &&
           (p1[n].attrs.blob == p1->attrs.blob))
    {
        n++;
    }

    return (n >= UC_REP_MIN) ? n : 0;
}

// -----------------------------------------------------------------------

static void scr_emit_num(int32_t v)
{
    char digits[12];
    int32_t i;

    i = 0;

    do
    {
        digits[i++] = (char)('0' + (v % 10));
        v /= 10;
    } while (v != 0);

    while (i != 0)
    {
        c_emit(digits[--i]);
    }
}

static void scr_rep_emit(uC_screen_t *scr, int32_t index, int32_t n)
{
    int32_t i;

    scr_cup(scr, index % scr->width, index / scr->width);

    if (scr->buffer1[index].code < 0x7f)
    {
        ti_vars->params[0] = scr->buffer1[index].code;
        ti_vars->params[1] = n;
        ti_rep();
    }
    else
    {
        // the glyph itself, then ESC [ n-1 b for the other n-1 copies

        uC_utf8_emit(scr->buffer1[index].code);

        c_emit(0x1b);
        c_emit('[');

        scr_emit_num(n - 1);
        c_emit('b');
    }

    for (i = 0; i < n; i++)
    {
        scr->buffer2[index + i] = scr->buffer1[index + i];
    }

    scr->cx += n;

    if (scr->cx == scr->width)
    {
        scr->cx = 0;
        scr->cy++;
    }
}

// -----------------------------------------------------------------------
// emits charcter from screen buffer1 to the escape buffer

void scr_emit(uC_screen_t *scr, int32_t index)
{
    cell_t *p1, *p2;
    int32_t n;

    p1 = &scr->buffer1[index];
    p2 = &scr->buffer2[index];

    // are we about to write a wide character here..

    if (p1->code != (uint32_t)DEADC0DE)
    {
        if (scr_rep_enabled())
        {
            n = scr_run_len(scr, index);

            if (n != 0)
            {
                scr_rep_emit(scr, index, n);
                return;
            }
        }
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
// inner loop of screen update.  Write every modified character which
// shares the attributes of the character at index.

static int32_t inner_update(uC_screen_t *scr, int32_t index, int32_t end)
{
    cell_t *p1;
    int32_t next;
    uC_attribs_t a;

    next = 0;

    // Select these attributes once, then bounce the cursor to every
    // modified character which shares them.  On slow connections this is
    // considerably cheaper than repeatedly changing terminal attributes.

    p1     = &scr->buffer1[index];
    a.blob = p1->attrs.blob;

    new_attrs(a);

    do
    {
        if (scr_is_modified(scr, index))
        {
            if (a.blob == p1->attrs.blob)
            {
                scr_emit(scr, index);
            }
            else if (next == 0)
            {
                // Remember the first modified character with different
                // attributes.  The outer loop can resume there rather
                // than rescanning all the unmodified characters.

                next = index;
            }
        }

        index++;
        p1++;
    } while (index != end);

    return next;
}

// -----------------------------------------------------------------------
// outer loop of screen update

void scr_outer_update(uC_screen_t *scr)
{
    int32_t index;
    int32_t end;

    if (scr == NULL)
    {
        return;
    }

    end   = (int32_t)scr->width * (int32_t)scr->height;
    index = 0;

    scr->cx = scr->cy = -1; // force a screen cursor position update

    do
    {
        // If this character is modified, emit it and every other modified
        // character which shares its attributes.

        if (scr_is_modified(scr, index))
        {
            index = inner_update(scr, index, end);

            // Zero means that inner_update() reached the end without
            // finding another modified attribute group.

            if (index == 0)
            {
                break;
            }

            continue;
        }

        index++;
    } while (index != end);
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// UC_STATS=1 in the environment: frames drawn, escape bytes written and
// time spent in uC_scr_draw_screen(), reported on stderr at exit.

static uint64_t stat_frames;
static uint64_t stat_ns;

static void stats_report(void)
{
    if (stat_frames != 0)
    {
        fprintf(stderr, "uC stats: %llu frames, %llu escape bytes "
            "(%llu per frame), %llu us per frame\n",
            (unsigned long long)stat_frames,
            (unsigned long long)uC_stat_bytes,
            (unsigned long long)(uC_stat_bytes / stat_frames),
            (unsigned long long)(stat_ns / stat_frames / 1000));
        fprintf(stderr, "uC stats: escape buffer peak %llu of 65535 "
            "(%llu%%), %llu full-buffer flushes, %llu flushes\n",
            (unsigned long long)uC_stat_peak,
            (unsigned long long)(uC_stat_peak * 100 / 65535),
            (unsigned long long)uC_stat_full,
            (unsigned long long)uC_stat_flushes);
    }
}

static uint64_t stat_now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static void uC_scr_draw_screen_(uC_screen_t *scr);

API void uC_scr_draw_screen(uC_screen_t *scr)
{
    static int8_t stats = -1;
    uint64_t t0;

    if (stats < 0)
    {
        stats = (getenv("UC_STATS") != NULL);

        if (stats)
        {
            atexit(stats_report);
        }
    }

    if (!stats)
    {
        uC_scr_draw_screen_(scr);
        return;
    }

    t0 = stat_now();
    uC_scr_draw_screen_(scr);
    stat_ns += stat_now() - t0;
    stat_frames++;
}

// -----------------------------------------------------------------------

static void uC_scr_draw_screen_(uC_screen_t *scr)
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

        if (scr_is_too_small(scr))
        {
#ifdef UC_POPUPS
            uC_scr_popup_cancel(scr);
#endif
            scr_draw_too_small_direct(scr);
            return;
        }

        if (scr->too_small_active)
        {
            scr->too_small_active = false;
            uC_clear();
        }

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
