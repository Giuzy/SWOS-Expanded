/* 70 completed league seasons, 11 leagues/216 teams, zero UI rendering.
 * Same original SWOS pairing tables and production Living World code.
 * This is a native C stress test, NOT a Windows career end-to-end test.
 * The manager/finance/transfer simulation is not implemented by this test. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "expanded_world_live.h"
#include "fixture_rounds.h"

typedef struct TestBuffer {
    char *ptr;
    size_t len,cap;
} TestBuffer;
static int emit_json(const char *data,int len,void *arg) {
    TestBuffer *b=(TestBuffer *)arg;
    if (!b || !data || len<0) return 0;
    size_t needed=b->len+(size_t)len+1u;
    if (needed>b->cap) {
        size_t cap=b->cap?b->cap:4096u;
        while(cap<needed) cap*=2u;
        char *ptr=(char *)realloc(b->ptr,cap);
        if(!ptr) return 0;
        b->ptr=ptr;b->cap=cap;
    }
    memcpy(b->ptr+b->len,data,(size_t)len);
    b->len+=(size_t)len;
    b->ptr[b->len]=0;
    return len;
}
static TestBuffer snapshot(void) {
    TestBuffer b={0};
    assert(emit_json("{\"living_world\":",16,&b)==16);
    assert(ExpandedWorldLive_WriteJson(emit_json,&b));
    assert(emit_json("}",1,&b)==1);
    return b;
}
static void round_trip(unsigned week,unsigned *count,size_t *max_bytes) {
    uint32_t fixtures=ExpandedWorldLive_Fixtures();
    uint32_t played=ExpandedWorldLive_Results();
    uint32_t archived=ExpandedWorldLive_ArchivedResults();
    uint32_t league_years=ExpandedWorldLive_ArchivedLeagues();
    uint16_t start=ExpandedWorldLive_WindowFirstYear();
    TestBuffer old=snapshot();
    assert(old.ptr && old.len>100u && old.len<2u*1024u*1024u);
    if(old.len>*max_bytes) *max_bytes=old.len;
    ExpandedWorldLive_End();
    assert(ExpandedWorldLive_ReadJson(old.ptr,(int)old.len));
    assert(ExpandedWorldLive_Fixtures()==fixtures);
    assert(ExpandedWorldLive_Results()==played);
    assert(ExpandedWorldLive_ArchivedResults()==archived);
    assert(ExpandedWorldLive_ArchivedLeagues()==league_years);
    assert(ExpandedWorldLive_WindowFirstYear()==start);
    assert(ExpandedWorldLive_Clubs()==216u);
    assert(ExpandedWorldLive_Leagues()==11u);
    assert(ExpandedWorldLive_Weeks()==week);
    /* Serialization is canonical: reloading must not silently reorder or
     * re-simulate fixtures, player records, seasons, or match results. */
    TestBuffer newer=snapshot();
    assert(old.len==newer.len && memcmp(old.ptr,newer.ptr,old.len)==0);
    free(old.ptr);free(newer.ptr);
    ++*count;
}
/* One chronological fixture list: no overdue games and no duplicate
 * result accounting even after a .CAR round-trip or 70 season rollovers. */
