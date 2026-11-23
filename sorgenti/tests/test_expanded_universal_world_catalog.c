/* Isolated WORLD2627 JSON-first catalog/pairing regression contract.
 * Mock of the JSON reader only; C11 catalog + circle algorithm are real.
 * This is NOT Windows gameplay, OG50Hz, full JSON parsing or CAR.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "expanded_universal_world_catalog.h"
#include "expanded_universal_pairings.h"
#include "expanded_universal_json_leagues.h"
#include "expanded_universal_date_planner.h"
#include "world.h"
static WorldId s_world=WORLD_2627;
static int s_json_ok=1;
WorldId World_Current(void){return s_world;}
const char *World_Tag(void){return s_world==WORLD_2627?"2627":"9697";}
uint16_t World_FirstYear(void){return s_world==WORLD_2627?2026u:1996u;}
uint8_t World_FileCountry(uint8_t file){return file;}
uint16_t World_Slot(uint8_t file,uint8_t ordinal)
{(void)file;return ordinal;}
uint16_t World_SlotTeam(uint8_t country,uint16_t slot)
{return (uint16_t)(((uint16_t)country<<8)|slot);}
/* Stub validates exactly the rule fields expected from actual JSON. */
void ExpandedUniversalJsonLeagues_Reset(void){}
int ExpandedUniversalJsonLeagues_Load(void){return s_json_ok;}
uint32_t ExpandedUniversalJsonLeagues_Count(void){return s_json_ok?3u:0u;}
int ExpandedUniversalJsonLeagues_Find(uint8_t country,uint8_t comp,
    uint8_t division,ExpandedUniversalJsonLeague *out)
{
    unsigned n=0,cycles=0,calendar=1;
    if(!s_json_ok || !out)return 0;
    if(country==45u && comp==89u && division==0u){n=4;cycles=2;}
    else if(country==45u && comp==89u && division==1u){n=5;cycles=1;}
    else if(country==44u && comp==77u && division==0u){n=30;cycles=1;}
    else if(country==43u && comp==70u && division==0u){n=4;cycles=1;calendar=0;}
    else if(country==42u && comp==71u && division==0u){n=40;cycles=2;}
    else return 0;
    memset(out,0,sizeof *out);
    out->country=country;out->comp_id=comp;out->division=division;
    out->begin_phase=56u;out->end_phase=88u;
    out->meetings=(uint8_t)cycles;out->declared_clubs=(uint16_t)n;
    /* v280 CE calendar: 1 Aug .. 31 Dec 2026, Sunday + Wednesday reserve. */
    out->has_calendar=(uint8_t)calendar;
    out->first_month=8u;out->first_day=1u;
    out->last_month=12u;out->last_day=31u;out->last_year_offset=0u;
    out->match_weekdays=(uint8_t)(1u<<6);   /* SUN */
    out->extra_weekdays=(uint8_t)(1u<<2);   /* WED */
    out->min_rest_days=3u;
    return 1;
}
static void add(uint8_t file,uint8_t ordinal,uint8_t division)
{
    TeamFileEx t={0};
    t.team.countryNumber=file;
    t.team.teamOrdinal=ordinal;
    t.team.league=division;
    t.players[0].price=(int8_t)(ordinal+1u);
    assert(ExpandedUniversalCatalog_AddClub(&t));
}
/* RG-T: genuine C circle algorithm, all even/odd n=2..48 and 1..4
 * independent cycles. No OG fixture data required or permitted.
 */
