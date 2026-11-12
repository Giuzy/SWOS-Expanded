#include "expanded_world_live.h"
#ifdef SWOS_SDL_BUILD
#include "expanded_world_time.h"
#include "expanded_world_ledger.h"
#include "mjson.h"      /* typed, portable .CAR world snapshot */
#include "miniz_swos.h" /* existing bundled zlib for compact fixture snapshots */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LIVE_MAX_CLUBS 4096u
#define LIVE_MAX_LEAGUES 512u
#define LIVE_MAX_FIXTURES 250000u
#define LIVE_MAX_LEAGUE_CLUBS 26u
#define NATIVE_FIXTURE_MAX_CLUBS 24u
/* Canonical OpenSWOS pairing tables: also used by DIY league fixtures.
 * The odd-sized tables are installed by FixtureRounds_Install at startup. */
extern const uint8_t *_fixtureSchedulePtrTable[];
/* Two seasons resident, compact completed seasons stored separately. */
#define LIVE_SEASONS 2u
/* Original SWOS resolves cup brackets; one world only archives their outcomes. */
#define LIVE_MAX_CUP_EVENTS 32768u
#define LIVE_MAX_CUP_TITLES 65536u
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
typedef struct LiveCupEvent {
    int32_t day;
    uint32_t competition_hash;
    uint16_t home,away;
    uint8_t kind,slot,round,match,leg,goals_home,goals_away,flags;
} LiveCupEvent;
static LiveCupEvent *s_cup_events;
static uint32_t s_cup_count,s_cup_capacity;
static uint32_t s_cup_archived_matches,s_cup_archived_titles;
typedef struct LiveCupTitle {
    uint16_t season,winner;
    uint32_t competition_hash;
} LiveCupTitle;
static LiveCupTitle *s_cup_titles;
static uint32_t s_cup_titles_count,s_cup_titles_capacity;

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
    uint8_t auto_up,auto_down,playoff_up,playoff_down;
    uint16_t clubs[LIVE_MAX_LEAGUE_CLUBS];
} LiveLeague;
static LiveClub s_clubs[LIVE_MAX_CLUBS];
static LiveLeague s_leagues[LIVE_MAX_LEAGUES];
static uint32_t s_club_count, s_league_count, s_weeks;
/* The two-year window may hold the CURRENT season while the next
 * season waits for its real promotion/relegation results. */
static uint8_t s_generated_fresh[LIVE_MAX_LEAGUES];
static uint32_t s_promotions,s_relegations,s_mobility_unsupported;
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
/* Recover native May/June fixtures that precede the July 1 start of the
 * manager's FIRST year. They must be scored before the oldest season can
 * be archived, without moving the global clock backwards. */
static int live_complete_prestart(void);

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
    free(s_cup_events);s_cup_events=NULL;
    s_cup_count=s_cup_capacity=0;
    s_cup_archived_matches=s_cup_archived_titles=0;
    free(s_cup_titles);s_cup_titles=NULL;
    s_cup_titles_count=s_cup_titles_capacity=0;
    free(s_history);s_history=NULL;s_history_count=0;
    s_history_capacity=0;s_archived_matches=0;
    s_native_matches=s_native_misses=0;
    s_native_order_applied=s_native_schedule_leagues=0;
    s_promotions=s_relegations=s_mobility_unsupported=0;
    memset(s_generated_fresh,0,sizeof s_generated_fresh);
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
int ExpandedWorldLive_SetLeagueMovement(uint8_t country,uint8_t division,
                                        uint8_t up,uint8_t down,
                                        uint8_t playoff_up,uint8_t playoff_down) {
    LiveLeague *l=league_by_id(country,division);
    if (!l || s_live || up>l->count || down>l->count ||
        playoff_up>l->count || playoff_down>l->count) return 0;
    l->auto_up=up;l->auto_down=down;
    l->playoff_up=playoff_up;l->playoff_down=playoff_down;
    return 1;
}
uint32_t ExpandedWorldLive_Promotions(void) {return s_promotions;}
uint32_t ExpandedWorldLive_Relegations(void) {return s_relegations;}
uint32_t ExpandedWorldLive_MobilityUnsupported(void) {return s_mobility_unsupported;}
uint8_t ExpandedWorldLive_ClubDivision(uint16_t id) {
    const LiveClub *c=club_by_id(id);
    return c?c->division:255u;
}
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

/* A future season's clubs can differ from the old native table after
 * promotion/relegation. Read each season's membership from its own fixture
 * ledger, not from the mutable most-recent league roster. */
