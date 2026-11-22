/* SWOS Expanded — one data-driven fixture/event authority for Expanded Career.
 * Preparation kernel for points 1+2+3, not activated in the live game yet.
 * Original OG 50 Hz match execution is provided ONLY by the caller callback;
 * the legacy OG career/TSKC is NEVER an authority for a fixture or next date.
 * An adapter must atomically persist OG+RNG+ledger+event in CAR, and validate
 * the same witness on restore, before installing this in the live dispatcher.
 */
#ifndef EXPANDED_UNIVERSAL_FIXTURE_REGISTRY_H
#define EXPANDED_UNIVERSAL_FIXTURE_REGISTRY_H
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include "expanded_event_scheduler.h"
#include "expanded_competition_identity.h"

typedef struct ExpandedUniversalFixture {
    ExpandedFixtureKey key;              /* world, competition, edition, stage, tie, leg */
    ExpandedClubKey home, away;         /* stable clubs, same world as edition */
    uint64_t event_id;                  /* caller-provided, globally unique, persistent */
    int32_t day_index;                  /* exact civil match date */
    uint16_t country;                   /* data provenance; never a routing switch */
    uint8_t slot, competition_type;     /* native type 0/1/2 is metadata only */
} ExpandedUniversalFixture;

typedef struct ExpandedUniversalRegistry {
    uint16_t world;                       /* one registry per football world */
    ExpandedEventScheduler scheduler;   /* only civil cursor AFTER migration */
    ExpandedFixtureLedger ledger;       /* one claim and one score per fixture */
    ExpandedUniversalFixture *fixtures;
    uint32_t fixture_count, fixture_capacity;
    uint8_t recovery_required;          /* no retry after uncertain OG/CAR effect */
    uint8_t fixtures_sorted_by_event;  /* transient lookup acceleration, not CAR */
    uint32_t pending_hint;            /* derived from event states, never authority */
} ExpandedUniversalRegistry;

typedef enum ExpandedUniversalResult {
    EXP_UNIVERSAL_INVALID = -4,
    EXP_UNIVERSAL_CONFLICT = -3,
    EXP_UNIVERSAL_FULL = -2,
    EXP_UNIVERSAL_RECOVERY = -1,
    EXP_UNIVERSAL_OK = 0,
    EXP_UNIVERSAL_ALREADY = 1,
    EXP_UNIVERSAL_DUE = 2,
    EXP_UNIVERSAL_EMPTY = 3,
    EXP_UNIVERSAL_BLOCKED = 4
} ExpandedUniversalResult;

/* True original OG result must carry the very fixture identity selected by
 * the scheduler. A successful call without a matching witness is a FAULT,
 * not permission to commit scores or advance standings. */
typedef struct ExpandedUniversalMatchWitness {
    ExpandedFixtureKey key;
    ExpandedClubKey home,away;
    int32_t day_index;
    ExpandedFixtureScore score;
    uint8_t valid; /* set by OG adapter only after original result confirmed */
} ExpandedUniversalMatchWitness;

typedef struct ExpandedUniversalCompletion {
    ExpandedUniversalMatchWitness witness; /* the one genuine OG occurrence */
    ExpandedFixtureLedgerEntry played; /* proposed durable score from OG witness */
    ExpandedScheduledEvent completed;  /* proposed event ACK, same CAR transaction */
} ExpandedUniversalCompletion;

typedef struct ExpandedUniversalOps {
    /* Persist CLAIMED state atomically BEFORE starting OG; no play on failure. */
    int (*save_claim)(void *user, const ExpandedUniversalRegistry *registry,
                      const ExpandedUniversalFixture *fixture);
    /* Invoke original OG match ONCE. Return actual OG score/witness, not AI score. */
    int (*run_match)(void *user, const ExpandedUniversalFixture *fixture,
                     ExpandedUniversalMatchWitness *witness);
    /* Persist proposed played+ACK with actual OG/RNG/world state in ONE CAR.
     * Must NOT merely save registry's still-CLAIMED in-memory state. */
    int (*save_completion)(void *user, const ExpandedUniversalRegistry *registry,
                           const ExpandedUniversalFixture *fixture,
                           const ExpandedUniversalCompletion *proposal);
    void *user;
} ExpandedUniversalOps;

