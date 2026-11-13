/*
 * 0.13.12 — exit finished native fixture screens and resume chronological catch-up.
 *
 * The prehire pilot is an independent round-robin, so its fixture index is
 * NOT interchangeable with the original SWOS league/cup calendar. We do not
 * patch league points or cursor bytes: the game plays the prior rounds through
 * the same menu handlers as its golden career regression harness.
 */
#include "expanded_midseason_join.h"
#include "expanded_world_time.h"
#include "world.h"
#ifdef SWOS_SDL_BUILD
#include "expanded_world_live.h"
#endif

#include <stdio.h>

/* One date-to-OG-phase mapping for first-season and later hires. */
static int native_cutoff_at(uint16_t season,ExpandedWorldDate now,uint8_t *out) {
    ExpandedWorldDate august={season,8u,1u},month_start,next_month;
    int32_t today=0,first=0,month_day=0,next_day=0;
    if (!out || now.year<season || now.year>season+1u ||
        !ExpandedWorld_DateToIndex(now,&today) ||
        !ExpandedWorld_DateToIndex(august,&first)) return 0;
    if (today<=first) { *out=0; return 1; }
    /* Original OG keycache is month eighths; next-year phases add 0x60. */
    month_start=(ExpandedWorldDate){now.year,now.month,1u};
    next_month=(ExpandedWorldDate){now.month==12u ?
                      (uint16_t)(now.year+1u):now.year,
                      now.month==12u?1u:(uint8_t)(now.month+1u),1u};
    if (!ExpandedWorld_DateToIndex(month_start,&month_day) ||
        !ExpandedWorld_DateToIndex(next_month,&next_day) ||
        next_day<=month_day) return 0;
    const uint32_t days=(uint32_t)(next_day-month_day);
    const uint32_t eighth=(uint32_t)(now.day-1u)*8u;
    uint32_t phase=(uint32_t)(now.month-1u)*8u+
                   (eighth+days-1u)/days;
    if (now.year>season) phase+=0x60u;
    if (phase>255u) return 0;
    *out=(uint8_t)phase;
    return 1;
}

int Expanded_MidseasonJoin_Cutoff(uint16_t year,uint32_t weeks,uint8_t *out) {
    ExpandedWorldDate july={year,7u,1u},now;
    return weeks<52u &&
           ExpandedWorld_DateAfterWeeks(july,weeks,&now) &&
           native_cutoff_at(year,now,out);
}

/* Unlike the legacy single-year helper, this uses the CURRENT world's
 * season, not the career start year. No replay of archived prior seasons. */
int Expanded_MidseasonJoin_AbsoluteCutoff(uint16_t first_year,uint32_t weeks,
                                          uint16_t *season,uint8_t *out) {
    ExpandedWorldDate july={first_year,7u,1u},now;
    if (!season || !out ||
        !ExpandedWorld_DateAfterWeeks(july,weeks,&now) ||
        now.year<first_year || now.year>2199u) return 0;
    const uint16_t current=(uint16_t)(now.year-(now.month<7u?1u:0u));
    if (current<first_year || current>2198u ||
        !native_cutoff_at(current,now,out)) return 0;
    *season=current;
    return 1;
}

/* July is not universally preseason. Southern leagues may already have
 * played native January-July rounds. Preserve the old northern-season
 * cutoff unless the real OG league cache starts before August.
 * This helper is pure and does not claim that world scores match OG. */
int Expanded_MidseasonJoin_SouthernCutoff(uint16_t first_year,uint32_t weeks,
                                         uint8_t earliest_league_key,
                                         uint16_t *season,uint8_t *cutoff) {
    ExpandedWorldDate july={first_year,7u,1u},now,month_start,next_month;
    int32_t first=0,next=0;
    uint16_t original=0;
    uint8_t previous=0;
    if(!season || !cutoff || earliest_league_key>=56u ||
       !Expanded_MidseasonJoin_AbsoluteCutoff(first_year,weeks,
                                              &original,&previous) ||
       previous!=0u ||
       !ExpandedWorld_DateAfterWeeks(july,weeks,&now) ||
       now.year!=original || now.month!=7u) return 0;
    month_start=(ExpandedWorldDate){now.year,7u,1u};
    next_month=(ExpandedWorldDate){now.year,8u,1u};
    if(!ExpandedWorld_DateToIndex(month_start,&first) ||
       !ExpandedWorld_DateToIndex(next_month,&next) || next<=first) return 0;
    const unsigned days=(unsigned)(next-first);
    const unsigned eighth=((unsigned)(now.day-1u)*8u+days-1u)/days;
    const unsigned phase=48u+eighth;
    if(phase<=earliest_league_key || phase>255u) return 0;
    *season=original;
    *cutoff=(uint8_t)phase;
    return 1;
}

/* A handoff cannot succeed before the requested original native phase.
 * Even if OG unexpectedly reaches a season-end menu, that is a FAILED
 * synchronization, not permission to report REACHED. Pure for C testing. */
int Expanded_MidseasonJoin_HandoffReached(uint8_t target,uint8_t actual) {
    return actual>=target;
}

#ifdef SWOS_SDL_BUILD
#include "game_types.h"
#include "menu_system.h"

