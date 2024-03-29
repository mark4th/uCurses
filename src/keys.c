// keys.c   - terminal keyboard handler
// -----------------------------------------------------------------------

#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

extern int8_t *esc_buff;
extern int16_t num_esc;
int8_t keybuff[32];
int16_t num_k;
static int16_t stuffed;

#define KEY_COUNT (sizeof(k_table) / sizeof(k_table[0]))

// -----------------------------------------------------------------------

static void noop(void)
{
    ;
}

// -----------------------------------------------------------------------

static struct pollfd pfd = //
    {                      //
        0,                 // stdin
        POLLIN,            // want to know when data is available
        0
    };

// -----------------------------------------------------------------------
// returns 0 = no keys available, 1 = keys available

int8_t test_keys(void)
{
    int x;

    if((x = stuffed) == 0)
    {
        x = poll(&pfd, 1, 0);

        if(x == -1)
        {
            x = 0;
            // TODO: log warning
        }
    }
    stuffed = 0;

    return (int8_t)x;
}

// -----------------------------------------------------------------------
// read single keypress

static INLINE int8_t read_key(void)
{
    int8_t buffer;
    ssize_t n;

    do
    {
        n = read(1, &buffer, 1);
        // todo this might be bad :)
    } while(n == -1);

    return buffer;
}

// -----------------------------------------------------------------------
// read escape sequence or singke keypress character

static void read_keys(void)
{
    num_k = 0;

    do
    {
        keybuff[num_k++] = read_key();
    } while(0 != test_keys());
}

// -----------------------------------------------------------------------
// set index zero of the escape buffer to specified char

static void set_esc0(uint8_t c)
{
    esc_buff[0] = c;
    num_esc = 1;
}

// -----------------------------------------------------------------------
// ti_kent seems to return 3 bytes for me and none of them are 0x0a

static void kent(void)
{
    set_esc0(0x0a);
}

// -----------------------------------------------------------------------
// due to "design decisions" made elsewhere your backspace key may return
// either an 0x08 as specified in the terminfo file (^h) or it may return
// a 0x7f.
// this is not based on what terminal you are using, the same terminal
// might return one value on one system but the other value on another
// system.
// LUNACY like this is why libncurses is 200k in size.. NOT because TED
// is incompetent but because the people upstream from him are.  So those
// of you who are ranting and raving against TED have picked the wrong
// target </2c>

static void kbs(void)
{
    set_esc0(0x08);
}

static void kbs2(void)
{
    set_esc0(0x7f);
}

// -----------------------------------------------------------------------

// each of these puts a key sequence in the esc_buff terminfo escape
// sequence buffer which is usually used to compile output data.
// these allow us to determine which key was pressed by comparing the
// actual sequence that was input with the data returned by each of these

// static void kent(void)  { ti_kent();  }
static void kcuu1(void)
{
    ti_kcuu1();
}

static void kcud1(void)
{
    ti_kcud1();
}

static void kcub1(void)
{
    ti_kcub1();
}

static void kcuf1(void)
{
    ti_kcuf1();
}

static void kdch1(void)
{
    ti_kdch1();
}

static void kich1(void)
{
    ti_kich1();
}

static void khome(void)
{
    ti_khome();
}

static void kend(void)
{
    ti_kend();
}

static void knp(void)
{
    ti_knp();
}

static void kpp(void)
{
    ti_kpp();
}

static void kf1(void)
{
    ti_kf1();
}

static void kf2(void)
{
    ti_kf2();
}

static void kf3(void)
{
    ti_kf3();
}

static void kf4(void)
{
    ti_kf4();
}

static void kf5(void)
{
    ti_kf5();
}

static void kf6(void)
{
    ti_kf6();
}

static void kf7(void)
{
    ti_kf7();
}

static void kf8(void)
{
    ti_kf8();
}

static void kf9(void)
{
    ti_kf9();
}

static void kf10(void)
{
    ti_kf10();
}

static void kf11(void)
{
    ti_kf11();
}

