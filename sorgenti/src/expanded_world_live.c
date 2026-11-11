#include "expanded_world_live.h"
#ifdef SWOS_SDL_BUILD
#include "expanded_world_time.h"
#include "expanded_world_ledger.h"
#include "mjson.h"      /* typed, portable .CAR world snapshot */
#include "miniz_swos.h" /* existing bundled zlib for compact fixture snapshots */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LIVE_MAX_CLUBS 1500u
#define LIVE_MAX_LEAGUES 128u
#define LIVE_MAX_FIXTURES 40000u
#define LIVE_MAX_LEAGUE_CLUBS 26u
#define NATIVE_FIXTURE_MAX_CLUBS 24u
/* Canonical OpenSWOS pairing tables: also used by DIY league fixtures.
 * The odd-sized tables are installed by FixtureRounds_Install at startup. */
extern const uint8_t *_fixtureSchedulePtrTable[];
/* Two seasons resident, compact completed seasons stored separately. */
#define LIVE_SEASONS 2u
#define LIVE_HISTORY_LIMIT (LIVE_MAX_LEAGUES * 300u)
typedef struct LiveArchivedLeague {
    uint16_t year,champion,champion_points,matches;
    uint8_t country,division;
    uint32_t digest;
} LiveArchivedLeague;
typedef struct LivePackedFixture {
    ExpandedWorldFixture fixture;
    uint8_t consumed;
} LivePackedFixture;
static LiveArchivedLeague *s_history;
static uint32_t s_history_count,s_history_capacity,s_archived_matches;
static uint16_t s_window_first_year;

typedef struct LiveClub {
    uint16_t id, rating;
    uint8_t country, division;
    char name[18];
    /* Pre-season is an independent simulation ONLY, not player injuries or
     * the native club bank account. These figures are reset every July. */
    ExpandedLivePreseason preseason;
} LiveClub;
typedef struct LiveLeague {
    uint8_t country, division, count, points, overflow;
    uint8_t begin_phase, end_phase, native_schedule, legs;
    uint16_t clubs[LIVE_MAX_LEAGUE_CLUBS];
} LiveLeague;
static LiveClub s_clubs[LIVE_MAX_CLUBS];
static LiveLeague s_leagues[LIVE_MAX_LEAGUES];
static uint32_t s_club_count, s_league_count, s_weeks;
static uint32_t s_preseason_games_total;
static uint16_t s_first_year;
static ExpandedWorldFixture *s_fixtures;
static ExpandedWorldLedger s_ledger;
static int s_live;
static int32_t s_initial_day;
/* Pilot results are immutable. A recorded match may supply a single native
 * fixture result only when home/away IDs and the calendar phase agree. */
static uint8_t s_native_result_consumed[LIVE_MAX_FIXTURES];
static uint32_t s_native_matches,s_native_misses;
static uint32_t s_native_order_applied,s_native_schedule_leagues;

static uint16_t competition_id(uint8_t country, uint8_t division, unsigned season) {
    return (uint16_t)(season * 8192u + ((uint16_t)country << 5) + division);
}
static const LiveClub *club_by_id(uint16_t id) {
    for (uint32_t i=0; i<s_club_count; ++i)
        if (s_clubs[i].id == id) return &s_clubs[i];
    return NULL;
}
static LiveLeague *league_by_id(uint8_t country, uint8_t division) {
    for (uint32_t i=0; i<s_league_count; ++i)
        if (s_leagues[i].country==country && s_leagues[i].division==division)
            return &s_leagues[i];
    return NULL;
}
void ExpandedWorldLive_End(void) {
    if (s_fixtures) free(s_fixtures);
    s_fixtures=NULL; s_live=0; s_club_count=0; s_league_count=0;
    s_weeks=0; s_first_year=0; s_window_first_year=0; s_initial_day=0;
    s_preseason_games_total=0;
    free(s_history);s_history=NULL;s_history_count=0;
    s_history_capacity=0;s_archived_matches=0;
    s_native_matches=s_native_misses=0;
    s_native_order_applied=s_native_schedule_leagues=0;
    memset(s_native_result_consumed,0,sizeof s_native_result_consumed);
    memset(&s_ledger,0,sizeof s_ledger);
}
int ExpandedWorldLive_Begin(uint16_t start_year) {
    /* July 1 is the start of the manager's European season; August
     * remains the start of competitive matches in this pilot. */
    ExpandedWorldDate start={start_year,7,1};
    ExpandedWorldLive_End();
    if (!ExpandedWorld_DateToIndex(start,&s_initial_day)) return 0;
    s_first_year=start_year;
    s_window_first_year=start_year;
    s_fixtures=(ExpandedWorldFixture *)malloc(sizeof(*s_fixtures)*LIVE_MAX_FIXTURES);
    if (!s_fixtures) { ExpandedWorldLive_End(); return 0; }
    if (!ExpandedWorldLedger_Init(&s_ledger,s_fixtures,LIVE_MAX_FIXTURES,s_initial_day)) {
        ExpandedWorldLive_End(); return 0;
    }
    return 1;
}
int ExpandedWorldLive_AddClub(uint8_t country, uint8_t division,
                              uint16_t club, uint16_t rating,
                              const char *name, uint8_t points_per_win) {
    LiveLeague *league;
    LiveClub *dest;
    if (!s_fixtures || !name || !*name || points_per_win < 2 || points_per_win > 3 ||
        division >= 16 || club==0xffffu || s_club_count==LIVE_MAX_CLUBS ||
        club_by_id(club)) return 0;
    league=league_by_id(country,division);
    if (!league) {
        if (s_league_count==LIVE_MAX_LEAGUES) return 0;
        league=&s_leagues[s_league_count++];
        memset(league,0,sizeof *league);
        league->country=country;
        league->division=division;
        league->points=points_per_win;
        /* Original SWOS league defaults: Aug (56) to May (32),
         * with the end month in the following calendar year. */
        league->begin_phase=0x38u;
        league->end_phase=0x20u;
        league->legs=2u;
    }
    if (league->count == LIVE_MAX_LEAGUE_CLUBS) {
        league->overflow=1;
        return 0;
    }
    if (league->points != points_per_win) return 0;
    dest=&s_clubs[s_club_count++];
    memset(dest,0,sizeof *dest);
    dest->id=club; dest->rating=rating;
    dest->country=country; dest->division=division;
    strncpy(dest->name,name,sizeof dest->name-1);
    dest->preseason.morale=50;
    league->clubs[league->count++]=club;
    return 1;
}
/* Read actual national competition begin/end months from the OpenSWOS
 * league record, not from an independent world calendar. The original
 * DIY competition uses these exact values to interpolate match phases. */
int ExpandedWorldLive_SetLeagueSchedule(uint8_t country,uint8_t division,
                                        uint8_t begin,uint8_t end,uint8_t legs) {
    LiveLeague *league=league_by_id(country,division);
    if (!league || !s_fixtures || s_live || begin>=96u || end>=96u ||
        legs<1u || legs>4u)
        return 0;
    league->begin_phase=begin;
    league->end_phase=end;
    league->legs=legs;
    return 1;
}

/* Career DIY leagues normally shuffle their grid indices. For leagues
 * covered by the world ledger, remap those indices to the SAME canonical
 * club order used by the original SWOS fixture table in the world.
 * Only apply when every native club exactly belongs to one imported league.
 * All unrelated competitions retain the original SWOS shuffle. */
int ExpandedWorldLive_NativeOrder(const uint16_t *ids,uint32_t count,
                                  uint8_t *order) {
    if (!s_live || !ids || !order || count<2u ||
        count>LIVE_MAX_LEAGUE_CLUBS) return 0;
    for (uint32_t l=0;l<s_league_count;++l) {
        const LiveLeague *league=&s_leagues[l];
        uint8_t indices[LIVE_MAX_LEAGUE_CLUBS];
        if (!league->native_schedule || league->count!=count) continue;
        for (uint32_t i=0;i<count;++i) {
            uint32_t k=0;
            while (k<count && ids[k]!=league->clubs[i]) ++k;
            if (k==count) break;
            indices[i]=(uint8_t)k;
            if (i+1u==count) {
                memcpy(order,indices,count);
                ++s_native_order_applied;
                return 1;
            }
        }
    }
    return 0;
}

uint32_t ExpandedWorldLive_NativeOrderApplied(void) {
    return s_native_order_applied;
}
uint32_t ExpandedWorldLive_NativeScheduleLeagues(void) {
    return s_native_schedule_leagues;
}

static int fixture_compare(const void *aa, const void *bb) {
    const ExpandedWorldFixture *a=(const ExpandedWorldFixture *)aa;
    const ExpandedWorldFixture *b=(const ExpandedWorldFixture *)bb;
    if (a->day != b->day) return a->day < b->day ? -1 : 1;
    if (a->competition != b->competition) return a->competition < b->competition ? -1 : 1;
    if (a->id != b->id) return a->id < b->id ? -1 : 1;
    return 0;
}
/* Exactly the SWOS native pairing order for a regular domestic league:
 * same _fixtureSchedulePtrTable[n-2] as DIY_GetCurrentFixtureTeamPtrs.
 * The first cycle follows each stored (home,away) entry; the second
 * cycle reverses it. All records share the native month-eighth clock,
 * interpolated like DIY_RecomputeCompetitionDerivedProgressionTable.
 * Odd-sized divisions use the odd-table installed by FixtureRounds.
 *
 * This is NOT a separate round-robin scheduler: the OG table is the
 * single source of pairing order for both unmanaged and managed clubs. */