/* Sorted fixtures (live batch and restored modern CAR) have an indexed
 * event lookup; older/isolated Add() users retain their linear fallback. */
static inline ExpandedUniversalFixture *ExpandedUniversal_Find(
    ExpandedUniversalRegistry *r, uint64_t event_id)
{
    uint32_t i;
    if (!r || !r->fixtures || r->fixture_count > r->fixture_capacity) return 0;
    if(r->fixtures_sorted_by_event) {
        uint32_t lo=0u,hi=r->fixture_count;
        while(lo<hi) {
            const uint32_t mid=lo+(hi-lo)/2u;
            if(r->fixtures[mid].event_id<event_id)lo=mid+1u;
            else hi=mid;
        }
        return lo<r->fixture_count && r->fixtures[lo].event_id==event_id?
            &r->fixtures[lo]:0;
    }
    for(i=0;i<r->fixture_count;++i)
        if(r->fixtures[i].event_id==event_id)return &r->fixtures[i];
    return 0;
}
/* All live bulk fixtures and CAR-decoded rows pair the ledger entry with
 * the same fixture index, so canonical key is checked in O(log N), with a
 * safe generic fallback for older Add() callers. Never trust index alone. */
static inline ExpandedFixtureLedgerEntry *ExpandedUniversal_EntryFor(
    ExpandedUniversalRegistry *r,const ExpandedUniversalFixture *f)
{
    if(!r || !f || !r->ledger.entries ||
       r->ledger.count>r->ledger.capacity)return NULL;
    if(r->fixtures_sorted_by_event) {
        ExpandedUniversalFixture *found=ExpandedUniversal_Find(r,f->event_id);
        if(found==f) {
            const uint32_t i=(uint32_t)(found-r->fixtures);
            return i<r->ledger.count &&
                   ExpandedFixtureKey_Equal(r->ledger.entries[i].key,f->key)?
                   &r->ledger.entries[i]:NULL;
        }
        return NULL;
    }
    return ExpandedFixtureLedger_Find(&r->ledger,f->key);
}
static inline int ExpandedUniversal_FixtureValid(const ExpandedUniversalFixture *f)
{
    ExpandedWorldDate date,start;
    return f && f->event_id && ExpandedFixtureKey_Valid(f->key) &&
        ExpandedClubKey_Valid(f->home) && ExpandedClubKey_Valid(f->away) &&
        ExpandedClubKey_Equal(f->home,f->away)==0 &&
        f->home.world == f->key.edition.competition.world &&
        f->away.world == f->key.edition.competition.world &&
        f->key.edition.start_day <= f->day_index &&
        ExpandedWorld_IndexToDate(f->key.edition.start_day,&start) &&
        f->competition_type <= 2u &&
        ExpandedWorld_IndexToDate(f->day_index,&date);
}
static inline ExpandedUniversalResult ExpandedUniversal_Init(
    ExpandedUniversalRegistry *r, ExpandedUniversalFixture *fixtures,
    ExpandedFixtureLedgerEntry *claims, ExpandedScheduledEvent *events,
    uint16_t world, uint32_t capacity, ExpandedWorldDate start)
{
    if (!r || !fixtures || !claims || !events || !capacity ||
        world >= (uint16_t)WORLD_COUNT ||
        ExpandedEventScheduler_Init(&r->scheduler,events,capacity,start)!=EXP_EVENT_OK)
        return EXP_UNIVERSAL_INVALID;
    r->world=world;
    r->fixtures=fixtures;
    r->fixture_count=0;
    r->fixture_capacity=capacity;
    r->ledger.entries=claims;
    r->ledger.count=0;
    r->ledger.capacity=capacity;
    r->recovery_required=0;
    r->fixtures_sorted_by_event=0u;
    r->pending_hint=0u;
    return EXP_UNIVERSAL_OK;
}
/* New fixtures are scheduled from verified data, NEVER from OG phase/TSKC.
 * Precheck ALL collisions before touching either scheduler or ledger.
 * Dynamic caller storage determines capacity; no country/team-size constant. */
