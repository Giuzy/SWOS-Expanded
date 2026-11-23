/* v00.13.283 RG-T: the LWE cursor is only a MIRROR of the CE day.
 * Real expanded_world_live.c + a fake CE clock provider (explicit
 * registration API). Isolated test: NOT Windows gameplay, CAR or OG.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "expanded_world_live.h"
#include "expanded_world_time.h"
#include "fixture_rounds.h"
static int32_t g_day;
static int g_active;
static int provider(int32_t *out){ if(!g_active)return 0; *out=g_day; return 1; }
static int32_t idx(unsigned y,unsigned m,unsigned d){
    int32_t v=0;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d},&v));
    return v;
}
static void start_world(void){
    FixtureRounds_Install();
    assert(ExpandedWorldLive_Begin(2026));
    for(unsigned i=1;i<=20;i++){
        char name[18]; snprintf(name,sizeof name,"CLUB %u",i);
        assert(ExpandedWorldLive_AddClub(8,0,(uint16_t)((8u<<8)|i),(uint16_t)(200+i*4),name,3));
    }
    assert(ExpandedWorldLive_Seal());
}
int main(void){
    ExpandedLiveCalendarState st;
    ExpandedWorldDate d;
    start_world();
    /* 1. no provider registered: legacy behaviour untouched (9697 / tests) */
    assert(!ExpandedWorldLive_ClockProviderActive());
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.year==2026 && st.today.month==7 && st.today.day==1);
    /* 2. provider registered but CE inactive: LWE still advances normally */
    ExpandedWorldLive_SetClockProvider(provider);
    g_active=0;
    assert(!ExpandedWorldLive_ClockProviderActive());
    d=(ExpandedWorldDate){2026,10,4};
    assert(ExpandedWorldLive_AdvanceToDate(d));
    assert(ExpandedWorldLive_CalendarState(&st) && st.today.month==10 && st.today.day==4);
    /* 3. CE active and EARLIER than the LWE cursor (stale-career/restore
     * case): readers must get the CE day, never the stale LWE one. */
    g_active=1;g_day=idx(2026,9,6);
    assert(ExpandedWorldLive_ClockProviderActive());
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.year==2026 && st.today.month==9 && st.today.day==6);
    /* 4. CE owns the clock: LWE advance is refused, nothing moves */
    d=(ExpandedWorldDate){2026,11,1};
    assert(!ExpandedWorldLive_AdvanceToDate(d));
    assert(ExpandedWorldLive_CalendarState(&st) && st.today.month==9 && st.today.day==6);
    /* 5. CE moves forward: readers follow immediately */
    g_day=idx(2026,12,20);
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.month==12 && st.today.day==20);
    /* 5b. CONTINUE in progress (v284): provider muted. A STALE CE registry of
     * the previous career must not answer for the restored LWE cursor. */
    g_day=idx(2027,3,1);               /* stale registry, different day */
    ExpandedWorldLive_ClockProviderHold(1);
    assert(!ExpandedWorldLive_ClockProviderActive());
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.month==12 && st.today.day==20); /* LWE cursor untouched */
    ExpandedWorldLive_ClockProviderHold(0);
    assert(ExpandedWorldLive_ClockProviderActive());
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.year==2027 && st.today.month==3 && st.today.day==1);
    g_day=idx(2026,12,20);             /* back, for the next steps */
    /* the mirror also follows the CE BACKWARDS (no LWE fixtures): never ahead */
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.year==2026 && st.today.month==12 && st.today.day==20);
    /* 6. CE clock disappears (career end / world change): back to LWE */
    g_active=0;
    assert(!ExpandedWorldLive_ClockProviderActive());
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.year==2026 && st.today.month>=9);
    /* 7. no LWE state at all but CE active: date still available */
    ExpandedWorldLive_End();
    assert(!ExpandedWorldLive_CalendarState(&st));
    g_active=1;g_day=idx(2027,1,17);
    assert(ExpandedWorldLive_CalendarState(&st));
    assert(st.today.year==2027 && st.today.month==1 && st.today.day==17);
    g_active=0;
    assert(!ExpandedWorldLive_CalendarState(&st));
    ExpandedWorldLive_SetClockProvider(NULL);
    puts("LWE_CLOCK_MIRROR_FOLLOWS_CE=PASS_ISOLATED_ONLY");
    puts("WINDOWS_START_FORWARD_SAVE_CONTINUE=NOT_TESTED");
    return 0;
}
