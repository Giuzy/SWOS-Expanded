/* World 2026/27 catalogue: one general source adapter for ALL native countries.
 * It never advances the world or runs/simulates a match. The old OG pairing
 * table and phase keys are read once and copied with explicit provenance.
 * Missing/ambiguous matches are reported, not fabricated or silently dated.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_world_catalog.h"
#include "world.h"
#include "expanded_universal_pairings.h"
#include "expanded_universal_json_leagues.h"
#include "expanded_universal_date_planner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


typedef struct CatalogClub {
    TeamFileEx *roster;              /* stable across shared TEAM file loads */
    ExpandedClubKey key;
    uint8_t country, division;
} CatalogClub;
typedef struct CatalogLeague {
    uint8_t country, comp_id, division, begin, end, legs;
    uint16_t expected_clubs;
    ExpandedUniversalJsonLeague rule; /* incl. CE calendar (v280) */
} CatalogLeague;
typedef struct CatalogDayClub {
    int32_t day;
    uint32_t club;
    uint32_t fixture_index;
} CatalogDayClub;

static struct {
    CatalogClub *clubs;
    CatalogLeague *leagues;
    ExpandedUniversalCatalogFixture *fixtures;
    uint32_t nclubs,cclubs,nleagues,cleagues,nfixtures,cfixtures;
    ExpandedUniversalCatalogAudit audit;
    int32_t start_day;
    uint8_t active;
} s_catalog;

static int cat_grow(void **array,uint32_t *cap,uint32_t need,size_t size)
{
    uint32_t next;
    void *tmp;
    if(need<=*cap)return 1;
    next=*cap?*cap:64u;
    while(next<need) {
        if(next>UINT32_MAX/2u)return 0;
        next*=2u;
    }
#if SIZE_MAX < UINT64_MAX
    /* On 64-bit sizeof(next) <= 4, so this is always false and GCC
     * -Wtype-limits rejects it under the strict standalone runner. */
    if((size_t)next>SIZE_MAX/size)return 0;
#endif
    tmp=realloc(*array,(size_t)next*size);
    if(!tmp)return 0;
    *array=tmp;*cap=next;
    return 1;
}
void ExpandedUniversalCatalog_Reset(void)
{
    uint32_t i;
    for(i=0;i<s_catalog.nclubs;++i)free(s_catalog.clubs[i].roster);
    free(s_catalog.clubs);
    free(s_catalog.leagues);
    free(s_catalog.fixtures);
    memset(&s_catalog,0,sizeof s_catalog);
    ExpandedUniversalJsonLeagues_Reset();
}
int ExpandedUniversalCatalog_Begin(void)
{
    ExpandedWorldDate july;
    ExpandedUniversalCatalog_Reset();
    if(World_Current()!=WORLD_2627)return 1; /* legacy 9697 unchanged */
    july=(ExpandedWorldDate){World_FirstYear(),7u,1u};
    if(!ExpandedWorld_DateToIndex(july,&s_catalog.start_day))return 0;
    s_catalog.active=1u;
    if(!ExpandedUniversalJsonLeagues_Load())
        ++s_catalog.audit.json_rule_failures;
    return 1;
}
static int cat_key(const TeamFileEx *team,ExpandedClubKey *out)
{
    uint8_t file,country,ordinal;
    uint16_t slot,packed;
    if(!team || !out || World_Current()!=WORLD_2627)return 0;
    file=team->team.countryNumber;ordinal=team->team.teamOrdinal;
    country=World_FileCountry(file);
    slot=World_Slot(file,ordinal);
    packed=(uint16_t)(((uint16_t)file<<8)|ordinal);
    if(slot>=WORLD_SLOTS || World_SlotTeam(country,slot)!=packed)return 0;
    out->world=(uint16_t)WORLD_2627;
    out->stable_club=((uint32_t)country<<16)|(uint32_t)(slot+1u);
    return ExpandedClubKey_Valid(*out);
}
int ExpandedUniversalCatalog_KeyOf(const TeamFileEx *team,ExpandedClubKey *out)
{
    return cat_key(team,out);
}
int ExpandedUniversalCatalog_AddClub(const TeamFileEx *team)
{
    ExpandedClubKey key={0};
    uint32_t i;
    TeamFileEx *copy;
    CatalogClub *dst;
    if(!s_catalog.active)return World_Current()!=WORLD_2627;
    if(s_catalog.audit.sealed || !cat_key(team,&key))goto reject;
    for(i=0;i<s_catalog.nclubs;++i)
        if(ExpandedClubKey_Equal(s_catalog.clubs[i].key,key))
            goto reject;
    if(!cat_grow((void **)&s_catalog.clubs,&s_catalog.cclubs,
                 s_catalog.nclubs+1u,sizeof *s_catalog.clubs))goto reject;
    copy=(TeamFileEx *)malloc(sizeof *copy);
    if(!copy)goto reject;
    memcpy(copy,team,sizeof *copy);
    dst=&s_catalog.clubs[s_catalog.nclubs++];
    dst->roster=copy;dst->key=key;
    dst->country=World_FileCountry(team->team.countryNumber);
    dst->division=team->team.league;
    s_catalog.audit.clubs=s_catalog.nclubs;
    return 1;
reject:
    ++s_catalog.audit.failed_clubs;
    return 0;
}
/* Fase 2: clubs of (country, division) as loaded from TEAM (state at
 * career START / CONTINUE), sorted by stable key. */
