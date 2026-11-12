#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_midseason_join.h"
/* The no-SDL fallback reads this native career global. The stand-alone
 * calendar unit test intentionally does not link the full game. */
uint8_t _lastFinancialWeekIndex=0;
int main(void) {
    uint8_t week=99;
    assert(Expanded_MidseasonJoin_Cutoff(1996,0,&week) && week==0);
    assert(Expanded_MidseasonJoin_Cutoff(1996,4,&week) && week==0);
    /* Stop at the first phase on/after signing; ceil fractional
     * months to include all earlier fixtures, not just full past phases.
     * Native sortKey: January=0, March=16, August=56; yearly +0x60. */
    assert(Expanded_MidseasonJoin_Cutoff(1996,5,&week) && week==58);
    assert(Expanded_MidseasonJoin_Cutoff(1996,10,&week) && week==67);
    assert(Expanded_MidseasonJoin_Cutoff(1996,35,&week) && week==113);
    assert(Expanded_MidseasonJoin_Cutoff(2026,35,&week) && week==113);
    assert(Expanded_MidseasonJoin_Cutoff(1996,45,&week) && week==131);
    assert(Expanded_MidseasonJoin_Cutoff(1996,51,&week) && week==142);
    assert(!Expanded_MidseasonJoin_Cutoff(1996,52,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,4000,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,10,NULL));
    puts("OK: native month-phase cutover July/August/January, leap-year bounds");
    return 0;
}