extern uint8_t _currentMenu[];
extern uint8_t _teamSelectionKeyCache[];
extern TeamFileEx _careerTeam;
extern uint16_t _gameType,_careerFinished,_seasonFinished;
extern uint16_t _teamSelectionKeyCacheCount,_teamSelectionKeyCacheIndex;
extern uint16_t _teamSelectionKeyCacheOffset;
extern volatile uint16_t _shortFire;
extern volatile uint16_t _fire;
extern uint8_t _pl1Fire;
extern uint8_t _left,_right,_up,_down;
extern int16_t _menuControlsDirection,_menuControlsDirection2;
extern uint8_t _lastFinancialWeekIndex;

extern void ContestMenuInit(void);
extern void CareerOverInit(void);
extern void DIYLeagueTournamentInit(void);
extern void DIYCupInit(void);
extern void PlayMatchMenuInit(void);
extern void NextMatch(MenuEntry *);
extern void DIYLeagueTournamentNextMatch(void);
extern void DIYLeagueTournamentExit(void);
extern void DIYLeagueTournamentMatch(MenuEntry *);
extern void DIYCupOnSelect1(void);
extern void DIYCupOnSelect3(MenuEntry *);
extern void DIYCupOnSelect5(MenuEntry *);
extern void DIYCupOnSelect8(MenuEntry *);
extern void DIYCupOnSelect10(void);
extern void DIYLeagueTournamentViewResult(MenuEntry *);
extern void DIYLeagueTournamentResult(MenuEntry *);
extern void MenuSystem_LatchSeasonSkip(void);
extern void SetCurrentEntry(uint16_t ordinal);
extern MenuEntry *CalcMenuEntryAddress(uint16_t ordinal);

static uint8_t s_active,s_old_controls,s_target_key;
static uint32_t s_passes,s_elapsed_weeks,s_actions,s_forced_simulations;
static uint32_t s_league_exits;
static int s_direct_league_import;
static int s_complete_season_handoff;
static uint32_t s_historical_nonleague_events;
static int s_last_result,s_last_key;
static uint32_t s_clock_samples,s_clock_changes;
static int s_clock_armed,s_clock_status,s_clock_previous_valid;
static int32_t s_clock_last_og,s_clock_last_world,s_clock_delta;
static char s_clock_last_event[48]="NOT_STARTED";
static char s_clock_first_problem[192]="NONE";
static uint16_t s_clock_sample_cursor=0xffffu;
static uint8_t s_clock_sample_phase=0xffu;
void Expanded_ClockTrace_Reset(void) {
    s_clock_samples=0;
    s_clock_changes=0;
    s_clock_armed=0;
    s_clock_status=0;
    s_clock_previous_valid=0;
    s_clock_last_og=0;
    s_clock_last_world=0;
    s_clock_delta=0;
    s_clock_sample_cursor=0xffffu;
    s_clock_sample_phase=0xffu;
    snprintf(s_clock_last_event,sizeof s_clock_last_event,"NOT_STARTED");
    snprintf(s_clock_first_problem,sizeof s_clock_first_problem,"NONE");
    fprintf(stderr,"[clock-trace] RESET new-career\n");
}
extern uint16_t _currentSeasonShowing;
void Expanded_ClockTrace_Sample(const char *event) {
    ExpandedLiveCalendarState cal;
    ExpandedWorldDate og={0};
    int32_t world_day=0,og_day=0;
    uint16_t season;
    uint8_t phase=0;
    int32_t previous_og=s_clock_last_og;
    int32_t previous_world=s_clock_last_world;
    int was_valid=s_clock_previous_valid;
    if (!event)event="UNSPECIFIED";
    snprintf(s_clock_last_event,sizeof s_clock_last_event,"%s",event);
    if (!ExpandedWorldLive_CalendarState(&cal) ||
        !ExpandedWorld_DateToIndex(cal.today,&world_day)) return;
    ++s_clock_samples;
    if (_gameType!=4 || _currentSeasonShowing>=100u) {
        if(!was_valid || world_day!=previous_world) {
            ++s_clock_changes;
            fprintf(stderr,"[clock-trace] event=%s mode=WORLD_ONLY world=%04u-%02u-%02u "
                           "world-index=%ld previous-world=%ld armed=%d\n",
                    event,(unsigned)cal.today.year,(unsigned)cal.today.month,
                    (unsigned)cal.today.day,(long)world_day,
                    (long)(was_valid?previous_world:world_day),s_clock_armed);
        }
        s_clock_last_world=world_day;
        s_clock_previous_valid=0; /* OG clock unavailable during unemployment */
        return;
    }
    season=(uint16_t)(World_FirstYear()+_currentSeasonShowing);
    phase=Expanded_Career_NativeCalendarKey();
    if (!ExpandedWorld_NativePhaseToDate(season,phase,&og) ||
        !ExpandedWorld_DateToIndex(og,&og_day)) return;
    s_clock_delta=og_day-world_day;
    if (!was_valid || og_day!=previous_og || world_day!=previous_world) {
        ++s_clock_changes;
        fprintf(stderr,"[clock-trace] event=%s state=%s season=%u phase=%u "
                       "OG=%04u-%02u-%02u world=%04u-%02u-%02u "
                       "delta-days=%ld prior-og=%ld prior-world=%ld "
                       "og-step=%ld world-step=%ld sample=%lu change=%lu\n",
                event,s_clock_armed?"POST_HANDOFF":"RECOVERING",
                (unsigned)season,(unsigned)phase,
                (unsigned)og.year,(unsigned)og.month,(unsigned)og.day,
                (unsigned)cal.today.year,(unsigned)cal.today.month,
                (unsigned)cal.today.day,(long)s_clock_delta,
                (long)(was_valid?previous_og:og_day),
                (long)(was_valid?previous_world:world_day),
                (long)(was_valid?og_day-previous_og:0),
                (long)(was_valid?world_day-previous_world:0),
                (unsigned long)s_clock_samples,
                (unsigned long)s_clock_changes);
    }
    if (s_clock_armed) {
        const char *problem=NULL;
        if (was_valid && world_day<previous_world)
            problem="WORLD_CLOCK_REWIND";
        else if (was_valid && og_day<previous_og)
            problem="OG_PHASE_REWIND";
        else if(s_clock_delta< -31 || s_clock_delta>31)
            problem="POST_HANDOFF_DELTA_GT_31";
        if(problem) {
            s_clock_status=2;
            if(s_clock_first_problem[0]=='N') {
                snprintf(s_clock_first_problem,sizeof s_clock_first_problem,
                         "%s at %s season=%u phase=%u delta=%ld OG-step=%ld WORLD-step=%ld",
                         problem,event,(unsigned)season,(unsigned)phase,
                         (long)s_clock_delta,
                         (long)(was_valid?og_day-previous_og:0),
                         (long)(was_valid?world_day-previous_world:0));
                fprintf(stderr,"[clock-trace-first-fail] %s\n",s_clock_first_problem);
            }
        } else if(!s_clock_status) s_clock_status=1;
    }
    s_clock_last_og=og_day;s_clock_last_world=world_day;
    s_clock_previous_valid=1;
}
int Expanded_ClockTrace_Status(void){return s_clock_status;}
uint32_t Expanded_ClockTrace_Samples(void){return s_clock_samples;}
uint32_t Expanded_ClockTrace_Changes(void){return s_clock_changes;}
int32_t Expanded_ClockTrace_DeltaDays(void){return s_clock_delta;}
const char *Expanded_ClockTrace_FirstProblem(void){return s_clock_first_problem;}
const char *Expanded_ClockTrace_LastEvent(void){return s_clock_last_event;}

