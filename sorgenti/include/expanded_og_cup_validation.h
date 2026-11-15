/* SWOS Expanded — strict validation of an ORIGINAL OG cup with byes.
 * Numeric helper only: no RNG, no bracket edits, no synthetic fixtures.
 * The source of the rules remains World_Cups() / WorldCups_CupMade().
 */
#ifndef EXPANDED_OG_CUP_VALIDATION_H
#define EXPANDED_OG_CUP_VALIDATION_H
static inline int ExpandedOGCup_NativeByeLayout(
    unsigned bracket, unsigned selected, unsigned nativeReal,
    unsigned nativeRound1Ties, unsigned actualBracket,
    unsigned actualRound1Ties, unsigned actualPairs)
{
    /* A proper 2^n bracket has more places than real participants. */
    if (bracket < 4u || bracket > 64u || (bracket & (bracket-1u)))
        return 0;
    if (selected < 2u || selected >= bracket || selected != nativeReal)
        return 0;
    if (!nativeRound1Ties || nativeRound1Ties*2u > selected)
        return 0;
    /* All real non-playing entrants must be byes, not missing clubs. */
    if (selected - nativeRound1Ties*2u != bracket - selected)
        return 0;
    if (actualBracket != bracket || actualPairs != bracket/2u ||
        actualRound1Ties != nativeRound1Ties)
        return 0;
    return 1;
}
#endif
