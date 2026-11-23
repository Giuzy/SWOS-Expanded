/* SWOS Expanded — experimental WORLD2627 single-owner career dispatch.
 * JSON fixture registry is the calendar, OG quick-scores unattended CPU\n * and the manager's VIEW RESULT; interactive PLAY remains OG 50 Hz.
 * WORLD9697 and non-career game types use their original paths.
 */
#ifndef EXPANDED_UNIVERSAL_LIVE_H
#define EXPANDED_UNIVERSAL_LIVE_H
#ifdef SWOS_SDL_BUILD
#include "expanded_world_time.h"
int ExpandedUniversalLive_Start(void);
int ExpandedUniversalLive_Active(void);
int ExpandedUniversalLive_Advance(ExpandedWorldDate target); /* WAIT manager */
int ExpandedUniversalLive_DebugAdvance(ExpandedWorldDate target); /* quick-sim manager */
int ExpandedUniversalLive_PlayManager(void); /* only manual real OG 50 Hz */
int ExpandedUniversalLive_ViewManagerResult(void); /* OG fast, same CAR */
int ExpandedUniversalLive_CanRestoreRosters(void);
#endif
#endif