static int s_southern_handoff_status; /* 0 NOT_TESTED, 1 PASS, 2 FAIL */
int Expanded_MidseasonJoin_SouthernStatus(void) {return s_southern_handoff_status;}

/* InitCareer constructs one fresh OG season. Put its season counters in
 * the same WORLD season before importing any scores or replaying cups.
 * This does NOT synthesize mobility or archived cup brackets. */
int Expanded_MidseasonJoin_AlignSeason(uint32_t elapsed) {
    uint16_t season=0;
    uint8_t cutoff=0;
    extern uint16_t _currentSeasonShowing;
    extern uint8_t _seasonPlaying;
    if (!Expanded_MidseasonJoin_AbsoluteCutoff(World_FirstYear(),elapsed,
                                               &season,&cutoff)) {
        fprintf(stderr,"[clock-audit] event=ALIGN_FAILED first-year=%u elapsed-weeks=%lu\n",
                (unsigned)World_FirstYear(),(unsigned long)elapsed);
        return 0;
    }
    {
        ExpandedWorldDate nativeDate,worldDate;
        ExpandedWorldDate july={World_FirstYear(),7u,1u};
        int32_t nativeIndex=0,worldIndex=0;
        if(ExpandedWorld_NativePhaseToDate(season,cutoff,&nativeDate) &&
           ExpandedWorld_DateAfterWeeks(july,elapsed,&worldDate) &&
           ExpandedWorld_DateToIndex(nativeDate,&nativeIndex) &&
           ExpandedWorld_DateToIndex(worldDate,&worldIndex))
            fprintf(stderr,"[clock-audit] event=ALIGN_PRE season=%u phase=%u "
                           "OG=%04u-%02u-%02u WORLD=%04u-%02u-%02u delta-days=%ld "
                           "old-season-index=%u old-native-key=%u\n",
                    (unsigned)season,(unsigned)cutoff,
                    (unsigned)nativeDate.year,(unsigned)nativeDate.month,
                    (unsigned)nativeDate.day,
                    (unsigned)worldDate.year,(unsigned)worldDate.month,
                    (unsigned)worldDate.day,(long)(nativeIndex-worldIndex),
                    (unsigned)_currentSeasonShowing,
                    (unsigned)Expanded_Career_NativeCalendarKey());
    }
    if (!ExpandedWorldLive_NativeMobilityPrepared()) {
        fprintf(stderr,"[midseason] native season members not synced to "
                       "world. Rejecting a fake multi-year join\n");
        return 0;
    }
    const uint32_t offset=(uint32_t)season-(uint32_t)World_FirstYear();
    if (offset>=100u) {
        fprintf(stderr,"[midseason] unsupported OG season index %lu\n",
                (unsigned long)offset);
        return 0;
    }
    _currentSeasonShowing=(uint16_t)offset;
    _seasonPlaying=(uint8_t)offset;
    Expanded_ClockTrace_Sample("ALIGN_APPLIED");
    fprintf(stderr,"[clock-audit] event=ALIGN_APPLIED season-index=%u native-playing=%u target-phase=%u\n",
            (unsigned)_currentSeasonShowing,(unsigned)_seasonPlaying,
            (unsigned)cutoff);
    fprintf(stderr,"[midseason] aligned OG season counters: %u/%02u "
                   "offset=%lu current_phase=%u (world authoritative)\n",
            (unsigned)season,((unsigned)season+1u)%100u,
            (unsigned long)offset,(unsigned)cutoff);
    return 1;
}

