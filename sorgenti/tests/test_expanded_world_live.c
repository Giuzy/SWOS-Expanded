#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_world_live.h"

/* A real serialized world round-trip, not only schedule generation.
 * The old July-1 bound rejected valid May fixtures after autosave. */
typedef struct WorldTestJson {
    char *text;
    size_t len, cap;
} WorldTestJson;
static int test_emit(const char *data,int n,void *ctx) {
    WorldTestJson *s=(WorldTestJson *)ctx;
    if (!s || !data || n<0) return 0;
    if (s->len+(size_t)n+1u > s->cap) {
        size_t cap=s->cap?s->cap:1024u;
        while (cap < s->len+(size_t)n+1u) cap*=2u;
        char *grown=(char *)realloc(s->text,cap);
        if (!grown) return 0;
        s->text=grown;s->cap=cap;
    }
    memcpy(s->text+s->len,data,(size_t)n);
    s->len+=(size_t)n;
    s->text[s->len]=0;
    return n;
}
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
    /* Club source-file inventory must cover every imported club without
     * treating league mobility or offers as a reason to lose its source. */
    {
        uint8_t files[256]={0};
        assert(ExpandedWorldLive_ClubFiles(files,256)==1u);
        assert(files[0]==8u);
        assert(ExpandedWorldLive_ClubFiles(NULL,256)==0u);
        assert(ExpandedWorldLive_ClubFiles(files,0)==0u);
    }
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
        /* Phase 70 is September: the world is still in August at week 15.
         * A future result MUST NOT be imported. This assertion verifies
         * the safety rule instead of bypassing it. Phase 58 is in the past
         * and is allowed to consume the one already-played fixture. */
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0802,70,&gh,&ga));
        assert(ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0802,58,&gh,&ga));
        assert(gh<=7 && ga<=7);
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0801,0x0802,58,&gh,&ga));
        /* Reading world state again is allowed even after a legacy
         * replay token has been consumed. */
        assert(ExpandedWorldLive_PeekNativeLeagueResult(0x0801,0x0802,&gh,&ga));
        assert(!ExpandedWorldLive_TakeNativeLeagueResult(0x0802,0x0801,70,&gh,&ga));
        assert(ExpandedWorldLive_TakeNativeLeagueResult(0x0803,0x0804,58,&gh,&ga));
        assert(ExpandedWorldLive_NativeMatches()==2);
        assert(ExpandedWorldLive_NativeMisses()==5);
        assert(ExpandedWorldLive_Results()==30);
    }

    /* Later the manager plays the actual native match (round 4, Aug 20).
     * The world ledger stores that authoritative result and simulates the
     * remaining CPU fixtures through the same native calendar phase.
     * Recording a score again must never count another played fixture. */
    /* Fasi OG 60/61: 16 e 20 agosto, NON settimane. La registrazione
     * autorevole fa avanzare l'unico orologio e non duplica la gara. */
    assert(ExpandedWorldLive_RecordNativeLeagueResult(0x0801,0x0805,60,2,1));
    ExpandedLiveCalendarState native_date;
    assert(ExpandedWorldLive_CalendarState(&native_date));
    assert(native_date.today.year==2026 && native_date.today.month==8 &&
           native_date.today.day==16);
    const uint32_t result_count=ExpandedWorldLive_Results();
    assert(result_count>=31u && result_count<=40u);
    assert(ExpandedWorldLive_RecordNativeLeagueResult(0x0801,0x0805,61,2,1));
    assert(ExpandedWorldLive_CalendarState(&native_date));
    assert(native_date.today.month==8 && native_date.today.day==20);
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

    /* Real 1996/97 data: country 21 league 0 has native begin=32 (May),
     * end=64 (September), unlike the European July 1 manager clock.
     * A serializer must preserve all its May/June matches across a .CAR
     * save/reload; these are NOT corrupt or from another career. */
    assert(ExpandedWorldLive_Begin(1996));
    for (unsigned i=0;i<10u;++i) {
        char name[18];
        snprintf(name,sizeof name,"MAY CLUB %u",i);
        assert(ExpandedWorldLive_AddClub(21,0,(uint16_t)(0x1500u+i),
                                        (uint16_t)(200+i),name,3));
    }
    assert(ExpandedWorldLive_SetLeagueSchedule(21,0,32,64,2));
    assert(ExpandedWorldLive_Seal());
    /* The first OG calendar starts before the July manager clock.
     * Its already-played May/June scores MUST be visible through the
     * exact same native league lookup, not excluded by August dates. */
    assert(ExpandedWorldLive_Results()>0u);
    unsigned may_fixtures_seen=0;
    for(unsigned home=0;home<10u;++home)
        for(unsigned away=0;away<10u;++away) {
            uint8_t gh=0,ga=0;
            if(home!=away &&
               ExpandedWorldLive_PeekNativeLeagueResult(
                   (uint16_t)(0x1500u+home),(uint16_t)(0x1500u+away),
                   &gh,&ga)) {
                assert(gh<=31u && ga<=31u);
                ++may_fixtures_seen;
            }
        }
    assert(may_fixtures_seen>0u);
    assert(ExpandedWorldLive_AdvanceWeeks(19));
    {
        const uint32_t saved_results=ExpandedWorldLive_Results();
        const uint32_t saved_fixtures=ExpandedWorldLive_Fixtures();
        WorldTestJson doc={0};
        assert(saved_results>0 && saved_fixtures>0);
        assert(test_emit("{\"living_world\":",16,&doc)==16);
        assert(ExpandedWorldLive_WriteJson(test_emit,&doc));
        assert(test_emit("}",1,&doc)==1);
        ExpandedWorldLive_End();
        assert(ExpandedWorldLive_ReadJson(doc.text,(int)doc.len));
        assert(ExpandedWorldLive_Fixtures()==saved_fixtures);
        assert(ExpandedWorldLive_Results()==saved_results);
        assert(ExpandedWorldLive_Clubs()==10);
        free(doc.text);
    }
    puts("OK: May-before-July native season survives compressed world .CAR round-trip");
    ExpandedWorldLive_End();
    return 0;
}
