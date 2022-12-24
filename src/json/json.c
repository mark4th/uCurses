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

#include <stdio.h>

#include "uCurses.h"
#include "uC_switch.h"
#include "uC_utils.h"
#include "uC_screen.h"
#include "uC_json.h"

extern screen_t *active_screen;

// -----------------------------------------------------------------------
// memory map flags

#define MAP_FLAGS PROT_READ | PROT_WRITE

// -----------------------------------------------------------------------
// Custom Json parser state machine

// There are 5 states in this state machine as follows
//
//      STATE_L_BRACE
//      STATE_KEY
//      STATE_VALUE
//      STATE_R_BRACE
//      STATE_DONE
//
// As the json text is parsed the text does not define the state, rather,
// the state defines what the next character must be.  All parsing is done
// on a space delimited token by token basis.
//
// The initial state is STATE_L_BRACE which expects the next token parsed
// to be the left brace '{' char.  If it is then the state is set to
// STATE_KEY.   If not... oopts!
//
// The handler for STATE_KEY expect to see one of several known tokens
// which will either be a 'key' token or an 'object' token.  Only recobnized
// tokens are valid, unrecognized tokens will cause the state machine to
// puke.
//
// All tokens are recognized by their 32 bit fnv-1a hash value, no string
// compare functions are called.
//
// If the parsed token is recognized as a key...
//
//      That specifc key is handled (see below) and the next state is set
//      to STATE_VALUE where we extract the value to be stored in the
//      specified key.  note: while the ':' character between a key and
//      its associated value are required there is no state ':'
//
// If the parsed token is one of the recognized objects...
//
//      The state is set once again to STATE_L_BRACE.
//
// Almost every STATE_L_BRACE indicates that a new object and thereby an
// associated C structure is being created.
//
// For every object and every key the machine will transition into a new
// state.  If the previous state was an object then that object state is
// pushed onto a state stack.  Key states simply replace the previous key
// state with the new one, they are never pushed onto the state stack.
//
// Any time a new object or key token is parsed we check to see if there
// is a comma on it.  If there is no comma the next state will
// instead be STATE_R_BRACE which pops the previous state off the stack.
//
// As key values are parsed we immediately set the specified item in
// their parents C structure.  When an object is completed we store
// the associated C structure within its parents C structure.  The
// structure types of both specify how and where.
//
// When all states have been popped off the stack we transition into
// STATE_DONE and the state machine terminates.
//
// The following is WHY I wrote a custom json parser (which literally
// doubled the size of my "micro" sized library)
//
// This code also knows which keys may go inside which objects and which
// objects may go inside which objects.  You cannot for example define
// a screen structure within a window structure.
//
// As stated above, some object states do not have an associated C
// structure.  Any keys specified within these pseudo object will be
// assigned instead to the pseudo objects parent C structure. I.E. the
// keys grandparent.
//
// I have not given a way to define arrays in this parser and I have also
// allowed for values to be expressed as a percentage.  For example you
// can set a window within to a percentage of its parent screens width.

// -----------------------------------------------------------------------
// should i make the state struct a child of the vars struct?

json_vars_t *json_vars;
json_state_t *json_state;

// -----------------------------------------------------------------------
// fnv-1a hash values for various json syntax chars

const int32_t json_syntax[] =
{
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
};

// -----------------------------------------------------------------------
// push current state onto end of state stack

static void json_push(json_state_t *j)
{
    list_push_head(&json_vars->json_stack, json_state);
    json_state = j;
}

// -----------------------------------------------------------------------
// deallocate current state structure and pop previous state off stack.
// the compiled C structure associated with the destroyed state structure
// remains intact

void json_pop(void)
{
    free(json_state);

    json_state = list_pop_head(&json_vars->json_stack);
}

// -----------------------------------------------------------------------

__attribute__((noreturn)) void json_error(const char *s)
{
    char msg[128];

    sprintf(msg, "%d: %d:%d %s\n",
        json_state->line_no,
        json_vars->line_no,
        json_vars->line_index, s);
    xabort(msg);
}

// -----------------------------------------------------------------------
// allocate a new structure.  this can be a state structure or a target c
// structure.

void *json_alloc(uint32_t size)
{
    void *v = calloc(1, size);

    if (v != NULL)  { return v; }

    json_error("Out of Memory!");
}