uint32_t Expanded_MidseasonJoin_Actions(void) { return s_actions; }
uint32_t Expanded_MidseasonJoin_LeagueExits(void) { return s_league_exits; }
uint32_t Expanded_MidseasonJoin_ForcedSimulations(void) { return s_forced_simulations; }
void Expanded_MidseasonJoin_RecordForcedSimulation(void) {
    if (s_active) ++s_forced_simulations;
}

int Expanded_MidseasonJoin_Active(void) { return s_active!=0; }
int Expanded_MidseasonJoin_LastResult(void) { return s_last_result; }
int Expanded_MidseasonJoin_LastKey(void) { return s_last_key; }
uint32_t Expanded_MidseasonJoin_HistoricalCupEvents(void) {
    return s_historical_nonleague_events;
}
uint8_t Expanded_MidseasonJoin_TargetPhase(void) { return s_target_key; }

/* The fixture cursor advances BEFORE the financial-week accumulator.
 * The in-game date must follow the NEXT unplayed fixture, while finance
 * continues to use lastFinancialWeekIndex for deductions. */
uint8_t Expanded_Career_NativeCalendarKey(void) {
    if (_gameType==4 && _teamSelectionKeyCacheCount &&
        _teamSelectionKeyCacheIndex<_teamSelectionKeyCacheCount &&
        _teamSelectionKeyCacheOffset<=8192u-sizeof(TeamSelectionKeyCacheEntry)) {
        const TeamSelectionKeyCacheEntry *entry=
            (const TeamSelectionKeyCacheEntry *)
            (_teamSelectionKeyCache+_teamSelectionKeyCacheOffset);
        if (entry->entryStride>=sizeof(*entry) &&
            entry->entryStride<=256u &&
            entry->entryStride<=8192u-_teamSelectionKeyCacheOffset)
            return entry->sortKey;
    }
    return _lastFinancialWeekIndex;
}

static int choose_action(MenuEntryFn fn) {
    const Menu *menu=(const Menu *)_currentMenu;
    if (!menu) return 0;
    for (uint16_t i=0;i<menu->numEntries;++i) {
        const MenuEntry *entry=CalcMenuEntryAddress(i);
        if (entry && entry->onSelect==fn &&
            !entry->isInvisible && !entry->isDisabled) {
            SetCurrentEntry(i);
            return 1;
        }
    }
    return 0;
}

static void finish(int reached,int key) {
    const uint8_t actual=Expanded_Career_NativeCalendarKey();
    if(reached && !Expanded_MidseasonJoin_HandoffReached(s_target_key,actual)) {
        fprintf(stderr,"[midseason] FAIL EARLY_HANDOFF target=%u actual=%u "
                       "reported-key=%d financial=%u actions=%lu "
                       "season-ended=%u career-ended=%u\n",
                (unsigned)s_target_key,(unsigned)actual,key,
                (unsigned)_lastFinancialWeekIndex,(unsigned long)s_actions,
                (unsigned)_seasonFinished,(unsigned)_careerFinished);
        reached=0;
        key=-15;
    }
    s_active=0;
    s_last_result=reached?1:2;
    s_last_key=key;
    _careerTeam.team.teamControls=s_old_controls;
    _shortFire=0;
    _fire=0;
    _pl1Fire=0;
    s_clock_armed=reached;
    Expanded_ClockTrace_Sample(reached?"JOIN_FINISH_OK":"JOIN_FINISH_FAIL");
    fprintf(stderr,"[clock-audit] event=JOIN_FINISH result=%s target-phase=%u "
                   "actual-phase=%d native-phase=%u financial-phase=%u actions=%lu\n",
            reached?"REACHED":"ABORTED",(unsigned)s_target_key,key,
            (unsigned)Expanded_Career_NativeCalendarKey(),
            (unsigned)_lastFinancialWeekIndex,(unsigned long)s_actions);
    fprintf(stderr,"[midseason] %s: target_key=%u final_key=%d "
                   "elapsed_july_weeks=%lu menu_passes=%lu actions=%lu forced_sims=%lu league_exits=%lu\n",
            reached?"native next-fixture calendar reached":"catch-up aborted",
            (unsigned)s_target_key,key,
            (unsigned long)s_elapsed_weeks,(unsigned long)s_passes,
            (unsigned long)s_actions,(unsigned long)s_forced_simulations,
            (unsigned long)s_league_exits);
    fflush(stderr);
    if (reached && _gameType==4) {
        extern void Expanded_AutoCareer_Save(const char *);
        Expanded_AutoCareer_Save("native-join-complete");
    }
}