static inline ExpandedUniversalResult ExpandedUniversal_Add(
    ExpandedUniversalRegistry *r, ExpandedUniversalFixture f)
{
    uint32_t i;
    ExpandedScheduledEvent e;
    ExpandedEventResult se;
    ExpandedFixtureDecision le;
    if (!r || r->recovery_required || !ExpandedEventScheduler_Valid(&r->scheduler) ||
        !r->ledger.entries || r->ledger.count > r->ledger.capacity ||
        !r->fixtures || r->fixture_count > r->fixture_capacity ||
        !ExpandedUniversal_FixtureValid(&f) ||
        f.key.edition.competition.world != r->world ||
        f.day_index < r->scheduler.clock.day_index)
        return EXP_UNIVERSAL_INVALID;
    for (i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *old=&r->fixtures[i];
        if (ExpandedFixtureKey_Equal(old->key,f.key) || old->event_id==f.event_id) {
            if (old->event_id==f.event_id &&
                ExpandedFixtureKey_Equal(old->key,f.key) &&
                ExpandedClubKey_Equal(old->home,f.home) &&
                ExpandedClubKey_Equal(old->away,f.away) &&
                old->day_index==f.day_index && old->country==f.country &&
                old->slot==f.slot && old->competition_type==f.competition_type)
                return EXP_UNIVERSAL_ALREADY;
            return EXP_UNIVERSAL_CONFLICT;
        }
        if (old->day_index==f.day_index &&
            (ExpandedClubKey_Equal(old->home,f.home) ||
             ExpandedClubKey_Equal(old->home,f.away) ||
             ExpandedClubKey_Equal(old->away,f.home) ||
             ExpandedClubKey_Equal(old->away,f.away)))
            return EXP_UNIVERSAL_CONFLICT; /* no two matches per club/day */
    }
    if (r->fixture_count==r->fixture_capacity ||
        r->ledger.count==r->ledger.capacity ||
        r->scheduler.count==r->scheduler.capacity)
        return EXP_UNIVERSAL_FULL;
    if (ExpandedFixtureLedger_Find(&r->ledger,f.key))return EXP_UNIVERSAL_CONFLICT;
    for(i=0;i<r->scheduler.count;++i)
        if(r->scheduler.events[i].id==f.event_id)return EXP_UNIVERSAL_CONFLICT;
    e.id=f.event_id; e.requires_id=0u; e.day_index=f.day_index;
    e.priority=10u; e.kind=EXP_EVENT_FIXTURE; e.state=EXP_EVENT_PENDING;
    se=ExpandedEventScheduler_Schedule(&r->scheduler,e);
    if (se!=EXP_EVENT_OK)return EXP_UNIVERSAL_CONFLICT;
    le=ExpandedFixtureLedger_Schedule(&r->ledger,f.key);
    if (le!=EXP_FIXTURE_OK) {
        /* Roll back an in-memory schedule; NEVER leave a half-registered pair. */
        for(i=0;i<r->scheduler.count;++i)if(r->scheduler.events[i].id==f.event_id) {
            uint32_t j;
            for(j=i+1u;j<r->scheduler.count;++j)
                r->scheduler.events[j-1u]=r->scheduler.events[j];
            --r->scheduler.count;
            break;
        }
        return EXP_UNIVERSAL_CONFLICT;
    }
    r->fixtures[r->fixture_count++]=f;
    r->fixtures_sorted_by_event=0u; /* Add() may receive arbitrary event order */
    r->pending_hint=0u; /* insert can precede an earlier cached pending */
    return EXP_UNIVERSAL_OK;
}
/* Read-only: the manager and CPU consume THIS same next event; no phase
 * drift or side effects from opening the calendar. Other event kinds are
 * blocked until their universal handlers are installed. */