static int make_fixtures(LiveLeague *league,unsigned season,int32_t unused_start) {
    (void)unused_start;
    unsigned teams=league->count;
    if (league->overflow || teams<2u || teams>NATIVE_FIXTURE_MAX_CLUBS)
        return 0;
    const uint8_t *pairs=_fixtureSchedulePtrTable[teams-2u];
    if (!pairs) return 0;
    const unsigned per_round=teams/2u;
    const unsigned one_leg=teams*(teams-1u)/2u;
    const unsigned rounds_per_leg=(teams&1u)?teams:teams-1u;
    const unsigned rounds_total=rounds_per_leg*league->legs;
    unsigned begin=league->begin_phase,end=league->end_phase;
    if (end<begin) end+=0x60u;
    if (end<=begin || rounds_total==0u || end>255u ||
        s_ledger.count+one_leg*league->legs>s_ledger.capacity) return 0;
    /* Reuse an OG 16-bit season slot only AFTER that previous season
     * has been archived and removed from the resident fixture ledger. */
    const uint16_t cid=competition_id(league->country,league->division,season&1u);
    const uint16_t year=(uint16_t)(s_first_year+season);
    for (unsigned leg=0;leg<league->legs;++leg) {
        for (unsigned i=0;i<one_leg;++i) {
            unsigned posA=pairs[2u*i], posB=pairs[2u*i+1u];
            if (posA>=teams || posB>=teams || posA==posB) return 0;
            unsigned round=leg*rounds_per_leg+i/per_round;
            unsigned phase=rounds_total>1u ?
                begin+(round*(end-begin))/(rounds_total-1u) : begin;
            ExpandedWorldDate date;
            int32_t day=0;
            if (phase>255u ||
                !ExpandedWorld_NativePhaseToDate(year,(uint8_t)phase,&date) ||
                !ExpandedWorld_DateToIndex(date,&day)) return 0;
            ExpandedWorldFixture *f=&s_fixtures[s_ledger.count++];
            memset(f,0,sizeof *f);
            f->day=day;
            f->id=(uint32_t)(leg*one_leg+i);
            f->competition=cid;
            /* OG uses the low bit of its remaining-section counter:
             * odd-numbered sections reverse the same fixture table. */
            unsigned swap=(league->legs-leg)&1u;
            f->home=league->clubs[swap?posB:posA];
            f->away=league->clubs[swap?posA:posB];
        }
    }
    league->native_schedule=1;
    if (season==0u) ++s_native_schedule_leagues;
    return 1;
}
int ExpandedWorldLive_Seal(void) {
    if (!s_fixtures || s_live) return 0;
    s_ledger.count=0;
    for (uint32_t i=0;i<s_league_count;++i) s_leagues[i].native_schedule=0;
    s_native_schedule_leagues=0;
    for (unsigned season=0;season<LIVE_SEASONS;++season) {
        ExpandedWorldDate start={(uint16_t)(s_first_year+season),8,1};
        int32_t day;
        if (!ExpandedWorld_DateToIndex(start,&day)) break;
        for(uint32_t i=0;i<s_league_count;++i)
            (void)make_fixtures(&s_leagues[i],season,day);
    }
    if (!s_ledger.count) return 0;
    qsort(s_fixtures,s_ledger.count,sizeof s_fixtures[0],fixture_compare);
    s_live=1;
    return 1;
}

/* One chronological boundary: the oldest of two resident league seasons is
 * completed by Dec 31 of the following year. Preserve a compact immutable
 * summary, remove all its fixtures and re-use its 16-bit OG slot only after
 * the old season is gone. This is NOT a parallel calendar generator. */
static int live_packed_compare(const void *aa,const void *bb) {
    const LivePackedFixture *a=(const LivePackedFixture *)aa;
    const LivePackedFixture *b=(const LivePackedFixture *)bb;
    return fixture_compare(&a->fixture,&b->fixture);
}
static int live_roll_window(void) {
    if (!s_live || !s_fixtures ||
        (unsigned)s_window_first_year+2u>2199u) return 0;
    const uint16_t retiring=s_window_first_year;
    const unsigned retiring_slot=(unsigned)(retiring-s_first_year)&1u;
    LiveArchivedLeague arch[LIVE_MAX_LEAGUES];
    uint32_t arch_count=0,removed=0;
    /* Validate and summarize BEFORE changing any resident world data.
     * No season is discarded if a result is missing or the archive is full. */
    for(uint32_t l=0;l<s_league_count;++l) {
        const LiveLeague *league=&s_leagues[l];
        if (!league->native_schedule) continue;
        ExpandedWorldStanding table[LIVE_MAX_LEAGUE_CLUBS];
        for(unsigned i=0;i<league->count;++i)
            table[i].club=league->clubs[i];
        const uint16_t cid=competition_id(league->country,league->division,
                                           retiring_slot);
        if (!ExpandedWorldLedger_Standings(&s_ledger,cid,table,
                                  league->count,league->points)) return 0;
        uint32_t matches=0,digest=2166136261u;
        for(uint32_t i=0;i<s_ledger.count;++i) {
            const ExpandedWorldFixture *f=&s_fixtures[i];
            if (f->competition!=cid) continue;
            if (!f->played) {
                fprintf(stderr,"[living-world] cannot archive unfinished fixture "
                               "year=%u league=%u/%u id=%lu\n",
                      (unsigned)retiring,(unsigned)league->country,
                      (unsigned)league->division,(unsigned long)f->id);
                return 0;
            }
            ++matches;
            digest=(digest^((uint32_t)f->day))*16777619u;
            digest=(digest^f->id)*16777619u;
            digest=(digest^(((uint32_t)f->home<<16)|f->away))*16777619u;
            digest=(digest^(((uint32_t)f->goals_home<<8)|f->goals_away))*16777619u;
        }
        if (!matches || matches>65535u) return 0;
        unsigned winner=0;
        for(unsigned i=1;i<league->count;++i) {
            const ExpandedWorldStanding *a=&table[i],*b=&table[winner];
            int agd=(int)a->scored-(int)a->conceded;
            int bgd=(int)b->scored-(int)b->conceded;
            if(a->points>b->points ||
               (a->points==b->points && agd>bgd) ||
               (a->points==b->points && agd==bgd && a->scored>b->scored) ||
               (a->points==b->points && agd==bgd &&
                a->scored==b->scored && a->club<b->club)) winner=i;
        }
        LiveArchivedLeague *r=&arch[arch_count++];
        r->year=retiring;r->country=league->country;
        r->division=league->division;r->champion=table[winner].club;
        r->champion_points=(uint16_t)table[winner].points;
        r->matches=(uint16_t)matches;r->digest=digest&0x7fffffffu;
        removed+=matches;
    }
    if (!arch_count || s_history_count+arch_count>LIVE_HISTORY_LIMIT) return 0;
    if (s_history_count+arch_count>s_history_capacity) {
        uint32_t cap=s_history_capacity ? s_history_capacity*2u : 64u;
        while (cap<s_history_count+arch_count) cap*=2u;
        if(cap>LIVE_HISTORY_LIMIT) cap=LIVE_HISTORY_LIMIT;
        LiveArchivedLeague *p=(LiveArchivedLeague *)realloc(s_history,
                                                  sizeof(*p)*cap);
        if(!p) return 0;
        s_history=p;s_history_capacity=cap;
    }
    ExpandedWorldFixture *next=(ExpandedWorldFixture *)malloc(
                              sizeof(*next)*LIVE_MAX_FIXTURES);
    uint8_t *consumed=(uint8_t *)calloc(LIVE_MAX_FIXTURES,1u);
    LivePackedFixture *packed=(LivePackedFixture *)malloc(
                                  sizeof(*packed)*LIVE_MAX_FIXTURES);
    if (!next || !consumed || !packed) {
        free(next);free(consumed);free(packed);return 0;
    }
    uint32_t kept=0,kept_played=0;
    for(uint32_t i=0;i<s_ledger.count;++i) {
        const ExpandedWorldFixture *f=&s_fixtures[i];
        if ((unsigned)(f->competition>>13)==retiring_slot) continue;
        next[kept]=*f;consumed[kept]=s_native_result_consumed[i];
        kept_played+=f->played;
        ++kept;
    }
    if (s_ledger.played != kept_played+removed) {
        free(next);free(consumed);free(packed);
        fprintf(stderr,"[living-world] archive count mismatch\n");
        return 0;
    }
    ExpandedWorldFixture *old_fixtures=s_fixtures;
    ExpandedWorldLedger old_ledger=s_ledger;
    s_fixtures=next;s_ledger.fixtures=next;
    s_ledger.count=kept;s_ledger.played=kept_played;
    const unsigned fresh_season=(unsigned)(retiring+2u-s_first_year);
    int generated=1;
    for(uint32_t l=0;l<s_league_count;++l)
        if (s_leagues[l].native_schedule &&
            !make_fixtures(&s_leagues[l],fresh_season,0)) {
            generated=0;break;
        }
    if (!generated) {
        s_fixtures=old_fixtures;s_ledger=old_ledger;
        free(next);free(consumed);free(packed);
        fprintf(stderr,"[living-world] generation failed for season %u\n",
                 (unsigned)(retiring+2u));
        return 0;
    }
    /* Existing native-result consumption bits belong to FIXTURES, not array
     * indices. Keep each bit attached while sorting interleaved calendars. */
    for(uint32_t i=0;i<s_ledger.count;++i) {
        packed[i].fixture=next[i];
        packed[i].consumed=i<kept?consumed[i]:0u;
    }
    qsort(packed,s_ledger.count,sizeof(*packed),live_packed_compare);
    /* Validate BEFORE touching the old consumption array: a failure must
     * leave the original in-RAM world as consistent as the old on-disk save. */
    for(uint32_t i=1;i<s_ledger.count;++i) {
        if (fixture_compare(&packed[i-1].fixture,&packed[i].fixture)>=0) {
            s_fixtures=old_fixtures;s_ledger=old_ledger;
            free(next);free(consumed);free(packed);
            fprintf(stderr,"[living-world] duplicate fixture in new window\n");
            return 0;
        }
    }
    for(uint32_t i=0;i<s_ledger.count;++i) {
        next[i]=packed[i].fixture;
        s_native_result_consumed[i]=packed[i].consumed;
    }
    memset(s_native_result_consumed+s_ledger.count,0,
           LIVE_MAX_FIXTURES-s_ledger.count);
    memcpy(s_history+s_history_count,arch,arch_count*sizeof(*arch));
    s_history_count+=arch_count;s_archived_matches+=removed;
    ++s_window_first_year;
    free(old_fixtures);free(consumed);free(packed);
    fprintf(stderr,"[living-world] archived season %u: %lu matches, "
                   "history=%lu league-years, window=%u-%u fixtures=%lu\n",
            (unsigned)retiring,(unsigned long)removed,
            (unsigned long)s_history_count,(unsigned)s_window_first_year,
            (unsigned)(s_window_first_year+1u),
            (unsigned long)s_ledger.count);
    return 1;
}
static uint32_t hash32(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352du;
    x ^= x >> 15; x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}