typedef struct PairCheck {
    uint8_t orient[4][48][48];
    uint8_t in_round[4][48][48];
    uint32_t per_cycle[4];
    unsigned n;
} PairCheck;
static int check_pair(void *user,uint32_t c,uint32_t round,
                      uint32_t serial,uint32_t home,uint32_t away)
{
    PairCheck *p=(PairCheck *)user;
    (void)serial;
    assert(c<4u && round<48u && home<p->n && away<p->n &&
           home!=away);
    assert(!p->orient[c][home][away] &&
           !p->orient[c][away][home]);
    assert(!p->in_round[c][round][home] &&
           !p->in_round[c][round][away]);
    p->orient[c][home][away]=1u;
    p->orient[c][away][home]=2u;
    p->in_round[c][round][home]=1u;
    p->in_round[c][round][away]=1u;
    ++p->per_cycle[c];
    return 1;
}
static void test_pairings_all(void)
{
    PairCheck p;
    unsigned n,legs,i,j,c;
    for(n=2u;n<=48u;++n)for(legs=1u;legs<=4u;++legs) {
        memset(&p,0,sizeof p);
        p.n=n;
        assert(ExpandedUniversal_GenerateRoundRobin(
            n,(uint8_t)legs,check_pair,&p));
        for(c=0u;c<legs;++c) {
            assert(p.per_cycle[c]==n*(n-1u)/2u);
            for(i=0u;i<n;++i)for(j=i+1u;j<n;++j)
                assert(p.orient[c][i][j]!=0u);
        }
        if(legs>=2u)
            for(i=0u;i<n;++i)for(j=i+1u;j<n;++j)
                assert(p.orient[0][i][j]!=p.orient[1][i][j]);
    }
}
/* RG-T v280: one civil day per round, rounds strictly ordered, only the
 * data weekdays, minimum rest, inside the data window, no OG phase. */
