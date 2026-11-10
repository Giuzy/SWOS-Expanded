#include <assert.h>
#include <stdio.h>
#include "expanded_world_live.h"
int main(void) {
    assert(ExpandedWorldLive_Begin(2026));
    for(unsigned i=1;i<=20;i++) {
        char name[18]; snprintf(name,sizeof name,"CLUB %u",i);
        assert(ExpandedWorldLive_AddClub(8,0,(uint16_t)((8u<<8)|i),
                                        (uint16_t)(200+i*4),name,3));
    }
    assert(ExpandedWorldLive_Seal());
    assert(ExpandedWorldLive_AdvanceWeeks(23));
    assert(ExpandedWorldLive_Results()==230);
    ExpandedLiveRow rows[26]; uint32_t count=0,played=0;
    assert(ExpandedWorldLive_Table(8,0,rows,26,&count) && count==20);
    for(unsigned i=0;i<count;i++) played+=rows[i].played;
    assert(played==460);
    for(unsigned i=1;i<count;i++) assert(rows[i-1].points>=rows[i].points);
    assert(ExpandedWorldLive_AdvanceWeeks(35));
    assert(ExpandedWorldLive_Results()==350);
    assert(!ExpandedWorldLive_AdvanceWeeks(34));
    assert(ExpandedWorldLive_AdvanceWeeks(52));
    assert(ExpandedWorldLive_Results()==380);
    assert(ExpandedWorldLive_AdvanceWeeks(53));
    assert(ExpandedWorldLive_SeasonYear()==2027);
    assert(ExpandedWorldLive_Table(8,0,rows,26,&count));
    puts("OK: actual team ID pairing, 35 simulated weeks, next season");
    ExpandedWorldLive_End();
    assert(ExpandedWorldLive_Begin(1996));
    for(unsigned i=0;i<5;i++)
        assert(ExpandedWorldLive_AddClub(13,3,(uint16_t)(0x0d00u+i),
                                        200,"TEAM",3));
    assert(ExpandedWorldLive_Seal());
    assert(ExpandedWorldLive_AdvanceWeeks(10));
    assert(ExpandedWorldLive_Results()==20);
    assert(ExpandedWorldLive_Table(13,3,rows,26,&count) && count==5);
    for(unsigned i=0;i<count;i++) assert(rows[i].played==8);
    puts("OK: odd-sized division and BYE rounds");
    ExpandedWorldLive_End();
    return 0;
}