/* Independent low-cost preseason events. Every simulated club gets the
 * same opportunity: four July friendly dates, with a BYE in odd leagues.
 * Results and economic/morale/injury-risk aggregates are deterministic and
 * remain in the pilot. Native club injuries, budgets and match results are
 * deliberately untouched until the world-save migration is implemented. */
static void preseason_reset(void) {
    for (uint32_t i=0;i<s_club_count;++i) {
        memset(&s_clubs[i].preseason,0,sizeof s_clubs[i].preseason);
        s_clubs[i].preseason.morale=50;
    }
}
static void preseason_match(LiveClub *home,LiveClub *away,
                            unsigned season,unsigned round,unsigned index) {
    uint32_t seed=hash32(((uint32_t)season<<24) ^
                         ((uint32_t)home->id<<8) ^ (uint32_t)away->id ^
                         (round*0x9e3779b9u) ^ index);
    int strength=(int)home->rating-(int)away->rating;
    unsigned hg=(seed&3u)+(strength>40 ? 1u : 0u);
    unsigned ag=((seed>>4)&3u)+(strength< -40 ? 1u : 0u);
    ExpandedLivePreseason *h=&home->preseason,*a=&away->preseason;
    ++h->friendlies; ++a->friendlies;
    h->goals_for+=(uint16_t)hg; h->goals_against+=(uint16_t)ag;
    a->goals_for+=(uint16_t)ag; a->goals_against+=(uint16_t)hg;
    /* Budget units and attendance will be replaced by the ticket-price
     * engine when CLUB and native finances share one saved world. */
    h->net_income += (int32_t)(350u+(unsigned)home->rating*2u);
    a->net_income -= 180;
    int hm=(int)h->morale+(hg>ag ? 3 : hg<ag ? -3 : 1);
    int am=(int)a->morale+(ag>hg ? 3 : ag<hg ? -3 : 1);
    h->morale=(uint8_t)(hm<0?0:hm>100?100:hm);
    a->morale=(uint8_t)(am<0?0:am>100?100:am);
    if (h->fatigue<95) h->fatigue+=5;
    if (a->fatigue<95) a->fatigue+=5;
    if ((seed>>12)%100u<3u) ++h->injury_events;
    if ((seed>>19)%100u<3u) ++a->injury_events;
    ++s_preseason_games_total;
}
static void preseason_round(unsigned season,unsigned round) {
    for (uint32_t l=0;l<s_league_count;++l) {
        LiveLeague *league=&s_leagues[l];
        unsigned n=league->count;
        if (n<2 || league->overflow) continue;
        for (unsigned pair=0;pair<n/2u;++pair) {
            unsigned first=(round+pair*2u)%n;
            unsigned second=(first+1u)%n;
            uint16_t home_id=league->clubs[first];
            uint16_t away_id=league->clubs[second];
            LiveClub *home=NULL,*away=NULL;
            for (uint32_t c=0;c<s_club_count;++c) {
                if(s_clubs[c].id==home_id) home=&s_clubs[c];
                if(s_clubs[c].id==away_id) away=&s_clubs[c];
            }
            if (home && away) preseason_match(home,away,season,round,pair);
        }
    }
}
static void preseason_advance(int32_t old_day,int32_t new_day) {
    ExpandedWorldDate first,last;
    if (!ExpandedWorld_IndexToDate(old_day,&first) ||
        !ExpandedWorld_IndexToDate(new_day,&last)) return;
    for (unsigned year=first.year;year<=last.year;++year) {
        if (year<s_first_year || year>2199u) continue;
        unsigned season=year-s_first_year;
        ExpandedWorldDate start={(uint16_t)year,7,1};
        int32_t july;
        if (!ExpandedWorld_DateToIndex(start,&july)) continue;
        if (old_day < july && new_day>=july) preseason_reset();
        for (unsigned round=0;round<4u;++round) {
            /* July 7, 14, 21, 28: weekly AI friendlies. */
            int32_t event_day=july+6+(int32_t)round*7;
            if (event_day>old_day && event_day<=new_day)
                preseason_round(season,round);
        }
    }
}
static int resolve_game(const ExpandedWorldFixture *f,uint8_t *home,uint8_t *away,void *ctx) {
    (void)ctx;
    const LiveClub *h=club_by_id(f->home),*a=club_by_id(f->away);
    if (!h || !a) return 0;
    uint32_t seed=hash32(((uint32_t)f->competition<<16) ^ f->id ^
                         ((uint32_t)f->home<<8) ^ f->away ^ (uint32_t)s_first_year);
    int diff=(int)h->rating-(int)a->rating;
    int hgoals=(int)(seed%4u)+((diff>20)?1:0)+((diff>70)?1:0);
    int agoals=(int)((seed>>5)%4u)+((diff< -20)?1:0)+((diff< -70)?1:0);
    if ((seed>>11)%4u==0u && agoals>0) agoals--;
    *home=(uint8_t)(hgoals>7?7:hgoals);
    *away=(uint8_t)(agoals>7?7:agoals);
    return 1;
}
/* The newest year's early-season games may begin in May, so create
 * that season on January 1, after the oldest season ends on Dec 31. */
int ExpandedWorldLive_AdvanceWeeks(uint32_t weeks) {
    uint64_t d=(uint64_t)(uint32_t)s_initial_day+(uint64_t)weeks*7u;
    if (!s_live || d>INT32_MAX || weeks<s_weeks) return 0;
    const int32_t target=(int32_t)d;
    while ((unsigned)s_window_first_year+2u<=2199u) {
        const uint16_t newest=(uint16_t)(s_window_first_year+2u);
        ExpandedWorldDate boundary={newest,1u,1u};
        int32_t boundary_day=0;
        if (!ExpandedWorld_DateToIndex(boundary,&boundary_day)) return 0;
        if (target<boundary_day) break;
        if (s_ledger.now<boundary_day) {
            int32_t old=s_ledger.now;
            if (!ExpandedWorldLedger_Advance(&s_ledger,boundary_day-1,
                                              resolve_game,NULL)) return 0;
            preseason_advance(old,boundary_day-1);
        }
        if (!live_roll_window()) return 0;
    }
    int32_t old=s_ledger.now;
    if (!ExpandedWorldLedger_Advance(&s_ledger,target,resolve_game,NULL)) return 0;
    preseason_advance(old,target);
    s_weeks=weeks;
    return 1;
}
/* Synchronize the live world at an original OpenSWOS season boundary.
 * Do not rewrite the OG competition clock; the published annual calendar
 * remains the same source that produced the original match table. */
int ExpandedWorldLive_AdvanceToSeason(uint16_t season_year) {
    if (!s_live || season_year<s_first_year || season_year>2199u) return 0;
    ExpandedWorldDate target_date={season_year,7u,1u};
    int32_t date=0;
    if (!ExpandedWorld_DateToIndex(target_date,&date) ||
        date<s_initial_day) return 0;
    uint32_t week=(uint32_t)(date-s_initial_day+6)/7u;
    if (week<s_weeks) return 1;
    return ExpandedWorldLive_AdvanceWeeks(week);
}
/* Read a played score from the canonical world ledger without modifying
 * it or consuming a native replay token. Called by the native league state
 * importer, which applies the result to the original SWOS fixture progress
 * and standings rather than opening 200+ historical league menus.
 *
 * Exact oriented pair, current season, played before the world clock. */
