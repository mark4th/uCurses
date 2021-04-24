// uCurses json parsing for user interface layout
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
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
// to be the left brace '{' char.  If it is then the current state is
// incremented to STATE_KEY.
//
// the handler for STATE_KEY expect to see one of several known tokens
// which will either be a 'key' token or an 'object' token.  when parsing
// tokens no C string compare routines are used to recognize them, all
// tokens are recognized by their 32 bit fnv-1a hash value
//
// if the parsed token is recognized as a key
//
//      that specified key is handled (see below) and the next state
//      will be STATE_VALUE where we extract the value to be stored in the
//      specified key of the parent structure.
//
// if the parsed token is recognized as an object
//
//      the next state is set to STATE_L_BRACE.
//
// almost every STATE_L_BRACE indicates a new object and thereby an
// associated C structure that needs to be populated (not always, see
// below).
//
// For every object and every key parsed this state machine will
// transition into a new state.  If the previous state was an object that
// object state is pushed onto a state stack.  key states simply replace
// the previous key state, they are never pushed onto the state stack.
//
// Any time a new object or key token is parsed in we check to see if
// there is a comma on it.  If there is no comma on it we transition
// to STATE_R_BRACE which pops the previous state off the stack.
//
// as key values are parsed we immediately set the specified item in
// their parents C structure.  when an object is completed and we store
// the C structure associated with that object in its parents C structure.
//
// When all states have been popped off the stack we transition into
// STATE_DONE and the state machine terminates.
//
// this code also knows what keys may go inside what objects and what
// objects may go inside what objects.  any time youo try to do something
// such as define a screen inside a window this code will abort with an
// error message indicating the offending json line
//
// as stated above, some object states do not have an associated C
// structure.  any keys specified within these psudo object will be
// written instead into the psudo objects parent C structure

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

j_state_t *j_state;

// -----------------------------------------------------------------------
// json syntax

const uint32_t json_syntax[] =
{
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
    0x050c5d44,             // [
    0x050c5d42              // ]
};

// -----------------------------------------------------------------------

static void j_push(j_state_t *j)
{
    list_append_node(&j_stack, j_state);
    j_state = j;
}

// -----------------------------------------------------------------------

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
// classic example of why i hate c switch statements :P

// an object has been completed and thereby the associated C structure is
// ready.  add this C structure to its parent objects C structure...
// or sometimes its grandparents

static void populate_parent(void)
{
    uint16_t i;
    list_t *l;
    window_t *win;
    screen_t *scr;

    j_state_t *parent = j_state->parent;
    j_state_t *gp     = parent->parent;

    void *pstruct     = parent->structure;
    uint32_t ptype    = parent->struct_type;

    // whe a psudo structure is completed all of the key values
    // specified within that psudo structure will have been
    // added to its parent - in this case this function will
    // be called to add that psudo structure to its parent but
    // it will not be any of the types specified below.  this is
    // an inconseauential waste of time as no action will be
    // taken below - this entire function becomes a NOP in that
    // case

    switch(j_state->struct_type)
    {
        case STRUCT_ATTRIBS:
            switch(ptype)
            {
                case STRUCT_WINDOW:
                case STRUCT_BACKDROP:
                    *(uint64_t *)((window_t *)pstruct)->attrs =
                        *(uint64_t *)j_state->structure;
                    break;

                case STRUCT_PULLDOWN:
                    *(uint64_t *)((pulldown_t *)pstruct)->normal =
                        *(uint64_t *)j_state->structure;
                    break;

                case STRUCT_MENU_BAR:
                    *(uint64_t *)((menu_bar_t *)pstruct)->normal =
                        *(uint64_t *)j_state->structure;
                    break;
            }
            break;

        case STRUCT_B_ATTRIBS:
               *(uint64_t *)((window_t *)pstruct)->bdr_attrs =
                   *(uint64_t *)j_state->structure;
                break;

        case STRUCT_S_ATTRIBS:
            switch(ptype)
            {
                case STRUCT_PULLDOWN:
                    *(uint64_t *)((pulldown_t *)pstruct)->selected =
                        *(uint64_t *)j_state->structure;
                    break;

                case STRUCT_MENU_BAR:
                    *(uint64_t *)((menu_bar_t *)pstruct)->selected =
                        *(uint64_t *)j_state->structure;
                    break;

            }
            break;

        case STRUCT_D_ATTRIBS:
            switch(ptype)
            {
                case STRUCT_PULLDOWN:
                    *(uint64_t *)((pulldown_t *)pstruct)->disabled =
                        *(uint64_t *)j_state->structure;
                    break;

                case STRUCT_MENU_BAR:
                    *(uint64_t *)((menu_bar_t *)pstruct)->disabled =
                        *(uint64_t *)j_state->structure;
                    break;

            }
            break;

        case STRUCT_PULLDOWN:
            i = ((menu_bar_t *)pstruct)->count++;
            ((menu_bar_t *)pstruct)->items[i] = j_state->structure;
            break;

        case STRUCT_MENU_ITEM:
            i = ((pulldown_t *)pstruct)->count++;
            ((pulldown_t *)pstruct)->items[i] = j_state->structure;
            break;

        case STRUCT_WINDOW:
            scr         = gp->structure;
            win         = j_state->structure;
            win->screen = scr;
            l = &scr->windows;
            list_append_node(l, win);
            break;

        case STRUCT_BACKDROP:
            ((screen_t *)pstruct)->backdrop = j_state->structure;
            break;
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

// when the state machine parses in the structues that define the
// applications menus it will need to add function pointers to every
// menu item to be executed when that menu item is selected.  this
// library can not determine the address of any functions in the
// application code it is linked against.   said application will need
// to supply this library with a pointer to a function that will return
// a pointer to a menu function based off of a string.  The HOW of this
// needs better documentation than im prepared to put in source file
// comments :)

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

        f = re_switch(states, NUM_STATES, j_state->state);
        if(f == -1)
        {
            json_error("Unknown Token");
        }
    }
}

// -----------------------------------------------------------------------

static void adjust_win_pos(screen_t *scr, window_t *win)
{
    uint8_t b = ((win->flags & WIN_BOXED) == 0) ? 0 : 1;

    if((win->xco + win->width  + b) > scr->width)
    {
        win->xco = scr->width  - (win->width + b);
    }
    if((win->yco + win->height + b) > scr->height)
    {
        win->xco = scr->height - (win->height + b);
    }
}

// -----------------------------------------------------------------------

int json_create_ui(char *path, fp_finder_t fp)
{
    window_t *win;
    screen_t *scr;

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
    json_state_machine();

    munmap(json_data, json_len);

    scr = active_screen;
    if(scr != NULL)
    {
        scr_alloc(scr);

        if(scr->backdrop != NULL)
        {
            win = scr->backdrop;
            win_alloc(win);

            win->screen = scr;
            win->width  = scr->width - 2;
            win->height = scr->height - 2;
            win->xco = 1;
            win->yco = 1;
            win->flags = WIN_BOXED;
        }

        node_t *n = scr->windows.head;

        while(n != NULL)
        {
            win = n->payload;
            adjust_win_pos(win->screen, win);
            win_alloc(win);
            n = n->next;
        }
    }
    return 0;
}

// =======================================================================
