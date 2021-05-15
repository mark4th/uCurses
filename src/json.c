// uCurses json parsing for user interface layout
// -----------------------------------------------------------------------

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------
// memory map flags

#define MAP_FLAGS PROT_READ | PROT_WRITE

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
//      That specifc key is handled (see below) and the next state is set
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
// their parents C structure.  When an object is completed we store
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

char *json_data;    // pointer to json data to be parsed
size_t json_len;    // total size of json data
int32_t json_index; // parse index into data (current line)

char line_buff[MAX_LINE_LEN];

int16_t line_no;
int16_t line_index; // line parse location
int16_t line_left;  // number of chars left to parse in line

// space delimited token extracted from data - +1 for the null

char json_token[TOKEN_LEN + 1];
int32_t json_hash;

list_t j_stack;

fp_finder_t fp_finder;

int16_t console_width;
int16_t console_height;

// -----------------------------------------------------------------------
// the current state

j_state_t *j_state;

// -----------------------------------------------------------------------
// fnv-1a hash values for various json syntax chars

const int32_t json_syntax[] = //
    {
        0x050c5d25, // :
        0x050c5d64, // {
        0x050c5d62, // }
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

__attribute__((noreturn)) void json_error(const char *s)
{
    char msg[128];
    sprintf(msg, "%d:%d %s\n", line_no, line_index, s);
    xabort(msg);
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

void json_new_state_struct(size_t struct_size, int32_t struct_type)
{
    j_state_t *j;
    void *structure;

    // allocate a structure for the new state
    j = j_alloc(sizeof(*j));

    // if there is one allocate buffer for C structure for subseqeuent
    // keys to populate

    structure =            //
        (struct_size != 0) //
            ? j_alloc(struct_size)
            : NULL;

    j->parent = j_state;
    j->structure = structure;
    j->struct_type = struct_type;

    // push previous state and make new state the current state
    j_push(j);
}

// -----------------------------------------------------------------------

static INLINE uint16_t check_comma(void)
{
    int16_t rv = 0;
    int16_t end = strlen(json_token) - 1;

    if(json_token[end] == ',')
    {
        json_token[end] = '\0';
        json_hash = fnv_hash(json_token);
        rv = 1;
    }
    return rv;
}

// -----------------------------------------------------------------------

void json_state_r_brace(void)
{
    int16_t has_comma;

    has_comma = check_comma();

    if(json_hash != json_syntax[JSON_R_BRACE])
    {
        json_error("Closing brace missing");
    }

    // thanks username234 for helping me find this!!!
    // if the current structures parent is null then the current structure
    // is the screen that has no parent so dont try to populate the non
    // existant parent with the screen structure! the real question here is
    // how the hell did this code work when compiled with clang? :)

    if(j_state->struct_type != STRUCT_SCREEN)
    {
        // a right brace terminates a json object.  we need to add
        // the current object to its parents structure (or in some cases
        // to its grandparent as its dirce parent is a dummy object)

        populate_parent();
    }

    j_state->state = STATE_DONE;

    if(j_stack.count != 0)
    {
        if(j_state->struct_type != STRUCT_SCREEN)
        {
            j_pop();

            if(j_state != NULL)
            {
                j_state->state =     //
                    (has_comma != 0) //
                        ? STATE_KEY
                        : STATE_R_BRACE;
            }
        }
    }
}

// -----------------------------------------------------------------------

static const switch_t states[] = //
    {
        { STATE_L_BRACE, json_state_l_brace },
        { STATE_KEY, json_state_key },
        { STATE_VALUE, json_state_value },
        { STATE_R_BRACE, json_state_r_brace },
    };

// -----------------------------------------------------------------------

static INLINE void json_state_machine(void)
{
    int f;

    j_state = j_alloc(sizeof(*j_state));

    j_state->struct_type = -1;
    j_state->state = JSON_L_BRACE;
    j_stack.count = 0;

    json_index = 0;

    do
    {
        token();
        json_hash = fnv_hash(json_token);

        // the token does not define what the state is
        // the state defines what the token must be
        f = re_switch(states, NUM_STATES, j_state->state);

        if(f == -1)
        {
            json_error("Unknown or out of place token");
        }

    } while(j_state->state != STATE_DONE);

    j_pop();

    free(j_state);
}

// -----------------------------------------------------------------------

// when the state machine parses in the structues that define the app
// menus it will need to add function pointers to every menu item to be
// executed when that menu item is selected.  this library can not
// determine the address of any functions in the application code it is
// linked against.   said application will need to supply this library
// with a pointer to a function that will return a pointer to a menu
// function based off of a string.  The HOW of this needs better
// documentation than im prepared to put in source file comments :)

void json_create_ui(char *path, fp_finder_t fp)
{
    int result;
    struct winsize w;

    ioctl(0, TIOCGWINSZ, &w);
    console_width = w.ws_col;
    console_height = w.ws_row;

    struct stat st;
    fp_finder = fp;

    int fd = open(path, O_RDONLY);

    if(fd < 0)
    {
        json_error("Cannot open JSON file");
    }

    result = fstat(fd, &st);
    if(result != 0)
    {
        json_error("Cannot stat JSON file");
    }
    json_len = st.st_size;

    json_data = mmap(NULL, json_len, MAP_FLAGS, MAP_PRIVATE, fd, 0);
    close(fd);

    if(json_data == MAP_FAILED)
    {
        json_error("Unable to map JSON file");
    }

    json_de_tab(json_data, json_len);
    line_no = 1;
    json_state_machine();

    munmap(json_data, json_len);

    if(active_screen != NULL)
    {
        json_build_ui();
    }
}

// =======================================================================
