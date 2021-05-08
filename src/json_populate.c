// json_populate.c   - populate parent structures with child structure
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>

#include "h/uCurses.h"

extern j_state_t *j_state;

// -----------------------------------------------------------------------
// this series of IF statements produces significantly smaller code than
// a switch statement does.   C sucks

static INLINE void populate_attribs(void *pstruct, uint32_t ptype)
{
    if((ptype == STRUCT_WINDOW) || (ptype == STRUCT_BACKDROP))
    {
        *(uint64_t *)((window_t *)pstruct)->attrs =
            *(uint64_t *)j_state->structure;
    }
    else if(ptype == STRUCT_PULLDOWN)
    {
        *(uint64_t *)((pulldown_t *)pstruct)->normal =
            *(uint64_t *)j_state->structure;
    }
    else // ptype == STRUCT_MENU_BAR:
    {
        *(uint64_t *)((menu_bar_t *)pstruct)->normal =
            *(uint64_t *)j_state->structure;
    }

    free(j_state->structure);
}

// -----------------------------------------------------------------------

static INLINE void populate_b_attribs(window_t *pstruct)
{
    *(uint64_t *)pstruct->bdr_attrs = *(uint64_t *)j_state->structure;

    free(j_state->structure);
}

// -----------------------------------------------------------------------

static INLINE void populate_s_attribs(void *pstruct, uint32_t ptype)
{
    if(ptype == STRUCT_PULLDOWN)
    {
        *(uint64_t *)((pulldown_t *)pstruct)->selected =
            *(uint64_t *)j_state->structure;
    }
    else // ptype == STRUCT_MENU_BAR:
    {
        *(uint64_t *)((menu_bar_t *)pstruct)->selected =
            *(uint64_t *)j_state->structure;
    }

    free(j_state->structure);
}

// -----------------------------------------------------------------------

static INLINE void populate_d_attribs(void *pstruct, uint32_t ptype)
{
    if(ptype == STRUCT_PULLDOWN)
    {
        *(uint64_t *)((pulldown_t *)pstruct)->disabled =
            *(uint64_t *)j_state->structure;
    }
    else // ptype == STRUCT_MENU_BAR:
    {
        *(uint64_t *)((menu_bar_t *)pstruct)->disabled =
            *(uint64_t *)j_state->structure;
    }

    free(j_state->structure);
}

// -----------------------------------------------------------------------

static INLINE void populate_pulldown(menu_bar_t *pstruct)
{
    uint16_t i;

    i = pstruct->count++;
    pstruct->items[i] = j_state->structure;
}

// -----------------------------------------------------------------------

static INLINE void populate_menu_item(pulldown_t *gstruct)
{
    uint16_t i;

    i = gstruct->count++;
    gstruct->items[i] = j_state->structure;
}

// -----------------------------------------------------------------------

static INLINE void populate_window(j_state_t *parent)
{
    window_t *win;
    screen_t *scr;

    j_state_t *gp = parent->parent;
    scr = gp->structure;
    win = j_state->structure;

    scr_win_attach(scr, win);
}

// -----------------------------------------------------------------------

static INLINE void populate_backdrop(screen_t *pstruct)
{
    pstruct->backdrop = j_state->structure;
}

// -----------------------------------------------------------------------

static INLINE void populate_bar(screen_t *scr)
{
    menu_bar_t *bar = j_state->structure;
    scr->menu_bar = bar;
}

// -----------------------------------------------------------------------
// an object has been completed and thereby the associated C structure is
// ready.  add this C structure to its parent objects C structure...
// or sometimes its grandparents

void INLINE populate_parent(void)
{
    uint32_t ptype;

    void *pstruct;
    void *gstruct;
    j_state_t *parent;
    j_state_t *gp;

    parent = j_state->parent;
    gp = parent->parent;
    pstruct = parent->structure;
    gstruct = gp->structure;

    ptype = parent->struct_type;

    // when a psudo structure is completed all of the key values
    // specified within that psudo structure will have been
    // added to its parent - in this case this function will
    // be called to add that psudo structure to its parent but
    // it will not be any of the types specified below.  this is
    // an inconseauential waste of time as no action will be
    // taken below - this entire function becomes a NOP in that
    // case

    // p.s. i cannot express strongly enough how much i loathe
    // and despise c swith statements but that is not the reason
    // why i have not used them in this project much.  the only
    // reason im using it here is because of the varying number
    // of parameters passed to each of these

    // my main reason for not using them in this project is because
    // a switch statement usually produces a significantly larger
    // blob of code compared to my re_switch() model and size is
    // what I am optimizing for here.

    switch(j_state->struct_type)
    {
        case STRUCT_ATTRIBS:
            populate_attribs(pstruct, ptype);
            break;
        case STRUCT_B_ATTRIBS:
            populate_b_attribs(pstruct);
            break;
        case STRUCT_S_ATTRIBS:
            populate_s_attribs(pstruct, ptype);
            break;
        case STRUCT_D_ATTRIBS:
            populate_d_attribs(pstruct, ptype);
            break;
        case STRUCT_PULLDOWN:
            populate_pulldown(gstruct);
            break;
        case STRUCT_MENU_ITEM:
            populate_menu_item(gstruct);
            break;
        case STRUCT_WINDOW:
            populate_window(parent);
            break;
        case STRUCT_BACKDROP:
            populate_backdrop(pstruct);
            break;
        case STRUCT_MENU_BAR:
            populate_bar(pstruct);
            break;
    }
}

// =======================================================================
