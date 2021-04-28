// uCurses json parsing for user interface layout
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "h/uCurses.h"
#include "h/list.h"

// -----------------------------------------------------------------------

// There are 5 states in this state machine as follows
//
//      STATE_L_BRACE
//      STATE_KEY
//      STATE_VALUE
//      STATE_R_BRACE
//      STATE_DONE
//
// The initial state is STATE_L_BRACE which expects the next token parsed
// to be the left brace '{' char.  If it is then the state is set to
// STATE_KEY.
//
// The handler for STATE_KEY expect to see one of several known tokens
// which will either be a 'key' token or an 'object' token.  When parsing
// tokens no C string compare routines are used, all tokens are recognized
// by their 32 bit fnv-1a hash value.
//
// If the parsed token is recognized as a key
//
//      That specifv key is handled (see below) and the next state is set
//      to STATE_VALUE where we extract the value to be stored in the
//      specified key.
//
// If the parsed token is one of the recognized objects
//
//      The state is set once again to STATE_L_BRACE.
//
// Almost every STATE_L_BRACE indicates that a new object and thereby an
// associated C structure is being created.
//
// For every object and every key the machine will transition into a new
// state.  If the previous state was an object that object state is
// pushed onto a state stack.  Key states simply replace the previous key
// state, they are never pushed onto the state stack.
//
// Any time a new object or key token is parsed in we check to see if
// there is a comma on it.  If there is no comma the next state will
// instead be STATE_R_BRACE.  This state pops the previous state off the
// stack.
//
// As key values are parsed we immediately set the specified item in
// their parents C structure.  When an object is completed and we store
// the associated C structure within its parents C structure.  The
// structure types of both specify how and where.
//
// When all states have been popped off the stack we transition into
// STATE_DONE and the state machine terminates.
//
// This code also knows what keys may go inside what objects and what
// objects may go inside what objects.  Any time you try to do something
// such as define a screen inside a window this code will abort with an
// error message indicating the offending json line.
//
// As stated above, some object states do not have an associated C
// structure.  Any keys specified within these psudo object will be
// assigned instead to the psudo objects parent C structure. I.E. the
// keys grandparent.

// -----------------------------------------------------------------------

char *json_data;        // pointer to json data to be parsed
size_t json_len;        // total size of json data
uint32_t json_index;    // parse index into data (current line)

char line_buff[MAX_LINE_LEN];

uint16_t line_no;
uint16_t line_index;    // line parse location
uint16_t line_left;     // number of chars left to parse in line

// space delimited token extracted from data - +1 for the null

char json_token[TOKEN_LEN + 1];
uint32_t json_hash;

list_t j_stack;

fp_finder_t fp_finder;

uint16_t console_width;
uint16_t console_height;

// -----------------------------------------------------------------------
// the current state

j_state_t *j_state;

// -----------------------------------------------------------------------
// fnv-1a hash values for various json syntax chars

const uint32_t json_syntax[] =
{
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
// not using arrays in this UI parser so not needed
//  0x050c5d44,             // [
//  0x050c5d42              // ]
};

// -----------------------------------------------------------------------
// push current state onto end of state stack

static void j_push(j_state_t *j)
{
    list_append_node(&j_stack, j_state);
    j_state = j;
}

// -----------------------------------------------------------------------
// deallocate current state structure but not the C structure associated
// with it.  then pop previous state off stack

void j_pop(void)
{
    free(j_state);

    j_state = list_pop(&j_stack);
}

// -----------------------------------------------------------------------

__attribute__((noreturn)) void json_error(char *s)
{
    fprintf(stderr, "%d:%d %s\n", line_no, line_index, s);
    uCurses_deInit();
    restore_term();
    _exit(1);
}

// -----------------------------------------------------------------------

void *j_alloc(uint32_t size)
{
    void *v = calloc(1, size);

    if(v != NULL)
    {
        return v;
    }
    json_error("Out of Memory!");
}

// -----------------------------------------------------------------------
// json files and individual objects must start with a {

static void json_state_l_brace(void)
{
    if(json_hash == json_syntax[JSON_L_BRACE])
    {
        j_state->state = STATE_KEY;
        return;
    }
    json_error("Opening brace missing");
}

// -----------------------------------------------------------------------
// every object and every key gets its own state structure allocation.
// the current object state level is always in the j_state variable.
// previous object states are pushed onto the json parse stack. key value
// states are never pushed onto the state stack

