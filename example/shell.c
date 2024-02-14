// shell sort by Mark I Manning IV
// -----------------------------------------------------------------------

// I am not attempting to find the worlds best shell sort.  I am trying to
// make the best shell sort I can make which wont be "THE" best :)

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------
// just so i can move the cursor to a specific location using cup()

#include "uCurses.h"    // im building this from the examples sub dir
#include "uC_terminfo.h"
#include "uC_attribs.h"

// -----------------------------------------------------------------------

 #define SHELL            // comment out to use quick sort from stdlib

#define ITEMS  1000000   // one million
#define ITTERS 10000     // ten thousahd

// -----------------------------------------------------------------------
// this constant can have any value from 0.50 to 0.9 and will give about
// the same results most of the time but some values will have a negative
// impact on performance (choose wisely).

// higher numbers increase the number of itterations through the outer
// loop of the sort but the initial itterations will be over a smaller
// portion of the data set and win run faster.  higher initial gaps do
// not actaully scan over the entire array...

// [xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx] unsorted array
// [^                                  ^            ] initial gap points
// [            ^XXXXXXXXXXXXXXXXXXXXXX            ^] end of first pass

// none of the items marked X were looked at on the first itteration of
// the loop - the only way they could be would be is if the initial gap
// fudge factor is set to 0.50 which is shells original gap calculation

// this turns out to not actually be much of a problem :)

#define FUDGE_FACTOR 0.61f

// 0.50     26m37.343s          0.71        25m59.262s
// 0.51     23m53.941s          0.72        24m51.721s
// 0.52     23m44.335s          0.73        24m24.013s
// 0.53     23m58.989s          0.74        23m59.126s
// 0.54     23m38.090s          0.75        24m53.724s
// 0.55     23m10.643s          0.76        24m38.027s
// 0.56     23m07.106s          0.77        24m56.914s
// 0.57     23m20.263s          0.78        25m1.031s
// 0.58     23m33.156s          0.79        25m5.379s
// 0.59     23m20.764s          0.80        26m3.024s
// 0.60     23m21.731s
// 0.61     22m58.694s **
// 0.62     25m12.751s
// 0.63     25m21.917s
// 0.64     23m11.275s
// 0.65     22m59.873s
// 0.66     22m58.816s **
// 0.67     23m28.065s
// 0.68     23m35.820s
// 0.69     25m15.453s
// 0.70     24m18.513s

// theoretically the fastest known gap seauence by Robert Sedgewick
// my calculated gap seauence seems to be faster

uint32_t gaps[] =
{
    1391376, 463792, 198768, 86961, 33936, 13776,
    4592,    1968,   861,    336,   112,   48,
    21,      7,      3,      1,     0
};

// -----------------------------------------------------------------------
// get next gap from list

static uint32_t new_gap(uint32_t gap)
{
    uint32_t *g = gaps;
    while (*g >= gap) { g++; }
    return *g;
}

// -----------------------------------------------------------------------
// used in the qsort algorithm which i benchmark myself against

int compare(const void *a, const void *b)
{
    const uint32_t *aa, *bb;
    aa = a; bb = b;
    if (*aa < *bb) return -1;
    return (*aa == *bb) ? 0 : 1;
}

// -----------------------------------------------------------------------
// pretty much everything else in here is adminicular

// this code is not recursive so does not hammer on the stack the way the
// quick sort implementation from stdlib does.  the qsort implementation
// in ulibc is a shell soft for this very reason

void shell(uint32_t *p, uint32_t n)
{
    uint32_t i = 0, swapped;
    uint32_t tmp;
    int hi1, lo1, hi2, lo2;
    int gap;

    gap = n;

    while ((gap *= FUDGE_FACTOR) > 0)
    // while ((gap = new_gap(gap)) != 0)
    {
        lo1 = 0;
        hi1 = gap;

        while (hi1 < n)
        {
            hi2 = hi1;
            lo2 = lo1;
            swapped = 0;

            // shuttle this down to lowest point if it is
            // out of order...
            tmp = p[hi2];

            while ((lo2 >= 0) && (p[lo2] > tmp))
            {
                swapped = 1;
                p[hi2] = p[lo2];

                hi2 -= gap;
                lo2 -= gap;
            }
            if (swapped == 1)
            {
                p[lo2 + gap] = tmp;
            }
            hi1++;
            lo1++;
        }
        i++;
    }
    uC_cup(7, 20);
    uC_terminfo_flush();
    printf("%8d\n", i);
}

// -----------------------------------------------------------------------
// create a buffer of random gobbldegook to sort

void make_data(uint32_t *p, uint32_t n)
{
    uint32_t i;

    for (i = 0; i < n; i++)
    {
        p[i] = random();
    }
}

// -----------------------------------------------------------------------
// dump the contents of the buffer to the console after sorting

void dump_buff(uint32_t *p, uint32_t n)
{
    uint32_t i;

    for (i = 0; i < n; i++)
    {
        if ((i & 7) == 0) { printf("\n"); }
        printf("%08x ", p[i]);
    }
    printf("\n\n");
}

// -----------------------------------------------------------------------
// verify implementation works as advertised

static void verify_sort(uint32_t *p, uint32_t n)
{
    uint32_t q = p[--n];
    while ((n > 0) && (p[n] <= q))
    {
        q = p[n--];
    }
    if ((n == 0) && (p[n] <= q))
    {
        uC_console_set_fg(5);
        uC_terminfo_flush();
        printf("\n == GOOD SORT ==\n");
    }
    else
    {
        uC_console_set_fg(1);
        uC_terminfo_flush();
        printf("\n ** BAD SORT **\n");
        printf("%08x > %08x\n", p[0], p[1]);
    }
    uC_console_set_fg(7);
    uC_terminfo_flush();
}

// -----------------------------------------------------------------------

int main(void)
{
    uint32_t i;
    uint32_t *data;

    srandom(0xa9018502);

    // not defining any windows, just used to position cursor
    // on the screen
    uCurses_init();
    uC_clear();
    uC_cup(5,5);
    uC_terminfo_flush();

#ifdef SHELL
    printf("Shell Sorting %d items %d times\n",
        ITEMS, ITTERS);
#else
    printf("Quick sorting %d items %d tiems\n",
        ITEMS, ITTERS);
#endif

    // assume success :)
    data =  calloc(ITEMS, sizeof(uint32_t));

    for (i = 0; i < ITTERS; i++)
    {
        make_data(data, ITEMS);
        uC_cup(7, 10);
        uC_terminfo_flush();
        printf("%8d\n", i);

#ifdef SHELL
        shell(data, ITEMS);
#else
        qsort(data, ITEMS, sizeof(uint32_t), compare);
#endif
    }

//    dump_buff(data, ITEMS);
    verify_sort(data, ITEMS);

    return 0;
}

// =======================================================================