static inline ExpandedUniversalResult ExpandedUniversal_Peek(
    ExpandedUniversalRegistry *r, int32_t target_day,
    const ExpandedUniversalFixture **due)
{
    uint32_t i,j;
    ExpandedWorldDate date;
    if (due)*due=0;
    if (!due || !r ||
        !ExpandedEventScheduler_Valid(&r->scheduler) ||
        target_day < r->scheduler.clock.day_index ||
        !ExpandedWorld_IndexToDate(target_day,&date))
        return EXP_UNIVERSAL_INVALID;
    if(r->recovery_required)return EXP_UNIVERSAL_RECOVERY;
    /* A single monotone pending cursor avoids scanning all COMPLETED
     * events after every subsequent CPU or manager result. */
    i=r->pending_hint;
    if(i>r->scheduler.count)i=0u;
    for(;i<r->scheduler.count;++i) {
        const ExpandedScheduledEvent *e=&r->scheduler.events[i];
        const ExpandedUniversalFixture *f;
        ExpandedFixtureLedgerEntry *claim;
        if(e->state==EXP_EVENT_COMPLETE)continue;
        r->pending_hint=i;
        if(e->state!=EXP_EVENT_PENDING || e->day_index<r->scheduler.clock.day_index)
            return EXP_UNIVERSAL_RECOVERY;
        if(e->day_index>target_day)return EXP_UNIVERSAL_EMPTY;
        if(e->requires_id) {
            int ready=0;
            for(j=0;j<r->scheduler.count;++j)
                if(r->scheduler.events[j].id==e->requires_id &&
                   r->scheduler.events[j].state==EXP_EVENT_COMPLETE)ready=1;
            if(!ready)return EXP_UNIVERSAL_BLOCKED;
        }
        if(e->kind!=EXP_EVENT_FIXTURE)return EXP_UNIVERSAL_BLOCKED;
        f=ExpandedUniversal_Find(r,e->id);
        if(!f || f->day_index!=e->day_index)return EXP_UNIVERSAL_RECOVERY;
        claim=ExpandedUniversal_EntryFor(r,f);
        if(!claim || claim->state!=EXP_FIXTURE_SCHEDULED)
            return EXP_UNIVERSAL_RECOVERY; /* claimed/played: never replay */
        *due=f;
        return EXP_UNIVERSAL_DUE;
    }
    r->pending_hint=r->scheduler.count;
    return EXP_UNIVERSAL_EMPTY;
}
/* Manager calendar is a read-only projection of the SAME world registry.
 * It never owns an extra fixture cursor and never advances the world clock.
 * DUE here means visible/pending, not permission to run before earlier
 * worldwide events have been committed by the central dispatcher. */