int ExpandedWorldLive_PeekNativeLeagueResult(uint16_t home,uint16_t away,
                                            uint8_t *gh,uint8_t *ga) {
    ExpandedWorldDate first,last;
    int32_t begin=0,end=0;
    if (!s_live || !s_ledger.fixtures || !gh || !ga || home==away ||
        !club_by_id(home) || !club_by_id(away)) return 0;
    const uint16_t season=ExpandedWorldLive_SeasonYear();
    first=(ExpandedWorldDate){season,8,1};
    last=(ExpandedWorldDate){(uint16_t)(season+1u),8,1};
    if (!ExpandedWorld_DateToIndex(first,&begin) ||
        !ExpandedWorld_DateToIndex(last,&end)) return 0;
    for (uint32_t i=0;i<s_ledger.count;++i) {
        const ExpandedWorldFixture *f=&s_fixtures[i];
        if (!f->played || f->home!=home || f->away!=away ||
            f->day<begin || f->day>=end || f->day>s_ledger.now ||
            f->goals_home>31u || f->goals_away>31u) continue;
        *gh=f->goals_home;
        *ga=f->goals_away;
        return 1;
    }
    return 0;
}
/* Historical bridge: the pilot and native calendars currently put the
 * SAME home/away pairing on different days. Match on club identity,
 * home/away and season, not on exact match date. A result is eligible only
 * after the pilot has played it (never predict a future result), and can
 * be consumed once. Native competition handlers still apply all results
 * through their normal bookkeeping.
 *
 * The preview table will not be guaranteed identical until the two
 * fixture schedules are replaced by one canonical world calendar. */
int ExpandedWorldLive_TakeNativeLeagueResult(uint16_t home,uint16_t away,
                                             uint8_t native_phase,
                                             uint8_t *goals_home,
                                             uint8_t *goals_away) {
    ExpandedWorldDate native_date,start_date,end_date;
    int32_t start_day=0,end_day=0;
    if (!s_live || !s_ledger.fixtures || !goals_home || !goals_away ||
        home==away || !club_by_id(home) || !club_by_id(away) ||
        !ExpandedWorld_NativePhaseToDate(ExpandedWorldLive_SeasonYear(),native_phase,
                                          &native_date)) {
        ++s_native_misses;
        return 0;
    }
    /* An OpenSWOS season begins in August. Before Aug 1 the native
     * key belongs to the previous season, not a new competition. */
    uint16_t season_year=(uint16_t)(native_date.year-
                                    (native_date.month<8u ? 1u : 0u));
    if (season_year<s_window_first_year ||
        (uint32_t)(season_year-s_window_first_year)>=LIVE_SEASONS) {
        ++s_native_misses;
        return 0;
    }
    start_date=(ExpandedWorldDate){season_year,8,1};
    end_date=(ExpandedWorldDate){(uint16_t)(season_year+1u),8,1};
    if (!ExpandedWorld_DateToIndex(start_date,&start_day) ||
        !ExpandedWorld_DateToIndex(end_date,&end_day)) {
        ++s_native_misses;
        return 0;
    }
    for (uint32_t i=0;i<s_ledger.count;i++) {
        const ExpandedWorldFixture *f=&s_fixtures[i];
        if (!f->played || s_native_result_consumed[i] ||
            f->home!=home || f->away!=away ||
            f->day<start_day || f->day>=end_day ||
            f->day>s_ledger.now) continue;
        if (f->goals_home>31u || f->goals_away>31u) continue;
        *goals_home=f->goals_home;
        *goals_away=f->goals_away;
        s_native_result_consumed[i]=1;
        ++s_native_matches;
        return 1;
    }
    ++s_native_misses;
    return 0;
}
/* Once the human manager has joined, the native match result is written
 * into the SAME global world ledger, not into a new parallel simulation.
 * Scores played during catch-up were already precomputed by that ledger.
 *
 * Require exact home/away identity AND the same original native match
 * phase to avoid accidentally jumping months ahead if a competition
 * uses a different schedule or a special league phase/split. */
int ExpandedWorldLive_RecordNativeLeagueResult(uint16_t home,uint16_t away,
                                               uint8_t phase,
                                               uint8_t goals_home,
                                               uint8_t goals_away) {
    ExpandedWorldDate date,next;
    int32_t phase_day=0,next_day=0;
    if (!s_live || !s_ledger.fixtures || home==away ||
        goals_home>31u || goals_away>31u ||
        !ExpandedWorld_NativePhaseToDate(ExpandedWorldLive_SeasonYear(),phase,&date) ||
        phase==255u ||
        !ExpandedWorld_NativePhaseToDate(ExpandedWorldLive_SeasonYear(),(uint8_t)(phase+1u),&next) ||
        !ExpandedWorld_DateToIndex(date,&phase_day) ||
        !ExpandedWorld_DateToIndex(next,&next_day)) return 0;
    for (uint32_t i=0;i<s_ledger.count;++i) {
        ExpandedWorldFixture *f=&s_fixtures[i];
        if (f->home!=home || f->away!=away ||
            f->day<phase_day || f->day>=next_day) continue;
        if (f->played) {
            /* A real played match is authoritative over a tentative
             * world score. All tables are recalculated from this ledger. */
            if (f->goals_home!=goals_home || f->goals_away!=goals_away) {
                f->goals_home=goals_home;
                f->goals_away=goals_away;
            }
            return 1;
        }
        f->goals_home=goals_home;
        f->goals_away=goals_away;
        f->played=1;
        ++s_ledger.played;
        if (f->day>s_ledger.now) {
            int32_t old=s_ledger.now;
            if (!ExpandedWorldLedger_Advance(&s_ledger,f->day,
                                              resolve_game,NULL)) return 0;
            preseason_advance(old,s_ledger.now);
            s_weeks=(uint32_t)(s_ledger.now-s_initial_day)/7u;
        }
        return 1;
    }
    return 0;
}

uint32_t ExpandedWorldLive_NativeMatches(void) { return s_native_matches; }
uint32_t ExpandedWorldLive_NativeMisses(void) { return s_native_misses; }
uint32_t ExpandedWorldLive_Results(void) { return s_archived_matches+s_ledger.played; }
uint32_t ExpandedWorldLive_ArchivedResults(void) { return s_archived_matches; }
uint32_t ExpandedWorldLive_ResidentResults(void) { return s_ledger.played; }
uint32_t ExpandedWorldLive_ArchivedLeagues(void) { return s_history_count; }
uint16_t ExpandedWorldLive_WindowFirstYear(void) { return s_window_first_year; }
int ExpandedWorldLive_HistoryChampion(uint16_t year,uint8_t country,
                                      uint8_t division,uint16_t *champion) {
    if (!champion) return 0;
    for(uint32_t i=0;i<s_history_count;++i) {
        const LiveArchivedLeague *r=&s_history[i];
        if(r->year==year && r->country==country && r->division==division) {
            *champion=r->champion; return 1;
        }
    }
    return 0;
}
uint32_t ExpandedWorldLive_Fixtures(void) { return s_live?s_ledger.count:0; }
uint32_t ExpandedWorldLive_PlayerSlots(void) { return s_live?s_club_count*16u:0; }
uint32_t ExpandedWorldLive_MemoryBytes(void) {
    if (!s_live) return 0;
    /* Allocated pilot RAM, not the disk size of the actual world pack. */
    return (uint32_t)(sizeof s_clubs+sizeof s_leagues+sizeof s_ledger+
                      sizeof(*s_fixtures)*LIVE_MAX_FIXTURES+
                      sizeof s_native_result_consumed+
                      s_history_capacity*sizeof(*s_history));
}
uint32_t ExpandedWorldLive_Clubs(void) { return s_club_count; }
uint32_t ExpandedWorldLive_Leagues(void) { return s_league_count; }
uint32_t ExpandedWorldLive_Weeks(void) { return s_weeks; }
uint32_t ExpandedWorldLive_PreseasonGamesTotal(void) { return s_preseason_games_total; }
int ExpandedWorldLive_Preseason(uint16_t club,ExpandedLivePreseason *out) {
    const LiveClub *c=club_by_id(club);
    if (!out || !s_live || !c) return 0;
    *out=c->preseason;
    return 1;
}
uint16_t ExpandedWorldLive_SeasonYear(void) {
    ExpandedWorldDate d;
    if (!s_live || !ExpandedWorld_IndexToDate(s_ledger.now,&d)) return s_first_year;
    return (uint16_t)(d.year-(d.month<7 ? 1 : 0));
}
static int row_before(const ExpandedLiveRow *a,const ExpandedLiveRow *b) {
    if (a->points != b->points) return a->points>b->points;
    int agd=(int)a->scored-(int)a->conceded;
    int bgd=(int)b->scored-(int)b->conceded;
    if (agd!=bgd) return agd>bgd;
    if (a->scored!=b->scored) return a->scored>b->scored;
    return a->club < b->club;
}
int ExpandedWorldLive_Table(uint8_t country, uint8_t division,
                            ExpandedLiveRow *rows, uint32_t capacity,
                            uint32_t *total) {
    LiveLeague *league=league_by_id(country,division);
    ExpandedWorldStanding table[LIVE_MAX_LEAGUE_CLUBS];
    int32_t season_delta;
    if (total) *total=0;
    if (!s_live || !rows || !total || !league || league->overflow ||
        league->count<2 || capacity<league->count) return 0;
    season_delta=(int32_t)ExpandedWorldLive_SeasonYear()-(int32_t)s_window_first_year;
    if (season_delta<0 || season_delta>=(int32_t)LIVE_SEASONS) return 0;
    for(unsigned i=0;i<league->count;++i) table[i].club=league->clubs[i];
    if (!ExpandedWorldLedger_Standings(&s_ledger,
            competition_id(country,division,
                 (unsigned)(ExpandedWorldLive_SeasonYear()-s_first_year)&1u),
            table,league->count,league->points)) return 0;
    for(unsigned i=0;i<league->count;++i) {
        const LiveClub *c=club_by_id(table[i].club);
        if (!c) return 0;
        ExpandedLiveRow *row=&rows[i];
        row->club=c->id;
        memcpy(row->name,c->name,sizeof row->name);
        row->played=table[i].played;
        row->wins=table[i].wins;
        row->drawn=table[i].drawn;
        row->lost=table[i].lost;
        row->scored=table[i].scored;
        row->conceded=table[i].conceded;
        row->points=table[i].points;
    }
    for(unsigned i=1;i<league->count;++i) {
        ExpandedLiveRow t=rows[i];
        unsigned j=i;
        while (j && row_before(&t,&rows[j-1])) { rows[j]=rows[j-1];--j; }
        rows[j]=t;
    }
    *total=league->count;
    return 1;
}


