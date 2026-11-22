/* SWOS Expanded — experimental WORLD2627 single-owner career dispatch.
 * JSON fixture registry is the calendar, OG only executes the 50Hz match.
 * WORLD9697 and non-career game types use their original paths.
 */
#ifndef EXPANDED_UNIVERSAL_LIVE_H
#define EXPANDED_UNIVERSAL_LIVE_H
#ifdef SWOS_SDL_BUILD
#include "expanded_world_time.h"
int ExpandedUniversalLive_Start(void);
int ExpandedUniversalLive_Active(void);
int ExpandedUniversalLive_Advance(ExpandedWorldDate target);
int ExpandedUniversalLive_PlayManager(void);
int ExpandedUniversalLive_CanRestoreRosters(void);
#endif
#endif
