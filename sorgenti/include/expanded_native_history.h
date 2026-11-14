#ifndef EXPANDED_NATIVE_HISTORY_H
#define EXPANDED_NATIVE_HISTORY_H
#include <stdint.h>
/* Match the NEXT original fixture, not the FIRST fixture with a date.
 * OG can place several player fixtures in the same 1/8-month phase,
 * especially in three/four-cycle domestic leagues. The history sequence
 * is in original match progression order. Fail closed if order diverges. */
static inline int ExpandedNativeHistory_ClaimNext(const uint8_t *phases,
        unsigned count,unsigned *next,uint8_t native_phase,uint8_t *index)
{
    if(!phases || !next || !index || *next >= count || *next > 255u ||
       phases[*next] != native_phase) return 0;
    *index=(uint8_t)*next;
    ++*next;
    return 1;
}
#endif
