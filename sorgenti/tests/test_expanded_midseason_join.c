#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_midseason_join.h"
int main(void) {
    uint8_t week=99;
    assert(Expanded_MidseasonJoin_Cutoff(1996,0,&week) && week==0);
    assert(Expanded_MidseasonJoin_Cutoff(1996,4,&week) && week==0);
    /* Native sortKey: January=0, March=16, August=56; yearly wrap +0x60. */
    assert(Expanded_MidseasonJoin_Cutoff(1996,5,&week) && week==57);
    assert(Expanded_MidseasonJoin_Cutoff(1996,10,&week) && week==66);
    assert(Expanded_MidseasonJoin_Cutoff(1996,35,&week) && week==112);
    assert(Expanded_MidseasonJoin_Cutoff(2026,35,&week) && week==112);
    assert(Expanded_MidseasonJoin_Cutoff(1996,45,&week) && week==130);
    assert(Expanded_MidseasonJoin_Cutoff(1996,51,&week) && week==141);
    assert(!Expanded_MidseasonJoin_Cutoff(1996,52,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,4000,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,10,NULL));
    puts("OK: native month-phase cutover July/August/January, leap-year bounds");
    return 0;
}
