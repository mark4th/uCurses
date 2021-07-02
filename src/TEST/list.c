#include <libchordtest/test.h>
#include "src/list.c"

TEST("what name", "what summary")
{
    log_debug(("name = %s, summary %s", __card__->name, __card__->summary ));
    ut_assert(true);
}