static int live_fixture_roster(const LiveLeague *league,unsigned year,
                               uint16_t out[LIVE_MAX_LEAGUE_CLUBS]) {
    if (!league || !s_live || !s_fixtures || year<s_first_year)
        return 0;
    const unsigned n=league->count;
    if(n<2u || n>NATIVE_FIXTURE_MAX_CLUBS) return 0;
    const uint8_t *pairs=_fixtureSchedulePtrTable[n-2u];
    if(!pairs) return 0;
    const uint16_t cid=competition_id(league->country,league->division,
                                     (year-s_first_year)&1u);
    const unsigned one_leg=n*(n-1u)/2u;
    uint8_t seen[LIVE_MAX_LEAGUE_CLUBS]={0};
    unsigned found=0;
    /* Recover ORIGINAL pairing-table order from fixture ids rather than
     * chronological first appearance. This is needed by NativeOrder after
     * promoted clubs change a division's participants. */
    for(uint32_t i=0;i<s_ledger.count;++i) {
        const ExpandedWorldFixture *f=&s_fixtures[i];
        if(f->competition!=cid) continue;
        const unsigned leg=(unsigned)f->id/one_leg;
        const unsigned fixture_index=(unsigned)f->id%one_leg;
        if(leg>=league->legs) return 0;
        const unsigned a=pairs[2u*fixture_index];
        const unsigned b=pairs[2u*fixture_index+1u];
        if(a>=n || b>=n || a==b) return 0;
        const int swap=(league->legs-leg)&1u;
        const uint16_t ida=swap?f->away:f->home;
        const uint16_t idb=swap?f->home:f->away;
        if(seen[a] && out[a]!=ida) return 0;
        if(seen[b] && out[b]!=idb) return 0;
        if(!seen[a]) {out[a]=ida;seen[a]=1;found++;}
        if(!seen[b]) {out[b]=idb;seen[b]=1;found++;}
    }
    return found==n;
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
        uint16_t season_clubs[LIVE_MAX_LEAGUE_CLUBS];
        if (!league->native_schedule || league->count!=count ||
            !live_fixture_roster(league,ExpandedWorldLive_SeasonYear(),
                                 season_clubs)) continue;
        for (uint32_t i=0;i<count;++i) {
            uint32_t k=0;
            while (k<count && ids[k]!=season_clubs[i]) ++k;
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
        for(uint32_t i=0;i<s_league_count;++i) {
            int mobile=0;
            if (season==1u)
                for(uint32_t j=0;j<s_league_count;++j)
                    if(s_leagues[j].country==s_leagues[i].country &&
                       (s_leagues[j].auto_up || s_leagues[j].auto_down ||
                        s_leagues[j].playoff_up || s_leagues[j].playoff_down)) {
                        mobile=1;break;
                    }
            if (!mobile)
                (void)make_fixtures(&s_leagues[i],season,day);
            if(season==1u) s_generated_fresh[i]=(uint8_t)!mobile;
        }
    }
    if (!s_ledger.count) return 0;
    qsort(s_fixtures,s_ledger.count,sizeof s_fixtures[0],fixture_compare);
    s_live=1;
    if (!live_complete_prestart()) {
        ExpandedWorldLive_End();
        return 0;
    }
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
        uint16_t season_roster[LIVE_MAX_LEAGUE_CLUBS]={0};
        if(!live_fixture_roster(league,retiring,season_roster)) return 0;
        for(unsigned i=0;i<league->count;++i)
            table[i].club=season_roster[i];
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
    /* Only static single-division/no-mobility leagues may be regenerated
     * immediately at January's archive boundary. Countries with native
     * promotion rules wait for the previous season's FINAL fixture: an
     * August-May competition is still playing when January arrives. */
    for(uint32_t l=0;l<s_league_count;++l) {
        s_generated_fresh[l]=0;
        if (!s_leagues[l].native_schedule) continue;
        int mobile=0;
        for(uint32_t k=0;k<s_league_count;++k)
            if(s_leagues[k].country==s_leagues[l].country &&
               (s_leagues[k].auto_up || s_leagues[k].auto_down ||
                s_leagues[k].playoff_up || s_leagues[k].playoff_down)) {
                mobile=1;break;
            }
        if (!mobile) {
            if (!make_fixtures(&s_leagues[l],fresh_season,0)) {
                generated=0;break;
            }
            s_generated_fresh[l]=1;
        }
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
    /* Archive OG cup outcomes ONLY after the league window commits. */
    {
        ExpandedWorldDate boundary={(uint16_t)(retiring+2u),1u,1u};
        int32_t limit=0;
        if(ExpandedWorld_DateToIndex(boundary,&limit)) {
            uint32_t kept_cups=0;
            for(uint32_t i=0;i<s_cup_count;++i) {
                const LiveCupEvent *e=&s_cup_events[i];
                if(e->day<limit) {
                    if(e->kind==1u) ++s_cup_archived_matches;
                    else ++s_cup_archived_titles;
                } else s_cup_events[kept_cups++]=*e;
            }
            s_cup_count=kept_cups;
        }
    }
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
/* League mobility uses the EXACT automatic places configured by the
 * installed OpenSWOS CompetitionLeague descriptors. Knockout promotion/
 * relegation playoff winners are NOT derived from the league table. Those
 * pairings remain with the OG knockout engine until it can share state here. */
typedef struct LiveMobilityRank {
    uint32_t index;
    uint8_t count;
    uint16_t ordered[LIVE_MAX_LEAGUE_CLUBS];
} LiveMobilityRank;
static int live_standing_better(const ExpandedWorldStanding *a,
                                const ExpandedWorldStanding *b) {
    int agd=(int)a->scored-(int)a->conceded;
    int bgd=(int)b->scored-(int)b->conceded;
    if(a->points!=b->points) return a->points>b->points;
    if(agd!=bgd) return agd>bgd;
    if(a->scored!=b->scored) return a->scored>b->scored;
    return a->club<b->club;
}
static int live_replace_member(LiveLeague *l,uint16_t from,uint16_t to) {
    for(unsigned i=0;i<l->count;++i)
        if(l->clubs[i]==from) { l->clubs[i]=to;return 1; }
    return 0;
}
static int live_country_ready_day(uint8_t country,int32_t *out) {
    int32_t last=INT32_MIN;
    int found=0;
    uint16_t previous=s_window_first_year;
    for(uint32_t l=0;l<s_league_count;++l) {
        const LiveLeague *league=&s_leagues[l];
        if(league->country!=country || !league->native_schedule ||
           s_generated_fresh[l]) continue;
        const uint16_t cid=competition_id(country,league->division,
                                          (previous-s_first_year)&1u);
        int has_fixture=0;
        for(uint32_t f=0;f<s_ledger.count;++f)
            if(s_fixtures[f].competition==cid) {
                has_fixture=1;
                if(s_fixtures[f].day>last) last=s_fixtures[f].day;
            }
        if(!has_fixture) return 0;
        found=1;
    }
    if(!found || last==INT32_MAX) return 0;
    *out=last+1;
    return 1;
}
static int live_generate_country(uint8_t country) {
    const uint16_t previous=s_window_first_year;
    const unsigned fresh=(unsigned)(previous+1u-s_first_year);
    LiveMobilityRank ranks[16]={0};
    LiveLeague backup[16];
    unsigned n=0;
    uint32_t before=s_ledger.count;
    uint32_t old_up=s_promotions,old_down=s_relegations;
    uint32_t old_skipped=s_mobility_unsupported;
    /* Native countries with >16 distinct divisions are not modeled. */
    for(uint32_t l=0;l<s_league_count;++l) {
        LiveLeague *league=&s_leagues[l];
        if(league->country!=country || !league->native_schedule) continue;
        if(n==16u || s_generated_fresh[l]) return 0;
        ranks[n].index=l;
        ranks[n].count=league->count;
        backup[n]=*league;
        uint16_t roster[LIVE_MAX_LEAGUE_CLUBS]={0};
        ExpandedWorldStanding table[LIVE_MAX_LEAGUE_CLUBS];
        if(!live_fixture_roster(league,previous,roster)) return 0;
        for(unsigned i=0;i<league->count;++i) table[i].club=roster[i];
        if(!ExpandedWorldLedger_Standings(&s_ledger,
             competition_id(country,league->division,
                            (previous-s_first_year)&1u),
             table,league->count,league->points)) return 0;
        for(unsigned i=1;i<league->count;++i) {
            ExpandedWorldStanding temp=table[i];unsigned j=i;
            while(j && live_standing_better(&temp,&table[j-1u])) {
                table[j]=table[j-1u];--j;
            }
            table[j]=temp;
        }
        for(unsigned i=0;i<league->count;++i)
            ranks[n].ordered[i]=table[i].club;
        ++n;
    }
    if(!n) return 0;
    /* Work from the standings of the PREVIOUS season: ranking arrays are
     * immutable even when an adjacent division has already swapped clubs. */
    for(unsigned i=0;i<n;++i) {
        LiveLeague *upper=&s_leagues[ranks[i].index];
        for(unsigned j=0;j<n;++j) {
            LiveLeague *lower=&s_leagues[ranks[j].index];
            if(lower->division!=(unsigned)upper->division+1u) continue;
            unsigned moves=upper->auto_down;
            if(!moves && !lower->auto_up) continue;
            if(moves!=lower->auto_up ||
               moves>ranks[i].count || moves>ranks[j].count) {
                ++s_mobility_unsupported;
                fprintf(stderr,"[living-world] unequal automatic mobility "
                  "requires native resolution country=%u divisions=%u/%u\n",
                  (unsigned)country,(unsigned)upper->division,
                  (unsigned)lower->division);
                continue;
            }
            /* Automatic league places remain valid even when a *separate*
             * OG promotion playoff also exists. The latter is not invented
             * here; F12 distinguishes automatic switches from pending cups. */
            if(upper->playoff_down || lower->playoff_up)
                ++s_mobility_unsupported;
            for(unsigned k=0;k<moves;++k) {
                uint16_t down=ranks[i].ordered[ranks[i].count-1u-k];
                uint16_t up=ranks[j].ordered[k];
                if(!live_replace_member(upper,down,up) ||
                   !live_replace_member(lower,up,down)) goto rollback;
                ++s_promotions;++s_relegations;
            }
        }
    }
    for(unsigned i=0;i<n;++i) {
        LiveLeague *league=&s_leagues[ranks[i].index];
        if(!make_fixtures(league,fresh,0)) goto rollback;
    }
    /* Attach native replay-consumption flags to their individual fixtures
     * while merging the delayed season into the single sorted event cursor. */
    {
        LivePackedFixture *packed=(LivePackedFixture *)malloc(
                                          s_ledger.count*sizeof *packed);
        if(!packed) goto rollback;
        for(uint32_t i=0;i<s_ledger.count;++i) {
            packed[i].fixture=s_fixtures[i];
            packed[i].consumed=i<before?s_native_result_consumed[i]:0u;
        }
        qsort(packed,s_ledger.count,sizeof *packed,live_packed_compare);
        for(uint32_t i=1;i<s_ledger.count;++i)
            if(fixture_compare(&packed[i-1].fixture,&packed[i].fixture)>=0) {
                free(packed);
                goto rollback;
            }
        for(uint32_t i=0;i<s_ledger.count;++i) {
            s_fixtures[i]=packed[i].fixture;
            s_native_result_consumed[i]=packed[i].consumed;
        }
        free(packed);
    }
    for(unsigned i=0;i<n;++i) {
        const LiveLeague *league=&s_leagues[ranks[i].index];
        s_generated_fresh[ranks[i].index]=1;
        for(unsigned j=0;j<league->count;++j)
            for(uint32_t c=0;c<s_club_count;++c)
                if(s_clubs[c].id==league->clubs[j])
                    s_clubs[c].division=league->division;
    }
    fprintf(stderr,"[living-world] season %u country %u: mobility "
                   "up=%lu down=%lu unresolved-native=%lu\n",
            (unsigned)(previous+1u),(unsigned)country,
            (unsigned long)(s_promotions-old_up),
            (unsigned long)(s_relegations-old_down),
            (unsigned long)(s_mobility_unsupported-old_skipped));
    return 1;
rollback:
    for(unsigned i=0;i<n;++i) s_leagues[ranks[i].index]=backup[i];
    s_ledger.count=before;
    s_promotions=old_up;s_relegations=old_down;
    s_mobility_unsupported=old_skipped;
    fprintf(stderr,"[living-world] mobility generation rolled back country %u\n",
            (unsigned)country);
    return 0;
}
static int live_advance_with_mobility(int32_t target) {
    while (1) {
        int found=0;
        int32_t earliest=INT32_MAX;
        uint8_t chosen=0;
        uint8_t seen[256]={0};
        for(uint32_t l=0;l<s_league_count;++l) {
            const LiveLeague *league=&s_leagues[l];
            if(!league->native_schedule || s_generated_fresh[l] ||
               seen[league->country]) continue;
            seen[league->country]=1;
            int32_t ready=0;
            if(!live_country_ready_day(league->country,&ready)) return 0;
            if(ready<=target && ready<earliest) {
                chosen=league->country;earliest=ready;found=1;
            }
        }
        if(!found) break;
        if(s_ledger.now<earliest) {
            int32_t old=s_ledger.now;
            if(!ExpandedWorldLedger_Advance(&s_ledger,earliest,
                                             resolve_game,NULL)) return 0;
            preseason_advance(old,earliest);
        }
        if(!live_generate_country(chosen)) return 0;
    }
    int32_t old=s_ledger.now;
    if(!ExpandedWorldLedger_Advance(&s_ledger,target,resolve_game,NULL))
        return 0;
    preseason_advance(old,target);
    return 1;
}
/* Original SWOS fixture tables remain the only league pairing generator.
 * League movement completes before creating the next season's entrants. */

/* The manager's first day is July 1, but the real OpenSWOS calendar
 * includes competitions already underway in May and June. Those past
 * fixtures are real historical results, not unplayed future matches.
 * This backfill is deterministic and runs only for fixtures before the
 * initial manager date; it never rewinds the 50 Hz match or world clock.
 * It also repairs legacy v2 snapshots written before this check existed. */
static int live_complete_prestart(void) {
    uint32_t completed=0;
    if (!s_live || !s_fixtures) return 0;
    for (uint32_t i=0;i<s_ledger.count;++i) {
        ExpandedWorldFixture *f=&s_fixtures[i];
        if (f->played || f->day>=s_initial_day) continue;
        uint8_t home=0,away=0;
        if (!resolve_game(f,&home,&away,NULL) ||
            home>31u || away>31u) return 0;
        f->goals_home=home;f->goals_away=away;f->played=1;
        ++s_ledger.played;
        ++completed;
    }
    if(completed)
        fprintf(stderr,"[living-world] recovered %lu pre-career May/June "
                       "fixtures before 1 July %u\n",
                (unsigned long)completed,(unsigned)s_first_year);
    return 1;
}
/* All world advances now pass through ONE exact civil-day cursor.
 * OpenSWOS weekly menus call AdvanceWeeks; native season transitions call
 * AdvanceToSeason; future in-week events can use AdvanceToDate directly.
 * The stored .CAR schema is unchanged (day-index + elapsed full weeks). */
int ExpandedWorldLive_AdvanceToDate(ExpandedWorldDate date) {
    int32_t target=0;
    if (!s_live || !ExpandedWorld_DateToIndex(date,&target) ||
        target<s_initial_day || target<s_ledger.now) return 0;
    while ((unsigned)s_window_first_year+2u<=2199u) {
        const uint16_t newest=(uint16_t)(s_window_first_year+2u);
        ExpandedWorldDate boundary={newest,1u,1u};
        int32_t boundary_day=0;
        if (!ExpandedWorld_DateToIndex(boundary,&boundary_day)) return 0;
        if (target<boundary_day) break;
        if (s_ledger.now<boundary_day &&
            !live_advance_with_mobility(boundary_day-1)) return 0;
        if (!live_roll_window()) return 0;
    }
    if (!live_advance_with_mobility(target)) return 0;
    /* Full weeks elapsed, never a rounded-up fictional event date. */
    s_weeks=(uint32_t)(target-s_initial_day)/7u;
    return 1;
}
int ExpandedWorldLive_AdvanceWeeks(uint32_t weeks) {
    uint64_t d=(uint64_t)(uint32_t)s_initial_day+(uint64_t)weeks*7u;
    ExpandedWorldDate date;
    if (!s_live || d>INT32_MAX || weeks<s_weeks) return 0;
    /* A mid-week event may have advanced the clock past this week's
     * weekly boundary. Repeating that same week must not rewind time. */
    if ((int32_t)d<s_ledger.now) return weeks==s_weeks;
    return ExpandedWorld_IndexToDate((int32_t)d,&date) &&
           ExpandedWorldLive_AdvanceToDate(date);
}
/* Native season boundary must be July 1, not rounded forward to an
 * arbitrary weekly tick (up to six days of premature results). */
int ExpandedWorldLive_AdvanceToSeason(uint16_t season_year) {
    if (!s_live || season_year<s_first_year || season_year>2199u) return 0;
    ExpandedWorldDate date={season_year,7u,1u};
    int32_t target=0;
    if (!ExpandedWorld_DateToIndex(date,&target) || target<s_initial_day) return 0;
    if (target<s_ledger.now) return 1; /* already at or past OG boundary */
    return ExpandedWorldLive_AdvanceToDate(date);
}
/* Read a played score from the canonical world ledger without modifying
 * it or consuming a native replay token. Called by the native league state
 * importer, which applies the result to the original SWOS fixture progress
 * and standings rather than opening 200+ historical league menus.
 *
 * Exact oriented pair, current season, played before the world clock. */
/* Native and Living World use one scored fixture per oriented pair/season.
 * The original calendar may assign a different date within that season.
 * Resolve by immutable club IDs, never by table row or rank. Special formats
 * with several identical oriented ties are deliberately rejected. */
static int native_fixture_index(uint16_t home,uint16_t away,
                                uint16_t season,uint32_t *index) {
    ExpandedWorldDate a={season,8u,1u},b={(uint16_t)(season+1u),8u,1u};
    int32_t start=0,end=0;
    unsigned matches=0;
    if(!index || !s_live || !s_fixtures ||
       !ExpandedWorld_DateToIndex(a,&start) ||
       !ExpandedWorld_DateToIndex(b,&end)) return 0;
    for(uint32_t i=0;i<s_ledger.count;++i) {
        const ExpandedWorldFixture *f=&s_fixtures[i];
        if(f->home!=home || f->away!=away || f->day<start || f->day>=end)
            continue;
        *index=i;
        ++matches;
    }
    return matches==1u;
}
static int native_current_season(uint8_t phase,uint16_t *year,int32_t *day) {
    ExpandedWorldDate date;
    const uint16_t base=ExpandedWorldLive_SeasonYear();
    if(!year || !day || phase==255u ||
       !ExpandedWorld_NativePhaseToDate(base,phase,&date) ||
       !ExpandedWorld_DateToIndex(date,day)) return 0;
    *year=(uint16_t)(date.year-(date.month<8u?1u:0u));
    return *year>=s_window_first_year &&
           (uint32_t)(*year-s_window_first_year)<LIVE_SEASONS;
}
int ExpandedWorldLive_PeekNativeLeagueResult(uint16_t home,uint16_t away,
                                             uint8_t *gh,uint8_t *ga) {
    uint32_t index=0;
    if(!gh || !ga || home==away ||
       !native_fixture_index(home,away,ExpandedWorldLive_SeasonYear(),
                             &index)) return 0;
    const ExpandedWorldFixture *f=&s_fixtures[index];
    if(!f->played || f->day>s_ledger.now || f->goals_home>31u ||
       f->goals_away>31u) return 0;
    *gh=f->goals_home;*ga=f->goals_away;
    return 1;
}
/* Pre-hire native replay MUST NOT roll a second result. A fixture missing
 * only because its OG calendar date precedes our computed fixture date
 * is scored once by the world resolver, then adopted into the OG state.
 * This is permitted only when the native event belongs to the past. */
/* Re-date a native match without losing result-consumption bits. Commit
 * only after allocation and ordering succeed, so an OOM leaves the world
 * unchanged. The native calendar date is authoritative for the played tie. */
static int native_retime(uint32_t index,int32_t day) {
    if(index>=s_ledger.count) return 0;
    if(s_fixtures[index].day==day) return 1;
    LivePackedFixture *p=(LivePackedFixture *)malloc(
                                s_ledger.count*sizeof(*p));
    if(!p) return 0;
    for(uint32_t i=0;i<s_ledger.count;++i) {
        p[i].fixture=s_fixtures[i];
        p[i].consumed=s_native_result_consumed[i];
    }
    p[index].fixture.day=day;
    qsort(p,s_ledger.count,sizeof(*p),live_packed_compare);
    for(uint32_t i=1;i<s_ledger.count;++i)
        if(fixture_compare(&p[i-1].fixture,&p[i].fixture)>=0) {
            free(p);return 0;
        }
    for(uint32_t i=0;i<s_ledger.count;++i) {
        s_fixtures[i]=p[i].fixture;
        s_native_result_consumed[i]=p[i].consumed;
    }
    free(p);
    return 1;
}
int ExpandedWorldLive_TakeNativeLeagueResult(uint16_t home,uint16_t away,
                                             uint8_t native_phase,
                                             uint8_t *gh,uint8_t *ga) {
    uint16_t season=0;
    int32_t native_day=0;
    uint32_t index=0;
    if(!gh || !ga || home==away ||
       !native_current_season(native_phase,&season,&native_day) ||
       native_day>s_ledger.now ||
       !native_fixture_index(home,away,season,&index)) {
        ++s_native_misses;
        return 0;
    }
    ExpandedWorldFixture *f=&s_fixtures[index];
    if(s_native_result_consumed[index]) {
        ++s_native_misses;
        fprintf(stderr,"[living-world] duplicate native league consumption "
                "%04x-%04x season=%u\n",home,away,(unsigned)season);
        return 0;
    }
    if(!f->played) {
        uint8_t h=0,a=0;
        if(!resolve_game(f,&h,&a,NULL)) {
            ++s_native_misses;return 0;
        }
        /* Allocate/sort before changing the played count. */
        if(f->day!=native_day) {
            if(!native_retime(index,native_day) ||
               !native_fixture_index(home,away,season,&index)) {
                ++s_native_misses;return 0;
            }
            f=&s_fixtures[index];
        }
        f->goals_home=h;f->goals_away=a;f->played=1;
        ++s_ledger.played;
    }
    if(f->goals_home>31u || f->goals_away>31u) return 0;
    *gh=f->goals_home;*ga=f->goals_away;
    s_native_result_consumed[index]=1;
    ++s_native_matches;
    return 1;
}
/* After the managed side plays, the OG result is authoritative. Match
 * by immutable IDs and the season, not by two independently interpolated
 * round dates. Never overwrite a different season or ambiguous pair. */
int ExpandedWorldLive_RecordNativeLeagueResult(uint16_t home,uint16_t away,
                                               uint8_t phase,
                                               uint8_t gh,uint8_t ga) {
    uint16_t season=0;
    int32_t native_day=0;
    uint32_t index=0;
    if(home==away || gh>31u || ga>31u ||
       !native_current_season(phase,&season,&native_day) ||
       !native_fixture_index(home,away,season,&index)) {
        ++s_native_misses;
        fprintf(stderr,"[living-world] cannot reconcile OG score "
                "%04x-%04x phase=%u\n",home,away,(unsigned)phase);
        return 0;
    }
    ExpandedWorldFixture *f=&s_fixtures[index];
    if(f->day!=native_day) {
        if(!native_retime(index,native_day) ||
           !native_fixture_index(home,away,season,&index)) {
            ++s_native_misses;return 0;
        }
        f=&s_fixtures[index];
    }
    if(f->played && (f->goals_home!=gh || f->goals_away!=ga))
        fprintf(stderr,"[living-world] OG final replaced world score "
                "%04x-%04x %u:%u -> %u:%u\n",home,away,
                (unsigned)f->goals_home,(unsigned)f->goals_away,
                (unsigned)gh,(unsigned)ga);
    if(!f->played) {
        f->played=1;
        ++s_ledger.played;
    }
    f->goals_home=gh;f->goals_away=ga;
    return 1;
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
/* Read the SAME sorted resident event ledger used by the match resolver.
 * This does not schedule a second set of games or mutate a competition. */
int ExpandedWorldLive_CalendarState(ExpandedLiveCalendarState *out) {
    uint32_t played=0;
    int32_t next_day=0;
    if (!out || !s_live || !s_fixtures) return 0;
    memset(out,0,sizeof *out);
    if (!ExpandedWorld_IndexToDate(s_ledger.now,&out->today)) return 0;
    for (uint32_t i=0;i<s_ledger.count;++i) {
        const ExpandedWorldFixture *f=&s_fixtures[i];
        if (f->played) { ++played; continue; }
        ++out->pending;
        if (f->day<s_ledger.now) ++out->overdue;
        if (!out->has_next) {
            next_day=f->day;
            out->next_home=f->home;
            out->next_away=f->away;
            out->has_next=1u;
        }
        if (f->day==next_day) ++out->next_matches;
    }
    if (played!=s_ledger.played ||
        played+out->pending!=s_ledger.count) return 0;
    if (out->has_next &&
        !ExpandedWorld_IndexToDate(next_day,&out->next_date)) return 0;
    return 1;
}
/* Offer source files come only from validated, scheduled native leagues.
 * A country's second TEAM file remains selectable by its original ID. */
uint32_t ExpandedWorldLive_OfferFiles(uint8_t *files,uint32_t capacity) {
    uint8_t eligible[256]={0};
    uint32_t n=0;
    if(!s_live || !files || !capacity) return 0;
    for(uint32_t l=0;l<s_league_count;++l) {
        const LiveLeague *league=&s_leagues[l];
        if(!league->native_schedule || league->overflow || league->count<2u)
            continue;
        for(uint32_t i=0;i<league->count;++i)
            eligible[league->clubs[i]>>8]=1u;
    }
    for(uint32_t i=0;i<256u && n<capacity;++i)
        if(eligible[i]) files[n++]=(uint8_t)i;
    return n;
}
/* A compact event journal derived from the ORIGINAL cup engine, not a
 * duplicate simulator. Date is the native month-eighth used by OG. */
static uint32_t cup_name_hash(const char *name,uint8_t slot) {
    uint32_t h=2166136261u;
    if(!name) return 0;
    for(unsigned i=0;i<36u && name[i];++i)
        h=(h^(uint8_t)name[i])*16777619u;
    return ((h^(uint32_t)slot)*16777619u)&0x7fffffffu;
}
static int cup_append(const LiveCupEvent *e) {
    if(!s_live || !e) return 0;
    for(uint32_t i=0;i<s_cup_count;++i) {
        LiveCupEvent *prior=&s_cup_events[i];
        if(prior->day==e->day && prior->competition_hash==e->competition_hash &&
           prior->kind==e->kind && prior->slot==e->slot &&
           prior->round==e->round && prior->match==e->match &&
           prior->leg==e->leg && prior->home==e->home &&
           prior->away==e->away) {
            if(prior->goals_home!=e->goals_home ||
               prior->goals_away!=e->goals_away ||
               prior->flags!=e->flags) {
                fprintf(stderr,"[living-world] CONFLICTING OG CUP RESULT "
                               "hash=%lu round=%u match=%u\n",
                        (unsigned long)e->competition_hash,
                        (unsigned)e->round,(unsigned)e->match);
                return 0;
            }
            return 1;
        }
    }
    if(s_cup_count==LIVE_MAX_CUP_EVENTS) {
        fprintf(stderr,"[living-world] OG cup event capacity reached\n");
        return 0;
    }
    if(s_cup_count==s_cup_capacity) {
        uint32_t cap=s_cup_capacity?s_cup_capacity*2u:128u;
        if(cap>LIVE_MAX_CUP_EVENTS) cap=LIVE_MAX_CUP_EVENTS;
        LiveCupEvent *grown=(LiveCupEvent *)realloc(s_cup_events,
                                                   cap*sizeof(*grown));
        if(!grown) return 0;
        s_cup_events=grown;s_cup_capacity=cap;
    }
    uint32_t at=s_cup_count++;
    while(at && s_cup_events[at-1u].day>e->day) {
        s_cup_events[at]=s_cup_events[at-1u];--at;
    }
    s_cup_events[at]=*e;
    return 1;
}
static int cup_record(uint16_t season,const char *name,uint8_t slot,
                      uint8_t phase,uint8_t kind,uint8_t round,uint8_t match,
                      uint8_t leg,uint16_t home,uint16_t away,
                      uint8_t gh,uint8_t ga,uint8_t flags) {
    ExpandedWorldDate date;
    int32_t day=0;
    if(!s_live || !name || !name[0] ||
       season<s_first_year || season>2199u || phase==255u ||
       kind<1u || kind>2u || home==65535u ||
       (kind==1u && (away==65535u || home==away)) ||
       (kind==2u && away!=65535u) || gh>31u || ga>31u ||
       !ExpandedWorld_NativePhaseToDate(season,phase,&date) ||
       !ExpandedWorld_DateToIndex(date,&day)) return 0;
    LiveCupEvent e={0};
    e.day=day;e.competition_hash=cup_name_hash(name,slot);
    e.home=home;e.away=away;e.kind=kind;e.slot=slot;
    e.round=round;e.match=match;e.leg=leg;
    e.goals_home=gh;e.goals_away=ga;e.flags=flags;
    return cup_append(&e);
}
int ExpandedWorldLive_RecordNativeCupMatch(uint16_t season,const char *name,
                          uint8_t slot,uint8_t phase,uint8_t round,
                          uint8_t match,uint8_t leg,uint16_t home,
                          uint16_t away,uint8_t gh,uint8_t ga,uint8_t flags) {
    return cup_record(season,name,slot,phase,1u,round,match,leg,
                      home,away,gh,ga,flags);
}
int ExpandedWorldLive_RecordNativeCupChampion(uint16_t season,const char *name,
                           uint8_t slot,uint8_t phase,uint16_t winner) {
    const uint32_t hash=cup_name_hash(name,slot);
    uint32_t pos=0;
    if(!s_live || !hash || winner==65535u) return 0;
    while(pos<s_cup_titles_count &&
          (s_cup_titles[pos].season<season ||
           (s_cup_titles[pos].season==season &&
            s_cup_titles[pos].competition_hash<hash))) ++pos;
    if(pos<s_cup_titles_count && s_cup_titles[pos].season==season &&
       s_cup_titles[pos].competition_hash==hash) {
        if(s_cup_titles[pos].winner!=winner) {
            fprintf(stderr,"[living-world] CONFLICTING OG CUP WINNER "
                           "year=%u hash=%lu\n",
                    (unsigned)season,(unsigned long)hash);
            return 0;
        }
        /* Once a season is archived, do not recreate a resident event. */
        if(season<s_window_first_year) return 1;
        return cup_record(season,name,slot,phase,2u,0u,0u,0u,
                          winner,65535u,0u,0u,0u);
    }
    if(season<s_window_first_year ||
       s_cup_titles_count>=LIVE_MAX_CUP_TITLES) return 0;
    if(s_cup_titles_count==s_cup_titles_capacity) {
        uint32_t cap=s_cup_titles_capacity?s_cup_titles_capacity*2u:128u;
        if(cap>LIVE_MAX_CUP_TITLES) cap=LIVE_MAX_CUP_TITLES;
        LiveCupTitle *grown=(LiveCupTitle *)realloc(
                       s_cup_titles,cap*sizeof(*grown));
        if(!grown) return 0;
        s_cup_titles=grown;s_cup_titles_capacity=cap;
    }
    if(!cup_record(season,name,slot,phase,2u,0u,0u,0u,
                   winner,65535u,0u,0u,0u)) return 0;
    memmove(s_cup_titles+pos+1u,s_cup_titles+pos,
            (s_cup_titles_count-pos)*sizeof(*s_cup_titles));
    s_cup_titles[pos]=(LiveCupTitle){season,winner,hash};
    ++s_cup_titles_count;
    return 1;
}
uint32_t ExpandedWorldLive_CupHistoricalTitles(void) {
    return s_cup_titles_count;
}
int ExpandedWorldLive_CupHistoryChampion(uint16_t season,const char *name,
                                          uint8_t slot,uint16_t *winner) {
    uint32_t hash=cup_name_hash(name,slot);
    if(!hash || !winner) return 0;
    for(uint32_t i=0;i<s_cup_titles_count;++i)
        if(s_cup_titles[i].season==season &&
           s_cup_titles[i].competition_hash==hash) {
            *winner=s_cup_titles[i].winner;
            return 1;
        }
    return 0;
}
uint32_t ExpandedWorldLive_CupResidentMatches(void) {
    uint32_t n=0;
    for(uint32_t i=0;i<s_cup_count;++i) n+=s_cup_events[i].kind==1u;
    return n;
}
uint32_t ExpandedWorldLive_CupResidentTitles(void) {
    uint32_t n=0;
    for(uint32_t i=0;i<s_cup_count;++i) n+=s_cup_events[i].kind==2u;
    return n;
}
uint32_t ExpandedWorldLive_CupArchivedMatches(void) {return s_cup_archived_matches;}
uint32_t ExpandedWorldLive_CupArchivedTitles(void) {return s_cup_archived_titles;}
uint32_t ExpandedWorldLive_Fixtures(void) { return s_live?s_ledger.count:0; }
uint32_t ExpandedWorldLive_PlayerSlots(void) { return s_live?s_club_count*16u:0; }
uint32_t ExpandedWorldLive_MemoryBytes(void) {
    if (!s_live) return 0;
    /* Allocated pilot RAM, not the disk size of the actual world pack. */
    return (uint32_t)(sizeof s_clubs+sizeof s_leagues+sizeof s_ledger+
                      sizeof(*s_fixtures)*LIVE_MAX_FIXTURES+
                      sizeof s_native_result_consumed+
                      s_history_capacity*sizeof(*s_history)+
                      s_cup_capacity*sizeof(*s_cup_events)+
                      s_cup_titles_capacity*sizeof(*s_cup_titles));
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
    uint16_t season_roster[LIVE_MAX_LEAGUE_CLUBS]={0};
    if (!live_fixture_roster(league,ExpandedWorldLive_SeasonYear(),
                              season_roster)) return 0;
    for(unsigned i=0;i<league->count;++i) table[i].club=season_roster[i];
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
 * Living World .CAR persistence (schema 6).
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
    uint32_t promotions,relegations,mobility_unsupported;
    uint32_t native_orders,native_schedules;
    uint16_t year,window_year;
    int32_t now,initial;
    LiveArchivedLeague *history;
    uint32_t history_count,history_capacity,archived_matches;
    LiveCupEvent *cups;
    uint32_t cup_count,cup_capacity,cup_archived_matches,cup_archived_titles;
    LiveCupTitle *cup_titles;
    uint32_t cup_titles_count,cup_titles_capacity;
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
static int live_parse_league_row(const char *src,int len,LiveLeague *l,
                                 int version) {
    long v[13+LIVE_MAX_LEAGUE_CLUBS]={0};
    const unsigned base=version>=5?13u:9u;
    unsigned n=0;
    if (!live_parse_int_row(src,len,v,13+LIVE_MAX_LEAGUE_CLUBS,&n) ||
        n<base || v[0]<0 || v[0]>255 || v[1]<0 || v[1]>=16 ||
        v[2]<2 || v[2]>(long)LIVE_MAX_LEAGUE_CLUBS ||
        v[3]<2 || v[3]>3 || v[4]<0 || v[4]>1 ||
        v[5]<0 || v[5]>255 || v[6]<0 || v[6]>255 ||
        v[7]<0 || v[7]>1 || v[8]<1 || v[8]>4 ||
        n!=base+(unsigned)v[2]) return 0;
    if(version>=5)
        for(unsigned i=9;i<13u;++i)
            if(v[i]<0 || v[i]>v[2]) return 0;
    memset(l,0,sizeof *l);
    l->country=(uint8_t)v[0]; l->division=(uint8_t)v[1];
    l->count=(uint8_t)v[2]; l->points=(uint8_t)v[3];
    l->overflow=(uint8_t)v[4]; l->begin_phase=(uint8_t)v[5];
    l->end_phase=(uint8_t)v[6]; l->native_schedule=(uint8_t)v[7];
    l->legs=(uint8_t)v[8];
    if(version>=5) {
        l->auto_up=(uint8_t)v[9];l->auto_down=(uint8_t)v[10];
        l->playoff_up=(uint8_t)v[11];l->playoff_down=(uint8_t)v[12];
    }
    for (unsigned i=0;i<l->count;++i) {
        if (v[base+i]<0 || v[base+i]>=65535) return 0;
        l->clubs[i]=(uint16_t)v[base+i];
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

static int live_parse_cup_row(const char *src,int len,LiveCupEvent *e) {
    long v[12]={0};
    unsigned n=0;
    if(!live_parse_int_row(src,len,v,12u,&n) || n!=12u ||
       v[0]<0 || v[1]<0 || v[1]>2147483647L ||
       v[2]<0 || v[2]>=65535 || v[3]<0 || v[3]>65535 ||
       v[4]<1 || v[4]>2 || v[5]<0 || v[5]>255 ||
       v[6]<0 || v[6]>255 || v[7]<0 || v[7]>255 ||
       v[8]<0 || v[8]>255 || v[9]<0 || v[9]>31 ||
       v[10]<0 || v[10]>31 || v[11]<0 || v[11]>255 ||
       (v[4]==1 && (v[3]>=65535 || v[2]==v[3])) ||
       (v[4]==2 && v[3]!=65535)) return 0;
    e->day=(int32_t)v[0];e->competition_hash=(uint32_t)v[1];
    e->home=(uint16_t)v[2];e->away=(uint16_t)v[3];
    e->kind=(uint8_t)v[4];e->slot=(uint8_t)v[5];
    e->round=(uint8_t)v[6];e->match=(uint8_t)v[7];
    e->leg=(uint8_t)v[8];e->goals_home=(uint8_t)v[9];
    e->goals_away=(uint8_t)v[10];e->flags=(uint8_t)v[11];
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
      "{%Q:6,%Q:%u,%Q:%lu,%Q:%d,%Q:%lu,%Q:[%lu,%lu,%lu,%lu],%Q:[%lu,%lu,%lu],%Q:%u,%Q:[",
      "version","first_year",(unsigned)s_first_year,
      "weeks",(unsigned long)s_weeks,"now",(int)s_ledger.now,
      "preseason_games",(unsigned long)s_preseason_games_total,
      "native",(unsigned long)s_native_matches,(unsigned long)s_native_misses,
      (unsigned long)s_native_order_applied,(unsigned long)s_native_schedule_leagues,
      "movement",(unsigned long)s_promotions,(unsigned long)s_relegations,
      (unsigned long)s_mobility_unsupported,
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
        mjson_printf(emit,ctx,"[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
          (unsigned)l->country,(unsigned)l->division,
          (unsigned)l->count,(unsigned)l->points,
          (unsigned)l->overflow,(unsigned)l->begin_phase,
          (unsigned)l->end_phase,(unsigned)l->native_schedule,
          (unsigned)l->legs,(unsigned)l->auto_up,(unsigned)l->auto_down,
          (unsigned)l->playoff_up,(unsigned)l->playoff_down);
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
    mjson_printf(emit,ctx,"],%Q:%lu,%Q:%Q,%Q:[%lu,%lu],%Q:[",
                  "fixture_json_bytes",(unsigned long)raw.len,
                  "fixtures_zlib",encoded,
                  "cup_archive",(unsigned long)s_cup_archived_matches,
                  (unsigned long)s_cup_archived_titles,"cups");
    for(uint32_t i=0;i<s_cup_count;++i) {
        const LiveCupEvent *e=&s_cup_events[i];
        if(i) mjson_printf(emit,ctx,"%s",",");
        mjson_printf(emit,ctx,
             "[%d,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]",
             (int)e->day,(unsigned long)e->competition_hash,
             (unsigned)e->home,(unsigned)e->away,(unsigned)e->kind,
             (unsigned)e->slot,(unsigned)e->round,(unsigned)e->match,
             (unsigned)e->leg,(unsigned)e->goals_home,
             (unsigned)e->goals_away,(unsigned)e->flags);
    }
    mjson_printf(emit,ctx,"],%Q:[","cup_titles");
    for(uint32_t i=0;i<s_cup_titles_count;++i) {
        const LiveCupTitle *t=&s_cup_titles[i];
        if(i) mjson_printf(emit,ctx,"%s",",");
        mjson_printf(emit,ctx,"[%u,%lu,%u]",
            (unsigned)t->season,(unsigned long)t->competition_hash,
            (unsigned)t->winner);
    }
    mjson_printf(emit,ctx,"%s","]}");
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
        (snapshot_version!=1 && snapshot_version!=2 &&
          snapshot_version!=3 && snapshot_version!=4 &&
          snapshot_version!=5 && snapshot_version!=6) ||
        mjson_get_long(obj,olen,"$.first_year",&v)<=0 ||
        v<1900 || v>2198) goto invalid;
    tmp->year=(uint16_t)v;
    tmp->window_year=tmp->year;
    if (snapshot_version>=3) {
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
    if(snapshot_version>=5) {
        long counts[3]={0};
        unsigned count=0;
        if(mjson_find(obj,olen,"$.movement",&arr,&alen)!=MJSON_TOK_ARRAY ||
           !live_parse_int_row(arr,alen,counts,3u,&count) || count!=3u)
            goto invalid;
        for(unsigned i=0;i<3u;++i)
            if(counts[i]<0 || (uint64_t)counts[i]>UINT32_MAX) goto invalid;
        tmp->promotions=(uint32_t)counts[0];
        tmp->relegations=(uint32_t)counts[1];
        tmp->mobility_unsupported=(uint32_t)counts[2];
    }

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
            !live_parse_league_row(arr+vo,vl,&tmp->leagues[tmp->leagues_count],
                                   (int)snapshot_version))
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
    /* World v4 records original SWOS cup matches and champions. */
    if(snapshot_version>=4) {
        long archived[2]={0};
        unsigned count=0;
        if(mjson_find(obj,olen,"$.cup_archive",&arr,&alen)!=MJSON_TOK_ARRAY ||
           !live_parse_int_row(arr,alen,archived,2u,&count) ||
           count!=2u || archived[0]<0 || archived[1]<0 ||
           mjson_find(obj,olen,"$.cups",&arr,&alen)!=MJSON_TOK_ARRAY ||
           alen<2 || arr[0]!='[' || arr[alen-1]!=']') goto invalid;
        tmp->cup_archived_matches=(uint32_t)archived[0];
        tmp->cup_archived_titles=(uint32_t)archived[1];
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
            if(p>=end || *p!='[' || tmp->cup_count>=LIVE_MAX_CUP_EVENTS)
                goto invalid;
            const char *row=p++;
            while(p<end && *p!=']') {
                if(*p=='[' || *p=='{' || *p=='"') goto invalid;
                ++p;
            }
            if(p>=end) goto invalid;
            ++p;
            if(tmp->cup_count==tmp->cup_capacity) {
                uint32_t cap=tmp->cup_capacity?tmp->cup_capacity*2u:128u;
                if(cap>LIVE_MAX_CUP_EVENTS) cap=LIVE_MAX_CUP_EVENTS;
                LiveCupEvent *grown=(LiveCupEvent *)realloc(tmp->cups,
                                                     cap*sizeof(*grown));
                if(!grown) goto invalid;
                tmp->cups=grown;tmp->cup_capacity=cap;
            }
            LiveCupEvent *e=&tmp->cups[tmp->cup_count];
            if(!live_parse_cup_row(row,(int)(p-row),e) ||
               e->day<earliest_fixture_day ||
               (tmp->cup_count && e->day<tmp->cups[tmp->cup_count-1u].day))
                goto invalid;
            ++tmp->cup_count;
            first=0;
        }
    }

    /* Immutable historical champions, including editions no longer
     * resident after the two-year world window rolls over. */
    if(snapshot_version>=6) {
        if(mjson_find(obj,olen,"$.cup_titles",&arr,&alen)!=MJSON_TOK_ARRAY ||
           alen<2 || arr[0]!='[' || arr[alen-1]!=']') goto invalid;
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
               tmp->cup_titles_count>=LIVE_MAX_CUP_TITLES) goto invalid;
            const char *row=p++;
            while(p<end && *p!=']') {
                if(*p=='[' || *p=='{' || *p=='"') goto invalid;
                ++p;
            }
            if(p>=end) goto invalid;
            ++p;
            long values[3]={0};
            unsigned count=0;
            if(!live_parse_int_row(row,(int)(p-row),values,3u,&count) ||
               count!=3u || values[0]<tmp->year || values[0]>2199 ||
               values[1]<=0 || values[1]>2147483647L ||
               values[2]<0 || values[2]>=65535) goto invalid;
            if(tmp->cup_titles_count) {
                const LiveCupTitle *prev=&tmp->cup_titles[tmp->cup_titles_count-1u];
                if(values[0]<prev->season ||
                   (values[0]==prev->season &&
                    (uint32_t)values[1]<=prev->competition_hash))
                    goto invalid;
            }
            if(tmp->cup_titles_count==tmp->cup_titles_capacity) {
                uint32_t cap=tmp->cup_titles_capacity?
                             tmp->cup_titles_capacity*2u:128u;
                if(cap>LIVE_MAX_CUP_TITLES) cap=LIVE_MAX_CUP_TITLES;
                LiveCupTitle *grown=(LiveCupTitle *)realloc(
                           tmp->cup_titles,cap*sizeof(*grown));
                if(!grown) goto invalid;
                tmp->cup_titles=grown;tmp->cup_titles_capacity=cap;
            }
            tmp->cup_titles[tmp->cup_titles_count++]=(LiveCupTitle){
                (uint16_t)values[0],(uint16_t)values[2],
                (uint32_t)values[1]};
            first=0;
        }
        /* 0.13.29/v4-v5 saves retain historical title COUNTS but not
         * all winner IDs. A future v6 save can therefore have a legitimate
         * partial title list; never discard an existing career. */
    } else if(snapshot_version>=4) {
        /* Versions 4-5 discarded champions before the resident window;
         * preserve only those still present rather than inventing history. */
        for(uint32_t i=0;i<tmp->cup_count;++i) {
            const LiveCupEvent *e=&tmp->cups[i];
            if(e->kind!=2u) continue;
            ExpandedWorldDate d;
            if(!ExpandedWorld_IndexToDate(e->day,&d) ||
               tmp->cup_titles_count>=LIVE_MAX_CUP_TITLES) goto invalid;
            uint32_t j=0;
            while(j<tmp->cup_titles_count &&
                  (tmp->cup_titles[j].season<d.year ||
                   (tmp->cup_titles[j].season==d.year &&
                    tmp->cup_titles[j].competition_hash<e->competition_hash)))
                ++j;
            if(j<tmp->cup_titles_count &&
               tmp->cup_titles[j].season==d.year &&
               tmp->cup_titles[j].competition_hash==e->competition_hash) {
                if(tmp->cup_titles[j].winner!=e->home) goto invalid;
                continue;
            }
            if(tmp->cup_titles_count==tmp->cup_titles_capacity) {
                uint32_t cap=tmp->cup_titles_capacity?
                             tmp->cup_titles_capacity*2u:128u;
                LiveCupTitle *grown=(LiveCupTitle *)realloc(
                          tmp->cup_titles,cap*sizeof(*grown));
                if(!grown) goto invalid;
                tmp->cup_titles=grown;tmp->cup_titles_capacity=cap;
            }
            memmove(tmp->cup_titles+j+1u,tmp->cup_titles+j,
                    (tmp->cup_titles_count-j)*sizeof(*tmp->cup_titles));
            tmp->cup_titles[j]=(LiveCupTitle){
                (uint16_t)(d.year-(d.month<7u?1u:0u)),
                e->home,e->competition_hash};
            ++tmp->cup_titles_count;
        }
    }
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
    s_cup_events=tmp->cups;s_cup_count=tmp->cup_count;
    s_cup_capacity=tmp->cup_capacity;
    s_cup_archived_matches=tmp->cup_archived_matches;
    s_cup_archived_titles=tmp->cup_archived_titles;
    tmp->cups=NULL;
    s_cup_titles=tmp->cup_titles;
    s_cup_titles_count=tmp->cup_titles_count;
    s_cup_titles_capacity=tmp->cup_titles_capacity;
    tmp->cup_titles=NULL;
    s_initial_day=tmp->initial;
    s_preseason_games_total=tmp->preseason_games;
    s_native_matches=tmp->native_matches;
    s_native_misses=tmp->native_misses;
    s_native_order_applied=tmp->native_orders;
    s_native_schedule_leagues=tmp->native_schedules;
    s_promotions=tmp->promotions;
    s_relegations=tmp->relegations;
    s_mobility_unsupported=tmp->mobility_unsupported;
    s_fixtures=store;
    s_ledger.fixtures=store;
    s_ledger.capacity=LIVE_MAX_FIXTURES;
    s_ledger.count=tmp->fixture_count;
    s_ledger.played=tmp->played;
    s_ledger.now=tmp->now;
    s_live=1;
    /* Reconstruct generated season flags from the persisted resident
     * fixture rosters: one transition only, even after save/load. */
    for(uint32_t l=0;l<s_league_count;++l) {
        uint16_t roster[LIVE_MAX_LEAGUE_CLUBS]={0};
        s_generated_fresh[l]=(uint8_t)(
            live_fixture_roster(&s_leagues[l],s_window_first_year+1u,
                               roster)?1u:0u);
    }
    if (!live_complete_prestart()) {
        /* The replacement became active only after complete validation.
         * Fail closed on an impossible missing club/invalid score. */
        ExpandedWorldLive_End();
        store=NULL;
        goto invalid;
    }
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
    if(tmp) {free(tmp->history);free(tmp->cups);free(tmp->cup_titles);}
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
int ExpandedWorldLive_SetLeagueMovement(uint8_t c,uint8_t d,uint8_t u,
                                        uint8_t dn,uint8_t pu,uint8_t pd) {
    (void)c;(void)d;(void)u;(void)dn;(void)pu;(void)pd;return 0;
}
uint32_t ExpandedWorldLive_Promotions(void) {return 0;}
uint32_t ExpandedWorldLive_Relegations(void) {return 0;}
uint32_t ExpandedWorldLive_MobilityUnsupported(void) {return 0;}
uint8_t ExpandedWorldLive_ClubDivision(uint16_t id) {(void)id;return 255u;}
int ExpandedWorldLive_SetLeagueSchedule(uint8_t c,uint8_t d,uint8_t s,uint8_t e,
                                        uint8_t legs) {
    (void)c;(void)d;(void)s;(void)e;(void)legs;return 0;
}
int ExpandedWorldLive_NativeOrder(const uint16_t *ids,uint32_t n,uint8_t *order) {
    (void)ids;(void)n;(void)order;return 0;
}
uint32_t ExpandedWorldLive_NativeOrderApplied(void) {return 0;}
uint32_t ExpandedWorldLive_NativeScheduleLeagues(void) {return 0;}
int ExpandedWorldLive_AdvanceToDate(ExpandedWorldDate d) {(void)d;return 0;}
int ExpandedWorldLive_CalendarState(ExpandedLiveCalendarState *s) {(void)s;return 0;}
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
uint32_t ExpandedWorldLive_OfferFiles(uint8_t *files,uint32_t n) {
    (void)files;(void)n;return 0;
}
int ExpandedWorldLive_RecordNativeCupMatch(uint16_t y,const char *name,
                 uint8_t slot,uint8_t phase,uint8_t round,uint8_t match,
                 uint8_t leg,uint16_t home,uint16_t away,uint8_t gh,
                 uint8_t ga,uint8_t flags) {
    (void)y;(void)name;(void)slot;(void)phase;(void)round;
    (void)match;(void)leg;(void)home;(void)away;(void)gh;
    (void)ga;(void)flags;return 0;
}
int ExpandedWorldLive_RecordNativeCupChampion(uint16_t y,const char *name,
                 uint8_t slot,uint8_t phase,uint16_t winner) {
    (void)y;(void)name;(void)slot;(void)phase;(void)winner;return 0;
}
uint32_t ExpandedWorldLive_CupResidentMatches(void) {return 0;}
uint32_t ExpandedWorldLive_CupResidentTitles(void) {return 0;}
uint32_t ExpandedWorldLive_CupArchivedMatches(void) {return 0;}
uint32_t ExpandedWorldLive_CupArchivedTitles(void) {return 0;}
uint32_t ExpandedWorldLive_CupHistoricalTitles(void) {return 0;}
int ExpandedWorldLive_CupHistoryChampion(uint16_t season,const char *name,
                                          uint8_t slot,uint16_t *winner) {
    (void)season;(void)name;(void)slot;(void)winner;return 0;
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
