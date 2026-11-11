#include "expanded_world_live.h"
#ifdef SWOS_SDL_BUILD
#include "expanded_world_time.h"
#include "expanded_world_ledger.h"
#include <stdlib.h>
#include <string.h>

#define LIVE_MAX_CLUBS 1500u
#define LIVE_MAX_LEAGUES 128u
#define LIVE_MAX_FIXTURES 40000u
#define LIVE_MAX_LEAGUE_CLUBS 26u
#define NATIVE_FIXTURE_MAX_CLUBS 24u
/* Canonical OpenSWOS pairing tables: also used by DIY league fixtures.
 * The odd-sized tables are installed by FixtureRounds_Install at startup. */
extern const uint8_t *_fixtureSchedulePtrTable[];
#define LIVE_SEASONS 2u

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
    s_weeks=0; s_first_year=0; s_initial_day=0;
    s_preseason_games_total=0;
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
    const uint16_t cid=competition_id(league->country,league->division,season);
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
    for (unsigned season=0;season<LIVE_SEASONS;++season) {
        ExpandedWorldDate start={(uint16_t)(s_first_year+season),7,1};
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
int ExpandedWorldLive_AdvanceWeeks(uint32_t weeks) {
    uint64_t d=(uint64_t)(uint32_t)s_initial_day+(uint64_t)weeks*7u;
    if (!s_live || d>INT32_MAX || weeks<s_weeks) return 0;
    int32_t old_day=s_ledger.now;
    if (!ExpandedWorldLedger_Advance(&s_ledger,(int32_t)d,resolve_game,NULL)) return 0;
    preseason_advance(old_day,(int32_t)d);
    s_weeks=weeks;
    return 1;
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
    if (season_year<s_first_year ||
        (uint32_t)(season_year-s_first_year)>=LIVE_SEASONS) {
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
uint32_t ExpandedWorldLive_Results(void) { return s_ledger.played; }
uint32_t ExpandedWorldLive_Fixtures(void) { return s_live?s_ledger.count:0; }
uint32_t ExpandedWorldLive_PlayerSlots(void) { return s_live?s_club_count*16u:0; }
uint32_t ExpandedWorldLive_MemoryBytes(void) {
    if (!s_live) return 0;
    /* Allocated pilot RAM, not the disk size of the actual world pack. */
    return (uint32_t)(sizeof s_clubs+sizeof s_leagues+sizeof s_ledger+
                      sizeof(*s_fixtures)*LIVE_MAX_FIXTURES+
                      sizeof s_native_result_consumed);
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
    season_delta=(int32_t)ExpandedWorldLive_SeasonYear()-(int32_t)s_first_year;
    if (season_delta<0 || season_delta>=(int32_t)LIVE_SEASONS) return 0;
    for(unsigned i=0;i<league->count;++i) table[i].club=league->clubs[i];
    if (!ExpandedWorldLedger_Standings(&s_ledger,
            competition_id(country,division,(unsigned)season_delta),
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
int ExpandedWorldLive_Table(uint8_t c,uint8_t d,ExpandedLiveRow *r,
                            uint32_t cap,uint32_t *total) {
    (void)c;(void)d;(void)r;(void)cap;if(total)*total=0;return 0;
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
