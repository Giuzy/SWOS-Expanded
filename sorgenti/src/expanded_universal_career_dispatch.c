/* One callable, opt-in chain: canonical event -> OG 50 Hz -> CAR.
 * A data importer must first populate the SAME per-world CarRegistry, and
 * the universal civil dispatcher must position the one clock on matchday.
 * No Bolivia branch, no TSKC cache, no old DIY result/competition writer.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_career_dispatch.h"

ExpandedUniversalResult ExpandedUniversal_CareerExecuteNext(
    int32_t target_day,ExpandedUniversalNativeMatch *match)
{
    ExpandedUniversalRegistry *const r=ExpandedUniversal_CarRegistry();
    const ExpandedUniversalFixture *due=NULL;
    ExpandedUniversalResult ready;
    ExpandedUniversalOps ops;
    if(!r)return EXP_UNIVERSAL_EMPTY; /* no migrated career yet */
    if(!match || r->recovery_required)return EXP_UNIVERSAL_RECOVERY;
    /* The shared civil clock must ALREADY be on the fixture's actual
     * day; do not advance OG/legacy career calendar as a side effect
     * of clicking PLAY. The universal day dispatcher is the owner. */
    if(target_day!=r->scheduler.clock.day_index)
        return EXP_UNIVERSAL_BLOCKED;
    ready=ExpandedUniversal_Peek(r,target_day,&due);
    if(ready!=EXP_UNIVERSAL_DUE)return ready;
    if(!due || due->day_index!=target_day)return EXP_UNIVERSAL_BLOCKED;
    /* Lineup cancellation must occur BEFORE CAR CLAIMED. This does not
     * call the 50 Hz engine or write a score/journal entry. */
    if(!ExpandedUniversal_NativePrepare(match,due))
        return EXP_UNIVERSAL_BLOCKED;
    ops.save_claim=ExpandedUniversal_CarSaveClaim;
    ops.run_match=ExpandedUniversal_NativeRunOriginal;
    ops.save_completion=ExpandedUniversal_CarSaveCompletion;
    ops.user=match;
    return ExpandedUniversal_RunDue(r,target_day,&ops);
}
#endif