/* --------------------------------------------------------------------------
 * Living World .CAR persistence (schema 4).
 *
 * The world has ONE authoritative fixture/result ledger, shared with native
 * match updates. Serializing it IN the career save avoids a second sidecar
 * database (which could diverge on rename/restore), an accidental re-sim,
 * and loading country TEAM files after the active career has been restored.
 * Arrays are typed numeric JSON, never raw pointer/struct dumps.
 *
 * Parsing is transactional: malformed/truncated arrays cannot half-replace
 * a working world. Every bounded count and referenced club is checked before
 * the new snapshot replaces the live state.
 * -------------------------------------------------------------------------- */
typedef struct LiveSaveImage {
    LiveClub clubs[LIVE_MAX_CLUBS];
    LiveLeague leagues[LIVE_MAX_LEAGUES];
    ExpandedWorldFixture fixtures[LIVE_MAX_FIXTURES];
    uint8_t consumed[LIVE_MAX_FIXTURES];
    uint32_t clubs_count,leagues_count,fixture_count,played;
    uint32_t weeks,preseason_games,native_matches,native_misses;
    uint32_t native_orders,native_schedules;
    uint16_t year,window_year;
    int32_t now,initial;
    LiveArchivedLeague *history;
    uint32_t history_count,history_capacity,archived_matches;
} LiveSaveImage;

/* mjson_next walks one array level. Require the exact row length and type:
 * accidental strings, fractions and oversized rows are not accepted. */
static int live_parse_int_row(const char *src,int len,long *out,
                              unsigned max,unsigned *num) {
    int off=0,ko,kl,vo,vl,type;
    unsigned n=0;
    if (!src || len<=0 || !out || !num) return 0;
    for (;;) {
        int next=mjson_next(src,len,off,&ko,&kl,&vo,&vl,&type);
        if (next==0) break;
        if (next<0 || next<=off || n>=max || type!=MJSON_TOK_NUMBER ||
            mjson_get_long(src+vo,vl,"$",&out[n])<=0) return 0;
        ++n;
        off=next;
    }
    *num=n;
    return 1;
}
static int live_parse_club_row(const char *src,int len,LiveClub *c) {
    long v[11]={0};
    unsigned n=0;
    int off=0,ko,kl,vo,vl,type;
    char name[18]={0};
    for (;;) {
        int next=mjson_next(src,len,off,&ko,&kl,&vo,&vl,&type);
        if (next==0) break;
        if (next<0 || next<=off || n>=12u) return 0;
        if (n==4u) {
            if (type!=MJSON_TOK_STRING ||
                mjson_get_string(src+vo,vl,"$",name,sizeof name)<=0)
                return 0;
        } else {
            unsigned idx=n<4u?n:n-1u;
            if (type!=MJSON_TOK_NUMBER ||
                mjson_get_long(src+vo,vl,"$",&v[idx])<=0) return 0;
        }
        ++n;
        off=next;
    }
    if (n!=12u || !name[0] || v[0]<0 || v[0]>=65535 ||
        v[1]<0 || v[1]>65535 || v[2]<0 || v[2]>255 ||
        v[3]<0 || v[3]>=16 || v[4]<0 || v[4]>65535 ||
        v[5]<0 || v[5]>65535 || v[6]<0 || v[6]>65535 ||
        v[7]<0 || v[7]>100 || v[8]<0 || v[8]>100 ||
        v[9]<0 || v[9]>255) return 0;
    memset(c,0,sizeof *c);
    c->id=(uint16_t)v[0]; c->rating=(uint16_t)v[1];
    c->country=(uint8_t)v[2]; c->division=(uint8_t)v[3];
    memcpy(c->name,name,sizeof name);
    c->preseason.friendlies=(uint16_t)v[4];
    c->preseason.goals_for=(uint16_t)v[5];
    c->preseason.goals_against=(uint16_t)v[6];
    c->preseason.morale=(uint8_t)v[7];
    c->preseason.fatigue=(uint8_t)v[8];
    c->preseason.injury_events=(uint8_t)v[9];
    c->preseason.net_income=(int32_t)v[10];
    return 1;
}
static int live_parse_league_row(const char *src,int len,LiveLeague *l) {
    long v[9+LIVE_MAX_LEAGUE_CLUBS]={0};
    unsigned n=0;
    if (!live_parse_int_row(src,len,v,9+LIVE_MAX_LEAGUE_CLUBS,&n) ||
        n<9 || v[0]<0 || v[0]>255 || v[1]<0 || v[1]>=16 ||
        v[2]<2 || v[2]>(long)LIVE_MAX_LEAGUE_CLUBS ||
        v[3]<2 || v[3]>3 || v[4]<0 || v[4]>1 ||
        v[5]<0 || v[5]>255 || v[6]<0 || v[6]>255 ||
        v[7]<0 || v[7]>1 || v[8]<1 || v[8]>4 ||
        n!=(unsigned)(9+v[2])) return 0;
    memset(l,0,sizeof *l);
    l->country=(uint8_t)v[0]; l->division=(uint8_t)v[1];
    l->count=(uint8_t)v[2]; l->points=(uint8_t)v[3];
    l->overflow=(uint8_t)v[4]; l->begin_phase=(uint8_t)v[5];
    l->end_phase=(uint8_t)v[6]; l->native_schedule=(uint8_t)v[7];
    l->legs=(uint8_t)v[8];
    for (unsigned i=0;i<l->count;++i) {
        if (v[9+i]<0 || v[9+i]>=65535) return 0;
        l->clubs[i]=(uint16_t)v[9+i];
    }
    return 1;
}
static int live_parse_fixture_row(const char *src,int len,
                                  ExpandedWorldFixture *f,uint8_t *consumed) {
    long v[9]={0};
    unsigned n=0;
    if (!live_parse_int_row(src,len,v,9,&n) || n!=9 ||
        v[0]<0 || v[1]<0 || v[2]<0 || v[2]>65535 ||
        v[3]<0 || v[3]>=65535 || v[4]<0 || v[4]>=65535 ||
        v[3]==v[4] || v[5]<0 || v[5]>1 ||
        v[6]<0 || v[6]>31 || v[7]<0 || v[7]>31 ||
        v[8]<0 || v[8]>1 || (v[8] && !v[5]))
        return 0;
    f->day=(int32_t)v[0]; f->id=(uint32_t)v[1];
    f->competition=(uint16_t)v[2];
    f->home=(uint16_t)v[3]; f->away=(uint16_t)v[4];
    f->played=(uint8_t)v[5];
    f->goals_home=(uint8_t)v[6]; f->goals_away=(uint8_t)v[7];
    *consumed=(uint8_t)v[8];
    return 1;
}

/* Binary-safe zlib+Base64 payload for the immutable fixture schedule and
 * mutable scores. The one .CAR JSON file remains self-contained. Both
 * the previous uncompressed v1 snapshot and the compressed v2 are readable.
 * Save errors abort the entire .CAR write: never silently save a null world. */