void Expanded_MidseasonJoin_Begin(uint32_t elapsed) {
    uint8_t cutoff=0;
    if (s_active) finish(0,-1);
    s_last_result=2; /* fail closed until a valid native handoff starts */
    s_last_key=-1;
    s_direct_league_import=0;
    s_complete_season_handoff=0;
    s_historical_nonleague_events=0;
    s_southern_handoff_status=0;
    /* Preserve this career's first clock failure until an explicit NEW
     * career; a second offer must not erase the evidence. */
    s_clock_armed=0;
    s_clock_previous_valid=0;
    s_clock_sample_cursor=0xffffu;
    s_clock_sample_phase=0xffu;
    uint16_t season=0;
    int southern_attempt=0;
    /* Archived seasons are final: only the current Living World season
     * may be imported/replayed into the live OG career. */
    if (_gameType!=4 ||
        !Expanded_MidseasonJoin_AbsoluteCutoff(World_FirstYear(),elapsed,
                                               &season,&cutoff)) {
        fprintf(stderr,"[midseason] no native catch-up for %lu July weeks "
                       "(current season preseason or unsupported date)\n",
                (unsigned long)elapsed);
        return;
    }
    if(cutoff==0u) {
        /* An early native schedule can have already played matches in July.
         * Never treat a zero northern-season cutoff as proof that the OG
         * table is in sync. Inspect the REAL selected OG league keycache. */
        uint8_t first_native=255u;
        size_t cursor=0;
        if(_teamSelectionKeyCacheCount>0u &&
           _teamSelectionKeyCacheCount<=256u) {
            for(uint16_t i=0;i<_teamSelectionKeyCacheCount;++i) {
                if(cursor>8192u-sizeof(TeamSelectionKeyCacheEntry)) break;
                const TeamSelectionKeyCacheEntry *e=
                    (const TeamSelectionKeyCacheEntry *)
                    (_teamSelectionKeyCache+cursor);
                if(e->entryStride<sizeof(*e) || e->entryStride>256u ||
                   e->entryStride>8192u-cursor) break;
                if(e->compSlot==0u && e->phaseFlag==0u &&
                   e->sortKey<first_native) first_native=e->sortKey;
                cursor+=e->entryStride;
            }
        }
        extern uint16_t _currentDivisionIndex;
        const uint8_t country=World_FileCountry(_careerTeam.team.countryNumber);
        ExpandedLiveRow rows[32];
        uint32_t count=0, played=0;
        const int available=ExpandedWorldLive_Table(
            country,(uint8_t)_currentDivisionIndex,rows,32u,&count);
        if(available)
            for(uint32_t i=0;i<count;++i) played+=rows[i].played;
        fprintf(stderr,"[midseason-southern] INSPECT week=%lu year=%u "
                       "country=%u division=%u native-first=%u "
                       "world-rows=%lu world-played-sum=%lu valid=%d\n",
                (unsigned long)elapsed,(unsigned)season,
                (unsigned)country,(unsigned)_currentDivisionIndex,
                (unsigned)first_native,(unsigned long)count,
                (unsigned long)played,available);
        if(first_native<56u && available && played>0u) {
            uint16_t actual_year=0;
            uint8_t actual_cutoff=0;
            southern_attempt=1;
            if(!Expanded_MidseasonJoin_SouthernCutoff(
                    World_FirstYear(),elapsed,first_native,
                    &actual_year,&actual_cutoff) ||
               actual_year!=season) {
                s_southern_handoff_status=2;
                s_last_key=-12;
                fprintf(stderr,"[midseason-southern] FAIL reason=CUTOFF "
                               "week=%lu first=%u world-played=%lu\n",
                    (unsigned long)elapsed,(unsigned)first_native,
                    (unsigned long)played);
                return;
            }
            cutoff=actual_cutoff;
            fprintf(stderr,"[midseason-southern] RECOVERY cutoff=%u "
                           "world-played=%lu native-first=%u\n",
                    (unsigned)cutoff,(unsigned long)played,
                    (unsigned)first_native);
        } else if(first_native<56u && !available) {
            s_southern_handoff_status=2;
            s_last_key=-13;
            fprintf(stderr,"[midseason-southern] FAIL reason=WORLD_TABLE_MISSING "
                           "country=%u division=%u\n",
                    (unsigned)country,(unsigned)_currentDivisionIndex);
            return;
        } else {
            s_last_result=3; /* No earlier recorded world fixtures. */
            return;
        }
    }
    extern uint16_t _currentSeasonShowing;
    const unsigned expected=(unsigned)season-(unsigned)World_FirstYear();
    if ((unsigned)_currentSeasonShowing!=expected) {
        fprintf(stderr,"[midseason] OG season mismatch: native=%u world=%u; "
                       "refusing historical cup replay on wrong year\n",
                (unsigned)_currentSeasonShowing,expected);
        return;
    }
    fprintf(stderr,"[clock-audit] event=JOIN_BEGIN season=%u phase=%u key=%u "
                   "financial-phase=%u cache-index=%u cache-count=%u elapsed-weeks=%lu\n",
            (unsigned)season,(unsigned)cutoff,
            (unsigned)Expanded_Career_NativeCalendarKey(),
            (unsigned)_lastFinancialWeekIndex,
            (unsigned)_teamSelectionKeyCacheIndex,
            (unsigned)_teamSelectionKeyCacheCount,
            (unsigned long)elapsed);
    fprintf(stderr,"[midseason] world season %u/%02u, absolute weeks %lu "
                   "current-season native phase=%u\n",
            (unsigned)season,((unsigned)season+1u)%100u,
            (unsigned long)elapsed,(unsigned)cutoff);
    /* Require an authentic future native fixture BEFORE arming. Without
     * one, the old fixture calendar could be consumed to season end. */
    if (!_teamSelectionKeyCacheCount || _teamSelectionKeyCacheCount>256u) {
        fprintf(stderr,"[midseason] no valid native keycache; leaving career unchanged\n");
        return;
    }
    /* The cache is a dedicated 8192-byte allocation (career_key_cache.c).
     * A valid stride by itself does NOT make 256 entries safe: their sum
     * can leave that allocation. Never inspect an out-of-bounds fixture. */
    size_t offset=0;
    int has_future=0;
    int has_past=0;
    uint8_t previous_key=0;
    for (uint16_t i=0;i<_teamSelectionKeyCacheCount;++i) {
        if (offset>8192u-sizeof(TeamSelectionKeyCacheEntry)) {
            fprintf(stderr,"[midseason] native cache truncated at entry %u; not arming\n",
                    (unsigned)i);
            return;
        }
        const TeamSelectionKeyCacheEntry *entry=
            (const TeamSelectionKeyCacheEntry *)(_teamSelectionKeyCache+offset);
        size_t stride=entry->entryStride;
        if (stride<sizeof(TeamSelectionKeyCacheEntry) || stride>256u ||
            stride>8192u-offset) {
            fprintf(stderr,"[midseason] bad native cache stride at entry %u; not arming\n",
                    (unsigned)i);
            return;
        }
        if (i && entry->sortKey<previous_key) {
            fprintf(stderr,"[midseason] unordered native calendar at entry %u; not arming\n",
                    (unsigned)i);
            return;
        }
        previous_key=entry->sortKey;
        if (entry->phaseFlag==0) {
            if (entry->sortKey>=cutoff) has_future=1;
            else has_past=1;
        }
        offset+=stride;
    }
    if (!has_past) {
        fprintf(stderr,"[midseason] no native fixtures before cutover %u; "
                       "nothing to catch up\n",(unsigned)cutoff);
        s_last_result=3;
        return;
    }
    if (!has_future) {
        /* An end-of-season job is legal, but never enter a freshly reset
         * zero-point league. Require a complete canonical world->OG table
         * import before advancing the genuine remaining cup callbacks. */
        fprintf(stderr,"[midseason] no future native fixture after phase %u; "
                       "testing completed-season world league import\n",
                (unsigned)cutoff);
    }
    /* 0.13.16: When the OG keycache has NO historical cup/other
     * competition events, we can import the WHOLE native league prefix
     * from the existing world score ledger and jump straight to the
     * first future original fixture. No competition simulation or
     * rendering after signing. If any old cup event exists, keep the
     * proven original catch-up: its brackets are not in the world
     * ledger yet and must NEVER be silently skipped. */
    unsigned oldNonLeague=0;
    size_t scan=0;
    for (uint16_t i=0;i<_teamSelectionKeyCacheCount;++i) {
        const TeamSelectionKeyCacheEntry *e=
            (const TeamSelectionKeyCacheEntry *)(_teamSelectionKeyCache+scan);
        if (e->phaseFlag==0 && e->sortKey<cutoff && e->compSlot!=0u)
            ++oldNonLeague;
        scan+=e->entryStride;
    }
    s_historical_nonleague_events=oldNonLeague;
    {
        extern int DIY_WorldDirectImportLeagueChecked(uint8_t cutoff,int strict);
        if (DIY_WorldDirectImportLeagueChecked(cutoff,southern_attempt)) {
            if(southern_attempt) {
                s_southern_handoff_status=1;
                fprintf(stderr,"[midseason-southern] PASS strict-world-table "
                               "season=%u cutoff=%u\n",
                        (unsigned)season,(unsigned)cutoff);
            }
            s_direct_league_import=1;
            /* Cup competitions are still OG and must continue chronologically.
             * Only a league-only history can bypass all replay callbacks. */
            if (!oldNonLeague) {
            size_t startOffset=0;
            for (uint16_t i=0;i<_teamSelectionKeyCacheCount;++i) {
                const TeamSelectionKeyCacheEntry *e=
                    (const TeamSelectionKeyCacheEntry *)
                    (_teamSelectionKeyCache+startOffset);
                if (e->phaseFlag==0 && e->sortKey>=cutoff) {
                    _teamSelectionKeyCacheIndex=i;
                    _teamSelectionKeyCacheOffset=(uint16_t)startOffset;
                    _lastFinancialWeekIndex=e->sortKey;
                    s_elapsed_weeks=elapsed;
                    s_target_key=cutoff;
                    s_last_key=e->sortKey;
                    s_last_result=3; /* Direct import, no catch-up started */
                    s_clock_armed=1;
                    Expanded_ClockTrace_Sample("DIRECT_IMPORT_HANDOFF");
                    fprintf(stderr,"[midseason] direct world->OG league state "
                                   "import: future phase %u, old cup events 0\n",
                            (unsigned)e->sortKey);
                    return;
                }
                startOffset+=e->entryStride;
            }
            /* Cannot normally happen: validated future native fixture. */
            }
        }
    }
    if(southern_attempt && !s_direct_league_import) {
        extern const char *DIY_WorldDirectImportStatus(void);
        s_southern_handoff_status=2;
        s_last_result=2;
        s_last_key=-14;
        fprintf(stderr,"[midseason-southern] FAIL reason=STRICT_NATIVE_IMPORT "
                       "country=%u cutoff=%u detail=%s; "
                       "not replaying incompatible scores\n",
                (unsigned)World_FileCountry(_careerTeam.team.countryNumber),
                (unsigned)cutoff,DIY_WorldDirectImportStatus());
        return;
    }
    if (!has_future && !s_direct_league_import) {
        fprintf(stderr,"[midseason] cannot enter completed season: "
                       "world/native league standings unavailable. "
                       "Refusing to label empty OG table as synced\n");
        s_last_result=2;
        s_last_key=-11;
        return;
    }
    s_complete_season_handoff=!has_future;
    if (oldNonLeague)
        fprintf(stderr,"[midseason] %u historical cup/other events need "
                       "OG replay: %s\n",oldNonLeague,
                s_direct_league_import?
                  "league imported, replay cups only":
                  "league/cup legacy fallback");
    s_elapsed_weeks=elapsed;
    s_target_key=cutoff;
    s_old_controls=_careerTeam.team.teamControls;
    s_passes=s_actions=s_forced_simulations=s_league_exits=0;
    s_active=1;
    s_last_result=0; /* Catch-up now owned by the original OG callbacks. */
    /* Keep the offered club HUMAN for fixture selection: making every team
     * AI here can cause the native "until human fixture" loops to consume
     * an entire competition in a single step. VIEW RESULT below simulates
     * the selected club's past fixture without launching match physics. */
    fprintf(stderr,"[midseason] begin: elapsed_july_weeks=%lu "
                   "first_future_native_key=%u (original league/cup fixtures)\n",
            (unsigned long)elapsed,(unsigned)cutoff);
    fflush(stderr);
}

