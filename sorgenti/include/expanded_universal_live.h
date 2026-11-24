/* SWOS Expanded — experimental WORLD2627 single-owner career dispatch.
 * JSON fixture registry is the calendar, OG quick-scores unattended CPU\n * and the manager's VIEW RESULT; interactive PLAY remains OG 50 Hz.
 * WORLD9697 and non-career game types use their original paths.
 */
#ifndef EXPANDED_UNIVERSAL_LIVE_H
#define EXPANDED_UNIVERSAL_LIVE_H
#ifdef SWOS_SDL_BUILD
#include "expanded_world_time.h"
int ExpandedUniversalLive_Start(void);
/* Clear only transient, unsaved OG quick batch accounting on world restore. */
void ExpandedUniversalLive_ResetCpuBatch(void);
int ExpandedUniversalLive_Active(void);
/* v282: THE world clock of WORLD2627 (ExpandedUniversalRegistry.scheduler.
 * clock.day_index). 1 = CE active and date valid. All readers use this (or
 * the LWE mirror that follows it); nothing advances any other clock. */
int ExpandedUniversalLive_TodayIndex(int32_t *out);
int ExpandedUniversalLive_Today(ExpandedWorldDate *out);
/* Register the CE clock with the LWE mirror (explicit, no constructor). */
void ExpandedUniversalLive_RegisterClock(void);
int ExpandedUniversalLive_Advance(ExpandedWorldDate target); /* WAIT manager */
int ExpandedUniversalLive_DebugAdvance(ExpandedWorldDate target); /* quick-sim manager */
/* Readonly next fixture: -1 invalid, 0 none, 1 FORWARD, 2 PLAY due. */
int ExpandedUniversalLive_ManagerNext(ExpandedWorldDate *due);
/* Fase 1: 1 = WORLD2627 CE active and the bound (not sacked) manager club
 * is a CE catalog club. Club change = manager rebinding only. */
int ExpandedUniversalLive_ManagerBound(void);
/* Fase 2: Competition Engine. CONTINUE hook (after rosters restore): build
 * the definitions, decode the CAR lifecycle (or rebuild it for a pre-v296
 * CAR) and check every edition against the current rules. 1 ok. */
int ExpandedUniversalLive_EngineRestore(void);
/* OG native NAME tie-break (catalog club names). */
#include "expanded_competition_identity.h"
int ExpandedUniversalLive_ClubNameCompare(void *user,ExpandedClubKey a,
                                          ExpandedClubKey b);
/* 0 not owned, 1 handled/blocked, 2 actual OG + CAR completed. */
int ExpandedUniversalLive_PlayManager(void); /* only manual real OG 50 Hz */
int ExpandedUniversalLive_ViewManagerResult(void); /* OG fast, same CAR */
/* OG board VIEW RESULT of a CPU fixture in the manager's league round:
 * 0 not owned, 1 blocked, 2 OG quick result recorded (CPU RAM batch). */
int ExpandedUniversalLive_ViewRoundCpuResult(uint64_t event_id);
int ExpandedUniversalLive_CanRestoreRosters(void);
#endif
#endif
