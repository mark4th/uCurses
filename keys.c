// keys.c   - terminal keyboard handler
// -----------------------------------------------------------------------

#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "h/uCurses.h"

extern uint8_t *str_buff;
extern uint16_t nb;

// -----------------------------------------------------------------------

static void noop(void){;}

// -----------------------------------------------------------------------

typedef void key_handler(void);
typedef uint8_t key_reader(void);

// -----------------------------------------------------------------------

    key_reader *_keys;      // pointer to func to test if keys available
    key_reader *_key;       // pointer to func to read keys

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

uint8_t keys(void)  { return _keys(); }
uint8_t key(void)   { return _key();  }

// -----------------------------------------------------------------------
// returns 0 = no keys available, 1 = keys available

uint8_t test_keys(void)
{
    uint8_t x = poll(&pfd, 1, 0);

    if(0xff == x)
    {
        x = 0;
        // TODO: log warning
    }

    return x;
}

// -----------------------------------------------------------------------
// read single keypress

uint8_t read_key(void)
{
    uint8_t buffer;
    ssize_t n;

    do
    {
        n = read(1, &buffer, 1);
        // todo this might be bad :)
    } while(-1 == n);

    return buffer;
}

// -----------------------------------------------------------------------
// read escape sequence or singke keypress character

void read_keys(void)
{
    num_k = 0;

    do
    {
         keybuff[num_k++] = read_key();
    } while( 0 != test_keys());
}

// -----------------------------------------------------------------------
// put a enter character in keyboard input buffer

void ent(void)
{
    str_buff[0] = 0x0a;
    nb = 1;
}

// -----------------------------------------------------------------------
// put a backspace character in the keyboard input buffer

void kbs(void)
{
    str_buff[0] = 0x7f;
    nb = 1;
}

// -----------------------------------------------------------------------

// each of these puts a key sequence in the str_buff terminfo escape
// sequence buffer which is usually used to compile output data.
// these allow us to determine which key was pressed by comparing the
// actual sequence that was input with the data returned by each of these

void kdch1(void) { ti_kdch1(); }
void kcud1(void) { ti_kcud1(); }
void kf1(void)   { ti_kf1();   }
void kf10(void)  { ti_kf10();  }
void kf2(void)   { ti_kf2();   }
void kf3(void)   { ti_kf3();   }
void kf4(void)   { ti_kf4();   }
void kf5(void)   { ti_kf5();   }
void kf6(void)   { ti_kf6();   }
void kf7(void)   { ti_kf7();   }
void kf8(void)   { ti_kf8();   }
void kf9(void)   { ti_kf9();   }
void khome(void) { ti_khome(); }
void kich1(void) { ti_kich1(); }
void kcub1(void) { ti_kcub1(); }
void knp(void)   { ti_knp();   }
void kpp(void)   { ti_kpp();   }
void kcuf1(void) { ti_kcuf1(); }
void kcuu1(void) { ti_kcuu1(); }
void kcbt(void)  { ti_kcbt();  }
void kend(void)  { ti_kend();  }
void kent(void)  { ti_kent();  }
void kDC(void)   { ti_kDC();   }
void kEND(void)  { ti_kEND();  }
void kHOM(void)  { ti_kHOM();  }
void kIC(void)   { ti_kIC();   }
void kLFT(void)  { ti_kLFT();  }
void kNXT(void)  { ti_kNXT();  }
void kPRV(void)  { ti_kPRV();  }
void kRIT(void)  { ti_kRIT();  }
void kf11(void)  { ti_kf11();  }
void kf12(void)  { ti_kf12();  }

// -----------------------------------------------------------------------
// array of pointers to functions to get each key escape sequence

// order of items is important here

void (*k_table[24])() =
{
    ent,   kcuu1, kcud1, kcub1, kcuf1, kbs,
    kdch1, kich1, khome, kend,  knp,   kpp,
    kf1,   kf2,   kf3,   kf4,   kf5,   kf6,
    kf7,   kf8,   kf9,   kf10,  kf11,  kf12
};

// -----------------------------------------------------------------------

// compare input key sequence with each key seuence returned by the
// functions referenced in the above k_table

uint16_t match_key(void)
{
    int i;

    for(i = 0; i < 24; i++)
    {
      nb = 0;               // number chars in escape sequence buffer

      (*(*k_table[i]))();   // compile escape sequence for ith entry

      // the above k_table() call compiled an escape sequence into the
      // str_buff[] array.  compare it with the sequence in the
      // keyboard input buffer which is the escape sequence or a single
      // character of the key that was pressed

      if(num_k == nb)
      {
          if(0 == strcmp((const char *)&keybuff, (const char *)&str_buff))
          {
              return i;     // sequences match.
          }
      }
  }

  return -1;
}

// -----------------------------------------------------------------------
// add eol char to keyboard input buffer

void k_ent(void)
{
    keybuff[0] = 0x0a;
    num_k = 1;
}

// -----------------------------------------------------------------------
// add delete char to keyboard input buffer

void k_bs(void)
{
    keybuff[0] = 8;
    num_k = 1;
}

// -----------------------------------------------------------------------
// pointers to default handlers for each key press

    // you can directly modify these
    // you should not directly modify these :)

    key_handler *_key_ent   = k_ent;
    key_handler *_key_up    = noop;
    key_handler *_key_down  = noop;
    key_handler *_key_left  = noop;
    key_handler *_key_right = noop;
    key_handler *_key_bs    = k_bs;
    key_handler *_key_del   = noop;
    key_handler *_key_ins   = noop;
    key_handler *_key_home  = noop;
    key_handler *_key_end   = noop;
    key_handler *_key_np    = noop;
    key_handler *_key_pp    = noop;
    key_handler *_key_f1    = noop;
    key_handler *_key_f2    = noop;
    key_handler *_key_f3    = noop;
    key_handler *_key_f4    = noop;
    key_handler *_key_f5    = noop;
    key_handler *_key_f6    = noop;
    key_handler *_key_f7    = noop;
    key_handler *_key_f8    = noop;
    key_handler *_key_f9    = noop;
    key_handler *_key_f10   = noop;
    key_handler *_key_f11   = noop;
    key_handler *_key_f12   = noop;

// -----------------------------------------------------------------------
// the system indirectly calls the function pointers pointed to by the
// items in this array.  the end user does not get to modify this array as
// the order of items within it is critical

static key_handler **key_actions[24] =
{
    &_key_ent,    &_key_up,   &_key_down,  &_key_left,
    &_key_right,  &_key_bs,   &_key_del,   &_key_ins,
    &_key_home,   &_key_end,  &_key_np,    &_key_pp,
    &_key_f1,     &_key_f2,   &_key_f3,    &_key_f4,
    &_key_f5,     &_key_f6,   &_key_f7,    &_key_f8,
    &_key_f9,     &_key_f10,  &_key_f11,   &_key_f12
};

// -----------------------------------------------------------------------
// this pointer can be modified to point to a user table by calling a
// setter below..

static key_handler **(*key_action)[24] = &key_actions;

// -----------------------------------------------------------------------
// keep processing key sequences till we get a normal key

uint8_t new_key(void)
{
    uint16_t c;

    do
    {
        read_keys();        // read key or sequence into keyboard buff

        c = match_key();    // compare input with all handled escapes

        if(0xffff != c)     // if escape sequence is one we handle
        {
            nb = 0;         // ensure there are no keys in the buffer
            (*(*key_action)[c])();
        }
    } while (1 != num_k);

  return keybuff[0];
}

// -----------------------------------------------------------------------

// todo: add setter mentioned above :)

// =======================================================================
