// keys.c   - terminal keyboard handler
// -----------------------------------------------------------------------

#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "h/uCurses.h"

extern uint8_t *esc_buff;
extern uint16_t num_esc;

// -----------------------------------------------------------------------

static void noop(void){;}

// -----------------------------------------------------------------------
// key sequence input buffer

static uint8_t keybuff[32];
static uint16_t num_k;

// -----------------------------------------------------------------------

static struct pollfd pfd =
{
    0,                      // stdin
    POLLIN,                 // want to know when data is available
    0
};

// -----------------------------------------------------------------------
// returns 0 = no keys available, 1 = keys available

uint8_t test_keys(void)
{
    uint8_t x = poll(&pfd, 1, 0);

    if(x == 0xff)
    {
        x = 0;
        // TODO: log warning
    }

    return x;
}

// -----------------------------------------------------------------------
// read single keypress

static uint8_t read_key(void)
{
    uint8_t buffer;
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
    } while( 0 != test_keys());
}

// -----------------------------------------------------------------------
// put a enter character in keyboard input buffer

static void ent(void)
{
    esc_buff[0] = 0x0a;
    num_esc = 1;
}

// -----------------------------------------------------------------------
// put a backspace character in the keyboard input buffer

static void kbs(void)
{
    esc_buff[0] = 0x7f;
    num_esc = 1;
}

// -----------------------------------------------------------------------

// each of these puts a key sequence in the esc_buff terminfo escape
// sequence buffer which is usually used to compile output data.
// these allow us to determine which key was pressed by comparing the
// actual sequence that was input with the data returned by each of these

 static void kcuu1(void) { ti_kcuu1(); }
 static void kcud1(void) { ti_kcud1(); }
 static void kcub1(void) { ti_kcub1(); }
 static void kcuf1(void) { ti_kcuf1(); }

 static void kdch1(void) { ti_kdch1(); }
 static void kich1(void) { ti_kich1(); }
 static void khome(void) { ti_khome(); }
 static void kend(void)  { ti_kend();  }
 static void knp(void)   { ti_knp();   }
 static void kpp(void)   { ti_kpp();   }

 static void kf1(void)   { ti_kf1();   }
 static void kf2(void)   { ti_kf2();   }
 static void kf3(void)   { ti_kf3();   }
 static void kf4(void)   { ti_kf4();   }
 static void kf5(void)   { ti_kf5();   }
 static void kf6(void)   { ti_kf6();   }
 static void kf7(void)   { ti_kf7();   }
 static void kf8(void)   { ti_kf8();   }
 static void kf9(void)   { ti_kf9();   }
 static void kf10(void)  { ti_kf10();  }
 static void kf11(void)  { ti_kf11();  }
 static void kf12(void)  { ti_kf12();  }

// -----------------------------------------------------------------------
// array of pointers to functions to get each key escape sequence

// order of items is important here

void (*k_table[24])() =
{
    ent,   kcuu1, kcud1, kcub1,
    kcuf1, kbs,   kdch1, kich1,
    khome, kend,  knp,   kpp,
    kf1,   kf2,   kf3,   kf4,
    kf5,   kf6,   kf7,   kf8,
    kf9,   kf10,  kf11,  kf12
};

// -----------------------------------------------------------------------

// compare input key sequence with each key seuence returned by the
// functions referenced in the above k_table

static uint16_t match_key(void)
{
    int i;
    uint16_t q;

    for(i = 0; i < 24; i++)
    {
        num_esc = 0;          // number chars in escape sequence buffer

        (*(*k_table[i]))();   // compile escape sequence for ith entry

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
                return i;     // sequences match.
            }
        }
    }

    return -1;
}

// -----------------------------------------------------------------------
// add eol char to keyboard input buffer

static void k_ent(void)
{
    keybuff[0] = 0x0a;
    num_k = 1;
}

// -----------------------------------------------------------------------
// add delete char to keyboard input buffer

static void k_bs(void)
{
    keybuff[0] = 8;
    num_k = 1;
}

// -----------------------------------------------------------------------
// the system indirectly calls the function pointers pointed to by the
// items in this array.  the end user does not get to modify this array as
// the order of items within it is critical

static key_handler_t *default_key_actions[24] =
{
//  ENTER  UP    DOWN  LEFT  RIGHT  BS    DEL   INSERT
    k_ent, noop, noop, noop, noop,  k_bs, noop, noop,
//  HOME   END   PDN   PUP   F1     F2    F3    F4
    noop,  noop, noop, noop, noop,  noop, noop, noop,
//  F5     F6    F7    F8    F9     F10   F11   F12
    noop,  noop, noop, noop, noop,  noop, noop, noop
};

// -----------------------------------------------------------------------

key_handler_t *user_key_actions[24] = { NULL };

// -----------------------------------------------------------------------

key_handler_t *set_key_action(key_index_t index, key_handler_t* action)
{
    key_handler_t *x = user_key_actions[index];
    user_key_actions[index] = action;

    return x;
}

// -----------------------------------------------------------------------
// keep processing key sequences till we get a normal key

uint8_t key(void)
{
    uint16_t c;

    do
    {
        read_keys();        // read key or sequence into keyboard buff

        c = match_key();    // compare input with all handled escapes

        if(c != 0xffff)     // if escape sequence is one we handle
        {
            num_esc = 0;    // ensure there are no keys in the buffer
            (user_key_actions[c])();
            return 0xff;
        }
    } while (1 != num_k);

    return keybuff[0];
}

// -----------------------------------------------------------------------

void init_key_handlers(void)
{
    memcpy(user_key_actions, default_key_actions,
        sizeof(user_key_actions));
}

// =======================================================================