static void check_round_days(uint32_t count)
{
    int32_t lo=0,hi=0;
    uint32_t i,j,wednesday=0u;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2026u,8u,1u},&lo));
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2026u,12u,31u},&hi));
    for(i=0;i<count;++i) {
        const ExpandedUniversalFixture *a=
            &ExpandedUniversalCatalog_Fixture(i)->fixture;
        const int wd=ExpandedWorld_Weekday(a->day_index);
        assert(a->day_index>=lo && a->day_index<=hi);
        assert(wd==6 || wd==2);
        if(wd==2)++wednesday;
        for(j=0;j<count;++j) {
            const ExpandedUniversalFixture *b=
                &ExpandedUniversalCatalog_Fixture(j)->fixture;
            if(!ExpandedEditionKey_Equal(a->key.edition,b->key.edition))
                continue;
            if(a->key.leg==b->key.leg && a->key.tie==b->key.tie)continue;
            /* different day => far enough apart */
            if(a->day_index!=b->day_index) {
                const int32_t d=a->day_index>b->day_index?
                    a->day_index-b->day_index:b->day_index-a->day_index;
                assert(d>=3);
            }
        }
    }
    /* 30 clubs single round robin = 29 rounds, only 22 Sundays: the
     * Wednesday reserve must fill exactly the shortfall (7 rounds). */
    assert(wednesday==7u*15u);
}
static void test_planner_direct(void)
{
    ExpandedDatePlanPolicy p;
    int32_t days[64],first=0,last=0,b0=0,b1=0;
    int reason=-1;
    uint32_t i;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2026u,9u,1u},&first));
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2027u,5u,31u},&last));
    memset(&p,0,sizeof p);
    p.first_day=first;p.last_day=last;
    p.match_weekdays=(uint8_t)(1u<<6);p.extra_weekdays=(uint8_t)(1u<<2);
    p.min_rest_days=3u;
    /* winter break 21 Dec .. 6 Jan: no round inside it */
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2026u,12u,21u},&b0));
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2027u,1u,6u},&b1));
    p.break_count=1u;p.breaks[0].first_day=b0;p.breaks[0].last_day=b1;
    assert(ExpandedDatePlanner_Plan(&p,38u,days,&reason) &&
           reason==EXP_DATE_PLAN_OK);
    assert(days[0]>=first && days[37]<=last);
    for(i=0;i<38u;++i) {
        assert(days[i]<b0 || days[i]>b1);
        if(i)assert(days[i]-days[i-1u]>=3);
    }
    /* last round lands on the last Sunday of the window (30 May 2027) */
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2027u,5u,30u},&b0));
    assert(days[37]==b0);
    /* impossible: 64 rounds in Sep..May Sundays+Wednesdays (~78 days)
     * with rest 5 rejects every Wednesday -> fail closed, no invention */
    p.min_rest_days=5u;
    assert(!ExpandedDatePlanner_Plan(&p,64u,days,&reason) &&
           reason==EXP_DATE_PLAN_NOT_ENOUGH_DAYS);
    /* no preferred weekday is malformed data */
    p.match_weekdays=0u;
    assert(!ExpandedDatePlanner_Plan(&p,10u,days,&reason) &&
           reason==EXP_DATE_PLAN_BAD_INPUT);
}
int main(void)
{
    ExpandedUniversalCatalogAudit audit={0};
    const ExpandedUniversalCatalogFixture *f=NULL;
    ExpandedClubKey first={WORLD_2627,(45u<<16)|1u};
    ExpandedWorldDate date={0};
    int32_t expected=0;
    test_pairings_all(); /* 188 n/meeting configurations, actual C11 */
    test_planner_direct();
    assert(ExpandedUniversalCatalog_Begin());
    for(uint8_t i=0;i<4u;++i)add(45u,i,0u);
    for(uint8_t i=4u;i<9u;++i)add(45u,i,1u);
    for(uint8_t i=0;i<30u;++i)add(44u,i,0u);
    for(uint8_t i=0;i<4u;++i)add(43u,i,0u);   /* no calendar row */
    for(uint8_t i=0;i<40u;++i)add(42u,i,0u);  /* 78 rounds: impossible */
    assert(ExpandedUniversalCatalog_Roster(first)!=NULL);
    assert(ExpandedUniversalCatalog_Roster(first)->players[0].price==1);
    assert(ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(ExpandedUniversalCatalog_AddLeague(45u,89u,1u,56u,88u,1u));
    assert(ExpandedUniversalCatalog_AddLeague(44u,77u,0u,56u,88u,1u));
    assert(ExpandedUniversalCatalog_AddLeague(43u,70u,0u,56u,88u,1u));
    assert(ExpandedUniversalCatalog_AddLeague(42u,71u,0u,56u,88u,2u));
    assert(!ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(ExpandedUniversalCatalog_AddCup(45u,164u,1u));
    assert(ExpandedUniversalCatalog_Seal());
    assert(ExpandedUniversalCatalog_Audit(&audit));
    assert(audit.clubs==83u && audit.league_definitions==5u);
    assert(audit.fixtures==457u && audit.future_fixtures==457u);
    /* missing calendar and impossible calendar: fail closed, no OG date */
    assert(audit.unsupported_leagues==2u && audit.day_collisions==0u);
    assert(audit.missing_cup_fixtures==1u && !audit.source_complete);
    assert(audit.unverified_native_order==0u && audit.json_rule_failures==0u);
    f=ExpandedUniversalCatalog_Fixture(0u);
    assert(f && f->fixture.key.edition.competition.world==WORLD_2627);
    assert(f->fixture.home.stable_club==((45u<<16)|1u));
    assert(f->fixture.away.stable_club==((45u<<16)|4u));
    assert(f->native_phase==0xffu && f->past_at_start==0u);
    assert(f->date_provenance==EXP_CATALOG_DATE_CE_PLANNER);
    /* 1 Aug 2026 is a Saturday: first round on the first Sunday, 2 Aug. */
    date=(ExpandedWorldDate){2026u,8u,2u};
    assert(ExpandedWorld_DateToIndex(date,&expected));
    assert(f->fixture.day_index==expected);
    assert(ExpandedWorld_Weekday(f->fixture.day_index)==6);
    assert(ExpandedUniversalCatalog_Fixture(457u)==NULL);
    check_round_days(457u);
    /* JSON absent: never fall back to original OG pair tables. */
    s_json_ok=0;
    assert(ExpandedUniversalCatalog_Begin());
    assert(!ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(ExpandedUniversalCatalog_Seal());
    assert(ExpandedUniversalCatalog_Audit(&audit));
    assert(audit.json_rule_failures>=1u && audit.fixtures==0u);
    s_world=WORLD_9697;
    assert(ExpandedUniversalCatalog_Begin());
    assert(!ExpandedUniversalCatalog_Audit(&audit));
    ExpandedUniversalCatalog_Reset();
    puts("UNIVERSAL_JSON_FIRST_CATALOG=PASS_ISOLATED_ONLY");
    puts("OG_NATIVE_MATCH/CAR/GP/WIN=NOT_TESTED");
    return 0;
}
