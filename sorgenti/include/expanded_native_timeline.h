/* Unified civil/native OG tick adapter.
 *
 * The date/OG resident held by the caller remain the ONLY authorities.
 * The scheduler below is a disposable plan for the current requested
 * advancement; no duplicate resident, second simulation, or mutable
 * competition clock is introduced.
 *
 * No event acknowledgement is issued by this ephemeral stack planner:
 * that requires a durable CAR transaction, which belongs to the caller.
 * The existing OG and civil fields are saved by the normal CAR writer.
 * Incomplete operations MUST latch recovery and preserve the prior CAR.
 *
 * This is a world-TICK carrier, not yet the universal per-fixture registry
 * or the manager Copa type2 matchday adapter (RG-014 remains open).
 */
#ifndef EXPANDED_NATIVE_TIMELINE_H
#define EXPANDED_NATIVE_TIMELINE_H

#include "expanded_event_scheduler.h"

typedef int (*ExpandedNativeTimelineAction)(ExpandedWorldDate date,void *user);

typedef struct ExpandedNativeTimelineOps {
    ExpandedNativeTimelineAction run_og;
    ExpandedNativeTimelineAction commit_civil;
    ExpandedNativeTimelineAction verify;
    void *user;
} ExpandedNativeTimelineOps;

typedef enum ExpandedNativeTimelineResult {
    EXP_NATIVE_TICK_INVALID = -4,
    EXP_NATIVE_TICK_OG_FAILED = -3,
    EXP_NATIVE_TICK_CIVIL_FAILED = -2,
    EXP_NATIVE_TICK_VERIFY_FAILED = -1,
    EXP_NATIVE_TICK_OK = 0,
    EXP_NATIVE_TICK_UNCHANGED = 1
} ExpandedNativeTimelineResult;

static inline const char *ExpandedNativeTimeline_ResultName(
    ExpandedNativeTimelineResult result)
{
    switch (result) {
        case EXP_NATIVE_TICK_OK: return "OG_AND_CIVIL_VERIFIED";
        case EXP_NATIVE_TICK_UNCHANGED: return "ALREADY_AT_DATE";
        case EXP_NATIVE_TICK_OG_FAILED: return "NATIVE_OG";
        case EXP_NATIVE_TICK_CIVIL_FAILED: return "CIVIL_COMMIT";
        case EXP_NATIVE_TICK_VERIFY_FAILED: return "OG_CIVIL_PROOF";
        default: return "INVALID";
    }
}

/* One stable world+day event ID; NEVER represent it as a fixture ID.
 * Fixtures require an ExpandedFixtureKey and a witness from OG.
 * This world-tick ID is ephemeral, NOT an event ACK persisted in CAR.
 * No new CAR schema or arbitrary event-count cap. */
static inline uint64_t ExpandedNativeTimeline_TickId(
    uint16_t world,int32_t day_index)
{
    if(world==UINT16_MAX || day_index<0)return 0;
    return ((uint64_t)(world+1u)<<48) |
           ((uint64_t)(uint32_t)(day_index+1)<<8) |
           (uint64_t)EXP_EVENT_NATIVE_TICK;
}

/* Drives exactly one ALREADY-EXISTING world OG advance. It does not
 * generate a fixture, perform LWE results, or apply finance ticks.
 * A caller MUST use its existing recovery latch after any failure,
 * because OG may have partially mutated resident state.
 *
 * Later, a fixture-level dispatcher will supply multiple entries here
 * with their stable Edition/FixtureKeys and actual OG witness/ACK.
 * Do not call this repeatedly for one target day. */
static inline ExpandedNativeTimelineResult ExpandedNativeTimeline_Advance(
    uint16_t world,ExpandedWorldDate from,ExpandedWorldDate target,
    const ExpandedNativeTimelineOps *ops)
{
    int32_t from_day=0,target_day=0;
    ExpandedScheduledEvent one[1]={{0}};
    ExpandedEventScheduler scheduler={0};
    const ExpandedScheduledEvent *due=0;
    if(!ops || !ops->run_og || !ops->commit_civil || !ops->verify ||
       !ExpandedWorld_DateToIndex(from,&from_day) ||
       !ExpandedWorld_DateToIndex(target,&target_day) ||
       target_day<from_day ||
       !ExpandedNativeTimeline_TickId(world,target_day))
        return EXP_NATIVE_TICK_INVALID;
    if(target_day==from_day)return EXP_NATIVE_TICK_UNCHANGED;

    if(ExpandedEventScheduler_Init(&scheduler,one,1u,from)!=EXP_EVENT_OK)
        return EXP_NATIVE_TICK_INVALID;
    const uint64_t id=ExpandedNativeTimeline_TickId(world,target_day);
    const ExpandedScheduledEvent event={
        id,0u,target_day,0u,EXP_EVENT_NATIVE_TICK,EXP_EVENT_PENDING
    };
    if(ExpandedEventScheduler_Schedule(&scheduler,event)!=EXP_EVENT_OK ||
       ExpandedEventScheduler_Next(&scheduler,target_day,&due)!=EXP_EVENT_DUE ||
       !due || due->id!=id || due->day_index!=target_day)
        return EXP_NATIVE_TICK_INVALID;

    /* The native OG executor is the SOLE score writer, and its success
     * must precede committing the single persistent civil clock. */
    if(!ops->run_og(target,ops->user))
        return EXP_NATIVE_TICK_OG_FAILED;
    if(!ops->commit_civil(target,ops->user))
        return EXP_NATIVE_TICK_CIVIL_FAILED;
    if(!ops->verify(target,ops->user))
        return EXP_NATIVE_TICK_VERIFY_FAILED;
    /* Do NOT call ExpandedEventScheduler_Complete here: its contract
     * requires an atomic CAR checkpoint. Neither this stack carrier
     * nor a mere OG/civil in-memory verification is durable evidence. */
    if(!ExpandedEventScheduler_ValidateRestore(&scheduler))
        return EXP_NATIVE_TICK_VERIFY_FAILED;
    return EXP_NATIVE_TICK_OK;
}
#endif /* EXPANDED_NATIVE_TIMELINE_H */