uint32_t ExpandedUniversalCatalog_DivisionClubs(uint8_t country,uint8_t division,
                                                ExpandedClubKey *out,uint32_t cap)
{
    uint32_t i,n=0u;
    if(!s_catalog.active || !out)return 0u;
    for(i=0;i<s_catalog.nclubs;++i)
        if(s_catalog.clubs[i].country==country &&
           s_catalog.clubs[i].division==division) {
            uint32_t j;
            if(n>=cap)return 0u;
            j=n++;
            while(j>0u && out[j-1u].stable_club>s_catalog.clubs[i].key.stable_club) {
                out[j]=out[j-1u];--j;
            }
            out[j]=s_catalog.clubs[i].key;
        }
    return n;
}
uint32_t ExpandedUniversalCatalog_RosterCount(void)
{
    return s_catalog.active?s_catalog.nclubs:0u;
}
const TeamFileEx *ExpandedUniversalCatalog_RosterAt(uint32_t i)
{
    return s_catalog.active && i<s_catalog.nclubs?
        s_catalog.clubs[i].roster:NULL;
}
TeamFileEx *ExpandedUniversalCatalog_Roster(ExpandedClubKey key)
{
    uint32_t i;
    if(!s_catalog.active || !ExpandedClubKey_Valid(key))return NULL;
    for(i=0;i<s_catalog.nclubs;++i)
        if(ExpandedClubKey_Equal(s_catalog.clubs[i].key,key))
            return s_catalog.clubs[i].roster;
    return NULL;
}
int ExpandedUniversalCatalog_AddLeague(uint8_t country,uint8_t native_comp_id,
                                       uint8_t division,uint8_t begin_phase,
                                       uint8_t end_phase,uint8_t legs)
{
    uint32_t i;
    CatalogLeague *dst;
    ExpandedUniversalJsonLeague rule={0};
    if(!s_catalog.active)return World_Current()!=WORLD_2627;
    if(s_catalog.audit.sealed || division>=16u ||
       !ExpandedUniversalJsonLeagues_Find(
           country,native_comp_id,division,&rule)) {
        ++s_catalog.audit.json_rule_failures;return 0;
    }
    for(i=0;i<s_catalog.nleagues;++i)
        if(s_catalog.leagues[i].country==country &&
           s_catalog.leagues[i].division==division &&
           s_catalog.leagues[i].comp_id==native_comp_id)return 0;
    if(begin_phase!=rule.begin_phase || end_phase!=rule.end_phase ||
       legs!=rule.meetings) {
        ++s_catalog.audit.native_rule_mismatches;
        fprintf(stderr,"[universal-json] NATIVE_RULE_DIFF "
            "country=%u comp=%u division=%u source=JSON_USED\n",
            (unsigned)country,(unsigned)native_comp_id,(unsigned)division);
    }
    if(!cat_grow((void **)&s_catalog.leagues,&s_catalog.cleagues,
                 s_catalog.nleagues+1u,sizeof *s_catalog.leagues))return 0;
    dst=&s_catalog.leagues[s_catalog.nleagues++];
    *dst=(CatalogLeague){country,native_comp_id,division,
         rule.begin_phase,rule.end_phase,rule.meetings,rule.declared_clubs,
         rule};
    s_catalog.audit.league_definitions=s_catalog.nleagues;
    return 1;
}
int ExpandedUniversalCatalog_AddCup(uint8_t country,uint8_t native_comp_id,
                                    uint8_t native_type)
{
    (void)country;(void)native_comp_id;
    if(!s_catalog.active)return World_Current()!=WORLD_2627;
    if(s_catalog.audit.sealed || (native_type!=1u && native_type!=2u))return 0;
    ++s_catalog.audit.cup_definitions;
    /* Entrants/KO brackets must be certified before becoming fixtures. */
    ++s_catalog.audit.missing_cup_fixtures;
    return 1;
}
static int cat_fixture(const ExpandedUniversalFixture *f)
{
    ExpandedUniversalCatalogFixture *dst;
    if(!cat_grow((void **)&s_catalog.fixtures,&s_catalog.cfixtures,
                 s_catalog.nfixtures+1u,sizeof *s_catalog.fixtures))return 0;
    dst=&s_catalog.fixtures[s_catalog.nfixtures++];
    memset(dst,0,sizeof *dst);
    dst->fixture=*f;
    /* v280: no OG phase is involved in the date any more. */
    dst->native_phase=0xffu;
    dst->date_provenance=EXP_CATALOG_DATE_CE_PLANNER;
    dst->past_at_start=f->day_index<s_catalog.start_day;
    if(dst->past_at_start)++s_catalog.audit.prestart_unresolved;
    else ++s_catalog.audit.future_fixtures;
    return 1;
}
/* Stable JSON club order, independent from TEAM insertion and OG seed. */
static int cat_club_cmp(const void *aa,const void *bb)
{
    const CatalogClub *a=*(CatalogClub *const *)aa;
    const CatalogClub *b=*(CatalogClub *const *)bb;
    return a->key.stable_club<b->key.stable_club?-1:
           a->key.stable_club>b->key.stable_club?1:0;
}
typedef struct CatalogPairPlan {
    const CatalogLeague *league;
    CatalogClub **clubs;
    uint32_t rounds,total_rounds;
    int32_t first_day;
    const int32_t *round_days; /* CE planner output, one civil day/round */
} CatalogPairPlan;
static int cat_pair_emit(void *user,uint32_t cycle,uint32_t round,
                         uint32_t serial,uint32_t home,uint32_t away)
{
    const CatalogPairPlan *p=(const CatalogPairPlan *)user;
    const CatalogLeague *l=p->league;
    const uint32_t round_id=cycle*p->rounds+round;
    ExpandedUniversalFixture f={0};
    int32_t day=0;
    if(home>=l->expected_clubs || away>=l->expected_clubs ||
       round_id>=p->total_rounds || serial>=(1u<<17))return 0;
    day=p->round_days[round_id];
    f.home=p->clubs[home]->key;f.away=p->clubs[away]->key;
    f.day_index=day;f.country=l->country;f.slot=l->division;
    f.competition_type=0u;
    f.key.edition.competition.world=(uint16_t)WORLD_2627;
    f.key.edition.competition.stable_competition=
        ((uint32_t)(l->country+1u)<<24)|
        ((uint32_t)l->comp_id<<16)|
        ((uint32_t)(l->division+1u)<<8)|1u;
    f.key.edition.start_day=p->first_day;
    f.key.edition.sequence=0u;
    f.key.stage=1u;
    f.key.tie=serial+1u;
    f.key.leg=(uint16_t)cycle;
    f.event_id=((uint64_t)WORLD_2627<<60)|
        ((uint64_t)World_FirstYear()<<44)|
        ((uint64_t)l->country<<36)|
        ((uint64_t)l->comp_id<<28)|
        ((uint64_t)l->division<<20)|
        ((uint64_t)cycle<<17)|
        ((uint64_t)serial+1u);
    return ExpandedUniversal_FixtureValid(&f) && cat_fixture(&f);
}
/* JSON MM-DD in the edition: first occurrence on/after 'from' (a break may
 * straddle New Year). Returns 0 on an invalid calendar day. */