static void kf12(void)
{
    ti_kf12();
}

// -----------------------------------------------------------------------
// array of pointers to functions to get each key escape sequence

// order of items is important here

void (*k_table[])() = //
    {                 //
        kent,  kcuu1, kcud1, kcub1, kcuf1, kbs,  kbs2, kdch1, kich1,
        khome, kend,  knp,   kpp,   kf1,   kf2,  kf3,  kf4,   kf5,
        kf6,   kf7,   kf8,   kf9,   kf10,  kf11, kf12
    };

// -----------------------------------------------------------------------

// compare input key sequence with each key seuence returned by the
// functions referenced in the above k_table

static INLINE int16_t match_key(void)
{
    uint16_t i;
    int16_t q;

    for(i = 0; i < KEY_COUNT; i++)
    {
        num_esc = 0; // number chars in escape sequence buffer

        (*(*k_table[i]))(); // compile escape sequence for ith entry

        // the above k_table() call compiled an escape sequence into the
        // esc_buff[] array.  compare it with the sequence in the
        // keyboard input buffer which is the escape sequence or a single
        // character of the key that was pressed

        if(num_k == num_esc)
        {
            q = memcmp((const char *)&keybuff[0],
                       (const char *)&esc_buff[0], num_k);
            if(q == 0)
            {
                return i; // sequences match.
            }
        }
    }

    return -1;
}

// -----------------------------------------------------------------------

static void set_kb0(int8_t c)
{
    keybuff[0] = c;
    num_k = 1;
}

// -----------------------------------------------------------------------
// add delete char to keyboard input buffer

static void k_bs(void)
{
    set_kb0(0x08);
}

static void k_ent(void)
{
    set_kb0(0x0a);
}

// -----------------------------------------------------------------------
// the system indirectly calls the function pointers pointed to by the
// items in this array.  the end user does not get to modify this array as
// the order of items within it is critical

static key_handler_t *default_key_actions[] = //
    {                                         //
      //  ENTER   UP     DOWN   LEFT   RIGHT   BS     BS2
        k_ent, noop, noop, noop, noop, k_bs, k_bs,
        //  DEL     INSERT HOME   END    PDN     PUP    F1
        noop, noop, noop, noop, noop, noop, noop,
        //  F2      F3     F4     F5     F6      F7     F8
        noop, noop, noop, noop, noop, noop, noop,
        //  F9      F10    F11    F12
        noop, noop, noop, noop
    };

// -----------------------------------------------------------------------

key_handler_t *user_key_actions[KEY_COUNT] = { NULL };

// -----------------------------------------------------------------------

key_handler_t *set_key_action(key_index_t index, key_handler_t *action)
{
    key_handler_t *x = user_key_actions[index];
    user_key_actions[index] = action;

    return x;
}

// -----------------------------------------------------------------------
// keep processing key sequences till we get a normal key

uint8_t key(void)
{
    int16_t c;

    while((num_k != 1) && (stuffed != 1))
    {
        memset(keybuff, 0, 32);
        read_keys(); // read key or sequence into keyboard buff

        c = match_key(); // compare input with all handled escapes

        if(c != -1)      // if escape sequence is one we handle
        {                // internally
            num_esc = 0; // flush the escape buffer
            (user_key_actions[c])();
            // right now i think this is only the delete key
            // which returns a different value based on
            // planetary alignment or something
            if(num_k == 1)
            {
                break;
            }
            num_k = 0;
            return 0;
        }
    }
    stuffed = 0;
    num_k = 0;
    return keybuff[0];
}

// -----------------------------------------------------------------------
// manyally stuff a key into the buffer as if a key had been pressed.

void stuff_key(int8_t c)
{
    stuffed = 1;
    num_k = 1;
    keybuff[0] = c;
}

// -----------------------------------------------------------------------

void init_key_handlers(void)
{
    memcpy(user_key_actions, default_key_actions,
           sizeof(user_key_actions));
}

// =======================================================================
