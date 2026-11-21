/* Bolivia vertical pilot: civil event source, NOT a second result engine.
 * League dates are GAME_TEMPLATE_2026 matchday dates (not authenticated
 * dates of individual matches). Copa group pairs/dates are the 64 known
 * 2026 entries; knockout opponents come exclusively from OG qualification.
 * Scheduler ownership is independent of OG's native phase/keycache.
 *
 * All fixture execution, standings, trophies, RNG and CAR writes remain OG.
 * Do not acknowledge a live fixture until OG witness + CAR are committed.
 */
#ifndef EXPANDED_BOLIVIA_EVENT_REGISTRY_H
#define EXPANDED_BOLIVIA_EVENT_REGISTRY_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "expanded_event_scheduler.h"
#include "bolivia_cup_sequence.h"
#include "bolivia_cup_knockout.h"
#include "bolivia_league_2026_phases.h" /* OG cursor translation only */

enum {
    EXP_BOLIVIA_TEAMS=16,
    EXP_BOLIVIA_LEAGUE_ROUNDS=30,
    EXP_BOLIVIA_LEAGUE_FIXTURES=240,
    EXP_BOLIVIA_KNOWN_GROUP_FIXTURES=64,
    EXP_BOLIVIA_UNRESOLVED_KO_DATES=14,
    EXP_BOLIVIA_PILOT_EVENTS=318
};
typedef enum ExpandedBoliviaEventKind {
    EXP_BOLIVIA_EVENT_LEAGUE=1,
    EXP_BOLIVIA_EVENT_COPA_GROUP=2,
    /* Reservation only: this is not a fixture until OG knows its entrants. */
    EXP_BOLIVIA_EVENT_COPA_KO_DATE=3
} ExpandedBoliviaEventKind;
typedef struct ExpandedBoliviaEvent {
    uint64_t id;
    int32_t day_index;
    uint16_t home,away;
    uint16_t occurrence;
    uint8_t kind,stage;
} ExpandedBoliviaEvent;

/* Matchday template, not OG phase control. No one should use the
 * previous kBoliviaLeague2026NativePhases array to decide event dates.
 * These 30 calendar dates came from the approved approximate 2026
 * matchday template; real per-fixture dates can replace them as data. */
static const uint8_t kExpandedBoliviaLeagueCivil[30][2]={
    {4,1},{4,8},{4,19},{4,23},{5,1},{5,8},{5,12},{5,16},
    {7,16},{7,20},{7,24},{7,28},{8,4},{8,8},{8,12},{8,24},
    {8,28},{9,4},{9,12},{9,16},{10,12},{10,16},{10,24},
    {10,28},{11,8},{11,23},{11,27},{12,1},{12,4},{12,12}
};
static inline int ExpandedBolivia_LeagueRoundDate(
    uint16_t year,unsigned round,ExpandedWorldDate *out)
{
    int32_t day=0;
    if(!out || round>=EXP_BOLIVIA_LEAGUE_ROUNDS)return 0;
    const ExpandedWorldDate date={year,
        kExpandedBoliviaLeagueCivil[round][0],
        kExpandedBoliviaLeagueCivil[round][1]};
    if(!ExpandedWorld_DateToIndex(date,&day))return 0;
    *out=date;
    return 1;
}
/* Date-driven CPU execution cutoff. Old native eighth-month phases
 * are used only to tell the immutable OG fixture iterator HOW FAR it may
 * execute: the approved civil programme alone decides WHEN a round is due.
 * The START call (phase 0) must initialize without consuming any fixture. */
