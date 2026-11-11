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
    assert(ExpandedWorldLive_SeasonYear()==2026);
    /* July is pre-season; first pilot competitive league fixture is Aug 8. */
    assert(ExpandedWorldLive_AdvanceWeeks(4));
    assert(ExpandedWorldLive_Results()==0);
    assert(ExpandedWorldLive_PreseasonGamesTotal()==40);
    for(unsigned i=1;i<=20;i++) {
        ExpandedLivePreseason ps;
        assert(ExpandedWorldLive_Preseason((uint16_t)((8u<<8)|i),&ps));
        assert(ps.friendlies==4 && ps.morale<=100 && ps.fatigue<=100);
    }
    assert(ExpandedWorldLive_AdvanceWeeks(4));
    assert(ExpandedWorldLive_PreseasonGamesTotal()==40);
    assert(ExpandedWorldLive_AdvanceWeeks(5));
    assert(ExpandedWorldLive_Results()==0);
    assert(ExpandedWorldLive_AdvanceWeeks(6));
    assert(ExpandedWorldLive_Results()==10);
    /* The same home/away encounter may be on a DIFFERENT match day in
     * the pilot and native schedules. Reuse its already-played score
     * anywhere in the SAME SEASON; never across season boundaries. */
    {
        uint8_t gh=99,ga=99;
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0814,0,&gh,&ga));
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0814,152,&gh,&ga));
        assert(gh==99 && ga==99);
        assert(ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0814,70,&gh,&ga));
        assert(gh<=7 && ga<=7);
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0814,58,&gh,&ga));
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0814,0x0801,70,&gh,&ga));
        assert(ExpandedWorldLive_TakeNativeLeagueResult(0x0813,0x0802,58,&gh,&ga));
        assert(ExpandedWorldLive_NativeMatches()==2);
        assert(ExpandedWorldLive_NativeMisses()==4);
        assert(ExpandedWorldLive_Results()==10);
    }
    assert(ExpandedWorldLive_AdvanceWeeks(23));
    assert(ExpandedWorldLive_Results()==180);
    ExpandedLiveRow rows[26]; uint32_t count=0,played=0;
    assert(ExpandedWorldLive_Table(8,0,rows,26,&count) && count==20);
    for(unsigned i=0;i<count;i++) played+=rows[i].played;
    assert(played==360);
    for(unsigned i=1;i<count;i++) assert(rows[i-1].points>=rows[i].points);
    assert(ExpandedWorldLive_AdvanceWeeks(35));
    assert(ExpandedWorldLive_Results()==300);
    assert(!ExpandedWorldLive_AdvanceWeeks(34));
    assert(ExpandedWorldLive_AdvanceWeeks(52));
    assert(ExpandedWorldLive_Results()==380);
    assert(ExpandedWorldLive_AdvanceWeeks(53));
    assert(ExpandedWorldLive_SeasonYear()==2027);
    assert(ExpandedWorldLive_PreseasonGamesTotal()==50);
    {
        ExpandedLivePreseason ps;
        assert(ExpandedWorldLive_Preseason((uint16_t)((8u<<8)|1u),&ps));
        assert(ps.friendlies==1); /* July 7 of following season */
    }
    assert(ExpandedWorldLive_Table(8,0,rows,26,&count));
    puts("OK: July start, four AI preseason dates, August fixtures and following season");
    ExpandedWorldLive_End();
    assert(ExpandedWorldLive_Begin(1996));
    for(unsigned i=0;i<5;i++)
        assert(ExpandedWorldLive_AddClub(13,3,(uint16_t)(0x0d00u+i),
                                        200,"TEAM",3));
    assert(ExpandedWorldLive_Seal());
    assert(ExpandedWorldLive_AdvanceWeeks(10));
    assert(ExpandedWorldLive_Results()==10);
    assert(ExpandedWorldLive_PreseasonGamesTotal()==8);
    assert(ExpandedWorldLive_Table(13,3,rows,26,&count) && count==5);
    for(unsigned i=0;i<count;i++) assert(rows[i].played==4);
    puts("OK: odd-sized division and BYE rounds");
    ExpandedWorldLive_End();
    return 0;
}
