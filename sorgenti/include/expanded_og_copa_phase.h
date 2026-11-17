#ifndef EXPANDED_OG_COPA_PHASE_H
#define EXPANDED_OG_COPA_PHASE_H
/* The first historical Copa retains the original OpenSWOS type-2 record:
 * March..June = native phases 16..40. Expanded careers START in July
 * (world key 48), so those original keys are already in the past.
 * Do not replay them in July. Use the following March..June window of
 * the SAME 1996/97 career season; only the resident Copa's execution
 * cutoff is rebased, never OG's stored dates, clock or match engine.
 *
 * 1996 Jul-Dec: 48..95 -> 0 (no Copa games)
 * 1997 Jan-Feb: 96..111 -> 0..15 (before original phase 16)
 * 1997 Mar-Jun: 112..143 -> 16..47 (OG plays native 16..40)
 * LastPhase in the resident context remains the ACTUAL world key,
 * so repeated FORWARD TIME cannot advance a phase twice. */
#include <stdint.h>
static inline uint8_t Expanded_OGCopa1996_FixturePhase(uint8_t worldPhase)
{
    return worldPhase>=96u?(uint8_t)(worldPhase-96u):0u;
}
#endif
