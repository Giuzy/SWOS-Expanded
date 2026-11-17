/* Isolated real header + civil date calculation. NO Windows/gameplay claim.
 * gcc -std=c11 -Wall -Wextra -Werror -pedantic -Isorgenti/include \
 *   sorgenti/tests/test_expanded_debug_june30.c \
 *   sorgenti/src/expanded_world_time.c -o /tmp/test_june30 && /tmp/test_june30
 */
#include "expanded_debug_month.h"
#include <assert.h>
#include <stdio.h>
static void check(ExpandedWorldDate start,ExpandedWorldDate expected) {
    ExpandedWorldDate end={0};
    int32_t a=0,b=0;
    assert(ExpandedDebug_June30BeforeSeason(start,&end));
    assert(end.year==expected.year);
    assert(end.month==6u && end.day==30u);
    assert(ExpandedWorld_DateToIndex(start,&a));
    assert(ExpandedWorld_DateToIndex(end,&b) && b>a);
}
int main(void) {
    check((ExpandedWorldDate){1996,7,1},(ExpandedWorldDate){1997,6,30});
    check((ExpandedWorldDate){1997,1,15},(ExpandedWorldDate){1997,6,30});
    check((ExpandedWorldDate){1997,6,29},(ExpandedWorldDate){1997,6,30});
    check((ExpandedWorldDate){2026,11,17},(ExpandedWorldDate){2027,6,30});
    check((ExpandedWorldDate){2027,5,31},(ExpandedWorldDate){2027,6,30});
    check((ExpandedWorldDate){2028,2,29},(ExpandedWorldDate){2028,6,30});
    check((ExpandedWorldDate){2199,7,1},(ExpandedWorldDate){2200,6,30});
    ExpandedWorldDate out={0};
    assert(!ExpandedDebug_June30BeforeSeason(
        (ExpandedWorldDate){1997,6,30},&out));
    assert(!ExpandedDebug_June30BeforeSeason(
        (ExpandedWorldDate){2027,6,31},&out));
    assert(!ExpandedDebug_June30BeforeSeason(
        (ExpandedWorldDate){2200,7,1},&out));
    assert(!ExpandedDebug_June30BeforeSeason(
        (ExpandedWorldDate){2026,7,1},NULL));
    puts("DEBUG_JUNE30_DATE_ISOLATED=PASS");
    return 0;
}