static inline int ExpandedBolivia_LeagueExecutionCutoff(
    uint16_t edition,ExpandedWorldDate today,uint8_t *cutoff)
{
    int32_t now=0;
    if(!cutoff || !ExpandedWorld_DateToIndex(today,&now) ||
       !BoliviaLeague2026_ValidatePhases())return 0;
    *cutoff=0u;
    for(unsigned r=0u;r<EXP_BOLIVIA_LEAGUE_ROUNDS;++r) {
        ExpandedWorldDate due;
        int32_t day=0;
        if(!ExpandedBolivia_LeagueRoundDate(edition,r,&due) ||
           !ExpandedWorld_DateToIndex(due,&day))return 0;
        if(day>now)break;
        /* OG advances only while (currentPhase > fixturePhase).
         * The civil due date INCLUDES this round, so hand OG the
         * EXCLUSIVE upper bound (phase+1), not fixturePhase itself.
         * Never invent/advance a match or alter the civil due date. */
        const uint8_t phase=kBoliviaLeague2026NativePhases[r];
        if(phase>=95u)return 0; /* native eighth-month phase range */
        *cutoff=(uint8_t)(phase+1u);
    }
    return 1;
}
/* The coordinator's World-Tick must never jump past an earlier Bolivia
 * event in the same call: split a requested advance at each distinct civil
 * date. Same-day league matches are an OG-owned matchday, not eight new
 * clocks. Copa group and KO dates are read from the data programme. */
static inline int ExpandedBolivia_NextBoundary(
    uint16_t edition,ExpandedWorldDate from,ExpandedWorldDate target,
    ExpandedWorldDate *next)
{
    int32_t now=0,end=0,best=0;
    if(!next || !ExpandedWorld_DateToIndex(from,&now) ||
       !ExpandedWorld_DateToIndex(target,&end) || end<now)return 0;
    best=end;
    for(unsigned r=0u;r<EXP_BOLIVIA_LEAGUE_ROUNDS;++r) {
        ExpandedWorldDate date;
        int32_t day=0;
        if(!ExpandedBolivia_LeagueRoundDate(edition,r,&date) ||
           !ExpandedWorld_DateToIndex(date,&day))return 0;
        if(day>now && day<best)best=day;
    }
    for(unsigned d=0u;d<8u;++d)
        for(unsigned m=0u;m<8u;++m) {
            const BoliviaCupRealPair *const f=&kBoliviaCup2026[d][m];
            const ExpandedWorldDate date={edition,f->month,f->day};
            int32_t day=0;
            if(!ExpandedWorld_DateToIndex(date,&day))return 0;
            if(day>now && day<best)best=day;
        }
    for(unsigned i=0u;i<BOLIVIA_KO_FIXTURES;++i) {
        const ExpandedWorldDate date={edition,
            kBoliviaKO2026[i].month,kBoliviaKO2026[i].day};
        int32_t day=0;
        if(!ExpandedWorld_DateToIndex(date,&day))return 0;
        if(day>now && day<best)best=day;
    }
    return ExpandedWorld_IndexToDate(best,next);
}
/* Deterministic ID: world, edition, competition kind, fixture ordinal.
 * It is NOT a native compId, a TSKC offset, or a result/ACK. */
static inline uint64_t ExpandedBolivia_EventId(
    uint8_t world,uint16_t year,uint8_t kind,uint16_t ordinal)
{
    return ((uint64_t)(world+1u)<<56) |
           ((uint64_t)year<<32) |
           ((uint64_t)kind<<24) |
           (uint64_t)(ordinal+1u);
}
/* Populate one readable fixture program from the native league's ORIGINAL
 * static pair DATA and the actual top-division clubs. This copies INPUT
 * pairings only; OG never schedules the date or receives a second result.
 * The caller supplies all 16 real club IDs in the pair table's order and
 * 256 template-ordinal -> actual team ID mappings (0xffff=absent).
 * KO events are date reservations, not unqualified phantom fixtures. */
