/* Exact registered fixture -> original OG fast/manual 50 Hz -> atomic CAR.
 * No historical OG keycache, standings, cups or second simulator.
 * Caller supplies long-lived rosters and an already-current civil day.
 * Until a verified fixture importer/manager+CPU caller is installed this
 * is an explicit opt-in function, never a replacement for legacy menus.
 */
#ifndef EXPANDED_UNIVERSAL_CAREER_DISPATCH_H
#define EXPANDED_UNIVERSAL_CAREER_DISPATCH_H
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_native_match.h"
#include "expanded_universal_car.h"

/* Returns one EXP_UNIVERSAL_* status. A cancelled lineup is NO_CLAIM,
 * while any fault AFTER CAR claim latches RECOVERY and prevents replay.
 */
ExpandedUniversalResult ExpandedUniversal_CareerExecuteNext(
    int32_t target_day,ExpandedUniversalNativeMatch *match);
#endif
#endif