static inline ExpandedUniversalResult ExpandedUniversal_NextForClub(
    ExpandedUniversalRegistry *r, ExpandedClubKey club, int32_t through_day,
    const ExpandedUniversalFixture **next)
{
    uint32_t i,j;
    ExpandedWorldDate date;
    const ExpandedUniversalFixture *best=0;
    if(next)*next=0;
    if(!r || !next || !ExpandedClubKey_Valid(club) ||
       club.world != r->world || !r->fixtures ||
       r->fixture_count > r->fixture_capacity ||
       !ExpandedEventScheduler_Valid(&r->scheduler) ||
       through_day < r->scheduler.clock.day_index ||
       !ExpandedWorld_IndexToDate(through_day,&date))
        return EXP_UNIVERSAL_INVALID;
    if(r->recovery_required)return EXP_UNIVERSAL_RECOVERY;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        ExpandedFixtureLedgerEntry *claim;
        const ExpandedScheduledEvent *event=0;
        if(!ExpandedClubKey_Equal(f->home,club) &&
           !ExpandedClubKey_Equal(f->away,club))continue;
        if(f->day_index>through_day)continue;
        for(j=0;j<r->scheduler.count;++j)
            if(r->scheduler.events[j].id==f->event_id) {
                event=&r->scheduler.events[j];break;
            }
        claim=ExpandedUniversal_EntryFor(r,f);
        if(!event || !claim || event->kind!=EXP_EVENT_FIXTURE ||
           event->day_index!=f->day_index)return EXP_UNIVERSAL_RECOVERY;
        if(event->state==EXP_EVENT_COMPLETE &&
           claim->state==EXP_FIXTURE_PLAYED)continue;
        if(f->day_index < r->scheduler.clock.day_index)
            return EXP_UNIVERSAL_RECOVERY;
        if(event->state!=EXP_EVENT_PENDING ||
           claim->state!=EXP_FIXTURE_SCHEDULED)return EXP_UNIVERSAL_RECOVERY;
        if(!best || f->day_index<best->day_index ||
           (f->day_index==best->day_index && f->event_id<best->event_id))
            best=f;
    }
    if(!best)return EXP_UNIVERSAL_EMPTY;
    *next=best;
    return EXP_UNIVERSAL_DUE;
}
/* Canonical, no memcmp(packed bytes), indices for snapshot validation. */
typedef struct ExpandedUniversalIdIndex {uint64_t id;uint32_t pos;} ExpandedUniversalIdIndex;
typedef struct ExpandedUniversalKeyIndex {ExpandedFixtureKey key;uint32_t pos;} ExpandedUniversalKeyIndex;
typedef struct ExpandedUniversalDayIndex {int32_t day;uint16_t world;uint32_t club;} ExpandedUniversalDayIndex;
static inline int ExpandedUniversal_IdCompare(const void *aa,const void *bb)
{
    const ExpandedUniversalIdIndex *a=(const ExpandedUniversalIdIndex *)aa,
        *b=(const ExpandedUniversalIdIndex *)bb;
    return a->id<b->id?-1:a->id>b->id?1:0;
}
static inline int ExpandedUniversal_KeyCompareValue(ExpandedFixtureKey a,
                                                    ExpandedFixtureKey b)
{
#define CMP_KEY(field) do {if(a.field!=b.field)return a.field<b.field?-1:1;}while(0)
    CMP_KEY(edition.competition.world);
    CMP_KEY(edition.competition.stable_competition);
    CMP_KEY(edition.start_day);
    CMP_KEY(edition.sequence);
    CMP_KEY(stage);CMP_KEY(tie);CMP_KEY(leg);CMP_KEY(replay);
#undef CMP_KEY
    return 0;
}
static inline int ExpandedUniversal_KeyCompare(const void *aa,const void *bb)
{
    const ExpandedUniversalKeyIndex *a=(const ExpandedUniversalKeyIndex *)aa,
        *b=(const ExpandedUniversalKeyIndex *)bb;
    return ExpandedUniversal_KeyCompareValue(a->key,b->key);
}
static inline int ExpandedUniversal_DayCompare(const void *aa,const void *bb)
{
    const ExpandedUniversalDayIndex *a=(const ExpandedUniversalDayIndex *)aa,
        *b=(const ExpandedUniversalDayIndex *)bb;
    if(a->day!=b->day)return a->day<b->day?-1:1;
    if(a->world!=b->world)return a->world<b->world?-1:1;
    return a->club<b->club?-1:a->club>b->club?1:0;
}
/* Read-only O(n log n) full validation: identical reject conditions as
 * the original nested scans, without quadratic START/CAR cost. */