static inline int ExpandedBolivia_BuildProgram(
    uint8_t world,uint16_t year,const uint16_t league_clubs[16],
    const uint8_t pairs_first_leg[240],
    const uint16_t cup_team_by_ordinal[256],
    ExpandedBoliviaEvent out[EXP_BOLIVIA_PILOT_EVENTS])
{
    if(!league_clubs || !pairs_first_leg || !cup_team_by_ordinal || !out)
        return 0;
    uint8_t seen[16]={0};
    for(unsigned i=0;i<16u;++i) {
        if(league_clubs[i]==0xffffu ||
           (uint8_t)(league_clubs[i]>>8)!=45u)return 0;
        for(unsigned j=0;j<i;++j)
            if(league_clubs[i]==league_clubs[j])return 0;
    }
    unsigned used=0;
    for(unsigned leg=0;leg<2u;++leg)
        for(unsigned i=0;i<120u;++i) {
            const unsigned round=leg*15u+i/8u;
            const unsigned h=pairs_first_leg[2u*i];
            const unsigned a=pairs_first_leg[2u*i+1u];
            ExpandedWorldDate date;
            int32_t index=0;
            if(h>=16u || a>=16u || h==a ||
               !ExpandedBolivia_LeagueRoundDate(year,round,&date) ||
               !ExpandedWorld_DateToIndex(date,&index))return 0;
            if(i%8u==0u)memset(seen,0,sizeof seen);
            if(seen[h]++ || seen[a]++)return 0; /* two matches/round */
            out[used++]=(ExpandedBoliviaEvent){
                ExpandedBolivia_EventId(world,year,EXP_BOLIVIA_EVENT_LEAGUE,
                                        (uint16_t)(leg*120u+i)),
                index,league_clubs[leg?a:h],league_clubs[leg?h:a],
                (uint16_t)(leg*120u+i),EXP_BOLIVIA_EVENT_LEAGUE,0u};
        }
    BoliviaCupFixture copa[64];
    if(BoliviaCup_ChronologicalFixtures(copa)!=64u)return 0;
    for(unsigned i=0;i<64u;++i) {
        ExpandedWorldDate date={year,copa[i].month,copa[i].day};
        int32_t index=0;
        const uint16_t h=cup_team_by_ordinal[copa[i].home];
        const uint16_t a=cup_team_by_ordinal[copa[i].away];
        if(!ExpandedWorld_DateToIndex(date,&index) ||
           h==0xffffu || a==0xffffu || h==a ||
           (uint8_t)(h>>8)!=45u || (uint8_t)(a>>8)!=45u)return 0;
        out[used++]=(ExpandedBoliviaEvent){
            ExpandedBolivia_EventId(world,year,EXP_BOLIVIA_EVENT_COPA_GROUP,
                                    (uint16_t)(copa[i].fixtureId-1u)),
            index,h,a,copa[i].fixtureId,EXP_BOLIVIA_EVENT_COPA_GROUP,
            (uint8_t)(copa[i].isDerby?1u:0u)};
    }
    if(!BoliviaKO_Validate())return 0;
    for(unsigned i=0;i<14u;++i) {
        ExpandedWorldDate date={year,kBoliviaKO2026[i].month,
                                      kBoliviaKO2026[i].day};
        int32_t index=0;
        if(!ExpandedWorld_DateToIndex(date,&index))return 0;
        out[used++]=(ExpandedBoliviaEvent){
            ExpandedBolivia_EventId(world,year,EXP_BOLIVIA_EVENT_COPA_KO_DATE,
                                    (uint16_t)i),
            index,0xffffu,0xffffu,(uint16_t)i,
            EXP_BOLIVIA_EVENT_COPA_KO_DATE,kBoliviaKO2026[i].round};
    }
    if(used!=EXP_BOLIVIA_PILOT_EVENTS)return 0;
    /* Global collision check for every known, named fixture. The KO
     * programme is checked only after OG supplies its actual qualifiers. */
    for(unsigned i=0;i<used;++i) {
        const ExpandedBoliviaEvent *a=&out[i];
        if(a->kind==EXP_BOLIVIA_EVENT_COPA_KO_DATE)continue;
        for(unsigned j=0;j<i;++j) {
            const ExpandedBoliviaEvent *b=&out[j];
            if(b->kind==EXP_BOLIVIA_EVENT_COPA_KO_DATE ||
               a->day_index!=b->day_index)continue;
            if(a->home==b->home || a->home==b->away ||
               a->away==b->home || a->away==b->away)return 0;
        }
    }
    return 1;
}
/* Construct the scheduler from this fixture program. This is a PLAN;
 * it must NOT be called as a second writer of OG scores or CAR. */