void Expanded_MidseasonJoin_Tick(void) {
    if (!s_active) return;
    /* Sample only on actual cursor/phase changes, not every SDL frame.
     * The OG catch-up loop can execute hundreds of frames per second. */
    const uint16_t cursor=_teamSelectionKeyCacheIndex;
    const uint8_t phase=Expanded_Career_NativeCalendarKey();
    if(cursor!=s_clock_sample_cursor || phase!=s_clock_sample_phase) {
        Expanded_ClockTrace_Sample("OG_CATCHUP_EVENT");
        s_clock_sample_cursor=cursor;
        s_clock_sample_phase=phase;
    }
    if (++s_passes>6000u) { finish(0,-2); return; }
    /* This is not a user-driven menu chain. Ignore mouse/controller input
     * until history reaches the date of the signed contract. F12 remains
     * available through the SDL event pump for diagnostics. */
    _fire=0; _shortFire=0; _pl1Fire=0;
    _left=_right=_up=_down=0;
    _menuControlsDirection=_menuControlsDirection2=-1;
    MenuSystem_LatchSeasonSkip();
    const Menu *m=(const Menu *)_currentMenu;
    if (!m) { finish(0,-3); return; }
    if (m->onInit==ContestMenuInit) {
        if ((_careerFinished || _seasonFinished) &&
            s_complete_season_handoff && s_direct_league_import) {
            finish(1,(int)_lastFinancialWeekIndex);
            return;
        }
        if (_careerFinished || _seasonFinished ||
            !_teamSelectionKeyCacheCount ||
            _teamSelectionKeyCacheIndex>=_teamSelectionKeyCacheCount) {
            finish(0,-4);
            return;
        }
        const TeamSelectionKeyCacheEntry *entry=
            (const TeamSelectionKeyCacheEntry *)
              (_teamSelectionKeyCache+_teamSelectionKeyCacheOffset);
        if (_teamSelectionKeyCacheOffset>8192u-sizeof(*entry) ||
            entry->entryStride<sizeof(*entry) ||
            entry->entryStride>256u ||
            entry->entryStride>8192u-_teamSelectionKeyCacheOffset) {
            finish(0,-5); return;
        }
        if (s_passes<=4u || (s_passes%30u)==0u ||
            (unsigned)entry->sortKey!= (unsigned)s_last_key) {
            fprintf(stderr,"[midseason] native cursor: index=%u/%u phase=%u "
                           "target=%u finance_phase=%u actions=%lu\n",
                    (unsigned)_teamSelectionKeyCacheIndex,
                    (unsigned)_teamSelectionKeyCacheCount,
                    (unsigned)entry->sortKey,(unsigned)s_target_key,
                    (unsigned)_lastFinancialWeekIndex,
                    (unsigned long)s_actions);
            s_last_key=(int)entry->sortKey;
        }
        /* OG league state has already been imported from the canonical
         * world. Advance the ORIGINAL chronological keycache without
         * simulating its score a second time. Crucially, use the native
         * cursor function: it still applies its financial/CupD/day hooks
         * for every skipped league calendar event. Cups themselves are
         * processed by the unchanged menu callbacks below. */
        if (s_direct_league_import &&
            entry->compSlot==0u && entry->phaseFlag==1 &&
            entry->sortKey<s_target_key) {
            extern void Career_AdvanceToNextTeamSelectionKeyCacheEntry(void);
            Career_AdvanceToNextTeamSelectionKeyCacheEntry();
            ++s_actions;
            return;
        }
        /* Stop at the FIRST native fixture on or after contract time;
         * a phase key is an eighth of a month, not an elapsed week. */
        if (entry->sortKey>=s_target_key) {
            if (s_direct_league_import) ContestMenuInit();
            finish(1,entry->sortKey);
            return;
        }
        if (!choose_action((MenuEntryFn)NextMatch)) {
            finish(0,-6); return;
        }
        _shortFire=1;
        ++s_actions;
        return;
    }
    if(m->onInit==CareerOverInit) {
        /* The last original fixture has genuinely completed. Returning to
         * native season-end UI is correct for an offseason hire, provided
         * that its entire league table came from the canonical world. */
        if (s_complete_season_handoff && s_direct_league_import)
            finish(1,(int)_lastFinancialWeekIndex);
        else finish(0,-7);
        return;
    }
    if(m->onInit==DIYLeagueTournamentInit) {
        /* Follow the battle-tested golden career harness ordering:
         * NEXT MATCH must win over viewing a result that has already
         * been played; otherwise the same result screen can loop. */
        static const MenuEntryFn play[]={
            (MenuEntryFn)DIYLeagueTournamentNextMatch,
            DIYLeagueTournamentViewResult,
            DIYLeagueTournamentResult,
            DIYLeagueTournamentMatch
        };
        for(unsigned i=0;i<sizeof play/sizeof play[0];++i)
            if(choose_action(play[i])) {
                _shortFire=1;
                ++s_actions;
                return;
            }
        /* Once the native league round is fully simulated, the engine
         * hides NEXT MATCH, MATCH and RESULT and leaves only EXIT.
         * This is normal: the golden career harness exits this menu
         * before Career_NextMatch advances the key-cache cursor.
         * The old driver aborted (-8) at precisely this transition.
         * Use the real callback, never modify the result/fixture cursor
         * directly or press EXIT before attempting playable actions. */
        if (choose_action((MenuEntryFn)DIYLeagueTournamentExit)) {
            fprintf(stderr,
                    "[midseason] league round complete: EXIT -> native career calendar "
                    "index=%u/%u phase=%u target=%u\n",
                    (unsigned)_teamSelectionKeyCacheIndex,
                    (unsigned)_teamSelectionKeyCacheCount,
                    (unsigned)Expanded_Career_NativeCalendarKey(),
                    (unsigned)s_target_key);
            _shortFire=1;
            ++s_actions;
            ++s_league_exits;
            return;
        }
        fprintf(stderr,
                "[midseason] no league action (including EXIT): phase=%u target=%u\n",
                (unsigned)Expanded_Career_NativeCalendarKey(),
                (unsigned)s_target_key);
        finish(0,-8); return;
    }
    if(m->onInit==DIYCupInit) {
        /* Cup menus distinguish PLAY MATCH and VIEW RESULT just like
         * leagues. The native cup handler redirects every historical
         * match action to the CPU simulation path (mode 0). */
        static const MenuEntryFn cup[]={
            (MenuEntryFn)DIYCupOnSelect1,
            DIYCupOnSelect5, DIYCupOnSelect8, DIYCupOnSelect3,
            (MenuEntryFn)DIYCupOnSelect10
        };
        for (unsigned i=0;i<sizeof cup/sizeof cup[0];++i)
            if (choose_action(cup[i])) {
                _shortFire=1;
                ++s_actions;
                return;
            }
        finish(0,-10);
        return;
    }
    if(m->onInit==PlayMatchMenuInit) {
        /* A historical simulation may never launch a live match. */
        finish(0,-9);
        return;
    }
    /* Follow the original golden-career harness for secondary/cup
     * result dialogs. These use only normal onSelect callbacks.
     * Instrument unknown menus rather than silently asking for input. */
    if ((s_passes<=8u || (s_passes%100u)==0u))
        fprintf(stderr,"[midseason] fallback menu=%p onInit=%p pass=%lu\n",
                (void *)m,(void *)(uintptr_t)m->onInit,
                (unsigned long)s_passes);
    _shortFire=1;
    ++s_actions;
}

#else
int Expanded_MidseasonJoin_AlignSeason(uint32_t weeks) {(void)weeks;return 0;}
int Expanded_MidseasonJoin_Active(void) {return 0;}
int Expanded_MidseasonJoin_LastResult(void) {return 0;}
int Expanded_MidseasonJoin_LastKey(void) {return -1;}
uint32_t Expanded_MidseasonJoin_HistoricalCupEvents(void) {return 0;}
uint8_t Expanded_MidseasonJoin_TargetPhase(void) {return 0;}
extern uint8_t _lastFinancialWeekIndex;
uint8_t Expanded_Career_NativeCalendarKey(void) {return _lastFinancialWeekIndex;}
uint32_t Expanded_MidseasonJoin_Actions(void) {return 0;}
uint32_t Expanded_MidseasonJoin_LeagueExits(void) {return 0;}
uint32_t Expanded_MidseasonJoin_ForcedSimulations(void) {return 0;}
void Expanded_MidseasonJoin_RecordForcedSimulation(void) {}
void Expanded_MidseasonJoin_Begin(uint32_t weeks) {(void)weeks;}
void Expanded_MidseasonJoin_Tick(void) {}
#endif