#define LIVE_FIXTURE_JSON_LIMIT (LIVE_MAX_FIXTURES * 96u + 2u)
typedef struct LiveJsonOut {
    char *ptr;
    size_t len,cap;
    int oom;
} LiveJsonOut;
static int live_json_emit(const char *s,int len,void *user) {
    LiveJsonOut *o=(LiveJsonOut *)user;
    if (!o || o->oom || len<0) return 0;
    const size_t n=(size_t)len;
    if (n>LIVE_FIXTURE_JSON_LIMIT || o->len>LIVE_FIXTURE_JSON_LIMIT-n) {
        o->oom=1;
        return 0;
    }
    if (o->len+n+1u>o->cap) {
        size_t cap=o->cap?o->cap:4096u;
        while (cap<o->len+n+1u) cap*=2u;
        char *p=(char *)realloc(o->ptr,cap);
        if (!p) { o->oom=1;return 0; }
        o->ptr=p;o->cap=cap;
    }
    memcpy(o->ptr+o->len,s,n);
    o->len+=n;
    o->ptr[o->len]=0;
    return len;
}
static char *live_base64_encode(const unsigned char *src,size_t n) {
    static const char abc[]=
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (!src || n>LIVE_FIXTURE_JSON_LIMIT) return NULL;
    const size_t size=4u*((n+2u)/3u);
    char *dst=(char *)malloc(size+1u);
    if (!dst) return NULL;
    size_t d=0;
    for (size_t i=0;i<n;i+=3u) {
        unsigned v=(unsigned)src[i]<<16;
        if (i+1u<n) v|=(unsigned)src[i+1u]<<8;
        if (i+2u<n) v|=src[i+2u];
        dst[d++]=abc[(v>>18)&63u];dst[d++]=abc[(v>>12)&63u];
        dst[d++]=(i+1u<n)?abc[(v>>6)&63u]:'=';
        dst[d++]=(i+2u<n)?abc[v&63u]:'=';
    }
    dst[d]=0;
    return dst;
}
static int live_b64_value(unsigned char c) {
    if (c>='A' && c<='Z') return (int)c-'A';
    if (c>='a' && c<='z') return (int)c-'a'+26;
    if (c>='0' && c<='9') return (int)c-'0'+52;
    if (c=='+') return 62;
    if (c=='/') return 63;
    return -1;
}
static int live_base64_decode(const char *src,size_t n,
                              unsigned char *dst,size_t cap,size_t *out) {
    size_t d=0;
    if (!src || !dst || !out || (n&3u)) return 0;
    for (size_t i=0;i<n;i+=4u) {
        int a=live_b64_value((unsigned char)src[i]);
        int b=live_b64_value((unsigned char)src[i+1u]);
        int c=(src[i+2u]=='=')?0:live_b64_value((unsigned char)src[i+2u]);
        int e=(src[i+3u]=='=')?0:live_b64_value((unsigned char)src[i+3u]);
        const int pad2=src[i+2u]=='=';
        const int pad3=src[i+3u]=='=';
        if (a<0 || b<0 || c<0 || e<0 || (pad2 && !pad3) ||
            ((pad2 || pad3) && i+4u!=n) ||
            (pad2 && (b&15)) || (!pad2 && pad3 && (c&3)))
            return 0;
        unsigned bits=((unsigned)a<<18)|((unsigned)b<<12)|
                      ((unsigned)c<<6)|(unsigned)e;
        unsigned count=pad2?1u:pad3?2u:3u;
        if (d+count>cap) return 0;
        dst[d++]=(unsigned char)(bits>>16);
        if (count>1u) dst[d++]=(unsigned char)(bits>>8);
        if (count>2u) dst[d++]=(unsigned char)bits;
    }
    *out=d;
    return 1;
}
static char *live_inflate_fixture_json(const char *src,int len,
                                       uint32_t raw_size) {
    unsigned char *packed=NULL;
    char *plain=NULL;
    size_t packed_len=0;
    if (!src || len<2 || src[0]!='"' || src[len-1]!='"' ||
        ((unsigned)(len-2)&3u) || (uint32_t)len>LIVE_FIXTURE_JSON_LIMIT*2u ||
        raw_size<2u || raw_size>LIVE_FIXTURE_JSON_LIMIT) return NULL;
    packed=(unsigned char *)malloc((size_t)(len-2)/4u*3u+1u);
    plain=(char *)malloc((size_t)raw_size+1u);
    if (!packed || !plain ||
        !live_base64_decode(src+1,(size_t)(len-2),packed,
                             (size_t)(len-2)/4u*3u+1u,&packed_len))
        goto fail;
    mz_ulong dstlen=(mz_ulong)raw_size;
    if (mz_uncompress((unsigned char *)plain,&dstlen,packed,
                      (mz_ulong)packed_len)!=MZ_OK ||
        dstlen!=(mz_ulong)raw_size) goto fail;
    plain[raw_size]=0;
    free(packed);
    return plain;
fail:
    free(packed);free(plain);
    return NULL;
}