// -----------------------------------------------------------------------
// json files and individual objects must start with a {

static void json_state_l_brace(void)
{
    // assert that the hash value of the most recently parsed json token
    // is equal to the expected left brace hash value
    if (json_vars->json_hash == json_syntax[JSON_L_BRACE])
    {
        json_state->state = STATE_KEY;
        return;
    }
    json_error("Opening brace missing");
}

// -----------------------------------------------------------------------
// every object and every key gets its own state structure allocation.
// the current object state level is always in the j_state variable.
// previous object states are pushed onto the json parse stack. key value
// states are never pushed onto the state stack

void json_new_state_struct(int struct_size, int32_t struct_type)
{
    void *structure;

    // allocate a structure for the new state
    json_state_t *j = json_alloc(sizeof(*j));

    // if there is one allocate buffer for C structure for subseqeuent
    // keys to populate

    structure = (struct_size != 0)
        ? json_alloc(struct_size)
        : NULL;

    j->parent      = json_state;
    j->structure   = structure;
    j->struct_type = struct_type;
    j->line_no     = json_vars->line_no;

    // push previous state and make new state the current state
    json_push(j);
}

// -----------------------------------------------------------------------
// strips comma and recalculates token hash

static bool check_comma(void)
{
    bool rv = false;

    int16_t end = strlen(json_vars->json_token) - 1;

    if (json_vars->json_token[end] == ',')
    {
        json_vars->json_token[end] = '\0';
        json_vars->json_hash = fnv_hash(json_vars->json_token);
        rv = true;
    }

    return rv;
}

// -----------------------------------------------------------------------

void json_state_r_brace(void)
{
    bool has_comma = check_comma();

    if (json_vars->json_hash != json_syntax[JSON_R_BRACE])
    {
        json_error("Closing brace missing");
    }

    json_state->state = STATE_DONE;

    if (json_state->struct_type != STRUCT_SCREEN)
    {
        populate_parent();
        json_pop();

        if (json_state != NULL)
        {
            json_state->state = (has_comma)
                ? STATE_KEY
                : STATE_R_BRACE;
        }
    }
}

// -----------------------------------------------------------------------

static const switch_t states[] =
{
    { STATE_L_BRACE, json_state_l_brace },
    { STATE_KEY,     json_state_key     },
    { STATE_VALUE,   json_state_value   },
    { STATE_R_BRACE, json_state_r_brace },
};

// -----------------------------------------------------------------------

static void json_state_machine(void)
{
    int f;

    json_state = json_alloc(sizeof(*json_state));

    json_state->struct_type = -1;
    json_state->state       = JSON_L_BRACE;

    do
    {
        token();
        json_vars->json_hash = fnv_hash(json_vars->json_token);

        // the token does not define what the state is
        // the state defines what the token must be

        f = re_switch(states, NUM_STATES, json_state->state);
        if (f == -1)
        {
            json_error("Unknown or out of place token");
        }
    } while (json_state->state != STATE_DONE);

    json_pop();

    free(json_state);
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

API void uC_json_create_ui(char *path, fp_finder_t fp)
{
    int result;
    struct winsize w;
    struct stat st;

    json_vars = calloc(1, sizeof(*json_vars));

    ioctl(0, TIOCGWINSZ, &w);
    json_vars->console_width  = w.ws_col;
    json_vars->console_height = w.ws_row;

    json_vars->fp_finder = fp;

    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        json_error("Cannot open JSON file");
    }

    result = fstat(fd, &st);
    if (result != 0)
    {
        json_error("Cannot stat JSON file");
    }
    json_vars->json_len = st.st_size;
    json_vars->json_data = mmap(NULL, json_vars->json_len,
        MAP_FLAGS, MAP_PRIVATE, fd, 0);
    close(fd);

    if (json_vars->json_data == MAP_FAILED)
    {
        json_error("Unable to map JSON file");
    }

    json_de_tab(json_vars->json_data, json_vars->json_len);
    json_vars->line_no = 1;

    json_state_machine();

    munmap(json_vars->json_data, json_vars->json_len);

    if (active_screen != NULL)
    {
        json_build_ui();
    }

    free(json_vars);
}

// =======================================================================