static void check_calendar(void) {
    ExpandedLiveCalendarState state;
    assert(ExpandedWorldLive_CalendarState(&state));
    assert(state.overdue==0u);
    assert(state.pending+ExpandedWorldLive_ResidentResults()==
           ExpandedWorldLive_Fixtures());
    if (state.has_next) {
        int32_t today=0,next=0;
        assert(ExpandedWorld_DateToIndex(state.today,&today));
        assert(ExpandedWorld_DateToIndex(state.next_date,&next));
        assert(next>=today);
        assert(state.next_matches>0u);
    } else assert(state.pending==0u);
}
int main(void) {
    const clock_t begin=clock();
    const unsigned leagues=11u,clubs=216u,years=70u,weeks=3679u;
    unsigned checkpoints=0;
    size_t max_snapshot=0;
    FixtureRounds_Install();
    assert(ExpandedWorldLive_Begin(1996));
    for(unsigned l=0;l<leagues;++l) {
        const unsigned members=l==10u?16u:20u;
        const uint8_t country=(uint8_t)(8u+l);
        for(unsigned c=0;c<members;++c) {
            char name[18];
            snprintf(name,sizeof name,"LW%02u CLUB%02u",l,c);
            assert(ExpandedWorldLive_AddClub(country,0,
                    (uint16_t)(((uint16_t)country<<8)|c),
                    (uint16_t)(160u+l*8u+c),name,(l%4u)?3u:2u));
        }
        const uint8_t from=(l%3u==0u)?32u:(l%3u==1u)?56u:72u;
        const uint8_t to=(l%3u==0u)?64u:(l%3u==1u)?32u:24u;
        assert(ExpandedWorldLive_SetLeagueSchedule(country,0,from,to,2));
    }
    assert(ExpandedWorldLive_Clubs()==clubs);
    assert(ExpandedWorldLive_Leagues()==leagues);
    assert(ExpandedWorldLive_Seal());
    assert(ExpandedWorldLive_Fixtures()==8080u);
    check_calendar();
    /* Four of the source OG leagues start in May. Their completed
     * May/June games MUST already be present on the manager's first
     * July 1 day: otherwise archiving the first season will stall. */
    assert(ExpandedWorldLive_Results()>0u);
    printf("INIT pre-career fixtures played=%lu\n",
           (unsigned long)ExpandedWorldLive_Results());
    uint32_t last_results=ExpandedWorldLive_Results();
    for(unsigned week=1u;week<=weeks;++week) {
        assert(ExpandedWorldLive_AdvanceWeeks(week));
        assert(ExpandedWorldLive_Weeks()==week);
        assert(ExpandedWorldLive_Fixtures()<=8080u);
        assert(ExpandedWorldLive_Results()>=last_results);
        assert(ExpandedWorldLive_Results()==
            ExpandedWorldLive_ArchivedResults() +
            ExpandedWorldLive_ResidentResults());
        assert(ExpandedWorldLive_ArchivedLeagues()==
               (unsigned)(ExpandedWorldLive_WindowFirstYear()-1996u)*leagues);
        assert(ExpandedWorldLive_ArchivedResults()==
               (unsigned)(ExpandedWorldLive_WindowFirstYear()-1996u)*4040u);
        last_results=ExpandedWorldLive_Results();
        /* Sept 1996: mid-week advance + snapshot without losing date,
         * followed by the next ordinary weekly tick. */
        if (week==11u) {
            ExpandedWorldDate start={1996u,7u,1u},midweek,back;
            int32_t day=0;
            assert(ExpandedWorld_DateAfterWeeks(start,week,&midweek));
            assert(ExpandedWorld_DateToIndex(midweek,&day));
            assert(ExpandedWorld_IndexToDate(day+3,&midweek));
            assert(ExpandedWorldLive_AdvanceToDate(midweek));
            assert(ExpandedWorldLive_Weeks()==week);
            check_calendar();
            TestBuffer before=snapshot();
            assert(before.ptr);
            ExpandedWorldLive_End();
            assert(ExpandedWorldLive_ReadJson(before.ptr,(int)before.len));
            TestBuffer after=snapshot();
            assert(after.ptr && before.len==after.len &&
                   memcmp(before.ptr,after.ptr,before.len)==0);
            free(before.ptr);free(after.ptr);
            check_calendar();
            assert(ExpandedWorld_DateAfterWeeks(start,week-1u,&back));
            assert(!ExpandedWorldLive_AdvanceToDate(back));
            puts("MIDWEEK_EVENT_SAVE_RELOAD=PASS");
        }
        if(week%260u==0u || week==weeks) {
            check_calendar();
            round_trip(week,&checkpoints,&max_snapshot);
            printf("CHECK week=%u window=%u resident=%lu archived_leagues=%lu "
                   "archived_games=%lu total_results=%lu max_save=%lu bytes\n",
                   week,(unsigned)ExpandedWorldLive_WindowFirstYear(),
                   (unsigned long)ExpandedWorldLive_Fixtures(),
                   (unsigned long)ExpandedWorldLive_ArchivedLeagues(),
                   (unsigned long)ExpandedWorldLive_ArchivedResults(),
                   (unsigned long)ExpandedWorldLive_Results(),
                   (unsigned long)max_snapshot);
        }
    }
    assert(ExpandedWorldLive_WindowFirstYear()==1996u+years);
    assert(ExpandedWorldLive_ArchivedLeagues()==years*leagues);
    assert(ExpandedWorldLive_ArchivedResults()==years*4040u);
    assert(ExpandedWorldLive_Fixtures()==8080u);
    assert(ExpandedWorldLive_SeasonYear()==2066u);
    for(unsigned year=1996u;year<1996u+years;++year)
        for(unsigned l=0;l<leagues;++l) {
            uint16_t champion=0xffffu;
            assert(ExpandedWorldLive_HistoryChampion((uint16_t)year,
                         (uint8_t)(l+8u),0,&champion));
            assert((champion>>8)==(l+8u));
        }
    const double elapsed=(double)(clock()-begin)/CLOCKS_PER_SEC;
    printf("\nLIVING WORLD 70-SEASON STRESS REPORT\n");
    printf("LEAGUE_SEASONS_ARCHIVED=%u\n",years);
    printf("CLUBS=%lu LEAGUES=%lu\n",
           (unsigned long)ExpandedWorldLive_Clubs(),
           (unsigned long)ExpandedWorldLive_Leagues());
    printf("RESULTS_ARCHIVED=%lu TOTAL_RESULTS=%lu\n",
           (unsigned long)ExpandedWorldLive_ArchivedResults(),
           (unsigned long)ExpandedWorldLive_Results());
    printf("RESIDENT_FIXTURES=%lu MAX_RESIDENT=8080\n",
           (unsigned long)ExpandedWorldLive_Fixtures());
    printf("SAVE_LOAD_CHECKPOINTS=%u MAX_WORLD_JSON_BYTES=%lu\n",
           checkpoints,(unsigned long)max_snapshot);
    printf("WORLD_RAM_MB=%.2f CPU_SECONDS=%.2f\n",
           (double)ExpandedWorldLive_MemoryBytes()/1048576.0,elapsed);
    puts("WORLD_EVENT_CURSOR=PASS");
    puts("STRESS_RESULT=PASS");
    ExpandedWorldLive_End();
    return 0;
}