int ExpandedWorldLive_WriteJson(int (*emit)(const char*,int,void*),void *ctx) {
    if (!emit) return 0;
    if (!s_live || !s_fixtures) {
        mjson_printf(emit,ctx,"%s","null");
        return 1;
    }
    mjson_printf(emit,ctx,
      "{%Q:3,%Q:%u,%Q:%lu,%Q:%d,%Q:%lu,%Q:[%lu,%lu,%lu,%lu],%Q:%u,%Q:[",
      "version","first_year",(unsigned)s_first_year,
      "weeks",(unsigned long)s_weeks,"now",(int)s_ledger.now,
      "preseason_games",(unsigned long)s_preseason_games_total,
      "native",(unsigned long)s_native_matches,(unsigned long)s_native_misses,
      (unsigned long)s_native_order_applied,(unsigned long)s_native_schedule_leagues,
      "window_first_year",(unsigned)s_window_first_year,
      "history");
    for(uint32_t i=0;i<s_history_count;++i) {
        const LiveArchivedLeague *r=&s_history[i];
        if(i) mjson_printf(emit,ctx,"%s",",");
        mjson_printf(emit,ctx,"[%u,%u,%u,%u,%u,%u,%lu]",
            (unsigned)r->year,(unsigned)r->country,
            (unsigned)r->division,(unsigned)r->champion,
            (unsigned)r->champion_points,(unsigned)r->matches,
            (unsigned long)r->digest);
    }
    mjson_printf(emit,ctx,"],%Q:[","clubs");
    for (uint32_t i=0;i<s_club_count;++i) {
        const LiveClub *c=&s_clubs[i];
        const ExpandedLivePreseason *p=&c->preseason;
        if (i) mjson_printf(emit,ctx,"%s",",");
        mjson_printf(emit,ctx,
          "[%u,%u,%u,%u,%Q,%u,%u,%u,%u,%u,%u,%d]",
          (unsigned)c->id,(unsigned)c->rating,(unsigned)c->country,
          (unsigned)c->division,c->name,(unsigned)p->friendlies,
          (unsigned)p->goals_for,(unsigned)p->goals_against,
          (unsigned)p->morale,(unsigned)p->fatigue,
          (unsigned)p->injury_events,(int)p->net_income);
    }
    mjson_printf(emit,ctx,"],%Q:[","leagues");
    for (uint32_t i=0;i<s_league_count;++i) {
        const LiveLeague *l=&s_leagues[i];
        if (i) mjson_printf(emit,ctx,"%s",",");
        mjson_printf(emit,ctx,"[%u,%u,%u,%u,%u,%u,%u,%u,%u",
          (unsigned)l->country,(unsigned)l->division,
          (unsigned)l->count,(unsigned)l->points,
          (unsigned)l->overflow,(unsigned)l->begin_phase,
          (unsigned)l->end_phase,(unsigned)l->native_schedule,
          (unsigned)l->legs);
        for (unsigned j=0;j<l->count;++j)
            mjson_printf(emit,ctx,",%u",(unsigned)l->clubs[j]);
        mjson_printf(emit,ctx,"%s","]");
    }
    /* Only the fixture ledger is compressed; club/league identities remain
     * readable typed JSON. Using the existing miniz avoids an extra DLL. */
    LiveJsonOut raw={0};
    unsigned char *compressed=NULL;
    char *encoded=NULL;
    int ok=0;
    mjson_printf(live_json_emit,&raw,"%s","[");
    for (uint32_t i=0;i<s_ledger.count && !raw.oom;++i) {
        const ExpandedWorldFixture *f=&s_fixtures[i];
        if (i) mjson_printf(live_json_emit,&raw,"%s",",");
        mjson_printf(live_json_emit,&raw,"[%d,%lu,%u,%u,%u,%u,%u,%u,%u]",
          (int)f->day,(unsigned long)f->id,(unsigned)f->competition,
          (unsigned)f->home,(unsigned)f->away,(unsigned)f->played,
          (unsigned)f->goals_home,(unsigned)f->goals_away,
          (unsigned)s_native_result_consumed[i]);
    }
    mjson_printf(live_json_emit,&raw,"%s","]");
    if (raw.oom || !raw.ptr || raw.len>LIVE_FIXTURE_JSON_LIMIT) goto finish;
    mz_ulong dest_len=mz_compressBound((mz_ulong)raw.len);
    if (!dest_len || dest_len>LIVE_FIXTURE_JSON_LIMIT) goto finish;
    compressed=(unsigned char *)malloc((size_t)dest_len);
    if (!compressed ||
        mz_compress2(compressed,&dest_len,(const unsigned char *)raw.ptr,
                     (mz_ulong)raw.len,6)!=MZ_OK) goto finish;
    encoded=live_base64_encode(compressed,(size_t)dest_len);
    if (!encoded) goto finish;
    mjson_printf(emit,ctx,"],%Q:%lu,%Q:%Q}",
                 "fixture_json_bytes",(unsigned long)raw.len,
                 "fixtures_zlib",encoded);
    ok=1;
finish:
    free(encoded);free(compressed);free(raw.ptr);
    return ok;
}
int ExpandedWorldLive_ReadJson(const char *json,int len) {
    const char *obj=NULL,*arr=NULL;
    int olen=0,alen=0;
    int kind;
    long v=0,raw[4]={0};
    LiveSaveImage *tmp=NULL;
    ExpandedWorldFixture *store=NULL;
    char *owned_fixture_json=NULL;
    long snapshot_version=0;
    uint8_t *present=NULL;
    int off,ko,kl,vo,vl,type;
    unsigned n=0;
    if (!json || len<=0) return 0;
    kind=mjson_find(json,len,"$.living_world",&obj,&olen);
    if (kind==MJSON_TOK_NULL || kind==MJSON_TOK_INVALID) {
        /* Old schema-3 careers predate the living world snapshot. */
        ExpandedWorldLive_End();
        return 1;
    }
    if (kind!=MJSON_TOK_OBJECT || olen<=0) return 0;
    tmp=(LiveSaveImage *)calloc(1,sizeof *tmp);
    present=(uint8_t *)calloc(65536u,1u);
    if (!tmp || !present) goto invalid;
    if (mjson_get_long(obj,olen,"$.version",&snapshot_version)<=0 ||
        (snapshot_version!=1 && snapshot_version!=2 && snapshot_version!=3) ||
        mjson_get_long(obj,olen,"$.first_year",&v)<=0 ||
        v<1900 || v>2198) goto invalid;
    tmp->year=(uint16_t)v;
    tmp->window_year=tmp->year;
    if (snapshot_version==3) {
        if (mjson_get_long(obj,olen,"$.window_first_year",&v)<=0 ||
            v<tmp->year || v>2199L) goto invalid;
        tmp->window_year=(uint16_t)v;
        if (mjson_find(obj,olen,"$.history",&arr,&alen)!=MJSON_TOK_ARRAY)
            goto invalid;
        /* Linear scan of flat numeric rows: mjson_next on the whole
         * archive for every entry is quadratic over 70+ seasons. */
        if(alen<2 || arr[0]!='[' || arr[alen-1]!=']') goto invalid;
        const char *p=arr+1,*end=arr+alen-1;
        int first=1;
        while(p<end) {
            while(p<end && (*p==' ' || *p=='\n' ||
                           *p=='\r' || *p=='\t')) ++p;
            if(p==end) break;
            if(!first) {
                if(*p++!=',') goto invalid;
                while(p<end && (*p==' ' || *p=='\n' ||
                               *p=='\r' || *p=='\t')) ++p;
            }
            if(p>=end || *p!='[' ||
               tmp->history_count>=LIVE_HISTORY_LIMIT) goto invalid;
            const char *row_start=p++;
            while(p<end && *p!=']') {
                if(*p=='[' || *p=='{' || *p=='"') goto invalid;
                ++p;
            }
            if(p>=end) goto invalid;
            ++p;
            long row[7]={0};
            unsigned count=0;
            if(!live_parse_int_row(row_start,(int)(p-row_start),
                                   row,7u,&count) || count!=7u ||
               row[0]<tmp->year || row[0]>=tmp->window_year ||
               row[1]<0 || row[1]>255 || row[2]<0 || row[2]>=16 ||
               row[3]<0 || row[3]>=65535 ||
               row[4]<0 || row[4]>500 || row[5]<1 || row[5]>65535 ||
               row[6]<0 || row[6]>2147483647L) goto invalid;
            if (tmp->history_count>=tmp->history_capacity) {
                uint32_t cap=tmp->history_capacity?tmp->history_capacity*2u:64u;
                if (cap>LIVE_HISTORY_LIMIT) cap=LIVE_HISTORY_LIMIT;
                LiveArchivedLeague *grown=(LiveArchivedLeague *)realloc(
                                            tmp->history,sizeof(*grown)*cap);
                if(!grown) goto invalid;
                tmp->history=grown;tmp->history_capacity=cap;
            }
            LiveArchivedLeague *r=&tmp->history[tmp->history_count];
            r->year=(uint16_t)row[0];
            r->country=(uint8_t)row[1];r->division=(uint8_t)row[2];
            r->champion=(uint16_t)row[3];
            r->champion_points=(uint16_t)row[4];
            r->matches=(uint16_t)row[5];
            r->digest=(uint32_t)row[6];
            if(tmp->history_count &&
               r->year < tmp->history[tmp->history_count-1u].year)
                goto invalid;
            for(uint32_t i=0;i<tmp->history_count;++i)
                if(tmp->history[i].year==r->year &&
                   tmp->history[i].country==r->country &&
                   tmp->history[i].division==r->division) goto invalid;
            if(tmp->archived_matches>UINT32_MAX-r->matches) goto invalid;
            tmp->archived_matches+=r->matches;
            ++tmp->history_count;
            first=0;
        }
        if(tmp->window_year>tmp->year && !tmp->history_count)
            goto invalid;
    }
    /* A country's native season may begin before the European July 1
     * career clock. In the real 1996/97 world, a May-to-September league
     * has 45 valid May/June fixtures. Check against January 1 of the
     * FIRST calendar year, not the manager's July entry date. */
    int32_t earliest_fixture_day=0;
    {
        ExpandedWorldDate start={tmp->year,7u,1u};
        ExpandedWorldDate calendar_start={tmp->year,1u,1u};
        if (!ExpandedWorld_DateToIndex(start,&tmp->initial) ||
            !ExpandedWorld_DateToIndex(calendar_start,&earliest_fixture_day))
            goto invalid;
    }
    /* The current fixture generator knows two seasons, but the live
     * clock can keep advancing after that horizon. Never refuse a valid
     * later-career save merely because no further fixtures were generated. */
    if (mjson_get_long(obj,olen,"$.weeks",&v)<=0 || v<0 ||
        v>16000L) goto invalid;
    tmp->weeks=(uint32_t)v;
    {
        ExpandedWorldDate end_date={2200u,12u,31u};
        int32_t final_day=0;
        if (!ExpandedWorld_DateToIndex(end_date,&final_day) ||
            mjson_get_long(obj,olen,"$.now",&v)<=0 ||
            v<tmp->initial || v>final_day) goto invalid;
    }
    tmp->now=(int32_t)v;
    if ((uint32_t)(tmp->now-tmp->initial)<tmp->weeks*7u) goto invalid;
    if (mjson_get_long(obj,olen,"$.preseason_games",&v)<=0 || v<0)
        goto invalid;
    tmp->preseason_games=(uint32_t)v;
    if (mjson_find(obj,olen,"$.native",&arr,&alen)!=MJSON_TOK_ARRAY ||
        !live_parse_int_row(arr,alen,raw,4,&n) || n!=4)
        goto invalid;
    for (unsigned i=0;i<4;++i) if (raw[i]<0) goto invalid;
    tmp->native_matches=(uint32_t)raw[0];
    tmp->native_misses=(uint32_t)raw[1];
    tmp->native_orders=(uint32_t)raw[2];
    tmp->native_schedules=(uint32_t)raw[3];

    if (mjson_find(obj,olen,"$.clubs",&arr,&alen)!=MJSON_TOK_ARRAY)
        goto invalid;
    off=0;
    for (;;) {
        int next=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&type);
        if (next==0) break;
        if (next<0 || next<=off || type!=MJSON_TOK_ARRAY ||
            tmp->clubs_count>=LIVE_MAX_CLUBS ||
            !live_parse_club_row(arr+vo,vl,&tmp->clubs[tmp->clubs_count]))
            goto invalid;
        const LiveClub *c=&tmp->clubs[tmp->clubs_count++];
        if (present[c->id]) goto invalid;
        present[c->id]=1u;
        off=next;
    }
    if (!tmp->clubs_count) goto invalid;

    if (mjson_find(obj,olen,"$.leagues",&arr,&alen)!=MJSON_TOK_ARRAY)
        goto invalid;
    off=0;
    for (;;) {
        int next=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&type);
        if (next==0) break;
        if (next<0 || next<=off || type!=MJSON_TOK_ARRAY ||
            tmp->leagues_count>=LIVE_MAX_LEAGUES ||
            !live_parse_league_row(arr+vo,vl,&tmp->leagues[tmp->leagues_count]))
            goto invalid;
        const LiveLeague *l=&tmp->leagues[tmp->leagues_count++];
        for (unsigned i=0;i<l->count;++i) {
            const uint16_t id=l->clubs[i];
            /* A club belongs to exactly one division: never import
             * overlapping or incomplete league rosters from a save. */
            if (present[id]!=1u) goto invalid;
            present[id]=3u;
            int belongs=0;
            for (uint32_t k=0;k<tmp->clubs_count;++k)
                if (tmp->clubs[k].id==id &&
                    tmp->clubs[k].country==l->country &&
                    tmp->clubs[k].division==l->division) {
                    belongs=1; break;
                }
            if (!belongs) goto invalid;
        }
        off=next;
    }
    if (!tmp->leagues_count) goto invalid;
    for (uint32_t i=0;i<tmp->clubs_count;++i)
        if (present[tmp->clubs[i].id]!=3u) goto invalid;
    if (snapshot_version>=2) {
        long raw_len=0;
        const char *blob=NULL;
        int blob_len=0;
        if (mjson_get_long(obj,olen,"$.fixture_json_bytes",&raw_len)<=0 ||
            raw_len<2 || raw_len>(long)LIVE_FIXTURE_JSON_LIMIT ||
            mjson_find(obj,olen,"$.fixtures_zlib",&blob,&blob_len)!=MJSON_TOK_STRING)
            goto invalid;
        owned_fixture_json=live_inflate_fixture_json(blob,blob_len,
                                                     (uint32_t)raw_len);
        if (!owned_fixture_json) goto invalid;
        arr=owned_fixture_json;
        alen=(int)raw_len;
    } else if (mjson_find(obj,olen,"$.fixtures",&arr,&alen)!=MJSON_TOK_ARRAY)
        goto invalid;
    if (alen<2 || arr[0]!='[' || arr[alen-1]!=']')
        goto invalid;
    /* mjson_next re-lexes from the beginning on every call. That is fine
     * for <=216 clubs but quadratic for 8,000-40,000 fixture rows.
     * Find each short numeric row in ONE linear pass; the typed parser
     * still validates every number, row length and score. */
    {
        const char *p=arr+1,*end=arr+alen-1;
        int first=1;
        while (p<end) {
            while (p<end && (*p==' ' || *p=='\n' || *p=='\r' || *p=='\t')) ++p;
            if (p==end) break;
            if (!first) {
                if (*p++!=',') goto invalid;
                while (p<end && (*p==' ' || *p=='\n' || *p=='\r' || *p=='\t')) ++p;
            }
            if (p>=end || *p!='[' || tmp->fixture_count>=LIVE_MAX_FIXTURES)
                goto invalid;
            const char *row=p++;
            while (p<end && *p!=']') {
                if (*p=='[' || *p=='{' || *p=='"') goto invalid;
                ++p;
            }
            if (p==end) goto invalid;
            ++p; /* closing row bracket */
            const uint32_t i=tmp->fixture_count++;
            if (!live_parse_fixture_row(row,(int)(p-row),&tmp->fixtures[i],
                                        &tmp->consumed[i]) ||
                tmp->fixtures[i].day<earliest_fixture_day ||
                !present[tmp->fixtures[i].home] ||
                !present[tmp->fixtures[i].away] ||
                (i && fixture_compare(&tmp->fixtures[i-1],&tmp->fixtures[i])>=0))
                goto invalid;
            if (tmp->fixtures[i].played) ++tmp->played;
            first=0;
        }
    }
    if (!tmp->fixture_count || tmp->native_schedules>tmp->leagues_count ||
        tmp->native_matches>tmp->fixture_count+tmp->archived_matches) goto invalid;
    /* Allocate the replacement BEFORE destroying the live world. */
    store=(ExpandedWorldFixture *)malloc(sizeof *store*LIVE_MAX_FIXTURES);
    if (!store) goto invalid;
    memcpy(store,tmp->fixtures,sizeof(*store)*tmp->fixture_count);
    ExpandedWorldLive_End();
    memcpy(s_clubs,tmp->clubs,sizeof(LiveClub)*tmp->clubs_count);
    memcpy(s_leagues,tmp->leagues,sizeof(LiveLeague)*tmp->leagues_count);
    memcpy(s_native_result_consumed,tmp->consumed,tmp->fixture_count);
    s_club_count=tmp->clubs_count;
    s_league_count=tmp->leagues_count;
    s_weeks=tmp->weeks;
    s_first_year=tmp->year;
    s_window_first_year=tmp->window_year;
    s_history=tmp->history;
    s_history_count=tmp->history_count;
    s_history_capacity=tmp->history_capacity;
    s_archived_matches=tmp->archived_matches;
    tmp->history=NULL;
    s_initial_day=tmp->initial;
    s_preseason_games_total=tmp->preseason_games;
    s_native_matches=tmp->native_matches;
    s_native_misses=tmp->native_misses;
    s_native_order_applied=tmp->native_orders;
    s_native_schedule_leagues=tmp->native_schedules;
    s_fixtures=store;
    s_ledger.fixtures=store;
    s_ledger.capacity=LIVE_MAX_FIXTURES;
    s_ledger.count=tmp->fixture_count;
    s_ledger.played=tmp->played;
    s_ledger.now=tmp->now;
    s_live=1;
    fprintf(stderr,"[living-world] restored .CAR snapshot: %lu clubs, "
      "%lu leagues, %lu/%lu results, week %lu\n",
      (unsigned long)s_club_count,(unsigned long)s_league_count,
      (unsigned long)s_ledger.played,(unsigned long)s_ledger.count,
      (unsigned long)s_weeks);
    free(owned_fixture_json);
    free(present);
    free(tmp);
    return 1;
