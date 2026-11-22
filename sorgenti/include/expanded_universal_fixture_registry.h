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

static inline ExpandedUniversalFixture *ExpandedUniversal_Find(
    ExpandedUniversalRegistry *r, uint64_t event_id)
{
    uint32_t i;
    if (!r || !r->fixtures || r->fixture_count > r->fixture_capacity) return 0;
    for (i=0; i<r->fixture_count; ++i)
        if (r->fixtures[i].event_id==event_id) return &r->fixtures[i];
    return 0;
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
    for(i=0;i<r->scheduler.count;++i) {
        const ExpandedScheduledEvent *e=&r->scheduler.events[i];
        const ExpandedUniversalFixture *f;
        ExpandedFixtureLedgerEntry *claim;
        if(e->state==EXP_EVENT_COMPLETE)continue;
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
        claim=ExpandedFixtureLedger_Find(&r->ledger,f->key);
        if(!claim || claim->state!=EXP_FIXTURE_SCHEDULED)
            return EXP_UNIVERSAL_RECOVERY; /* claimed/played: never replay */
        *due=f;
        return EXP_UNIVERSAL_DUE;
    }
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
        claim=ExpandedFixtureLedger_Find(&r->ledger,f->key);
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
/* Validate the three structures together after an OPTIONAL CAR restore.
 * A valid CLAIMED/PENDING fixture is still NOT safe to replay: Peek blocks it.
 * No fabricated completion/score or implicit replay during restore. */
static inline int ExpandedUniversal_ValidateRestore(ExpandedUniversalRegistry *r)
{
    uint32_t i,j;
    if(!r || r->world >= (uint16_t)WORLD_COUNT || !r->fixtures ||
       r->fixture_count > r->fixture_capacity ||
       !r->ledger.entries || r->ledger.count!=r->fixture_count ||
       !ExpandedEventScheduler_ValidateRestore(&r->scheduler) ||
       r->scheduler.count!=r->fixture_count)return 0;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        ExpandedFixtureLedgerEntry *claim;
        const ExpandedScheduledEvent *event=0;
        if(!ExpandedUniversal_FixtureValid(f) ||
           f->key.edition.competition.world != r->world)return 0;
        for(j=0;j<r->scheduler.count;++j)
            if(r->scheduler.events[j].id==f->event_id) {
                if(event)return 0;
                event=&r->scheduler.events[j];
            }
        if(!event || event->kind!=EXP_EVENT_FIXTURE ||
           event->day_index!=f->day_index)return 0;
        claim=ExpandedFixtureLedger_Find(&r->ledger,f->key);
        if(!claim ||
           (claim->state==EXP_FIXTURE_PLAYED &&
             event->state!=EXP_EVENT_COMPLETE) ||
           (claim->state!=EXP_FIXTURE_PLAYED &&
             event->state!=EXP_EVENT_PENDING) ||
           (claim->state!=EXP_FIXTURE_SCHEDULED &&
            claim->state!=EXP_FIXTURE_CLAIMED &&
            claim->state!=EXP_FIXTURE_PLAYED) ||
           (claim->state==EXP_FIXTURE_SCHEDULED &&
            f->day_index < r->scheduler.clock.day_index))return 0;
        for(j=0;j<i;++j) {
            const ExpandedUniversalFixture *other=&r->fixtures[j];
            if(other->event_id==f->event_id ||
               ExpandedFixtureKey_Equal(other->key,f->key) ||
               (other->day_index==f->day_index &&
                (ExpandedClubKey_Equal(other->home,f->home) ||
                 ExpandedClubKey_Equal(other->home,f->away) ||
                 ExpandedClubKey_Equal(other->away,f->home) ||
                 ExpandedClubKey_Equal(other->away,f->away))))return 0;
        }
    }
    return 1;
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
    ExpandedFixtureDecision state;
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
    state=ExpandedFixtureLedger_Begin(&r->ledger,f->key);
    if(state!=EXP_FIXTURE_OK) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    /* Pre-claim CAR checkpoint: crash/restart never treats uncertain match
     * as SCHEDULED. The caller must not invoke OG before this succeeds. */
    if(!ops->save_claim(ops->user,r,f)) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    proposed.played=*ExpandedFixtureLedger_Find(&r->ledger,f->key);
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
    state=ExpandedFixtureLedger_Finish(&r->ledger,f->key,proposed.played.score);
    if(state!=EXP_FIXTURE_OK ||
       ExpandedEventScheduler_Complete(&r->scheduler,f->event_id)!=EXP_EVENT_OK) {
        r->recovery_required=1u;
        return EXP_UNIVERSAL_RECOVERY;
    }
    return EXP_UNIVERSAL_OK;
}
#endif /* EXPANDED_UNIVERSAL_FIXTURE_REGISTRY_H */
