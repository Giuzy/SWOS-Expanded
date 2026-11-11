#include <assert.h>
#include <stdio.h>
#include "expanded_world_live.h"
#include "fixture_rounds.h"

int main(void) {
    /* Use precisely the original OpenSWOS source fixture tables, including
     * the installed odd-team schedule from the native fixture module. */
    FixtureRounds_Install();
    assert(ExpandedWorldLive_Begin(2026));
    for(unsigned i=1;i<=20;i++) {
        char name[18]; snprintf(name,sizeof name,"CLUB %u",i);
        assert(ExpandedWorldLive_AddClub(8,0,(uint16_t)((8u<<8)|i),
                                        (uint16_t)(200+i*4),name,3));
    }
    assert(ExpandedWorldLive_Seal());
    assert(ExpandedWorldLive_NativeScheduleLeagues()==1);
    assert(ExpandedWorldLive_SeasonYear()==2026);
    {
        uint16_t ids[20];
        uint8_t order[20];
        for(unsigned i=0;i<20;i++) ids[i]=(uint16_t)(0x0800u+20u-i);
        assert(ExpandedWorldLive_NativeOrder(ids,20,order));
        for(unsigned i=0;i<20;i++) assert(order[i]==19u-i);
        assert(ExpandedWorldLive_NativeOrderApplied()==1);
    }

    /* July is pre-season. Native original August phases produce the
     * first two rounds before the old synthetic weekly pilot did. */
    assert(ExpandedWorldLive_AdvanceWeeks(4));
    assert(ExpandedWorldLive_Results()==0);
    assert(ExpandedWorldLive_PreseasonGamesTotal()==40);
    for(unsigned i=1;i<=20;i++) {
        ExpandedLivePreseason ps;
        assert(ExpandedWorldLive_Preseason((uint16_t)((8u<<8)|i),&ps));
        assert(ps.friendlies==4 && ps.morale<=100 && ps.fatigue<=100);
    }
    assert(ExpandedWorldLive_AdvanceWeeks(5));
    assert(ExpandedWorldLive_Results()==20);
    assert(ExpandedWorldLive_AdvanceWeeks(6));
    assert(ExpandedWorldLive_Results()==30);

    /* Same OG pair (club1 vs club2), one result. Native order and
     * world ledger use the same home/away pairing and month phases. */
    {
        uint8_t gh=99,ga=99;
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0802,0,&gh,&ga));
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0802,152,&gh,&ga));
        assert(gh==99 && ga==99);
        /* Direct OG import must be read-only and must not mark the
         * ledger fixture consumed for future native replay. */
        assert(ExpandedWorldLive_PeekNativeLeagueResult(0x0801,0x0802,&gh,&ga));
        assert(gh<=7 && ga<=7);
        assert(!ExpandedWorldLive_PeekNativeLeagueResult(0x0802,0x0801,&gh,&ga));
        assert(ExpandedWorldLive_NativeMatches()==0);
        assert(ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0802,70,&gh,&ga));
        assert(gh<=7 && ga<=7);
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0802,58,&gh,&ga));
        /* Reading world state again is allowed even after a legacy
         * replay token has been consumed. */
        assert(ExpandedWorldLive_PeekNativeLeagueResult(0x0801,0x0802,&gh,&ga));
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0802,0x0801,70,&gh,&ga));
        assert(ExpandedWorldLive_TakeNativeLeagueResult(0x0803,0x0804,58,&gh,&ga));
        assert(ExpandedWorldLive_NativeMatches()==2);
        assert(ExpandedWorldLive_NativeMisses()==4);
        assert(ExpandedWorldLive_Results()==30);
    }

    /* Later the manager plays the actual native match (round 4, Aug 20).
     * The world ledger stores that authoritative result and simulates the
     * remaining CPU fixtures through the same native calendar phase.
     * Recording a score again must never count another played fixture. */
    assert(!ExpandedWorldLive_RecordNativeLeagueResult(0x0801,0x0805,60,2,1));
    assert(ExpandedWorldLive_Results()==30);
    assert(ExpandedWorldLive_RecordNativeLeagueResult(0x0801,0x0805,61,2,1));
    assert(ExpandedWorldLive_Results()==40);
    assert(ExpandedWorldLive_RecordNativeLeagueResult(0x0801,0x0805,61,3,0));
    assert(ExpandedWorldLive_Results()==40);

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
        assert(ps.friendlies==1);
    }
    assert(ExpandedWorldLive_Table(8,0,rows,26,&count));
    puts("OK: native SWOS pairings, phase calendar, exact club order and score replay");
    ExpandedWorldLive_End();

    assert(ExpandedWorldLive_Begin(1996));
    for(unsigned i=0;i<5;i++)
        assert(ExpandedWorldLive_AddClub(13,3,(uint16_t)(0x0d00u+i),
                                        200,"TEAM",3));
    assert(ExpandedWorldLive_Seal());
    assert(ExpandedWorldLive_NativeScheduleLeagues()==1);
    assert(ExpandedWorldLive_AdvanceWeeks(10));
    assert(ExpandedWorldLive_Results()==4);
    assert(ExpandedWorldLive_PreseasonGamesTotal()==8);
    assert(ExpandedWorldLive_Table(13,3,rows,26,&count) && count==5);
    unsigned total_played=0;
    for(unsigned i=0;i<count;i++) {
        assert(rows[i].played<=2);
        total_played+=rows[i].played;
    }
    assert(total_played==8);
    assert(ExpandedWorldLive_AdvanceWeeks(52));
    assert(ExpandedWorldLive_Results()==20);
    assert(ExpandedWorldLive_Table(13,3,rows,26,&count));
    for(unsigned i=0;i<count;i++) assert(rows[i].played==8);
    puts("OK: original native odd-club schedule, BYEs and return fixtures");
    ExpandedWorldLive_End();

    /* Single-cycle championships keep the native one-leg rules and
     * reverse the original pairing orientation (SWOS minSubstitutes bit). */
    assert(ExpandedWorldLive_Begin(2026));
    for (unsigned i=1;i<=4;++i) {
        char name[18]; snprintf(name,sizeof name,"ONELEG %u",i);
        assert(ExpandedWorldLive_AddClub(7,0,(uint16_t)(0x0700u+i),
                                         200,name,2));
    }
    assert(ExpandedWorldLive_SetLeagueSchedule(7,0,56,32,1));
    assert(ExpandedWorldLive_Seal());
    assert(ExpandedWorldLive_NativeScheduleLeagues()==1);
    assert(ExpandedWorldLive_AdvanceWeeks(52));
    assert(ExpandedWorldLive_Results()==6);
    assert(ExpandedWorldLive_Table(7,0,rows,26,&count) && count==4);
    for(unsigned i=0;i<count;i++) assert(rows[i].played==3);
    puts("OK: original native single-leg championship");
    ExpandedWorldLive_End();
    return 0;
}