invalid:
    free(store);
    free(owned_fixture_json);
    free(present);
    if(tmp) free(tmp->history);
    free(tmp);
    fprintf(stderr,"[living-world] invalid or incomplete .CAR world snapshot\n");
    return 0;
}

#else
/* The living-world pilot is Windows/SDL only; the legacy Amiga build
 * keeps its existing career mode and does not require hosted stdlib. */
int ExpandedWorldLive_Begin(uint16_t y) { (void)y; return 0; }
int ExpandedWorldLive_AddClub(uint8_t c,uint8_t d,uint16_t id,uint16_t rating,
                              const char *name,uint8_t ppw) {
    (void)c;(void)d;(void)id;(void)rating;(void)name;(void)ppw;return 0;
}
int ExpandedWorldLive_Seal(void) {return 0;}
int ExpandedWorldLive_SetLeagueSchedule(uint8_t c,uint8_t d,uint8_t s,uint8_t e,
                                        uint8_t legs) {
    (void)c;(void)d;(void)s;(void)e;(void)legs;return 0;
}
int ExpandedWorldLive_NativeOrder(const uint16_t *ids,uint32_t n,uint8_t *order) {
    (void)ids;(void)n;(void)order;return 0;
}
uint32_t ExpandedWorldLive_NativeOrderApplied(void) {return 0;}
uint32_t ExpandedWorldLive_NativeScheduleLeagues(void) {return 0;}
int ExpandedWorldLive_AdvanceWeeks(uint32_t w) {(void)w;return 0;}
int ExpandedWorldLive_AdvanceToSeason(uint16_t y) {(void)y;return 0;}
int ExpandedWorldLive_Table(uint8_t c,uint8_t d,ExpandedLiveRow *r,
                            uint32_t cap,uint32_t *total) {
    (void)c;(void)d;(void)r;(void)cap;if(total)*total=0;return 0;
}
int ExpandedWorldLive_PeekNativeLeagueResult(uint16_t h,uint16_t a,
                                            uint8_t *gh,uint8_t *ga) {
    (void)h;(void)a;(void)gh;(void)ga;return 0;
}
int ExpandedWorldLive_TakeNativeLeagueResult(uint16_t h,uint16_t a,uint8_t phase,
                                             uint8_t *gh,uint8_t *ga) {
    (void)h;(void)a;(void)phase;(void)gh;(void)ga;return 0;
}
int ExpandedWorldLive_RecordNativeLeagueResult(uint16_t h,uint16_t a,uint8_t phase,
                                               uint8_t gh,uint8_t ga) {
    (void)h;(void)a;(void)phase;(void)gh;(void)ga;return 0;
}
uint32_t ExpandedWorldLive_NativeMatches(void) {return 0;}
uint32_t ExpandedWorldLive_NativeMisses(void) {return 0;}
uint32_t ExpandedWorldLive_Results(void) {return 0;}
uint32_t ExpandedWorldLive_ArchivedResults(void) {return 0;}
uint32_t ExpandedWorldLive_ResidentResults(void) {return 0;}
uint32_t ExpandedWorldLive_ArchivedLeagues(void) {return 0;}
uint16_t ExpandedWorldLive_WindowFirstYear(void) {return 0;}
int ExpandedWorldLive_HistoryChampion(uint16_t y,uint8_t c,uint8_t d,uint16_t *o) {
    (void)y;(void)c;(void)d;(void)o;return 0;
}
uint32_t ExpandedWorldLive_Fixtures(void) {return 0;}
uint32_t ExpandedWorldLive_PlayerSlots(void) {return 0;}
uint32_t ExpandedWorldLive_MemoryBytes(void) {return 0;}
uint32_t ExpandedWorldLive_Clubs(void) {return 0;}
uint32_t ExpandedWorldLive_Leagues(void) {return 0;}
uint16_t ExpandedWorldLive_SeasonYear(void) {return 0;}
uint32_t ExpandedWorldLive_Weeks(void) {return 0;}
uint32_t ExpandedWorldLive_PreseasonGamesTotal(void) {return 0;}
int ExpandedWorldLive_Preseason(uint16_t club,ExpandedLivePreseason *out) {
    (void)club;(void)out;return 0;
}
void ExpandedWorldLive_End(void) {}
#endif
