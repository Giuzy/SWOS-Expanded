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
#ifdef SWOS_SDL_BUILD
#include "expanded_debug_month.h"
#endif
#include "world.h"
#ifdef SWOS_SDL_BUILD
#include "expanded_world_live.h"
#include "career_status.h"
#include "expanded_bolivia_event_registry.h"
#endif

#include <stdio.h>
#include <string.h> /* read-only calendar audit initialization */

/* One date-to-OG-phase mapping for first-season and later hires. */
static int native_cutoff_at(uint16_t season,ExpandedWorldDate now,uint8_t *out) {
    ExpandedWorldDate august={season,8u,1u};
    int32_t today=0,first=0;
    if(!out || now.year<season || now.year>season+1u ||
       !ExpandedWorld_DateToIndex(now,&today) ||
       !ExpandedWorld_DateToIndex(august,&first)) return 0;
    /* No northern league games before season start; southern fixtures
     * are detected separately from their native keycache. */
    if(today<=first){*out=0;return 1;}
    /* ONE canonical date -> NEXT phase mapping, shared with all tests.
     * Never duplicate the eighth-of-month rounding formula here. */
    return ExpandedWorld_DateToNativePhaseNext(season,now,out);
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
static int southern_cutoff_at_date(uint16_t first_year,ExpandedWorldDate now,
                                  uint8_t earliest_league_key,
                                  uint16_t *season,uint8_t *cutoff) {
    uint8_t previous=0,phase=0;
    const uint16_t current=(uint16_t)(now.year-(now.month<7u?1u:0u));
    if(!season || !cutoff || earliest_league_key>=56u ||
       now.year<first_year || now.month!=7u ||
       !native_cutoff_at(current,now,&previous) || previous!=0u ||
       !ExpandedWorld_DateToNativePhaseNext(current,now,&phase) ||
       phase<=earliest_league_key) return 0;
    *season=current;*cutoff=phase;
    return 1;
}
int Expanded_MidseasonJoin_SouthernCutoff(uint16_t first_year,uint32_t weeks,
                                         uint8_t earliest_league_key,
                                         uint16_t *season,uint8_t *cutoff) {
    ExpandedWorldDate july={first_year,7u,1u},now;
    return ExpandedWorld_DateAfterWeeks(july,weeks,&now) &&
           southern_cutoff_at_date(first_year,now,earliest_league_key,
                                  season,cutoff);
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
/* The resident LWE civil day is authoritative. In particular, the
 * exact 5-year debug jump can land BETWEEN weekly tick boundaries. */
static int world_native_cutoff(uint32_t elapsed,uint16_t *season,
                               uint8_t *cutoff) {
    ExpandedLiveCalendarState cal;
    if(ExpandedWorldLive_CalendarState(&cal)) {
        const uint16_t year=(uint16_t)(cal.today.year-
                                         (cal.today.month<7u?1u:0u));
        if(!season || !cutoff || year<World_FirstYear() ||
           !native_cutoff_at(year,cal.today,cutoff))return 0;
        *season=year;return 1;
    }
    if(ExpandedWorldLive_Clubs()) {
        fprintf(stderr,"[calendar] FAIL LIVE_CLOCK_SNAPSHOT_UNAVAILABLE\n");
        return 0; /* Never guess a date from the rounded week counter. */
    }
    return Expanded_MidseasonJoin_AbsoluteCutoff(World_FirstYear(),elapsed,
                                                 season,cutoff);
}

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
/* The monthly debug button no longer uses the midseason VIEW RESULT
 * autopilot. Only signing a new contract drives that menu replay path. */

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
    ExpandedWorldDate next={0};
    int32_t world_day=0,next_day=0;
    uint16_t season;
    uint8_t phase=0,roundtrip=255u;
    const int32_t previous_next=s_clock_last_og;
    const int32_t previous_world=s_clock_last_world;
    const int was_valid=s_clock_previous_valid;
    if(!event)event="UNSPECIFIED";
    snprintf(s_clock_last_event,sizeof s_clock_last_event,"%s",event);
    if(!ExpandedWorldLive_CalendarState(&cal) ||
       !ExpandedWorld_DateToIndex(cal.today,&world_day))return;
    ++s_clock_samples;
    if(_gameType!=4 || _currentSeasonShowing>=100u) {
        if(!was_valid || world_day!=previous_world) {
            ++s_clock_changes;
            fprintf(stderr,"[clock-trace] event=%s mode=CIVIL_ONLY "
                           "civil=%04u-%02u-%02u civil-day=%ld\n",
                    event,(unsigned)cal.today.year,(unsigned)cal.today.month,
                    (unsigned)cal.today.day,(long)world_day);
        }
        s_clock_last_world=world_day;
        s_clock_previous_valid=0;
        return;
    }
    season=(uint16_t)(World_FirstYear()+_currentSeasonShowing);
    phase=Expanded_Career_NativeCalendarKey();
    if(phase==0u && cal.today.year==season && cal.today.month>=7u) {
        /* An uninitialized native key is NOT January's calendar date. */
        if(!was_valid || world_day!=previous_world) {
            ++s_clock_changes;
            fprintf(stderr,"[clock-trace] event=%s mode=NATIVE_PHASE_UNSET "
                           "civil=%04u-%02u-%02u next=UNKNOWN\n",
                    event,(unsigned)cal.today.year,(unsigned)cal.today.month,
                    (unsigned)cal.today.day);
        }
        s_clock_last_world=world_day;
        s_clock_previous_valid=0;
        return;
    }
    if(!ExpandedWorld_NativePhaseToDate(season,phase,&next) ||
       !ExpandedWorld_DateToIndex(next,&next_day) ||
       !ExpandedWorld_DateToNativePhaseFloor(season,next,&roundtrip)) {
        if(s_clock_armed && s_clock_status!=2) {
            s_clock_status=2;
            if(s_clock_first_problem[0]=='N') {
                snprintf(s_clock_first_problem,sizeof s_clock_first_problem,
                         "NATIVE_PHASE_INVALID at %s season=%u key=%u",
                         event,(unsigned)season,(unsigned)phase);
                fprintf(stderr,"[clock-trace-first-fail] %s\n",
                        s_clock_first_problem);
            }
        }
        return;
    }
    /* This delta means days UNTIL NEXT OG EVENT, not a civil-clock
     * mismatch: the native format represents eighths of months. */
    s_clock_delta=next_day-world_day;
    if(!was_valid || next_day!=previous_next || world_day!=previous_world) {
        ++s_clock_changes;
        fprintf(stderr,"[clock-trace] event=%s state=%s "
                       "civil=%04u-%02u-%02u next-og=%04u-%02u-%02u "
                       "season=%u native-phase=%u phase-back=%u "
                       "days-until-event=%ld civil-step=%ld next-step=%ld "
                       "sample=%lu change=%lu\n",
                event,s_clock_armed?"POST_HANDOFF":"RECOVERING",
                (unsigned)cal.today.year,(unsigned)cal.today.month,
                (unsigned)cal.today.day,
                (unsigned)next.year,(unsigned)next.month,(unsigned)next.day,
                (unsigned)season,(unsigned)phase,(unsigned)roundtrip,
                (long)s_clock_delta,
                (long)(was_valid?world_day-previous_world:0),
                (long)(was_valid?next_day-previous_next:0),
                (unsigned long)s_clock_samples,
                (unsigned long)s_clock_changes);
    }
    if(s_clock_armed) {
        const char *problem=NULL;
        if(roundtrip!=phase) problem="OG_PHASE_ROUNDTRIP";
        else if(was_valid && world_day<previous_world)
            problem="WORLD_DATE_REWIND";
        else if(was_valid && next_day<previous_next)
            problem="OG_NEXT_EVENT_REWIND";
        else if(next_day<world_day)
            problem="OG_NEXT_EVENT_OVERDUE";
        if(problem) {
            s_clock_status=2;
            if(s_clock_first_problem[0]=='N') {
                snprintf(s_clock_first_problem,sizeof s_clock_first_problem,
                         "%s at %s season=%u phase=%u "
                         "days-until-event=%ld civil-step=%ld og-step=%ld",
                         problem,event,(unsigned)season,(unsigned)phase,
                         (long)s_clock_delta,
                         (long)(was_valid?world_day-previous_world:0),
                         (long)(was_valid?next_day-previous_next:0));
                fprintf(stderr,"[clock-trace-first-fail] %s\n",
                        s_clock_first_problem);
            }
        } else if(!s_clock_status)s_clock_status=1; /* observed, not proven */
    }
    s_clock_last_og=next_day;
    s_clock_last_world=world_day;
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
    if (!world_native_cutoff(elapsed,&season,&cutoff)) {
        fprintf(stderr,"[clock-audit] event=ALIGN_FAILED first-year=%u elapsed-weeks=%lu\n",
                (unsigned)World_FirstYear(),(unsigned long)elapsed);
        return 0;
    }
    {
        ExpandedWorldDate nativeDate,worldDate;
        ExpandedWorldDate july={World_FirstYear(),7u,1u};
        int32_t nativeIndex=0,worldIndex=0;
        ExpandedLiveCalendarState live_now;
        const int from_world=ExpandedWorldLive_CalendarState(&live_now);
        if(ExpandedWorld_NativePhaseToDate(season,cutoff,&nativeDate) &&
           (from_world?(worldDate=live_now.today,1):
            ExpandedWorld_DateAfterWeeks(july,elapsed,&worldDate)) &&
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
    /* For OG-only careers the native OG competition state is canonical.
     * The old LWE roster staging was only required when transplanting a
     * separately simulated world. No LWE fixtures means no transplant. */
    if (!EXPANDED_WORLD_OG_ONLY && ExpandedWorldLive_Fixtures()>0u &&
        !ExpandedWorldLive_NativeMobilityPrepared()) {
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
    /* Native OG seasons begin at 96; offset 0 is not 1996. */
    _seasonPlaying=(uint8_t)((96u+offset)%100u);
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

/* A signed contract must not replay historical OG results. The original
 * key-cache remains the calendar source for match menus; start at the first
 * event on or after the actual world date. Validate all strides BEFORE
 * changing the cursor, so a corrupt cache is never partially consumed. */
/* Dry-run the native career keycache before touching the resident OG
 * world. The same scan is reused by first-hire positioning and the
 * calendar's direct-engine month skip. An empty future is legitimate:
 * the completed season must remain available for a MANUAL transition. */
static int expanded_og_calendar_plan(uint16_t year,ExpandedWorldDate now,
                                     uint8_t *phase,uint16_t *index,
                                     size_t *chosenOffset)
{
    uint8_t cutoff=0;
    if(!phase || !index || !chosenOffset ||
       !ExpandedWorld_DateToNativePhaseNext(year,now,&cutoff) ||
       !_teamSelectionKeyCacheCount || _teamSelectionKeyCacheCount>256u)
        return 0;
    size_t offset=0,selectedOffset=0,lastValidOffset=0;
    uint16_t selected=_teamSelectionKeyCacheCount;
    uint8_t previous=0;
    for(uint16_t i=0;i<_teamSelectionKeyCacheCount;++i) {
        if(offset>8192u-sizeof(TeamSelectionKeyCacheEntry))return 0;
        const TeamSelectionKeyCacheEntry *e=
            (const TeamSelectionKeyCacheEntry *)(_teamSelectionKeyCache+offset);
        if(e->entryStride<sizeof(*e) || e->entryStride>256u ||
           e->entryStride>8192u-offset || (i && e->sortKey<previous))
            return 0;
        if(selected==_teamSelectionKeyCacheCount &&
           e->phaseFlag==0u && e->sortKey>=cutoff) {
            selected=i;selectedOffset=offset;
        }
        lastValidOffset=offset;
        previous=e->sortKey;
        offset+=e->entryStride;
    }
    *phase=cutoff;*index=selected;
    /* A completed keycache points to its LAST VALID header, not byte 0
     * of an old season or the out-of-range end-of-buffer sentinel. */
    *chosenOffset=selected==_teamSelectionKeyCacheCount?
                  lastValidOffset:selectedOffset;
    return 1;
}
int Expanded_OGWorld_PreflightClubCalendar(uint16_t year,ExpandedWorldDate now,
                                            int *hasFuture)
{
    uint8_t phase=0;uint16_t index=0;size_t offset=0;
    if(!hasFuture ||
       !expanded_og_calendar_plan(year,now,&phase,&index,&offset))return 0;
    *hasFuture=index<_teamSelectionKeyCacheCount;
    return 1;
}
int Expanded_OGWorld_PositionClubCalendar(uint16_t year,ExpandedWorldDate now) {
    uint8_t phase=0;uint16_t chosenIndex=0;size_t chosenOffset=0;
    if(!expanded_og_calendar_plan(year,now,&phase,
                                  &chosenIndex,&chosenOffset))return 0;
    if(chosenIndex==_teamSelectionKeyCacheCount) {
        extern int Expanded_OGWorld_ManagerLeagueFinished(const DIYCompetitionEx *);
        extern DIYCompetitionEx _diyCompetitionStart;
        extern void Career_FillCurrentSeasonInfo(void);
        /* The absence of a future keycache row is NOT proof of a completed
         * championship. Verify BOTH native OG counters, or fail closed. */
        if(!Expanded_OGWorld_ManagerLeagueFinished(&_diyCompetitionStart)) {
            fprintf(stderr,"[og-calendar] INVALID_NO_FUTURE year=%u phase=%u "
                           "resident=NOT_CONFIRMED no-replay=1\n",
                    (unsigned)year,(unsigned)phase);
            return 0;
        }
        _teamSelectionKeyCacheIndex=(uint16_t)(_teamSelectionKeyCacheCount-1u);
        _teamSelectionKeyCacheOffset=(uint16_t)chosenOffset;
        _lastFinancialWeekIndex=phase;
        _seasonFinished=1;
        _careerFinished=1;
        Career_FillCurrentSeasonInfo(); /* the native season summary */
        fprintf(stderr,"[og-calendar] COMPLETED_BIND year=%u phase=%u "
                       "index=%u/%u resident=VERIFIED "
                       "next=MANUAL_FINISH_SEASON no-replay=1\n",
                (unsigned)year,(unsigned)phase,
                (unsigned)_teamSelectionKeyCacheIndex,
                (unsigned)_teamSelectionKeyCacheCount);
        return 2; /* legitimate completed calendar, not a failed bind */
    }
    _teamSelectionKeyCacheIndex=chosenIndex;
    _teamSelectionKeyCacheOffset=(uint16_t)chosenOffset;
    _lastFinancialWeekIndex=phase;
    fprintf(stderr,"[og-calendar] BOUND year=%u cutoff=%u "
                   "next-index=%u/%u next-phase=%u "
                   "HISTORICAL_REPLAY=NO\n",
            (unsigned)year,(unsigned)phase,(unsigned)chosenIndex,
            (unsigned)_teamSelectionKeyCacheCount,
            (unsigned)((TeamSelectionKeyCacheEntry *)
                (_teamSelectionKeyCache+chosenOffset))->sortKey);
    return 1;
}


int Expanded_OGWorld_ClubCalendarAudit(uint16_t year,ExpandedWorldDate now,
                                        ExpandedOGClubCalendarAudit *out)
{
    extern uint16_t _selTeamNumber;
    extern int Expanded_OGWorld_ClubBound(void);
    extern int Expanded_OGWorld_ManagerLeagueFinished(const DIYCompetitionEx *);
    extern DIYCompetitionEx _diyCompetitionStart;
    if(!out)return 0;
    memset(out,0,sizeof(*out));
    out->status=EXP_OG_CLUB_CALENDAR_ND;
    if(_gameType!=4 || _selTeamNumber==0xffffu ||
       !Expanded_OGWorld_ClubBound())return 1;
    uint16_t planned=0;
    size_t offset=0;
    uint8_t phase=0;
    out->total=_teamSelectionKeyCacheCount;
    out->current_index=_teamSelectionKeyCacheIndex;
    out->current_phase=Expanded_Career_NativeCalendarKey();
    if(!expanded_og_calendar_plan(year,now,&phase,&planned,&offset)) {
        out->status=EXP_OG_CLUB_CALENDAR_INVALID;
        return 1;
    }
    out->cutoff=phase;
    out->expected_index=planned;
    const int complete=Expanded_OGWorld_ManagerLeagueFinished(
        &_diyCompetitionStart);
    out->status=ExpandedOGClubCalendar_Classify(
        planned,out->total,out->current_index,1,complete);
    if(planned<out->total)
        out->next_phase=((const TeamSelectionKeyCacheEntry *)
                          (_teamSelectionKeyCache+offset))->sortKey;
    else
        out->next_phase=0xffu;
    return 1;
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
        !world_native_cutoff(elapsed,&season,&cutoff)) {
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
            ExpandedLiveCalendarState live_now;
            ExpandedWorldDate actual_date={World_FirstYear(),7u,1u};
            if(!ExpandedWorldLive_CalendarState(&live_now) ||
               !southern_cutoff_at_date(World_FirstYear(),live_now.today,
                                        first_native,&actual_year,
                                        &actual_cutoff) ||
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
        /* OG-only mode has no scored LWE fixtures. Never import a fake
         * table or replay a second score. The unchanged native callback
         * path below will advance every earlier OG match chronologically. */
        if (!EXPANDED_WORLD_OG_ONLY && ExpandedWorldLive_Fixtures()>0u &&
            DIY_WorldDirectImportLeagueChecked(cutoff,southern_attempt)) {
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
        /* In OG-only mode the club league has ALREADY been rebound to its
         * real resident table. It is illegal to replay all its old results.
         * A genuine completed native season needs NO legacy catch-up. */
        extern int Expanded_OGWorld_ClubBound(void);
        extern int Expanded_OGWorld_ManagerLeagueFinished(const DIYCompetitionEx *);
        extern DIYCompetitionEx _diyCompetitionStart;
        if(EXPANDED_WORLD_OG_ONLY && Expanded_OGWorld_ClubBound() &&
           Expanded_OGWorld_ManagerLeagueFinished(&_diyCompetitionStart)) {
            s_complete_season_handoff=1;
            s_last_result=3; /* verified native state, no catch-up needed */
            s_last_key=cutoff;
            fprintf(stderr,"[midseason] COMPLETED_RESIDENT_HANDOFF "
                           "season=%u phase=%u actions=0 "
                           "native-scores=ALREADY_APPLIED next=MANUAL_FINISH_SEASON\n",
                    (unsigned)season,(unsigned)cutoff);
            return;
        }
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


/* DEBUG +1 MONTH is a SINGLE engine transaction on the native menu
 * thread, NOT a replay of VIEW RESULT/NEXT MATCH or an SDL key injector.
 * The resident OG world alone plays the month's fixtures. Then its actual
 * league table is rebound to the manager and the local cursor moves past
 * the already-played native calendar events, without playing them again.
 * No artificial results, no change to the real season-boundary action. */
/* The civilian world continues through July even if the manager has no
 * OG fixture before August. Never simulate the manager's first kickoff in
 * the resident CPU world: cap the week to the day BEFORE that native key. */
static int exp_career_july_plan(ExpandedWorldDate *target,
                                ExpandedLiveCalendarState *live)
{
    extern uint16_t _selTeamNumber,_currentSeasonShowing;
    extern uint16_t _youreSacked;
    extern volatile uint16_t _isNationalSelector;
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    extern int Expanded_OGWorld_ClubBound(void);
    ExpandedWorldDate next;
    if(!target || !live || _gameType!=4 || s_active ||
       _selTeamNumber==0xffffu || _youreSacked || _isNationalSelector ||
       !Expanded_OGWorld_ClubBound() ||
       !ExpandedWorldLive_CalendarState(live))return 0;
    const uint16_t year=live->today.year;
    if(year!=(uint16_t)(World_FirstYear()+_currentSeasonShowing) ||
       year!=Expanded_OGWorld_SaveYear() ||
       !ExpandedWorld_NativePhaseToDate(
            year,Expanded_Career_NativeCalendarKey(),&next) ||
       !ExpandedWorld_CareerJulyForwardTarget(live->today,next,target))
        return 0;
    return 1;
}
void Expanded_BoliviaManagerPrepareMenu(void);
void Expanded_CareerJulyForward(MenuEntry *entry);
/* Preserve OG ContestSetButtons/PlayMatchMenu/NextMatch exactly as the
 * career menu defines them. The special July button rewrite was not requested
 * and must not disable PLAY MATCH or replace NEXT MATCH with FORWARD TIME.
 * Keep only the already-existing Bolivia match-date routing here. */
void Expanded_CareerJulyPrepareMenu(void)
{
    Expanded_BoliviaManagerPrepareMenu();
}
/* Compatibility entrypoint: the native OG menu is never intercepted for July.
 * Keeping the symbol avoids breaking any older external test harness. */
int Expanded_CareerJulyIntercept(MenuEntry *entry,unsigned ordinal)
{
    (void)entry;
    (void)ordinal;
    return 0;
}
void Expanded_CareerJulyForward(MenuEntry *entry)
{
    extern uintptr_t _currentMenuPtr;
    extern uint8_t _contestMenu[];
    extern uint16_t _selTeamNumber,_currentDivisionIndex,_fireResetFlag;
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    extern int Expanded_OGWorld_DebugCanAdvanceClub(uint16_t,uint8_t,uint16_t);
    extern int Expanded_CompetitionCoordinator_Advance(
        ExpandedWorldDate,const char *);
    extern void Expanded_AutoCareer_Save(const char *);
    extern void ContestMenuOnReturn(void);
    extern void DrawMenu(void);
    ExpandedLiveCalendarState live;
    ExpandedWorldDate target;
    int future=0;
    _fireResetFlag=1;
    const int menuOk=_currentMenuPtr==(uintptr_t)_contestMenu;
    const int planned=menuOk && exp_career_july_plan(&target,&live);
    const int calendarOk=planned && Expanded_OGWorld_PreflightClubCalendar(
        Expanded_OGWorld_SaveYear(),target,&future);
    const int residentOk=calendarOk && future &&
        Expanded_OGWorld_DebugCanAdvanceClub(
            _selTeamNumber,(uint8_t)_currentDivisionIndex,
            Expanded_OGWorld_SaveYear());
    if(!residentOk) {
        fprintf(stderr,"[career-july-forward] BLOCKED before-OG "
                       "menu=%d plan=%d calendar=%d future=%d "
                       "resident-exact=%d club=%04x next-key=%u "
                       "no-native-result=1\n",
                menuOk,planned,calendarOk,future,residentOk,
                (unsigned)_selTeamNumber,
                (unsigned)Expanded_Career_NativeCalendarKey());
        return;
    }
    /* This single coordinator advances only OTHER clubs' resident states.
     * Expanded_OGWorld_AdvanceLeague skips the bound manager league, and
     * the date is always strictly BEFORE the next local OG event. */
    if(!Expanded_CompetitionCoordinator_Advance(target,"CAREER_JULY_FORWARD")) {
        fprintf(stderr,"[career-july-forward] FAILED "
                       "prior-CAR-preserved=1 do-not-retry-partial=1\n");
        return;
    }
    fprintf(stderr,"[career-july-forward] COMMITTED "
            "world=%s from=%04u-%02u-%02u to=%04u-%02u-%02u "
            "next-manager-fixture=NOT_PLAYED\n",
            World_Tag(),(unsigned)live.today.year,
            (unsigned)live.today.month,(unsigned)live.today.day,
            (unsigned)target.year,(unsigned)target.month,
            (unsigned)target.day);
    ContestMenuOnReturn(); /* native rows + button eligibility */
    /* Focus stays on whichever REAL button the player used, until the
     * last safe July phase. At July 28 (when OG kickoff is August 1),
     * select native PLAY MATCH, never pretend there is another safe
     * July week or silently run an August result. */
    {
        ExpandedLiveCalendarState after;
        ExpandedWorldDate more;
        const int another=exp_career_july_plan(&more,&after);
        const uint16_t origin=(entry && entry->ordinal==10u)?10u:7u;
        SetCurrentEntry(another?origin:7u);
    }
    DrawMenu();
    Expanded_AutoCareer_Save("career-july-week");
}

/* RG-014. Before the legacy TSKC callback, compare the next real resident
 * Copa fixture with the currently actionable manager league fixture.
 * No calendar is authored here: native date / original OG pair only. */
/* Bolivia-only civil pilot: the world registry schedules BOTH competitions.
 * The next OG Copa resident pair is evidence of identity, not a date gate;
 * the next Liga occurrence is counted from immutable OG score witnesses,
 * never from the legacy keycache/phase as a scheduling authority.
 * Only the true native OG match path is allowed to write a result. */
static int exp_bolivia_manager_plan(ExpandedBoliviaManagerFixture *pending,
                                   ExpandedLiveCalendarState *live,
                                   ExpandedWorldDate *target)
{
    extern uint16_t _selTeamNumber;
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    extern int Expanded_OGWorld_ClubBound(void);
    unsigned leaguePlayed=0u;
    if(!pending || !live || !target || _gameType!=4 ||
       _selTeamNumber==0xffffu || !Expanded_OGWorld_ClubBound() ||
       !Expanded_OGWorld_BoliviaManagerNext(pending) ||
       !ExpandedWorldLive_CalendarState(live))return 0;
    *target=(ExpandedWorldDate){pending->year,pending->month,pending->day};
    if(pending->year!=Expanded_OGWorld_SaveYear() ||
       !Expanded_OGWorld_BoliviaLeagueWitnessProgress(&leaguePlayed))
        return -2; /* unknown league occurrence: never bypass OG integrity */
    return ExpandedBolivia_ChooseNext(
        (uint8_t)World_Current(),pending->year,live->today,
        leaguePlayed,leaguePlayed<EXP_BOLIVIA_LEAGUE_ROUNDS,
        *target,pending->fixtureId,1);
}
/* A short-lived dispatch claim. The permanent completion evidence is
 * the existing OG witness written in the same atomic CAR as the game
 * itself; on CONTINUE the registry recovers by re-reading that journal.
 * This is NOT an independent persistent result writer or manual ACK. */
static struct {
    ExpandedBoliviaManagerFixture fixture;
    uint16_t club;
    unsigned before;
    uint8_t armed;
} s_expBoliviaLeagueClaim;
/* Called at the OUTER career menu boundary, never recursively from
 * inside OG's modal ShowMenu/50-Hz match loop. The date/priority are from
 * our civil event program; OG remains sole fixture/result executor.
 * No TSKC phase decides when the next DPN match occurs. */
int Expanded_BoliviaLeaguePreflight(void)
{
    extern uint16_t _selTeamNumber,_currentDivisionIndex;
    extern int Expanded_CompetitionCoordinator_Advance(
        ExpandedWorldDate,const char *);
    extern int Expanded_OGWorld_ClubBound(void);
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    if(_gameType!=4 || World_Current()!=WORLD_2627 ||
       (uint8_t)(_selTeamNumber>>8)!=45u)return 1;
    /* Only the top-division 16-club pilot has a certified event
     * template. Other Bolivia divisions retain their native OG path. */
    if(_currentDivisionIndex!=0u)return 1;
    if(_selTeamNumber==0xffffu ||
       !Expanded_OGWorld_ClubBound())return 0;
    ExpandedLiveCalendarState live;
    ExpandedBoliviaManagerFixture league[EXP_BOLIVIA_LEAGUE_ROUNDS];
    ExpandedBoliviaManagerFixture cup;
    unsigned played=0u;
    if(!ExpandedWorldLive_CalendarState(&live) ||
       !Expanded_OGWorld_BoliviaLeagueWitnessProgress(&played) ||
       Expanded_OGWorld_BoliviaLeagueFixtures(
           league,EXP_BOLIVIA_LEAGUE_ROUNDS)!=
               EXP_BOLIVIA_LEAGUE_ROUNDS)
        return 0; /* do not fall through to phase-driven legacy calendar */
    if(played>=EXP_BOLIVIA_LEAGUE_ROUNDS)return 1;
    const ExpandedBoliviaManagerFixture *const next=&league[played];
    const ExpandedWorldDate date={next->year,next->month,next->day};
    int32_t now=0,when=0;
    if(!ExpandedWorld_DateToIndex(live.today,&now) ||
       !ExpandedWorld_DateToIndex(date,&when) || when<now) {
        fprintf(stderr,"[bolivia-event-preflight] BLOCK_OVERDUE "
                       "round=%u witness=%u\n",
                (unsigned)next->serial,played);
        return 0;
    }
    /* A Copa earlier than the Liga date MUST be routed by the same
     * planner first; a same-day club collision is not a valid event. */
    if(Expanded_OGWorld_BoliviaManagerNext(&cup)) {
        const ExpandedWorldDate cupDate={cup.year,cup.month,cup.day};
        const int decision=ExpandedBolivia_ChooseNext(
            (uint8_t)World_Current(),next->year,live.today,
            played,1,cupDate,cup.fixtureId,1);
        if(decision!=0) {
            fprintf(stderr,"[bolivia-event-preflight] BLOCK_ORDER "
                           "decision=%d league-round=%u cup-fixture=%u\n",
                    decision,played,(unsigned)cup.fixtureId);
            return 0;
        }
    }
    if(now<when &&
       !Expanded_CompetitionCoordinator_Advance(
           date,"CAREER_BOLIVIA_LEAGUE_EVENT"))return 0;
    ExpandedLiveCalendarState after;
    int32_t observed=0;
    if(!ExpandedWorldLive_CalendarState(&after) ||
       !ExpandedWorld_DateToIndex(after.today,&observed) ||
       observed!=when)return 0;
    s_expBoliviaLeagueClaim.fixture=*next;
    s_expBoliviaLeagueClaim.club=_selTeamNumber;
    s_expBoliviaLeagueClaim.before=played;
    s_expBoliviaLeagueClaim.armed=1u;
    fprintf(stderr,"[bolivia-event-preflight] LEAGUE_DUE "
           "year=%u fixture=%u round=%u date=%04u-%02u-%02u "
           "scheduler=CIVIL_ONLY native=OG_MENU\n",
           (unsigned)next->year,(unsigned)next->fixtureId,
           (unsigned)next->serial,(unsigned)date.year,
           (unsigned)date.month,(unsigned)date.day);
    return 1;
}
/* The VISUAL focus must follow the exact same chronological plan that
 * routes PLAY/SIMULATE to OG. No duplicate date comparison, no TSKC writes. */
int Expanded_BoliviaManagerActionableCup(ExpandedBoliviaManagerFixture *out)
{
    ExpandedLiveCalendarState live;
    ExpandedWorldDate target;
    if(!out)return 0;
    return exp_bolivia_manager_plan(out,&live,&target);
}
/* Update ONLY a real career action caption/callback. This does not install
 * a fake TSKC fixture, alter hitboxes or introduce a tournament simulator. */
void Expanded_BoliviaManagerPrepareMenu(void)
{
    extern uintptr_t _currentMenuPtr;
    extern uint8_t _contestMenu[];
    extern MenuEntry _menuEntries[];
    extern void ContestSetButtons(void);
    extern void NextMatch(MenuEntry *);
    /* Same original button captions for LEAGUE and CUP; only OG routing
     * changes, never the menu callback or the visible action name. */
    static const char kPlayCup[]="PLAY MATCH";
    static const char kNextCup[]="SIMULATE MATCH";
    static const char kClash[]="CUP DATE CONFLICT";
    static const char kLate[]="CUP FIXTURE OVERDUE";
    static int wasCup;
    ExpandedBoliviaManagerFixture m;
    ExpandedLiveCalendarState live;
    ExpandedWorldDate date;
    if(_gameType!=4 || _currentMenuPtr!=(uintptr_t)_contestMenu)return;
    const int plan=exp_bolivia_manager_plan(&m,&live,&date);
    MenuEntry *primary=&_menuEntries[7];
    MenuEntry *next=&_menuEntries[10];
    if(plan!=0) {
        const char *caption=plan==1?kPlayCup:
                            plan==-1?kClash:kLate;
        const char *nextCaption=plan==1?kNextCup:caption;
        primary->fg=(uintptr_t)caption;
        primary->isInvisible=0;
        primary->isDisabled=plan!=1;
        next->fg=(uintptr_t)nextCaption;
        next->isInvisible=0;
        next->isDisabled=plan!=1;
        next->onSelect=(MenuEntryFn)NextMatch;
        wasCup=1;
    } else {
        unsigned leaguePlayed=0u;
        ExpandedBoliviaManagerFixture league[EXP_BOLIVIA_LEAGUE_ROUNDS];
        const int isBolivia=_gameType==4 &&
            World_Current()==WORLD_2627 &&
            (uint8_t)(_selTeamNumber>>8)==45u;
        const int leagueReady=isBolivia &&
            Expanded_OGWorld_BoliviaLeagueWitnessProgress(
                &leaguePlayed) &&
            leaguePlayed<EXP_BOLIVIA_LEAGUE_ROUNDS &&
            Expanded_OGWorld_BoliviaLeagueFixtures(league,
                EXP_BOLIVIA_LEAGUE_ROUNDS)==EXP_BOLIVIA_LEAGUE_ROUNDS;
        if(leagueReady) {
            /* Button eligibility comes from our civil event/OG witness
             * projection rather than TSKC sortKey. Actual PLAY/SIM
             * continues via the unchanged original OG callbacks. */
            primary->fg=(uintptr_t)kPlayCup;
            primary->isInvisible=0;
            primary->isDisabled=0;
            next->fg=(uintptr_t)kNextCup;
            next->isInvisible=0;
            next->isDisabled=0;
            next->onSelect=(MenuEntryFn)NextMatch;
            wasCup=1;
        } else if(wasCup) {
            ExpandedLiveCalendarState current;
            ExpandedWorldDate future;
            /* Don't undo the genuine preseason FORWARD action. */
            if(!exp_career_july_plan(&future,&current))
                ContestSetButtons();
            wasCup=0;
        }
    }
}
/* This is the actual original OG PLAY MATCH route; NEXT MATCH and
 * FINISH SEASON enter the same function before touching the legacy cache.
 * Canceled match: no witness, no result and NO post-match autosave. */
int Expanded_BoliviaManagerIntercept(MenuEntry *entry,unsigned ordinal)
{
    extern uintptr_t _currentMenuPtr;
    extern uint8_t _contestMenu[];
    extern uint16_t _fireResetFlag;
    extern void ContestMenuOnReturn(void);
    extern void DrawMenu(void);
    extern void Expanded_AutoCareer_Save(const char *);
    extern int Expanded_CompetitionCoordinator_Advance(
        ExpandedWorldDate,const char *);
    extern int Expanded_HandleSackingAfterMatch(void);
    ExpandedBoliviaManagerFixture m;
    ExpandedLiveCalendarState live;
    ExpandedWorldDate date;
    int32_t oldDay=0,eventDay=0;
    if((ordinal!=7u && ordinal!=10u) ||
       _currentMenuPtr!=(uintptr_t)_contestMenu || _gameType!=4)return 0;
    const int plan=exp_bolivia_manager_plan(&m,&live,&date);
    if(plan==0)return 0;
    _fireResetFlag=1;
    if(plan<0) {
        fprintf(stderr,"[og-bolivia-manager] BLOCK calendar-integrity "
            "reason=%s fixture=%u date=%04u-%02u-%02u "
            "no-two-matches-in-day no-replay=1\n",
            plan==-1?"SAME_DAY_UNCERTIFIED":"OVERDUE",
            (unsigned)m.fixtureId,(unsigned)date.year,
            (unsigned)date.month,(unsigned)date.day);
        return 1;
    }
    if(!ExpandedWorld_DateToIndex(live.today,&oldDay) ||
       !ExpandedWorld_DateToIndex(date,&eventDay))return 1;
    if(oldDay<eventDay &&
       !Expanded_CompetitionCoordinator_Advance(
            date,"CAREER_BOLIVIA_MANAGER_FIXTURE")) {
        fprintf(stderr,"[og-bolivia-manager] CLOCK_PRECHECK_FAILED "
            "fixture=%u no-game=1 prior-CAR-preserved=1\n",
            (unsigned)m.fixtureId);
        return 1;
    }
    /* Ordinal 7 = manual, ordinal 10 = OG native score simulation.
     * Both consume precisely the same dated resident fixture. */
    if(!Expanded_OGWorld_BoliviaManagerPlay(&m,ordinal==10u)) {
        fprintf(stderr,"[og-bolivia-manager] NO_NATIVE_COMMIT "
            "fixture=%u canceled-or-fault=1 no-postmatch-save=1\n",
            (unsigned)m.fixtureId);
        return 1;
    }
    UpdateCareerAfterMatch();
    if(Expanded_HandleSackingAfterMatch())return 1;
    ContestMenuOnReturn();
    SetCurrentEntry(entry && entry->ordinal==10u?10u:7u);
    DrawMenu();
    Expanded_AutoCareer_Save("bolivia-manager-og");
    return 1;
}
void Expanded_CareerSyncWorldAfterLeagueMatch(uint8_t phase,
                                               unsigned playedBefore)
{
    extern DIYCompetitionEx _diyCompetitionStart;
    extern uint16_t _currentSeasonShowing,_selTeamNumber;
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    extern int Expanded_OGWorld_ClubBound(void);
    extern int Expanded_OGWorld_PreflightLocalLeague(const DIYCompetitionEx *);
    extern int Expanded_CompetitionCoordinator_Advance(
        ExpandedWorldDate,const char *);
    ExpandedLiveCalendarState live;
    ExpandedWorldDate fixture;
    if(_gameType==4 && World_Current()==WORLD_2627 &&
       _selTeamNumber!=0xffffu && (uint8_t)(_selTeamNumber>>8)==45u &&
       _currentDivisionIndex==0u) {
        /* Native phases are execution internals, NOT a scheduling
         * authority for this pilot. The coordinator already advanced
         * to the civil event BEFORE the original OG menu opened. */
        const ExpandedBoliviaManagerFixture expected=
            s_expBoliviaLeagueClaim.fixture;
        unsigned witnesses=0u;
        uint8_t gh=0u,ga=0u;
        const int progress=Expanded_OGWorld_BoliviaLeagueWitnessProgress(
            &witnesses);
        const int score=s_expBoliviaLeagueClaim.armed?
            Expanded_OGWorld_BoliviaLeagueScore(&expected,&gh,&ga):-1;
        if(!s_expBoliviaLeagueClaim.armed ||
           s_expBoliviaLeagueClaim.club!=_selTeamNumber || !progress ||
           witnesses<s_expBoliviaLeagueClaim.before ||
           witnesses>s_expBoliviaLeagueClaim.before+1u ||
           (witnesses==s_expBoliviaLeagueClaim.before+1u && score!=1) ||
           (witnesses==s_expBoliviaLeagueClaim.before && score==1)) {
            extern void Expanded_CompetitionCoordinator_ReportFixtureFault(
                const char *);
            Expanded_CompetitionCoordinator_ReportFixtureFault(
                "BOLIVIA_LEAGUE_EVENT_OG_WITNESS_MISMATCH");
            fprintf(stderr,"[bolivia-event-postflight] REJECT "
                   "claim=%u before=%u after=%u score=%d no-CAR=1\n",
                   (unsigned)s_expBoliviaLeagueClaim.armed,
                   s_expBoliviaLeagueClaim.before,witnesses,score);
        } else {
            fprintf(stderr,"[bolivia-event-postflight] OG_WITNESS "
                   "fixture=%u before=%u after=%u result=%s "
                   "event-ack=DERIVE_FROM_OG_CAR\n",
                   (unsigned)expected.fixtureId,
                   s_expBoliviaLeagueClaim.before,witnesses,
                   witnesses>s_expBoliviaLeagueClaim.before?
                       "OG_PLAYED":"CANCELLED_NO_RESULT");
        }
        s_expBoliviaLeagueClaim.armed=0u;
        return;
    }
    if(_gameType!=4 || _selTeamNumber==0xffffu ||
       _diyCompetitionStart.base.competitionType!=0) return;
    if(!Expanded_OGWorld_ClubBound() ||
       _diyCompetitionStart.base.competitionType!=0 ||
       _diyCompetitionStart.matchesPlayedCount<=playedBefore ||
       !ExpandedWorldLive_CalendarState(&live))return;
    const uint16_t season=(uint16_t)(World_FirstYear()+
                                     _currentSeasonShowing);
    int32_t a=0,b=0;
    if(season!=Expanded_OGWorld_SaveYear() ||
       !ExpandedWorld_NativePhaseToDate(season,phase,&fixture) ||
       !ExpandedWorld_DateToIndex(live.today,&a) ||
       !ExpandedWorld_DateToIndex(fixture,&b) || b<=a ||
       !Expanded_OGWorld_PreflightLocalLeague(&_diyCompetitionStart))return;
    if(!Expanded_CompetitionCoordinator_Advance(fixture,"CAREER_NATIVE_MATCH")) {
        fprintf(stderr,"[career-match-clock] FAILED "
                       "source=OG_LEAGUE prior-CAR-preserved=1\n");
        return;
    }
    fprintf(stderr,"[career-match-clock] COMMITTED "
                   "world=%s phase=%u from=%04u-%02u-%02u "
                   "to=%04u-%02u-%02u native-score=UNCHANGED\n",
            World_Tag(),(unsigned)phase,(unsigned)live.today.year,
            (unsigned)live.today.month,(unsigned)live.today.day,
            (unsigned)fixture.year,(unsigned)fixture.month,
            (unsigned)fixture.day);
}
void Expanded_CareerDebugMonthStart(MenuEntry *unused) {
    extern uintptr_t _currentMenuPtr;
    extern uint8_t _contestMenu[];
    extern uint16_t _selTeamNumber,_currentDivisionIndex,_currentSeasonShowing;
    extern uint16_t _fireResetFlag;
    extern int CareerAutopilot_Active(void);
    extern int Expanded_OGWorld_DebugCanAdvanceClub(uint16_t,uint8_t,uint16_t);
    extern int Expanded_OGWorld_BindClub(uint16_t,uint8_t,uint16_t);
    extern int Expanded_OGWorld_AdvanceToDate(ExpandedWorldDate);
    extern int Expanded_OGWorld_ClubBound(void);
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    extern void Expanded_AutoCareer_Save(const char *);
    extern void ContestMenuOnReturn(void);
    extern void DrawMenu(void);
    extern void Career_FillCurrentSeasonInfo(void);
    ExpandedLiveCalendarState live;
    ExpandedWorldDate target;
    uint8_t cutoff=0;
    uint16_t season=0;
    int hasFuture=0;
    (void)unused;
    _fireResetFlag=1;
    if(_gameType!=4 || _currentMenuPtr!=(uintptr_t)_contestMenu ||
       s_active || CareerAutopilot_Active() ||
       _selTeamNumber==0xffffu || !Expanded_OGWorld_ClubBound() ||
       !ExpandedWorldLive_CalendarState(&live)) {
        fprintf(stderr,"[debug-month-career] BLOCKED invalid menu, "
                       "world or manager binding\n");
        return;
    }
    if(!ExpandedDebug_NextMonthBeforeSeason(live.today,&target)) {
        fprintf(stderr,"[debug-month-career] STOP_BEFORE_SEASON_CHANGE "
                       "date=%04u-%02u-%02u (regular FINISH SEASON required)\n",
                (unsigned)live.today.year,(unsigned)live.today.month,
                (unsigned)live.today.day);
        return;
    }
    season=(uint16_t)(live.today.year-(live.today.month<7u?1u:0u));
    if(season!=(uint16_t)(World_FirstYear()+_currentSeasonShowing) ||
       season!=Expanded_OGWorld_SaveYear() ||
       !ExpandedWorld_DateToNativePhaseNext(season,target,&cutoff) ||
       cutoff<48u || cutoff>144u ||
       !Expanded_OGWorld_PreflightClubCalendar(season,target,&hasFuture) ||
       !Expanded_OGWorld_DebugCanAdvanceClub(
           _selTeamNumber,(uint8_t)_currentDivisionIndex,season)) {
        fprintf(stderr,"[debug-month-career] BLOCKED preflight "
                       "season=%u OG-year=%u native-index=%u "
                       "target=%04u-%02u-%02u cutoff=%u cache=%u "
                       "bound=%d (NO MUTATION)\n",
                (unsigned)season,(unsigned)Expanded_OGWorld_SaveYear(),
                (unsigned)_currentSeasonShowing,(unsigned)target.year,
                (unsigned)target.month,(unsigned)target.day,
                (unsigned)cutoff,(unsigned)_teamSelectionKeyCacheCount,
                Expanded_OGWorld_ClubBound());
        return;
    }
    fprintf(stderr,"[debug-month-career] ENGINE_BEGIN "
                   "from=%04u-%02u-%02u target=%04u-%02u-%02u "
                   "phase=%u future=%d mode=OG_RESIDENT_ONLY\n",
            (unsigned)live.today.year,(unsigned)live.today.month,
            (unsigned)live.today.day,(unsigned)target.year,
            (unsigned)target.month,(unsigned)target.day,
            (unsigned)cutoff,hasFuture);
    /* This call owns every OG fixture, including the managed club's league.
     * No VIEW RESULT, no CPU-control flag changes, no direct scores written. */
    if(!Expanded_OGWorld_AdvanceToDate(target)) {
        fprintf(stderr,"[debug-month-career] FAILED OG_WORLD_TICK "
                       "target=%04u-%02u-%02u "
                       "date=NOT_COMMITTED do-not-save-partial-world\n",
                (unsigned)target.year,(unsigned)target.month,(unsigned)target.day);
        return;
    }
    /* The resident OG standings and actual score counters are imported
     * into the original career league slot, not recomputed by a second
     * manager-local simulator. The local keycache's historical per-match
     * score fields still lack a complete resident fixture-ID bridge. */
    if(!Expanded_OGWorld_BindClub(
            _selTeamNumber,(uint8_t)_currentDivisionIndex,season)) {
        fprintf(stderr,"[debug-month-career] FAILED OG_CLUB_REBIND "
                       "date=NOT_COMMITTED do-not-save-partial-world\n");
        return;
    }
    if(hasFuture) {
        if(!Expanded_OGWorld_PositionClubCalendar(season,target)) {
            fprintf(stderr,"[debug-month-career] FAILED CURSOR_REPOSITION "
                           "date=NOT_COMMITTED do-not-save-partial-world\n");
            return;
        }
    } else {
        /* OG resident reached the last local calendar event before 1 July.
         * Do not automatically run ContinueCareerNextSeason. The player
         * triggers the original FINISH SEASON button to cross this boundary. */
        _careerFinished=1;
        _seasonFinished=1;
        _lastFinancialWeekIndex=cutoff;
        Career_FillCurrentSeasonInfo();
        fprintf(stderr,"[debug-month-career] NATIVE_CALENDAR_COMPLETE "
                       "next=MANUAL_FINISH_SEASON (NO AUTOMATIC ROLLOVER)\n");
    }
    if(!ExpandedWorldLive_AdvanceToDate(target)) {
        fprintf(stderr,"[debug-month-career] FAILED CIVIL_CLOCK_COMMIT "
                       "date=NOT_COMMITTED do-not-save-partial-world\n");
        return;
    }
    ContestMenuOnReturn();
    SetCurrentEntry(13);
    DrawMenu();
    Expanded_AutoCareer_Save("debug-career-month-engine");
    fprintf(stderr,"[debug-month-career] ENGINE_COMMITTED "
                   "date=%04u-%02u-%02u phase=%u future=%d "
                   "VIEW_RESULT=NOT_CALLED "
                   "local-fixture-scores=NOT_RECONCILED "
                   "financial-ticks=NOT_REPLAYED\n",
            (unsigned)target.year,(unsigned)target.month,
            (unsigned)target.day,(unsigned)cutoff,hasFuture);
    _fireResetFlag=1;
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
