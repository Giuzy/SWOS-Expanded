/* Isolated date/policy test for the direct OG calendar +1 month trigger.
 * Compile with src/expanded_world_time.c; NO simulated fixture or menu
 * callback is stubbed and NO Windows gameplay PASS is implied. */
#include "expanded_debug_month.h"
#include <assert.h>
#include <stdio.h>
static int same(ExpandedWorldDate a,ExpandedWorldDate b) {
    return a.year==b.year && a.month==b.month && a.day==b.day;
}
static void check(ExpandedWorldDate start,ExpandedWorldDate expected) {
    ExpandedWorldDate out={0};
    assert(ExpandedDebug_NextMonthBeforeSeason(start,&out));
    assert(same(out,expected));
    uint16_t season=(uint16_t)(start.year-(start.month<7u));
    uint8_t phase=0;
    assert(ExpandedWorld_DateToNativePhaseNext(season,out,&phase));
    assert(phase<=144u);
}
int main(void) {
    check((ExpandedWorldDate){1996,7,1},(ExpandedWorldDate){1996,8,1});
    check((ExpandedWorldDate){2026,11,17},(ExpandedWorldDate){2026,12,17});
    check((ExpandedWorldDate){2026,12,31},(ExpandedWorldDate){2027,1,31});
    check((ExpandedWorldDate){2027,1,31},(ExpandedWorldDate){2027,2,28});
    check((ExpandedWorldDate){2028,1,31},(ExpandedWorldDate){2028,2,29});
    check((ExpandedWorldDate){2027,3,31},(ExpandedWorldDate){2027,4,30});
    check((ExpandedWorldDate){2027,5,31},(ExpandedWorldDate){2027,6,30});
    check((ExpandedWorldDate){2027,6,15},(ExpandedWorldDate){2027,6,30});
    ExpandedWorldDate out;
    assert(!ExpandedDebug_NextMonthBeforeSeason(
        (ExpandedWorldDate){2027,6,30},&out));
    assert(!ExpandedDebug_NextMonthBeforeSeason(
        (ExpandedWorldDate){2027,7,0},&out));
    assert(!ExpandedDebug_NextMonthBeforeSeason(
        (ExpandedWorldDate){2027,6,30},NULL));
    puts("PASS isolated: calendar-month targets, native phases and 30 June boundary");
    return 0;
}
