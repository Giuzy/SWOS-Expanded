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
    const uint32_t cup_matches=ExpandedWorldLive_CupResidentMatches();
    const uint32_t cup_titles=ExpandedWorldLive_CupResidentTitles();
    const uint32_t cup_old_matches=ExpandedWorldLive_CupArchivedMatches();
    const uint32_t cup_old_titles=ExpandedWorldLive_CupArchivedTitles();
    const uint32_t historical_titles=ExpandedWorldLive_CupHistoricalTitles();
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
    assert(ExpandedWorldLive_CupResidentMatches()==cup_matches);
    assert(ExpandedWorldLive_CupResidentTitles()==cup_titles);
    assert(ExpandedWorldLive_CupArchivedMatches()==cup_old_matches);
    assert(ExpandedWorldLive_CupArchivedTitles()==cup_old_titles);
    assert(ExpandedWorldLive_CupHistoricalTitles()==historical_titles);
    if(historical_titles) {
        uint16_t first_winner=0;
        assert(ExpandedWorldLive_CupHistoryChampion(1996u,
                 "NATIONAL CUP",1u,&first_winner));
        assert(first_winner==0x0801u);
    }
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
    /* Eligibility comes from EVERY scheduled country, not the player's
     * nationality or the previous five-country testing shortlist. */
    {
        uint8_t files[256]={0};
        assert(ExpandedWorldLive_OfferFiles(files,256u)==leagues);
        for(unsigned i=0;i<leagues;++i) assert(files[i]==(uint8_t)(8u+i));
    }
    assert(ExpandedWorldLive_Fixtures()==8080u);
    check_calendar();
    /* Four of the source OG leagues start in May. Their completed
     * May/June games MUST already be present on the manager's first
     * July 1 day: otherwise archiving the first season will stall. */
    assert(ExpandedWorldLive_Results()>0u);
    printf("INIT pre-career fixtures played=%lu\n",
           (unsigned long)ExpandedWorldLive_Results());
    uint32_t last_results=ExpandedWorldLive_Results();
    uint16_t last_cup_year=0;
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
        /* Exercise 70 OG-style knockout outcomes in the SAME long-lived
         * world snapshot, with a stable champion and no duplicate entries.
         * This is a journal/serialization test, NOT a replacement for OG
         * cup draws, extra time, penalties or Windows match callbacks. */
        {
            ExpandedLiveCalendarState cs;
            assert(ExpandedWorldLive_CalendarState(&cs));
            if(cs.today.month==9u && cs.today.day<=7u &&
               cs.today.year<2066u && cs.today.year!=last_cup_year) {
                const uint16_t y=cs.today.year;
                const uint16_t c1=0x0801u,c2=0x0802u,c3=0x0803u,c4=0x0804u;
                const uint32_t was=ExpandedWorldLive_CupResidentMatches();
                const uint32_t titles=ExpandedWorldLive_CupResidentTitles();
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "NATIONAL CUP",1u,56u,0u,0u,0u,c1,c2,2u,1u,0u));
                /* Re-reporting a game is idempotent: no duplicate. */
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "NATIONAL CUP",1u,56u,0u,0u,0u,c1,c2,2u,1u,0u));
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "NATIONAL CUP",1u,56u,0u,1u,0u,c3,c4,1u,0u,0u));
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "NATIONAL CUP",1u,57u,1u,0u,0u,c1,c3,3u,2u,0u));
                assert(ExpandedWorldLive_RecordNativeCupChampion(y,
                         "NATIONAL CUP",1u,57u,c1));
                /* Independent concurrent native cup with legs and replay.
                 * OG supplies results/winners; the world only journals them.
                 * Verify stable identities over 70 archived seasons. */
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "REGIONAL CUP",2u,56u,0u,0u,0u,c1,c4,1u,1u,0u));
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "REGIONAL CUP",2u,57u,0u,0u,1u,c4,c1,2u,2u,0u));
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "REGIONAL CUP",2u,58u,0u,0u,2u,c1,c4,2u,0u,0u));
                assert(ExpandedWorldLive_RecordNativeCupMatch(y,
                         "REGIONAL CUP",2u,57u,0u,0u,1u,c4,c1,2u,2u,0u));
                assert(ExpandedWorldLive_RecordNativeCupChampion(y,
                         "REGIONAL CUP",2u,58u,c1));
                assert(ExpandedWorldLive_CupResidentMatches()==was+6u);
                assert(ExpandedWorldLive_CupResidentTitles()==titles+2u);
                /* The F12 inspector must expose REAL resident journal events.
                 * Neither archived scores nor absent OG cups are fabricated. */
                {
                    ExpandedLiveCupDebugEvent seen[4];
                    assert(ExpandedWorldLive_CupRecentEvents(seen,4u)==4u);
                    assert(seen[0].kind==2u && seen[0].home==c1);
                    assert(seen[0].date.year==y);
                    assert(seen[1].kind==1u && seen[1].leg==2u);
                }
                /* OG replay/save/load must not change a recorded winner. */
                assert(ExpandedWorldLive_RecordNativeCupChampion(y,
                         "NATIONAL CUP",1u,57u,c1));
                assert(!ExpandedWorldLive_RecordNativeCupChampion(y,
                         "NATIONAL CUP",1u,57u,c2));
                assert(!ExpandedWorldLive_RecordNativeCupMatch(y,
                         "NATIONAL CUP",1u,56u,0u,0u,0u,
                         c1,c2,0u,3u,0u));
                uint16_t stored=0;
                assert(ExpandedWorldLive_CupHistoryChampion(y,
                         "NATIONAL CUP",1u,&stored) && stored==c1);
                assert(ExpandedWorldLive_CupHistoryChampion(y,
                         "REGIONAL CUP",2u,&stored) && stored==c1);
                assert(ExpandedWorldLive_CupHistoricalTitles()==
                        2u*(uint32_t)(y-1996u+1u));
                last_cup_year=y;
            }
        }
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
    assert(last_cup_year==2065u);
    assert(ExpandedWorldLive_CupHistoricalTitles()==140u);
    for(uint16_t year=1996u;year<=2065u;++year) {
        uint16_t champion=0;
        assert(ExpandedWorldLive_CupHistoryChampion(year,
                "NATIONAL CUP",1u,&champion));
        assert(champion==0x0801u);
        assert(ExpandedWorldLive_CupHistoryChampion(year,
                "REGIONAL CUP",2u,&champion));
        assert(champion==0x0801u);
    }
    assert(ExpandedWorldLive_CupArchivedMatches()==years*6u);
    assert(ExpandedWorldLive_CupArchivedTitles()==years*2u);
    assert(ExpandedWorldLive_CupResidentMatches()==0u);
    assert(ExpandedWorldLive_CupResidentTitles()==0u);
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
    printf("OG_CUP_MATCHES_ARCHIVED=%lu OG_CUP_TITLES_ARCHIVED=%lu\n",
           (unsigned long)ExpandedWorldLive_CupArchivedMatches(),
           (unsigned long)ExpandedWorldLive_CupArchivedTitles());
    puts("GLOBAL_OFFERS_DISCOVERY=PASS");
    puts("CUP_NATIVE_JOURNAL=PASS");
    puts("CUP_ARCHIVE_70=PASS");
    puts("CUP_HISTORY_70=PASS");
    puts("CUP_RESULT_IDEMPOTENCE=PASS");
    puts("CUP_MULTICOMPETITION_LEGS_REPLAY_70=PASS");
    puts("WORLD_EVENT_CURSOR=PASS");
    puts("STRESS_RESULT=PASS");
    ExpandedWorldLive_End();
    return 0;
}
