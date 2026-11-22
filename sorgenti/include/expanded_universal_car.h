/* Universal Competition Engine state carried inside the SAME career CAR.
 * No sidecar, no replacement for Zstandard or swos_io_save_atomic.
 * Dynamic per-world registry; absent/null field keeps legacy CAR compatible.
 */
#ifndef EXPANDED_UNIVERSAL_CAR_H
#define EXPANDED_UNIVERSAL_CAR_H
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_fixture_registry.h"
#include "mjson.h"

/* Register verified fixture INPUTS on the active world only; this cannot
 * start a second game, change the career calendar or invent a score. */
ExpandedUniversalRegistry *ExpandedUniversal_CarRegistry(void);
int ExpandedUniversal_CarBegin(ExpandedWorldDate date,uint32_t capacity);
void ExpandedUniversal_CarResetCurrent(void);

/* SaveJson_WriteCareer_Impl calls Emit before the closing JSON object.
 * The same prewrite validator, Zstd3 roundtrip and atomic writer remain
 * responsible for durability. Old CAR without this optional field is valid. */
int ExpandedUniversal_CarEmit(mjson_print_fn_t output,void *user);
/* Read-only integrity check BEFORE any legacy/OG world mutates on load. */
int ExpandedUniversal_CarValidate(const char *json,int size);
/* Apply only after the preflight checks passed (no replay). */
int ExpandedUniversal_CarRestore(const char *json,int size);

/* Real CAR callbacks for the generic dispatcher, not fake acknowledgements.
 * Claim = durable CLAIMED before original game; Complete = persist proposed
 * PLAYED+event COMPLETE+OG/RNG/state in exactly one existing AUTO*.CAR.
 * The in-memory proposal is restored until RunDue finishes its own ACK. */
int ExpandedUniversal_CarSaveClaim(void *user,
    const ExpandedUniversalRegistry *registry,
    const ExpandedUniversalFixture *fixture);
int ExpandedUniversal_CarSaveCompletion(void *user,
    const ExpandedUniversalRegistry *registry,
    const ExpandedUniversalFixture *fixture,
    const ExpandedUniversalCompletion *proposal);
#endif
#endif
