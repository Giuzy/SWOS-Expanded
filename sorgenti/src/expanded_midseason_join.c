/*
 * 0.13.06 — cautious first-job native calendar catch-up.
 *
 * The prehire pilot is an independent round-robin, so its fixture index is
 * NOT interchangeable with the original SWOS league/cup calendar. We do not
 * patch league points or cursor bytes: the game plays the prior rounds through
 * the same menu handlers as its golden career regression harness.
 */
#include "expanded_midseason_join.h"
#include "expanded_world_time.h"
#include "world.h"

#include <stdio.h>

int Expanded_MidseasonJoin_Cutoff(uint16_t year,uint32_t weeks,uint8_t *out) {
    ExpandedWorldDate july={year,7,1}, august={year,8,1}, now;
    int32_t today=0,first=0;
    if (!out || weeks>=52 || !ExpandedWorld_DateAfterWeeks(july,weeks,&now) ||
        !ExpandedWorld_DateToIndex(now,&today) ||
        !ExpandedWorld_DateToIndex(august,&first)) return 0;
    if(today<=first) { *out=0; return 1; }
    uint32_t weeks_after_august=(uint32_t)(today-first+6)/7u;
    if (weeks_after_august>255u) return 0;
    *out=(uint8_t)weeks_after_august;
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

extern void ContestMenuInit(void);
extern void CareerOverInit(void);
extern void DIYLeagueTournamentInit(void);
extern void PlayMatchMenuInit(void);
extern void NextMatch(MenuEntry *);
extern void DIYLeagueTournamentNextMatch(void);
extern void DIYLeagueTournamentViewResult(MenuEntry *);
extern void DIYLeagueTournamentResult(MenuEntry *);
extern void MenuSystem_LatchSeasonSkip(void);
extern void SetCurrentEntry(uint16_t ordinal);
extern MenuEntry *CalcMenuEntryAddress(uint16_t ordinal);

static uint8_t s_active,s_old_controls,s_target_key;
static uint32_t s_passes,s_elapsed_weeks,s_actions;

int Expanded_MidseasonJoin_Active(void) { return s_active!=0; }

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
    _careerTeam.team.teamControls=s_old_controls;
    _shortFire=0;
    _fire=0;
    _pl1Fire=0;
    fprintf(stderr,"[midseason] %s: target_key=%u final_key=%d "
                   "elapsed_july_weeks=%lu menu_passes=%lu actions=%lu\n",
            reached?"native next-fixture calendar reached":"catch-up aborted",
            (unsigned)s_target_key,key,
            (unsigned long)s_elapsed_weeks,(unsigned long)s_passes,
            (unsigned long)s_actions);
    fflush(stderr);
}

void Expanded_MidseasonJoin_Begin(uint32_t elapsed) {
    uint8_t cutoff=0;
    if (s_active) finish(0,-1);
    /* End-of-year / multiple-season unemployment is not yet supported:
     * never auto-start from the wrong historical season. */
    if (_gameType!=4 ||
        !Expanded_MidseasonJoin_Cutoff(World_FirstYear(),elapsed,&cutoff) ||
        cutoff==0) {
        fprintf(stderr,"[midseason] no native catch-up for %lu July weeks "
                       "(preseason or unsupported year)\n",(unsigned long)elapsed);
        return;
    }
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
        return;
    }
    if (!has_future) {
        fprintf(stderr,"[midseason] no future competition fixture at native key %u; "
                       "leaving career unchanged\n",(unsigned)cutoff);
        return;
    }
    s_elapsed_weeks=elapsed;
    s_target_key=cutoff;
    s_old_controls=_careerTeam.team.teamControls;
    s_passes=s_actions=0;
    s_active=1;
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
    MenuSystem_LatchSeasonSkip();
    const Menu *m=(const Menu *)_currentMenu;
    if (!m) { finish(0,-3); return; }
    if (m->onInit==ContestMenuInit) {
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
        if (entry->sortKey>=s_target_key) {
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
        /* No implicit promotion/relegation rollover in catch-up mode. */
        finish(0,-7); return;
    }
    if(m->onInit==DIYLeagueTournamentInit) {
        static const MenuEntryFn play[]={
            DIYLeagueTournamentViewResult,
            DIYLeagueTournamentResult,
            (MenuEntryFn)DIYLeagueTournamentNextMatch
        };
        for(unsigned i=0;i<sizeof play/sizeof play[0];++i)
            if(choose_action(play[i])) {
                _shortFire=1;
                ++s_actions;
                return;
            }
        /* Never fall back to PLAY MATCH in a historical catch-up.
         * Only simulated results are permitted. */
        finish(0,-8);return;
    }
    if(m->onInit==PlayMatchMenuInit) {
        /* A historical simulation may never launch a live match. */
        finish(0,-9);
        return;
    }
    /* Follow the original golden-career harness for secondary/cup
     * result dialogs. These use only normal onSelect callbacks. */
    _shortFire=1;
    ++s_actions;
}

#else
int Expanded_MidseasonJoin_Active(void) {return 0;}
void Expanded_MidseasonJoin_Begin(uint32_t weeks) {(void)weeks;}
void Expanded_MidseasonJoin_Tick(void) {}
#endif
