// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_terminfo.h"
#include "uC_utils.h"

extern ti_parse_t *uC_ti_parse;
extern int8_t keybuff[KEY_BUFF_SZ];
extern int16_t num_k;

// -----------------------------------------------------------------------
// set index zero of the escape buffer to specified char

static void set_esc0(uint8_t c)
{
    uC_ti_parse->esc_buff[0] = c;
    uC_ti_parse->num_esc = 1;
}

// -----------------------------------------------------------------------
// ti_kent seems to return 3 bytes for me and none of them are 0x0a

static void kent(void) { set_esc0(0x0a); }

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

static void kbs(void)  { set_esc0(0x08); }
static void kbs2(void) { set_esc0(0x7f); }

// -----------------------------------------------------------------------

// each of these puts a key sequence in the esc_buff terminfo escape
// sequence buffer which is usually used to compile output data.
// these allow us to determine which key was pressed by comparing the
// actual sequence that was input with the data returned by each of these

// static void kent(void)  { ti_kent();  }

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

void (*k_table[])() =
{
    kent,  kcuu1, kcud1, kcub1, kcuf1, kbs,  kbs2,
    kdch1, kich1, khome, kend,  knp,   kpp,  kf1,
    kf2,   kf3,   kf4,   kf5,   kf6,   kf7,  kf8,
    kf9,   kf10,  kf11, kf12
};

#define KEY_COUNT (sizeof(k_table) / sizeof(k_table[0]))

// -----------------------------------------------------------------------

// compare input key sequence with each key seuence returned by the
// functions referenced in the above k_table

int16_t match_key(void)
{
    uint16_t i;
    int32_t hash1, hash2;

    for (i = 0; i < KEY_COUNT; i++)
    {
        // number chars in escape sequence buffer
        uC_ti_parse->num_esc = 0;

        // this isnt even a little bit obfuscated... honest!
        (*(*k_table[i]))(); // compile escape sequence for ith entry

        // the above k_table() call compiled an escape sequence into the
        // esc_buff[] array.  compare it with the sequence in the
        // keyboard input buffer which is the escape sequence or a single
        // character of the key that was pressed

        if (num_k == uC_ti_parse->num_esc)
        {
            uC_ti_parse->esc_buff[uC_ti_parse->num_esc] = '\0';
            // memcmp not safe
            hash1 = fnv_hash((char *)&keybuff[0]);
            hash2 = fnv_hash(&uC_ti_parse->esc_buff[0]);

            if (hash1 == hash2) { return i; }
        }
    }

    return -1;
}

// =======================================================================
