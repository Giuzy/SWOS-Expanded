/* One canonical fixture -> original OG quick or manual 50 Hz -> same CAR.
 * A data importer must first populate the SAME per-world CarRegistry, and
 * the universal civil dispatcher must position the one clock on matchday.
 * No Bolivia branch, no TSKC cache, no old DIY result/competition writer.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_career_dispatch.h"

/* CPU quick matches may complete in RAM until a whole block is committed.
 * The standard manual dispatcher below still has a durable CLAIMED barrier.
 * An interrupted non-durable CPU block is replayed from the preceding CAR,
 * never from a half-written score/ledger. */
static int exp_batch_claim(void *user,const ExpandedUniversalRegistry *r,
                           const ExpandedUniversalFixture *f)
{
    ExpandedFixtureLedgerEntry *e;
    (void)user;
    e=ExpandedUniversal_EntryFor((ExpandedUniversalRegistry *)r,f);
    return e && e->state==EXP_FIXTURE_CLAIMED &&
        ExpandedFixtureKey_Equal(e->key,f->key);
}
static int exp_batch_completion(void *user,const ExpandedUniversalRegistry *r,
                                const ExpandedUniversalFixture *f,
                                const ExpandedUniversalCompletion *proposal)
{
    ExpandedFixtureLedgerEntry *e;
    (void)user;
    e=ExpandedUniversal_EntryFor((ExpandedUniversalRegistry *)r,f);
    return e && e->state==EXP_FIXTURE_CLAIMED &&
        proposal && proposal->witness.valid &&
        ExpandedFixtureKey_Equal(proposal->witness.key,f->key) &&
        ExpandedFixtureScore_Equal(proposal->witness.score,
                                   proposal->played.score) &&
        proposal->played.state==EXP_FIXTURE_PLAYED &&
        proposal->completed.id==f->event_id &&
        proposal->completed.state==EXP_EVENT_COMPLETE;
}


static ExpandedUniversalResult exp_execute_next(
    int32_t target_day,ExpandedUniversalNativeMatch *match,int cpu_batch)
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
    /* Manual lineup cancellation must occur BEFORE CAR CLAIMED. Quick
     * CPU results do not prompt a lineup and never enter GameLoop. */
    if(!ExpandedUniversal_NativePrepare(match,due))
        return EXP_UNIVERSAL_BLOCKED;
    /* Never batch an explicitly interactive match: its anti-save-scum
     * CLAIMED transaction must remain on disk before the 50 Hz runner. */
    ops.save_claim=cpu_batch && !match->interactive?
        exp_batch_claim:ExpandedUniversal_CarSaveClaim;
    ops.run_match=ExpandedUniversal_NativeRunOriginal;
    ops.save_completion=cpu_batch && !match->interactive?
        exp_batch_completion:ExpandedUniversal_CarSaveCompletion;
    ops.user=match;
    return ExpandedUniversal_RunDue(r,target_day,&ops);
}
ExpandedUniversalResult ExpandedUniversal_CareerExecuteNext(
    int32_t target_day,ExpandedUniversalNativeMatch *match)
{
    return exp_execute_next(target_day,match,0);
}
ExpandedUniversalResult ExpandedUniversal_CareerExecuteNextCpuBatch(
    int32_t target_day,ExpandedUniversalNativeMatch *match)
{
    if(!match || match->interactive)return EXP_UNIVERSAL_INVALID;
    return exp_execute_next(target_day,match,1);
}
#endif
