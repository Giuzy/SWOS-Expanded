#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_midseason_join.h"
int main(void) {
    uint8_t week=99;
    assert(Expanded_MidseasonJoin_Cutoff(1996,0,&week) && week==0);
    assert(Expanded_MidseasonJoin_Cutoff(1996,4,&week) && week==0);
    assert(Expanded_MidseasonJoin_Cutoff(1996,5,&week) && week==1);
    assert(Expanded_MidseasonJoin_Cutoff(1996,10,&week) && week==6);
    assert(Expanded_MidseasonJoin_Cutoff(1996,35,&week) && week==31);
    assert(Expanded_MidseasonJoin_Cutoff(2026,35,&week) && week==31);
    assert(!Expanded_MidseasonJoin_Cutoff(1996,52,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,4000,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,10,NULL));
    puts("OK: joining dates July/August/January, late hire bounds, world years");
    return 0;
}