void json_new_state_struct(size_t struct_size, uint32_t struct_type)
{
    j_state_t *j;
    void *structure;

    // allocate a structure for the new state
    j = j_alloc(sizeof(*j));

    // if there is one allocate buffer for C structure for subseqeuent
    // keys to populate

    structure = (struct_size != 0)
        ? j_alloc(struct_size)
        : NULL;

    j->parent      = j_state;
    j->structure   = structure;
    j->struct_type = struct_type;

    // push previous state and make new state the current state
    j_push(j);
}

// -----------------------------------------------------------------------
// this series of IF statements produces significantly smaller code than
// a switch statement does.   C sucks

static void populate_attribs(void *pstruct, uint32_t ptype)
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
    else   // ptype == STRUCT_MENU_BAR:
    {
        *(uint64_t *)((menu_bar_t *)pstruct)->normal =
            *(uint64_t *)j_state->structure;
    }
    free(j_state->structure);
}

// -----------------------------------------------------------------------

static void populate_b_attribs(window_t *pstruct)
{
    *(uint64_t *)pstruct->bdr_attrs =
        *(uint64_t *)j_state->structure;

    free(j_state->structure);
}

// -----------------------------------------------------------------------

static void populate_s_attribs(void *pstruct, uint32_t ptype)
{
    if(ptype == STRUCT_PULLDOWN)
    {
        *(uint64_t *)((pulldown_t *)pstruct)->selected =
            *(uint64_t *)j_state->structure;
    }
    else   // ptype == STRUCT_MENU_BAR:
    {
        *(uint64_t *)((menu_bar_t *)pstruct)->selected =
            *(uint64_t *)j_state->structure;
    }

    free(j_state->structure);
}

// -----------------------------------------------------------------------

static void populate_d_attribs(void *pstruct, uint32_t ptype)
{
    if(ptype == STRUCT_PULLDOWN)
    {
        *(uint64_t *)((pulldown_t *)pstruct)->disabled =
            *(uint64_t *)j_state->structure;
    }
    else   // ptype == STRUCT_MENU_BAR:
    {
        *(uint64_t *)((menu_bar_t *)pstruct)->disabled =
            *(uint64_t *)j_state->structure;
    }

    free(j_state->structure);
}

// -----------------------------------------------------------------------

static void populate_pulldown(menu_bar_t *pstruct)
{
    uint16_t i;

    i = pstruct->count++;
    pstruct->items[i] = j_state->structure;
}

// -----------------------------------------------------------------------

static void populate_menu_item(pulldown_t *gstruct)
{
    uint16_t i;

    i = gstruct->count++;
    gstruct->items[i] = j_state->structure;
}

// -----------------------------------------------------------------------

static void populate_window(j_state_t *parent)
{
    window_t *win;
    screen_t *scr;

    j_state_t *gp = parent->parent;
    scr           = gp->structure;
    win           = j_state->structure;

    scr_win_attach(scr, win);
}

// -----------------------------------------------------------------------

static void populate_backdrop(screen_t *pstruct)
{
    pstruct->backdrop = j_state->structure;
}

// -----------------------------------------------------------------------

static void populate_bar(screen_t *scr)
{
    menu_bar_t *bar = j_state->structure;
    scr->menu_bar = bar;
}

// -----------------------------------------------------------------------
// an object has been completed and thereby the associated C structure is
// ready.  add this C structure to its parent objects C structure...
// or sometimes its grandparents

static void populate_parent(void)
{
    j_state_t *parent = j_state->parent;
    j_state_t *gp     = parent->parent;

    void *pstruct     = parent->structure;
    void *gstruct     = gp->structure;

    uint32_t ptype    = parent->struct_type;

    // when a psudo structure is completed all of the key values
    // specified within that psudo structure will have been
    // added to its parent - in this case this function will
    // be called to add that psudo structure to its parent but
    // it will not be any of the types specified below.  this is
    // an inconseauential waste of time as no action will be
    // taken below - this entire function becomes a NOP in that
    // case

    switch(j_state->struct_type)
    {
        case STRUCT_ATTRIBS:    populate_attribs(pstruct, ptype);   break;
        case STRUCT_B_ATTRIBS:  populate_b_attribs(pstruct);        break;
        case STRUCT_S_ATTRIBS:  populate_s_attribs(pstruct, ptype); break;
        case STRUCT_D_ATTRIBS:  populate_d_attribs(pstruct, ptype); break;
        case STRUCT_PULLDOWN:   populate_pulldown(gstruct);         break;
        case STRUCT_MENU_ITEM:  populate_menu_item(gstruct);        break;
        case STRUCT_WINDOW:     populate_window(parent);            break;
        case STRUCT_BACKDROP:   populate_backdrop(pstruct);         break;
        case STRUCT_MENU_BAR:   populate_bar(pstruct);              break;
    }
}

