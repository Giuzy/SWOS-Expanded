#include "expanded_og_season_retention.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    assert(ExpandedOG_RetainContinentalType2(1,1,1996,1996));
    assert(ExpandedOG_RetainContinentalType2(1,1,2026,2026));
    assert(!ExpandedOG_RetainContinentalType2(0,1,1996,1996));
    assert(!ExpandedOG_RetainContinentalType2(1,0,1996,1996));
    assert(!ExpandedOG_RetainContinentalType2(1,1,1996,1997));
    assert(!ExpandedOG_RetainContinentalType2(1,1,0,0));
    for(unsigned slot=1;slot<=2;++slot)
        assert(ExpandedOG_RetainContinentalType1(1,1,254,(uint8_t)slot,1996,1996));
    assert(!ExpandedOG_RetainContinentalType1(1,1,254,3,1996,1996));
    assert(!ExpandedOG_RetainContinentalType1(1,1,8,2,1996,1996));
    assert(!ExpandedOG_RetainContinentalType1(1,0,254,2,1996,1996));
    assert(!ExpandedOG_RetainContinentalType1(0,1,254,2,1996,1996));
    puts("PASS: preserve previous genuine continental native contexts, not domestic or foreign edition");
    return 0;
}
