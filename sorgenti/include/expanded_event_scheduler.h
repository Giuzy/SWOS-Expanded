/* Competition Engine phase 3: deterministic event-date cursor, not a match engine.
 * This component neither calls OG nor serializes CAR. A caller must durably
 * commit OG effects and the corresponding event state as ONE transaction.
 * Until fixture single-writer and CAR integration exist, it must not be used
 * to dispatch live matches. No second clock or autonomous LWE simulation.
 */
#ifndef EXPANDED_EVENT_SCHEDULER_H
#define EXPANDED_EVENT_SCHEDULER_H
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include "expanded_world_time.h"

/* IDs must be stable in the edition/world registry. IDs are not OG compId. */
typedef enum ExpandedEventKind {
    EXP_EVENT_QUALIFICATION = 1,
    EXP_EVENT_FIXTURE = 2,
    EXP_EVENT_CUP_ADVANCE = 3,
    EXP_EVENT_SEASON_CLOSE = 4,
    /* Native OG + civil tick. Not a match and not a second simulator. */
    EXP_EVENT_NATIVE_TICK = 5
} ExpandedEventKind;
typedef enum ExpandedEventState {
    EXP_EVENT_PENDING = 0,
    EXP_EVENT_COMPLETE = 1
} ExpandedEventState;
typedef struct ExpandedScheduledEvent {
    uint64_t id, requires_id; /* 0 requires_id means no prerequisite */
    int32_t day_index;        /* exact civil date, not OG 1/8-month phase */
    uint16_t priority;        /* same day: lower priority first */
    uint8_t kind, state;
} ExpandedScheduledEvent;
typedef struct ExpandedEventScheduler {
    ExpandedWorldClock clock; /* sole caller-supplied civil cursor */
    ExpandedScheduledEvent *events;
    uint32_t count, capacity;
    uint32_t pending_hint; /* transient scan cache; never a date/score owner */
} ExpandedEventScheduler;
typedef enum ExpandedEventResult {
    EXP_EVENT_INVALID = -3,
    EXP_EVENT_CONFLICT = -2,
    EXP_EVENT_FULL = -1,
    EXP_EVENT_OK = 0,
    EXP_EVENT_ALREADY = 1,
    EXP_EVENT_DUE = 2,
    EXP_EVENT_BLOCKED = 3,
    EXP_EVENT_EMPTY = 4
} ExpandedEventResult;

static inline int ExpandedEvent_EqualDefinition(const ExpandedScheduledEvent *a,
                                                const ExpandedScheduledEvent *b)
{
    return a->id == b->id && a->requires_id == b->requires_id &&
           a->day_index == b->day_index && a->priority == b->priority &&
           a->kind == b->kind;
}
static inline int ExpandedEvent_Before(const ExpandedScheduledEvent *a,
                                       const ExpandedScheduledEvent *b)
{
    if (a->day_index != b->day_index) return a->day_index < b->day_index;
    if (a->priority != b->priority) return a->priority < b->priority;
    return a->id < b->id;
}
static inline int ExpandedEventScheduler_Valid(const ExpandedEventScheduler *s)
{
    ExpandedWorldDate date;
    return s && s->events && s->count <= s->capacity &&
           ExpandedWorld_IndexToDate(s->clock.day_index, &date);
}
static inline ExpandedEventResult ExpandedEventScheduler_Init(
    ExpandedEventScheduler *s, ExpandedScheduledEvent *buffer,
    uint32_t capacity, ExpandedWorldDate start)
{
    int32_t day;
    if (!s || !buffer || !capacity || !ExpandedWorld_DateToIndex(start,&day))
        return EXP_EVENT_INVALID;
    s->clock.day_index=day;
    s->events=buffer;
    s->count=0;
    s->capacity=capacity;
    s->pending_hint=0u;
    return EXP_EVENT_OK;
}
/* Never insert a past event, change an already scheduled definition, or
 * silently reschedule a completed occurrence. The array stays ordered. */
static inline ExpandedEventResult ExpandedEventScheduler_Schedule(
    ExpandedEventScheduler *s, ExpandedScheduledEvent event)
{
    ExpandedWorldDate date;
    uint32_t i,pos;
    if (!ExpandedEventScheduler_Valid(s) || !event.id ||
        event.requires_id == event.id ||
        event.kind < EXP_EVENT_QUALIFICATION || event.kind > EXP_EVENT_NATIVE_TICK ||
        event.state != EXP_EVENT_PENDING ||
        !ExpandedWorld_IndexToDate(event.day_index,&date))return EXP_EVENT_INVALID;
    for (i=0;i<s->count;++i)
        if (s->events[i].id==event.id)
            return ExpandedEvent_EqualDefinition(&s->events[i],&event)?
                   EXP_EVENT_ALREADY:EXP_EVENT_CONFLICT;
    if(event.day_index < s->clock.day_index)return EXP_EVENT_INVALID;
    if(s->count==s->capacity)return EXP_EVENT_FULL;
    pos=s->count;
    while(pos && ExpandedEvent_Before(&event,&s->events[pos-1u])) {
        s->events[pos]=s->events[pos-1u];
        --pos;
    }
    s->events[pos]=event;
    ++s->count;
    if(pos<s->pending_hint)s->pending_hint=pos;
    return EXP_EVENT_OK;
}
/* Peek advances only to the earliest pending event, never one day at a
 * time. A blocked prerequisite halts ALL later events: none can consume
 * a competition result that has not been finalized. No callback executes. */
