#define _XOPEN_SOURCE

#include <libchordtest/test.h>
#include "src/utf8.c"

TEST("what name", "what summary")
{
    log_debug(("name = %s, summary %s", __card__->name, __card__->summary ));
    ut_assert(true);
}
