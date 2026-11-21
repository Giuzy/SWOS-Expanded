/* RG-001/DATE: Bolivia 2026 Division Profesional 30-round phase template.
 * These are ONE representative OG phase per round, NOT eight verified
 * civil fixture dates. Native phases are 1/8 of a month, so some dates
 * differ by 1-4 days; delayed real matches of rounds 6-10 cannot be
 * represented by one phase without a per-fixture historical import.
 * All entries are strictly increasing. No league round falls on any
 * 2026 Copa Bolivia group/KO civil match day; check globally in QA.
 * Subsequent years reuse this GAME_TEMPLATE_2026, never official dates.
 * Only the existing OG league progression table consumes these bytes.
 */
#ifndef BOLIVIA_LEAGUE_2026_PHASES_H
#define BOLIVIA_LEAGUE_2026_PHASES_H
#include <stdint.h>
enum { BOLIVIA_LEAGUE_ROUNDS=30 };
static const uint8_t kBoliviaLeague2026NativePhases[BOLIVIA_LEAGUE_ROUNDS]={
    24,26,29,30,32,34,35,36,  /* April 1 through May 16 */
    52,53,54,55,57,58,59,62,  /* July 16 through August 24 */
    63,65,67,68,75,76,78,79,  /* August 28 through October 28 */
    82,86,87,88,89,91          /* November 8 through December 12 */
};
static inline int BoliviaLeague2026_ValidatePhases(void) {
    if(kBoliviaLeague2026NativePhases[0]!=24u ||
       kBoliviaLeague2026NativePhases[29]!=91u)return 0;
    for(unsigned i=1;i<BOLIVIA_LEAGUE_ROUNDS;++i)
        if(kBoliviaLeague2026NativePhases[i]<=
           kBoliviaLeague2026NativePhases[i-1])return 0;
    return 1;
}
#endif