static inline ExpandedEventResult ExpandedEventScheduler_Next(
    ExpandedEventScheduler *s, int32_t target_day,
    const ExpandedScheduledEvent **due)
{
    ExpandedWorldDate date;
    uint32_t i,j;
    if (due) *due=0;
    if (!ExpandedEventScheduler_Valid(s) || !due ||
        target_day<s->clock.day_index ||
        !ExpandedWorld_IndexToDate(target_day,&date))return EXP_EVENT_INVALID;
    i=s->pending_hint;
    if(i>s->count)i=0u;
    for(;i<s->count;++i) {
        const ExpandedScheduledEvent *e=&s->events[i];
        if(e->state==EXP_EVENT_COMPLETE)continue;
        s->pending_hint=i;
        if(e->state!=EXP_EVENT_PENDING)return EXP_EVENT_INVALID;
        if(e->day_index>target_day)break;
        if(e->day_index<s->clock.day_index)return EXP_EVENT_CONFLICT;
        s->clock.day_index=e->day_index;
        if(e->requires_id) {
            const ExpandedScheduledEvent *prior=0;
            for(j=0;j<s->count;++j)
                if(s->events[j].id==e->requires_id) {
                    prior=&s->events[j];break;
                }
            if(!prior || prior->state!=EXP_EVENT_COMPLETE)
                return EXP_EVENT_BLOCKED;
        }
        *due=e;
        return EXP_EVENT_DUE;
    }
    s->pending_hint=s->count;
    s->clock.day_index=target_day;
    return EXP_EVENT_EMPTY;
}
/* Acknowledgement is only legal for the earliest ready event, after its
 * OG result and CAR checkpoint have been committed by the caller. */
static inline ExpandedEventResult ExpandedEventScheduler_Complete(
    ExpandedEventScheduler *s, uint64_t id)
{
    const ExpandedScheduledEvent *due=0;
    ExpandedEventResult result;
    uint32_t i;
    if(!ExpandedEventScheduler_Valid(s) || !id)return EXP_EVENT_INVALID;
    for(i=0;i<s->count;++i)
        if(s->events[i].id==id && s->events[i].state==EXP_EVENT_COMPLETE)
            return EXP_EVENT_ALREADY;
    result=ExpandedEventScheduler_Next(s,s->clock.day_index,&due);
    if(result!=EXP_EVENT_DUE || !due || due->id!=id)
        return result==EXP_EVENT_DUE?EXP_EVENT_CONFLICT:result;
    /* The event pointed to by Next resides in the caller-owned array. */
    s->events[(uint32_t)(due-s->events)].state=EXP_EVENT_COMPLETE;
    return EXP_EVENT_OK;
}
/* Validate a restored snapshot without replaying any event or changing OG.
 * Build a read-only event-ID index once, then check all dependency edges
 * in O(n log n), not O(n²) at every CAR checkpoint. */
typedef struct ExpandedEventIdState {
    uint64_t id;
    uint8_t state;
} ExpandedEventIdState;
static inline int ExpandedEvent_IdCompare(const void *aa,const void *bb)
{
    const ExpandedEventIdState *a=(const ExpandedEventIdState *)aa;
    const ExpandedEventIdState *b=(const ExpandedEventIdState *)bb;
    return a->id<b->id?-1:a->id>b->id?1:0;
}
static inline int ExpandedEvent_IdCompleted(const ExpandedEventIdState *ids,
                                             uint32_t count,uint64_t id)
{
    uint32_t lo=0u,hi=count;
    while(lo<hi) {
        const uint32_t mid=lo+(hi-lo)/2u;
        if(ids[mid].id<id)lo=mid+1u;
        else hi=mid;
    }
    return lo<count && ids[lo].id==id &&
           ids[lo].state==EXP_EVENT_COMPLETE;
}
static inline int ExpandedEventScheduler_ValidateRestore(const ExpandedEventScheduler *s)
{
    uint32_t i;
    ExpandedEventIdState *ids;
    if(!ExpandedEventScheduler_Valid(s))return 0;
    if(!s->count)return 1;
#if SIZE_MAX < UINT64_MAX
    if((size_t)s->count>SIZE_MAX/sizeof *ids)return 0;
#endif
    ids=(ExpandedEventIdState *)malloc((size_t)s->count*sizeof *ids);
    if(!ids)return 0;
    for(i=0;i<s->count;++i) {
        const ExpandedScheduledEvent *e=&s->events[i];
        ExpandedWorldDate date;
        if(!e->id || e->requires_id==e->id ||
           !ExpandedWorld_IndexToDate(e->day_index,&date) ||
           e->kind<EXP_EVENT_QUALIFICATION || e->kind>EXP_EVENT_NATIVE_TICK ||
           (e->state!=EXP_EVENT_PENDING && e->state!=EXP_EVENT_COMPLETE) ||
           (e->state==EXP_EVENT_COMPLETE && e->day_index>s->clock.day_index) ||
           (i && !ExpandedEvent_Before(&s->events[i-1u],e))) {
            free(ids);return 0;
        }
        ids[i]=(ExpandedEventIdState){e->id,e->state};
    }
    qsort(ids,(size_t)s->count,sizeof *ids,ExpandedEvent_IdCompare);
    for(i=0;i<s->count;++i)
        if(i && ids[i].id==ids[i-1u].id) {
            free(ids);return 0;
        }
    for(i=0;i<s->count;++i) {
        const ExpandedScheduledEvent *e=&s->events[i];
        if(e->state==EXP_EVENT_COMPLETE && e->requires_id &&
           !ExpandedEvent_IdCompleted(ids,s->count,e->requires_id)) {
            free(ids);return 0;
        }
    }
    free(ids);
    return 1;
}
#endif
