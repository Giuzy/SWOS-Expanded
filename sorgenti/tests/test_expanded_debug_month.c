/* Isolated calendar target tests: no game loop, fixtures or save writes.
 * cc -std=c11 -Wall -Wextra -Werror -Isorgenti/include \
 *   sorgenti/tests/test_expanded_debug_month.c \
 *   sorgenti/src/expanded_world_time.c -o /tmp/test_month && /tmp/test_month
 */
#include "expanded_debug_month.h"
#include <assert.h>
#include <stdio.h>
static void check(unsigned y,unsigned m,unsigned d,
                  unsigned ey,unsigned em,unsigned ed)
{
    ExpandedWorldDate out={0};
    assert(ExpandedDebug_NextMonthBeforeSeason(
        (ExpandedWorldDate){y,m,d},&out));
    assert(out.year==ey && out.month==em && out.day==ed);
}
int main(void)
{
    check(2026,7,1,2026,8,1);
    check(2026,11,17,2026,12,17);
    check(2026,12,31,2027,1,31);
    check(2027,1,31,2027,2,28);
    check(2028,1,31,2028,2,29);
    check(2027,5,31,2027,6,30);
    check(2027,6,1,2027,6,30);
    check(2027,6,29,2027,6,30);
    assert(!ExpandedDebug_NextMonthBeforeSeason(
        (ExpandedWorldDate){2027,6,30},&(ExpandedWorldDate){0}));
    assert(!ExpandedDebug_NextMonthBeforeSeason(
        (ExpandedWorldDate){2027,6,31},&(ExpandedWorldDate){0}));
    assert(!ExpandedDebug_NextMonthBeforeSeason(
        (ExpandedWorldDate){2026,7,1},NULL));
    puts("DEBUG_MONTH_DATE_ISOLATED=PASS");
    return 0;
}
