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
int ExpandedUniversalLive_Advance(ExpandedWorldDate target); /* WAIT manager */
int ExpandedUniversalLive_DebugAdvance(ExpandedWorldDate target); /* quick-sim manager */
/* Readonly next fixture: -1 invalid, 0 none, 1 FORWARD, 2 PLAY due. */
int ExpandedUniversalLive_ManagerNext(ExpandedWorldDate *due);
/* 0 not owned, 1 handled/blocked, 2 actual OG + CAR completed. */
int ExpandedUniversalLive_PlayManager(void); /* only manual real OG 50 Hz */
int ExpandedUniversalLive_ViewManagerResult(void); /* OG fast, same CAR */
/* OG board VIEW RESULT of a CPU fixture in the manager's league round:
 * 0 not owned, 1 blocked, 2 OG quick result recorded (CPU RAM batch). */
int ExpandedUniversalLive_ViewRoundCpuResult(uint64_t event_id);
int ExpandedUniversalLive_CanRestoreRosters(void);
#endif
#endif