// -----------------------------------------------------------------------

static void json_state_r_brace(void)
{
    uint16_t has_comma = 0;
    uint16_t end = strlen(json_token) - 1;

    if(json_token[end] == ',')
    {
        json_token[end] = '\0';
        json_hash = fnv_hash(json_token);
        has_comma = 1;
    }

    if(json_hash != json_syntax[JSON_R_BRACE])
    {
        json_error("Closing brace missing");
    }

    // a right brace terminates a json object.  we need to add
    // the current object to its parents structure

    populate_parent();

    j_state->state = STATE_DONE;

    if(j_stack.count != 0)
    {
        j_pop();

        j_state->state = (has_comma != 0)
            ? STATE_KEY
            : STATE_R_BRACE;
    }
}

// -----------------------------------------------------------------------

static const switch_t states[] =
{
    { STATE_L_BRACE,  json_state_l_brace },
    { STATE_KEY,      json_state_key     },
    { STATE_VALUE,    json_state_value   },
    { STATE_R_BRACE,  json_state_r_brace },
};

// -----------------------------------------------------------------------

static void json_state_machine(void)
{
    int f;
    j_state = j_alloc(sizeof(*j_state));

    j_state->struct_type = -1;
    j_state->state = JSON_L_BRACE;

    while(j_state->state != STATE_DONE)
    {
        token();
        json_hash = fnv_hash(json_token);

        // the token does not define what the state is
        // the state defines what the token must be
        f = re_switch(states, NUM_STATES, j_state->state);

        if(f == -1)
        {
            json_error("Unknown or miss placed token");
        }
    }

    free(j_state);
}

// -----------------------------------------------------------------------

static void bounds_check(window_t *win)
{
    uint16_t xco = win->xco;
    uint16_t yco = win->yco;
    uint16_t z = (win->flags & WIN_BOXED) ? 1 :0;

    if(((xco + win->width  + z) <= console_width)  &&
       ((yco + win->height + z) <= console_height) &&
       ((xco - z) <= console_width) &&
       ((yco - z) <= console_height))
    {
        return;
    }

    json_error("Window outside bounds of screen");
}

// -----------------------------------------------------------------------
// fix far window position

static void fix_win(screen_t *scr, window_t *win)
{
    uint16_t fudge = 1;
    window_t *bd = scr->backdrop;

    if((bd != NULL) && ((bd->flags & WIN_BOXED) != 0))
    {
        fudge++;
    }

    if(win->xco == FAR)
    {
        win->xco = scr->width - (win->width + fudge);
    }

    if(win->yco == FAR)
    {
        win->yco = scr->height - (win->height + fudge);
    }
}

// -----------------------------------------------------------------------

static void build_ui(void)
{
    screen_t *scr = active_screen;
    window_t *win;

    if(scr_alloc(scr) != 0)
    {
        json_error("Error building UI from JSON data");
    }

    if(scr->backdrop != NULL)
    {
        init_backdrop(scr, scr->backdrop);
        win_alloc(scr->backdrop);
        win_clear(scr->backdrop);
    }

    node_t *n = scr->windows.head;

    while(n != NULL)
    {
        win = n->payload;
        fix_win(win->screen, win);
        bounds_check(win);
        win_alloc(win);
        win->blank = 0x20;
        win_clear(win);
        n = n->next;
    }
}

// -----------------------------------------------------------------------

// when the state machine parses in the structues that define the app
// menus it will need to add function pointers to every menu item to be
// executed when that menu item is selected.  this library can not
// determine the address of any functions in the application code it is linked against.   said application will need
// to supply this library with a pointer to a function that will return
// a pointer to a menu function based off of a string.  The HOW of this
// needs better documentation than im prepared to put in source file
// comments :)

int json_create_ui(char *path, fp_finder_t fp)
{
    struct winsize w;

    ioctl(0, TIOCGWINSZ, &w);
    console_width  = w.ws_col;
    console_height = w.ws_row;

    struct stat st;
    fp_finder = fp;

    int fd = open(path, O_RDONLY);

    if(fd < 0)
    {
        return -1;
    }

    fstat(fd, &st);
    json_len  = st.st_size;

    json_data = mmap(NULL, json_len, PROT_READ | PROT_WRITE,
        MAP_PRIVATE, fd, 0);

    close(fd);

    if(json_data == MAP_FAILED)
    {
        return -1;
    }

    json_de_tab(json_data, json_len);
    line_no = 1;
    json_state_machine();

    munmap(json_data, json_len);

    if(active_screen != NULL)
    {
        build_ui();
    }

    return 0;
}

// =======================================================================