static inline int ExpandedBolivia_BuildScheduler(
    ExpandedEventScheduler *scheduler,ExpandedScheduledEvent *buffer,
    const ExpandedBoliviaEvent program[EXP_BOLIVIA_PILOT_EVENTS],
    uint16_t year)
{
    if(!scheduler || !buffer || !program)return 0;
    if(ExpandedEventScheduler_Init(scheduler,buffer,
         EXP_BOLIVIA_PILOT_EVENTS,(ExpandedWorldDate){year,1u,1u})
         !=EXP_EVENT_OK)return 0;
    for(unsigned i=0;i<EXP_BOLIVIA_PILOT_EVENTS;++i) {
        const ExpandedBoliviaEvent *f=&program[i];
        const ExpandedScheduledEvent e={
            f->id,0u,f->day_index,(uint16_t)f->kind,
            (uint8_t)(f->kind==EXP_BOLIVIA_EVENT_COPA_KO_DATE?
                EXP_EVENT_CUP_ADVANCE:EXP_EVENT_FIXTURE),
            EXP_EVENT_PENDING};
        if(ExpandedEventScheduler_Schedule(scheduler,e)!=EXP_EVENT_OK)
            return 0;
    }
    return 1;
}
/* Manager's next event choice: dates ONLY from our civil registry,
 * never from the legacy OG phase key. The caller must verify the same
 * native OG fixture/club identity before entering the match engine.
 * Return: -2 overdue, -1 conflict, 0 Liga first, 1 Copa first. */
static inline int ExpandedBolivia_ChooseNext(
    uint8_t world,uint16_t year,ExpandedWorldDate today,
    unsigned league_round,int has_league,
    ExpandedWorldDate copa_date,uint16_t copa_fixture,int has_copa)
{
    int32_t now=0,league_day=0,cup_day=0;
    if(!ExpandedWorld_DateToIndex(today,&now))return -2;
    if(has_league) {
        ExpandedWorldDate league_date;
        if(!ExpandedBolivia_LeagueRoundDate(year,league_round,
                                           &league_date) ||
           !ExpandedWorld_DateToIndex(league_date,&league_day) ||
           league_day<now)return -2;
    }
    if(has_copa &&
       (!ExpandedWorld_DateToIndex(copa_date,&cup_day) || cup_day<now))
        return -2;
    if(!has_copa)return 0;
    if(has_league && league_day==cup_day)return -1;
    ExpandedScheduledEvent entries[2]={{0}};
    ExpandedEventScheduler plan={0};
    if(ExpandedEventScheduler_Init(&plan,entries,2u,today)!=EXP_EVENT_OK)
        return -2;
    if(copa_fixture<1u || copa_fixture>78u)return -2;
    const uint8_t cup_kind=(uint8_t)(copa_fixture<=64u?
        EXP_BOLIVIA_EVENT_COPA_GROUP:EXP_BOLIVIA_EVENT_COPA_KO_DATE);
    const uint16_t ordinal=(uint16_t)(copa_fixture<=64u?
        copa_fixture-1u:copa_fixture-65u);
    const uint64_t cup_id=ExpandedBolivia_EventId(
        world,year,cup_kind,ordinal);
    ExpandedScheduledEvent event={
        cup_id,0u,cup_day,2u,EXP_EVENT_FIXTURE,EXP_EVENT_PENDING};
    if(ExpandedEventScheduler_Schedule(&plan,event)!=EXP_EVENT_OK)return -2;
    if(has_league) {
        event=(ExpandedScheduledEvent){
            ExpandedBolivia_EventId(world,year,EXP_BOLIVIA_EVENT_LEAGUE,
                                     (uint16_t)league_round),
            0u,league_day,1u,EXP_EVENT_FIXTURE,EXP_EVENT_PENDING};
        if(ExpandedEventScheduler_Schedule(&plan,event)!=EXP_EVENT_OK)
            return -2;
    }
    const ExpandedScheduledEvent *due=0;
    if(ExpandedEventScheduler_Next(&plan,
        has_league && league_day>cup_day?league_day:cup_day,
        &due)!=EXP_EVENT_DUE || !due)return -2;
    return due->id==cup_id?1:0;
}
#endif