static inline int ExpandedUniversal_ValidateRestore(ExpandedUniversalRegistry *r)
{
    uint32_t i,n;
    ExpandedUniversalIdIndex *fixturesById=NULL,*eventsById=NULL;
    ExpandedUniversalKeyIndex *fixturesByKey=NULL,*claimsByKey=NULL;
    ExpandedUniversalDayIndex *days=NULL;
    int ok=0;
    if(!r || r->world >= (uint16_t)WORLD_COUNT || !r->fixtures ||
       r->fixture_count > r->fixture_capacity ||
       !r->ledger.entries || r->ledger.count!=r->fixture_count ||
       !ExpandedEventScheduler_ValidateRestore(&r->scheduler) ||
       r->scheduler.count!=r->fixture_count)return 0;
    n=r->fixture_count;
    if(!n)return 1;
    if(n>UINT32_MAX/2u)return 0;
#if SIZE_MAX < UINT64_MAX
    if((size_t)n>SIZE_MAX/sizeof *fixturesById ||
       (size_t)n>SIZE_MAX/sizeof *eventsById ||
       (size_t)n>SIZE_MAX/sizeof *fixturesByKey ||
       (size_t)n>SIZE_MAX/sizeof *claimsByKey ||
       (size_t)(n*2u)>SIZE_MAX/sizeof *days)return 0;
#endif
    fixturesById=(ExpandedUniversalIdIndex *)malloc((size_t)n*sizeof *fixturesById);
    eventsById=(ExpandedUniversalIdIndex *)malloc((size_t)n*sizeof *eventsById);
    fixturesByKey=(ExpandedUniversalKeyIndex *)malloc((size_t)n*sizeof *fixturesByKey);
    claimsByKey=(ExpandedUniversalKeyIndex *)malloc((size_t)n*sizeof *claimsByKey);
    days=(ExpandedUniversalDayIndex *)malloc((size_t)(2u*n)*sizeof *days);
    if(!fixturesById || !eventsById || !fixturesByKey ||
       !claimsByKey || !days)goto done;
    for(i=0;i<n;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        if(!ExpandedUniversal_FixtureValid(f) ||
           f->key.edition.competition.world!=r->world)goto done;
        fixturesById[i]=(ExpandedUniversalIdIndex){f->event_id,i};
        eventsById[i]=(ExpandedUniversalIdIndex){r->scheduler.events[i].id,i};
        fixturesByKey[i]=(ExpandedUniversalKeyIndex){f->key,i};
        claimsByKey[i]=(ExpandedUniversalKeyIndex){r->ledger.entries[i].key,i};
        days[2u*i]=(ExpandedUniversalDayIndex){
            f->day_index,f->home.world,f->home.stable_club};
        days[2u*i+1u]=(ExpandedUniversalDayIndex){
            f->day_index,f->away.world,f->away.stable_club};
    }
    qsort(fixturesById,n,sizeof *fixturesById,ExpandedUniversal_IdCompare);
    qsort(eventsById,n,sizeof *eventsById,ExpandedUniversal_IdCompare);
    qsort(fixturesByKey,n,sizeof *fixturesByKey,ExpandedUniversal_KeyCompare);
    qsort(claimsByKey,n,sizeof *claimsByKey,ExpandedUniversal_KeyCompare);
    qsort(days,2u*n,sizeof *days,ExpandedUniversal_DayCompare);
    for(i=0;i<n;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[fixturesById[i].pos];
        const ExpandedScheduledEvent *e=
            &r->scheduler.events[eventsById[i].pos];
        const ExpandedFixtureLedgerEntry *claim=
            &r->ledger.entries[claimsByKey[i].pos];
        if(!f->event_id || f->event_id!=e->id ||
           ExpandedUniversal_KeyCompareValue(
               fixturesByKey[i].key,claimsByKey[i].key)!=0 ||
           (i && (fixturesById[i-1u].id==fixturesById[i].id ||
                  eventsById[i-1u].id==eventsById[i].id ||
                  ExpandedUniversal_KeyCompareValue(
                      fixturesByKey[i-1u].key,fixturesByKey[i].key)==0 ||
                  ExpandedUniversal_KeyCompareValue(
                      claimsByKey[i-1u].key,claimsByKey[i].key)==0)))goto done;
        /* Retrieve ledger by the actual fixture key, not the unrelated
         * position of the i-th event when sorted by ID. */
        {
            uint32_t lo=0u,hi=n;
            while(lo<hi) {
                const uint32_t mid=lo+(hi-lo)/2u;
                if(ExpandedUniversal_KeyCompareValue(
                    claimsByKey[mid].key,f->key)<0)lo=mid+1u;
                else hi=mid;
            }
            if(lo>=n || ExpandedUniversal_KeyCompareValue(
                claimsByKey[lo].key,f->key)!=0)goto done;
            claim=&r->ledger.entries[claimsByKey[lo].pos];
        }
        if(e->kind!=EXP_EVENT_FIXTURE || e->day_index!=f->day_index ||
           (claim->state==EXP_FIXTURE_PLAYED &&
             e->state!=EXP_EVENT_COMPLETE) ||
           (claim->state!=EXP_FIXTURE_PLAYED &&
             e->state!=EXP_EVENT_PENDING) ||
           (claim->state!=EXP_FIXTURE_SCHEDULED &&
            claim->state!=EXP_FIXTURE_CLAIMED &&
            claim->state!=EXP_FIXTURE_PLAYED) ||
           (claim->state==EXP_FIXTURE_SCHEDULED &&
            f->day_index<r->scheduler.clock.day_index))goto done;
    }
    for(i=1;i<n*2u;++i)
        if(ExpandedUniversal_DayCompare(&days[i-1u],&days[i])==0)
            goto done; /* including two appearances by same club/day */
    ok=1;
done:
    free(fixturesById);free(eventsById);
    free(fixturesByKey);free(claimsByKey);free(days);
    return ok;
}
/* A genuine durable transaction is REQUIRED at the adapters before wiring
 * this into a live menu. A crash after OG but before CAR must retain CLAIMED
 * and require recovery, not silently simulate the same fixture again. */
