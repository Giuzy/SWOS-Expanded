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

/* InitCareer constructs one fresh OG season. Put its season counters in
 * the same WORLD season before importing any scores or replaying cups.
 * This does NOT synthesize mobility or archived cup brackets. */
int Expanded_MidseasonJoin_AlignSeason(uint32_t elapsed) {
    uint16_t season=0;
    uint8_t cutoff=0;
    extern uint16_t _currentSeasonShowing;
    extern uint8_t _seasonPlaying;
    if (!Expanded_MidseasonJoin_AbsoluteCutoff(World_FirstYear(),elapsed,
                                               &season,&cutoff)) return 0;
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
    s_active=0;
    s_last_result=reached?1:2;
    s_last_key=key;
    _careerTeam.team.teamControls=s_old_controls;
    _shortFire=0;
    _fire=0;
    _pl1Fire=0;
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
    uint16_t season=0;
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
        s_last_result=3; /* No past OG league/cup events yet. */
        return;
    }
    extern uint16_t _currentSeasonShowing;
    const unsigned expected=(unsigned)season-(unsigned)World_FirstYear();
    if ((unsigned)_currentSeasonShowing!=expected) {
        fprintf(stderr,"[midseason] OG season mismatch: native=%u world=%u; "
                       "refusing historical cup replay on wrong year\n",
                (unsigned)_currentSeasonShowing,expected);
        return;
    }
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
        extern int DIY_WorldDirectImportLeague(uint8_t cutoff);
        if (DIY_WorldDirectImportLeague(cutoff)) {
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