static int cat_month_day_on_or_after(uint16_t year,uint8_t month,uint8_t day,
                                     int32_t from,int32_t *out)
{
    uint32_t k;
    for(k=0;k<3u;++k) {
        ExpandedWorldDate d={(uint16_t)(year+k),month,day};
        int32_t idx=0;
        if(month==2u && day==29u &&
           !ExpandedWorld_DateToIndex(d,&idx))d.day=28u; /* leap safety */
        if(!ExpandedWorld_DateToIndex(d,&idx))return 0;
        if(idx>=from) { *out=idx; return 1; }
    }
    return 0;
}
/* Data-driven civil dates for every round: window, weekdays, rest, breaks
 * all from league_calendar.json. OG native phases are NOT consulted. */
static int cat_plan_league_days(const CatalogLeague *l,uint32_t rounds,
                                int32_t *days,int32_t *first_day)
{
    const ExpandedUniversalJsonLeague *r=&l->rule;
    const uint16_t year=World_FirstYear();
    ExpandedDatePlanPolicy policy;
    ExpandedWorldDate first,last;
    uint32_t i;
    int reason=0;
    memset(&policy,0,sizeof policy);
    if(!r->has_calendar) {
        fprintf(stderr,"[universal-dates] NO_CALENDAR country=%u comp=%u "
            "division=%u source=league_calendar.json fail-closed=1 "
            "og-phase-fallback=0\n",(unsigned)l->country,
            (unsigned)l->comp_id,(unsigned)l->division);
        return 0;
    }
    first=(ExpandedWorldDate){year,r->first_month,r->first_day};
    last=(ExpandedWorldDate){(uint16_t)(year+r->last_year_offset),
                             r->last_month,r->last_day};
    if(!ExpandedWorld_DateToIndex(first,&policy.first_day) ||
       !ExpandedWorld_DateToIndex(last,&policy.last_day) ||
       policy.last_day<policy.first_day)return 0;
    policy.match_weekdays=r->match_weekdays;
    policy.extra_weekdays=r->extra_weekdays;
    policy.min_rest_days=r->min_rest_days;
    for(i=0;i<r->break_count && i<EXP_DATE_PLAN_MAX_BREAKS;++i) {
        const ExpandedUniversalJsonLeagueBreak *b=&r->breaks[i];
        ExpandedDatePlanBreak *dst=&policy.breaks[policy.break_count];
        if(!cat_month_day_on_or_after(year,b->from_month,b->from_day,
                                      policy.first_day,&dst->first_day) ||
           !cat_month_day_on_or_after(year,b->to_month,b->to_day,
                                      dst->first_day,&dst->last_day))
            return 0;
        ++policy.break_count;
    }
    if(!ExpandedDatePlanner_Plan(&policy,rounds,days,&reason)) {
        fprintf(stderr,"[universal-dates] PLAN_FAIL country=%u comp=%u "
            "division=%u rounds=%lu reason=%d window=%04u-%02u-%02u.."
            "%04u-%02u-%02u fail-closed=1 og-phase-fallback=0\n",
            (unsigned)l->country,(unsigned)l->comp_id,(unsigned)l->division,
            (unsigned long)rounds,reason,
            (unsigned)first.year,(unsigned)first.month,(unsigned)first.day,
            (unsigned)last.year,(unsigned)last.month,(unsigned)last.day);
        return 0;
    }
    *first_day=policy.first_day;
    return 1;
}
static int cat_make_league(const CatalogLeague *l)
{
    CatalogClub **clubs=NULL;
    int32_t *round_days=NULL;
    uint32_t i,n=0;
    int32_t first_day=0;
    CatalogPairPlan p={0};
    const uint32_t saved_count=s_catalog.nfixtures;
    const uint32_t saved_future=s_catalog.audit.future_fixtures;
    const uint32_t saved_prestart=s_catalog.audit.prestart_unresolved;
    for(i=0;i<s_catalog.nclubs;++i)
        if(s_catalog.clubs[i].country==l->country &&
           s_catalog.clubs[i].division==l->division)++n;
    if(n!=l->expected_clubs || n<2u ||
       n>UINT32_MAX/sizeof *clubs)goto rollback;
    clubs=(CatalogClub **)malloc((size_t)n*sizeof *clubs);
    if(!clubs)goto rollback;
    n=0u;
    for(i=0;i<s_catalog.nclubs;++i)
        if(s_catalog.clubs[i].country==l->country &&
           s_catalog.clubs[i].division==l->division)
            clubs[n++]=&s_catalog.clubs[i];
    qsort(clubs,(size_t)n,sizeof *clubs,cat_club_cmp);
    for(i=1;i<n;++i)
        if(ExpandedClubKey_Equal(clubs[i-1u]->key,clubs[i]->key))
            goto rollback;
    p.league=l;p.clubs=clubs;
    p.rounds=(n&1u)?n:n-1u;
    p.total_rounds=p.rounds*l->legs+l->rule.extras.rounds;
    if(!p.total_rounds || p.total_rounds>4096u)goto rollback;
    round_days=(int32_t *)malloc((size_t)p.total_rounds*sizeof *round_days);
    if(!round_days ||
       !cat_plan_league_days(l,p.total_rounds,round_days,&first_day))
        goto rollback;
    p.first_day=first_day;p.round_days=round_days;
    {
        uint32_t keys[EXP_PAIR_MAX_CLUBS];
        const uint32_t id=((uint32_t)(l->country+1u)<<24)|
            ((uint32_t)l->comp_id<<16)|((uint32_t)(l->division+1u)<<8)|1u;
        if(l->rule.extras.rounds) {
            if(n>EXP_PAIR_MAX_CLUBS)goto rollback;
            for(i=0u;i<n;++i)keys[i]=clubs[i]->key.stable_club;
        }
        if(!ExpandedUniversal_GenerateLeague(n,l->legs,
            l->rule.extras.rounds?keys:NULL,&l->rule.extras,
            ExpPair_Seed(id,first_day),cat_pair_emit,&p))goto rollback;
    }
    free(round_days);
    free(clubs);
    return 1;
rollback:
    free(round_days);
    free(clubs);
    s_catalog.nfixtures=saved_count;
    s_catalog.audit.future_fixtures=saved_future;
    s_catalog.audit.prestart_unresolved=saved_prestart;
    ++s_catalog.audit.bad_pairs;
    return 0;
}
static int cat_day_cmp(const void *aa,const void *bb)
{
    const CatalogDayClub *a=(const CatalogDayClub *)aa;
    const CatalogDayClub *b=(const CatalogDayClub *)bb;
    if(a->day!=b->day)return a->day<b->day?-1:1;
    if(a->club!=b->club)return a->club<b->club?-1:1;
    return a->fixture_index<b->fixture_index?-1:
           a->fixture_index>b->fixture_index?1:0;
}
int ExpandedUniversalCatalog_Seal(void)
{
    CatalogDayClub *days;
    uint32_t i,n;
    if(!s_catalog.active)return World_Current()!=WORLD_2627;
    if(s_catalog.audit.sealed)return 1;
    for(i=0;i<s_catalog.nleagues;++i)
        if(!cat_make_league(&s_catalog.leagues[i]))
            ++s_catalog.audit.unsupported_leagues;
    n=s_catalog.nfixtures;
    s_catalog.audit.fixtures=n;
    if(n>UINT32_MAX/2u)return 0;
#if SIZE_MAX < UINT64_MAX
    if((size_t)(n*2u)>SIZE_MAX/sizeof *days)return 0;
#endif
    days=(CatalogDayClub *)malloc((size_t)(n?n*2u:1u)*sizeof *days);
    if(!days)return 0;
    for(i=0;i<n;++i) {
        const ExpandedUniversalFixture *f=&s_catalog.fixtures[i].fixture;
        days[2u*i]=(CatalogDayClub){f->day_index,f->home.stable_club,i};
        days[2u*i+1u]=(CatalogDayClub){f->day_index,f->away.stable_club,i};
    }
    qsort(days,(size_t)n*2u,sizeof *days,cat_day_cmp);
    for(i=1;i<n*2u;++i)
        if(days[i].day==days[i-1u].day &&
           days[i].club==days[i-1u].club &&
           days[i].fixture_index!=days[i-1u].fixture_index) {
            ++s_catalog.audit.day_collisions;
            s_catalog.fixtures[days[i].fixture_index].date_collision=1u;
            s_catalog.fixtures[days[i-1u].fixture_index].date_collision=1u;
        }
    free(days);
    s_catalog.audit.sealed=1u;
    s_catalog.audit.source_complete=(uint8_t)(
        s_catalog.audit.unsupported_leagues==0u &&
        s_catalog.audit.failed_clubs==0u &&
        s_catalog.audit.bad_pairs==0u &&
        s_catalog.audit.day_collisions==0u &&
        s_catalog.audit.prestart_unresolved==0u &&
        s_catalog.audit.missing_cup_fixtures==0u &&
        s_catalog.audit.json_rule_failures==0u);
    fprintf(stderr,"[universal-catalog] world=%s clubs=%lu leagues=%lu "
                   "cup-definitions=%lu fixtures=%lu future=%lu "
                   "prestart-unresolved=%lu unsupported-leagues=%lu "
                   "day-collisions=%lu cup-fixtures-unavailable=%lu "
                   "json-rule-failures=%lu native-rule-diffs=%lu "
                   "pair-source=JSON_CIRCLE_NO_OG "
                   "date-source=CE_PLANNER_JSON og-phase-dates=0 "
                   "source-complete=%u OWNER=INPUT_ONLY not-simulated=1\n",
            World_Tag(),(unsigned long)s_catalog.audit.clubs,
            (unsigned long)s_catalog.audit.league_definitions,
            (unsigned long)s_catalog.audit.cup_definitions,
            (unsigned long)s_catalog.audit.fixtures,
            (unsigned long)s_catalog.audit.future_fixtures,
            (unsigned long)s_catalog.audit.prestart_unresolved,
            (unsigned long)s_catalog.audit.unsupported_leagues,
            (unsigned long)s_catalog.audit.day_collisions,
            (unsigned long)s_catalog.audit.missing_cup_fixtures,
            (unsigned long)s_catalog.audit.json_rule_failures,
            (unsigned long)s_catalog.audit.native_rule_mismatches,
            (unsigned)s_catalog.audit.source_complete);
    return 1;
}
int ExpandedUniversalCatalog_Audit(ExpandedUniversalCatalogAudit *out)
{
    if(!out)return 0;
    *out=s_catalog.audit;
    return s_catalog.active!=0u;
}
const ExpandedUniversalCatalogFixture *ExpandedUniversalCatalog_Fixture(uint32_t i)
{
    return s_catalog.active && s_catalog.audit.sealed &&
           i<s_catalog.nfixtures?&s_catalog.fixtures[i]:NULL;
}
#endif