static inline ExpandedUniversalResult ExpandedUniversal_RunDue(
    ExpandedUniversalRegistry *r, int32_t target_day,
    const ExpandedUniversalOps *ops)
{
    const ExpandedUniversalFixture *f=0;
    const ExpandedScheduledEvent *e=0;
    ExpandedUniversalCompletion proposed;
    ExpandedEventResult next;
    ExpandedFixtureLedgerEntry *claim;
    ExpandedUniversalResult ready;
    if(!ops || !ops->save_claim || !ops->run_match ||
       !ops->save_completion || !r)return EXP_UNIVERSAL_INVALID;
    ready=ExpandedUniversal_Peek(r,target_day,&f);
    if(ready!=EXP_UNIVERSAL_DUE)return ready;
    next=ExpandedEventScheduler_Next(&r->scheduler,f->day_index,&e);
    if(next!=EXP_EVENT_DUE || !e || e->id!=f->event_id) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    claim=ExpandedUniversal_EntryFor(r,f);
    if(!claim || claim->state!=EXP_FIXTURE_SCHEDULED) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    claim->state=EXP_FIXTURE_CLAIMED;
    /* Pre-claim CAR checkpoint: crash/restart never treats uncertain match
     * as SCHEDULED. The caller must not invoke OG before this succeeds. */
    if(!ops->save_claim(ops->user,r,f)) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    proposed.played=*claim;
    {
        ExpandedUniversalMatchWitness witness={0};
        if(!ops->run_match(ops->user,f,&witness) || !witness.valid ||
           !ExpandedFixtureKey_Equal(witness.key,f->key) ||
           !ExpandedClubKey_Equal(witness.home,f->home) ||
           !ExpandedClubKey_Equal(witness.away,f->away) ||
           witness.day_index!=f->day_index) {
            /* OG may already have changed its RNG and match state:
             * never release and retry the same fixture automatically. */
            r->recovery_required=1u;
            return EXP_UNIVERSAL_RECOVERY;
        }
        proposed.witness=witness;
        proposed.played.score=witness.score;
    }
    proposed.played.state=EXP_FIXTURE_PLAYED;
    proposed.completed=*e;
    proposed.completed.state=EXP_EVENT_COMPLETE;
    /* The save writer must atomically persist the PROPOSED changes plus
     * OG outcome, RNG, club standings and the rest of the world. */
    if(!ops->save_completion(ops->user,r,f,&proposed)) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    if(claim->state!=EXP_FIXTURE_CLAIMED) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    claim->score=proposed.played.score;
    claim->state=EXP_FIXTURE_PLAYED;
    if(ExpandedEventScheduler_Complete(&r->scheduler,f->event_id)!=EXP_EVENT_OK) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    return EXP_UNIVERSAL_OK;
}
#endif /* EXPANDED_UNIVERSAL_FIXTURE_REGISTRY_H */
