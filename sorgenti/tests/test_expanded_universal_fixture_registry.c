/* Isolated generic kernel test; mock OG is NOT gameplay or CAR persistence. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_fixture_registry.h"

static int32_t day(unsigned y,unsigned m,unsigned d) {
    int32_t n=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d},&n));
    return n;
}
typedef struct MockIO {
    unsigned claims, games, commits;
    int fail_claim, fail_game, fail_commit, wrong_witness;
} MockIO;
static int save_claim(void *u,const ExpandedUniversalRegistry *r,
                      const ExpandedUniversalFixture *f) {
    MockIO *io=(MockIO *)u;
    const ExpandedFixtureLedgerEntry *claim=ExpandedFixtureLedger_Find(
        (ExpandedFixtureLedger *)&r->ledger,f->key);
    ++io->claims;
    assert(claim && claim->state==EXP_FIXTURE_CLAIMED);
    return !io->fail_claim;
}
static int run_og(void *u,const ExpandedUniversalFixture *f,
                  ExpandedUniversalMatchWitness *witness) {
    MockIO *io=(MockIO *)u;
    ++io->games;
    assert(f->home.stable_club && f->away.stable_club);
    if (io->fail_game)return 0;
    witness->key=f->key;
    witness->home=f->home;
    witness->away=f->away;
    witness->day_index=f->day_index;
    witness->score=(ExpandedFixtureScore){2u,1u,0u,0u,0u};
    witness->valid=1u;
    if (io->wrong_witness) witness->away.stable_club++;
    return 1;
}
static int save_completion(void *u,const ExpandedUniversalRegistry *r,
                           const ExpandedUniversalFixture *f,
                           const ExpandedUniversalCompletion *proposal) {
    MockIO *io=(MockIO *)u;
    const ExpandedFixtureLedgerEntry *stored=ExpandedFixtureLedger_Find(
        (ExpandedFixtureLedger *)&r->ledger,f->key);
    ++io->commits;
    assert(stored && stored->state==EXP_FIXTURE_CLAIMED);
    assert(proposal->witness.valid==1u);
    assert(ExpandedFixtureKey_Equal(proposal->witness.key,f->key));
    assert(ExpandedClubKey_Equal(proposal->witness.home,f->home));
    assert(ExpandedClubKey_Equal(proposal->witness.away,f->away));
    assert(proposal->witness.day_index==f->day_index);
    assert(proposal->played.state==EXP_FIXTURE_PLAYED);
    assert(proposal->played.score.home==2u && proposal->played.score.away==1u);
    assert(proposal->completed.id==f->event_id);
    assert(proposal->completed.state==EXP_EVENT_COMPLETE);
    return !io->fail_commit;
}
static ExpandedUniversalFixture match(uint16_t world,uint32_t competition,
    uint32_t home,uint32_t away,uint64_t event,int32_t start,int32_t date,
    uint16_t country,uint8_t slot,uint8_t type,uint32_t tie) {
    ExpandedUniversalFixture f={0};
    f.key.edition.competition=(ExpandedCompetitionKey){world,competition};
    f.key.edition.start_day=start;
    f.key.edition.sequence=0u;
    f.key.stage=1u;f.key.tie=tie;
    f.home=(ExpandedClubKey){world,home};
    f.away=(ExpandedClubKey){world,away};
    f.event_id=event;f.day_index=date;
    f.country=country;f.slot=slot;f.competition_type=type;
    return f;
}
int main(void) {
    ExpandedUniversalFixture storage[8]={0};
    ExpandedFixtureLedgerEntry claims[8]={0};
    ExpandedScheduledEvent events[8]={0};
    ExpandedUniversalRegistry r={0};
    const ExpandedUniversalFixture *due=0;
    const int32_t july=day(2026,7,1),a=day(2026,8,18),b=day(2026,8,19);
    const ExpandedUniversalFixture f0=match(WORLD_2627,45u,101u,102u,800u,
        july,a,45u,0u,0u,1u);
    ExpandedUniversalFixture f1=match(WORLD_2627,46u,103u,104u,801u,
        july,a,45u,2u,2u,1u);
    ExpandedUniversalFixture conflicting=f1;
    ExpandedUniversalFixture after=match(WORLD_2627,45u,101u,105u,802u,
        july,b,45u,0u,0u,2u);
    MockIO io={0};
    ExpandedUniversalOps ops={save_claim,run_og,save_completion,&io};
    assert(ExpandedUniversal_Init(&r,storage,claims,events,WORLD_2627,8u,
           (ExpandedWorldDate){2026,7,1})==EXP_UNIVERSAL_OK);
    assert(ExpandedUniversal_Add(&r,f0)==EXP_UNIVERSAL_OK);
    assert(ExpandedUniversal_Add(&r,f0)==EXP_UNIVERSAL_ALREADY);
    assert(ExpandedUniversal_Add(&r,f1)==EXP_UNIVERSAL_OK);
    conflicting.home=f0.home;
    assert(ExpandedUniversal_Add(&r,conflicting)==EXP_UNIVERSAL_CONFLICT);
    conflicting=f1;conflicting.event_id=800u;
    assert(ExpandedUniversal_Add(&r,conflicting)==EXP_UNIVERSAL_CONFLICT);
    conflicting=f1;conflicting.key.tie=3u; conflicting.event_id=803u;
    conflicting.away=f0.away;
    assert(ExpandedUniversal_Add(&r,conflicting)==EXP_UNIVERSAL_CONFLICT);
    conflicting=f1;conflicting.key.tie=4u;conflicting.event_id=804u;
    conflicting.competition_type=3u;
    assert(ExpandedUniversal_Add(&r,conflicting)==EXP_UNIVERSAL_INVALID);
    assert(ExpandedUniversal_Add(&r,after)==EXP_UNIVERSAL_OK);
    assert(ExpandedUniversal_ValidateRestore(&r));
    {
        ExpandedUniversalFixture other=match(WORLD_9697,400u,101u,102u,900u,july,a,13u,1u,1u,1u);
        assert(ExpandedUniversal_Add(&r,other)==EXP_UNIVERSAL_INVALID);
        assert(ExpandedUniversal_ValidateRestore(&r));
    }
    {
        const ExpandedUniversalFixture *club_due=0;
        const int32_t before=r.scheduler.clock.day_index;
        assert(ExpandedUniversal_NextForClub(&r,(ExpandedClubKey){WORLD_2627,103u},
               a,&club_due)==EXP_UNIVERSAL_DUE);
        assert(club_due && club_due->event_id==801u);
        assert(r.scheduler.clock.day_index==before);
        assert(ExpandedUniversal_NextForClub(&r,(ExpandedClubKey){WORLD_2627,999u},
               b,&club_due)==EXP_UNIVERSAL_EMPTY);
        assert(ExpandedUniversal_NextForClub(&r,(ExpandedClubKey){WORLD_9697,103u},
               b,&club_due)==EXP_UNIVERSAL_INVALID);
    }
    assert(ExpandedUniversal_Peek(&r,day(2026,8,17),&due)==EXP_UNIVERSAL_EMPTY);
    assert(!due && r.scheduler.clock.day_index==july);
    assert(ExpandedUniversal_Peek(&r,a,&due)==EXP_UNIVERSAL_DUE);
    assert(due && due->event_id==800u && r.scheduler.clock.day_index==july);
    assert(ExpandedUniversal_RunDue(&r,a,&ops)==EXP_UNIVERSAL_OK);
    assert(io.claims==1 && io.games==1 && io.commits==1);
    assert(r.ledger.entries[0].state==EXP_FIXTURE_PLAYED);
    assert(r.scheduler.events[0].state==EXP_EVENT_COMPLETE);
    assert(ExpandedUniversal_RunDue(&r,a,&ops)==EXP_UNIVERSAL_OK);
    assert(io.games==2 && io.commits==2); /* independent same-day fixture */
    assert(ExpandedUniversal_Peek(&r,a,&due)==EXP_UNIVERSAL_EMPTY);
    assert(ExpandedUniversal_RunDue(&r,b,&ops)==EXP_UNIVERSAL_OK);
    assert(io.games==3 && io.commits==3);
    {
        const ExpandedUniversalFixture *club_due=0;
        /* Completed historical match must not poison later club calendar. */
        assert(ExpandedUniversal_NextForClub(&r,(ExpandedClubKey){WORLD_2627,101u},
               b,&club_due)==EXP_UNIVERSAL_EMPTY);
        assert(!club_due);
    }
    assert(ExpandedUniversal_Peek(&r,b,&due)==EXP_UNIVERSAL_EMPTY);
    assert(ExpandedUniversal_RunDue(&r,b,&ops)==EXP_UNIVERSAL_EMPTY);
    assert(io.games==3); /* no replay after ACK */
    assert(ExpandedEventScheduler_ValidateRestore(&r.scheduler));
    assert(ExpandedUniversal_ValidateRestore(&r));
    r.scheduler.events[0].state=EXP_EVENT_PENDING;
    assert(!ExpandedUniversal_ValidateRestore(&r));
    r.scheduler.events[0].state=EXP_EVENT_COMPLETE;
    assert(ExpandedUniversal_ValidateRestore(&r));

    /* Persisting CLAIMED fails: OG must not start, recovery blocks retry. */
    {
        ExpandedUniversalRegistry bad={0};
        ExpandedUniversalFixture fs[2]={0};
        ExpandedFixtureLedgerEntry ls[2]={0};
        ExpandedScheduledEvent es[2]={0};
        MockIO failure={0};
        ExpandedUniversalOps fo={save_claim,run_og,save_completion,&failure};
        failure.fail_claim=1;
        assert(ExpandedUniversal_Init(&bad,fs,ls,es,WORLD_2627,2u,
            (ExpandedWorldDate){2026,7,1})==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_Add(&bad,f0)==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_RunDue(&bad,a,&fo)==EXP_UNIVERSAL_RECOVERY);
        assert(failure.claims==1 && failure.games==0 && failure.commits==0);
        assert(ExpandedUniversal_RunDue(&bad,a,&fo)==EXP_UNIVERSAL_RECOVERY);
        assert(failure.games==0);
    }
    /* OG completed, postmatch CAR failed: NEVER silently replay OG. */
    {
        ExpandedUniversalRegistry bad={0};
        ExpandedUniversalFixture fs[2]={0};
        ExpandedFixtureLedgerEntry ls[2]={0};
        ExpandedScheduledEvent es[2]={0};
        MockIO failure={0};
        ExpandedUniversalOps fo={save_claim,run_og,save_completion,&failure};
        failure.fail_commit=1;
        assert(ExpandedUniversal_Init(&bad,fs,ls,es,WORLD_2627,2u,
            (ExpandedWorldDate){2026,7,1})==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_Add(&bad,f0)==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_RunDue(&bad,a,&fo)==EXP_UNIVERSAL_RECOVERY);
        assert(failure.games==1 && failure.commits==1);
        assert(ExpandedUniversal_RunDue(&bad,a,&fo)==EXP_UNIVERSAL_RECOVERY);
        assert(failure.games==1);
    }
    /* If OG returns the wrong fixture, NEVER commit a score or retry. */
    {
        ExpandedUniversalRegistry bad={0};
        ExpandedUniversalFixture fs[2]={0};
        ExpandedFixtureLedgerEntry ls[2]={0};
        ExpandedScheduledEvent es[2]={0};
        MockIO failure={0};
        ExpandedUniversalOps fo={save_claim,run_og,save_completion,&failure};
        failure.wrong_witness=1;
        assert(ExpandedUniversal_Init(&bad,fs,ls,es,WORLD_2627,2u,
            (ExpandedWorldDate){2026,7,1})==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_Add(&bad,f0)==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_RunDue(&bad,a,&fo)==EXP_UNIVERSAL_RECOVERY);
        assert(failure.games==1 && failure.commits==0);
        assert(ExpandedUniversal_RunDue(&bad,a,&fo)==EXP_UNIVERSAL_RECOVERY);
        assert(failure.games==1);
    }
    /* A fresh independent 96/97 world can use the same stable club numbers. */
    {
        ExpandedUniversalRegistry historical={0};
        ExpandedUniversalFixture fs[2]={0};
        ExpandedFixtureLedgerEntry ls[2]={0};
        ExpandedScheduledEvent es[2]={0};
        ExpandedUniversalFixture other=match(WORLD_9697,400u,101u,102u,
            900u,july,a,13u,1u,1u,1u);
        assert(ExpandedUniversal_Init(&historical,fs,ls,es,WORLD_9697,2u,
            (ExpandedWorldDate){2026,7,1})==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_Add(&historical,other)==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_ValidateRestore(&historical));
    }
    /* Stress the exact indexed validator with 512 original distinct fixtures.
     * No mock score is assumed valid without the ledger/event witness. */
    {
        enum { N=512 };
        ExpandedUniversalRegistry bulk={0};
        ExpandedUniversalFixture *fs=
            (ExpandedUniversalFixture *)calloc(N,sizeof *fs);
        ExpandedFixtureLedgerEntry *ls=
            (ExpandedFixtureLedgerEntry *)calloc(N,sizeof *ls);
        ExpandedScheduledEvent *es=
            (ExpandedScheduledEvent *)calloc(N,sizeof *es);
        const ExpandedUniversalFixture *found=NULL;
        assert(fs && ls && es);
        assert(ExpandedUniversal_Init(&bulk,fs,ls,es,WORLD_2627,N,
            (ExpandedWorldDate){2026,7,1})==EXP_UNIVERSAL_OK);
        for(unsigned i=0;i<N;++i) {
            fs[i]=match(WORLD_2627,500u,2u*i+1u,2u*i+2u,
                        (uint64_t)i+1000u,july,a+(int32_t)(i/64u),
                        45u,0u,0u,i+1u);
            ls[i].key=fs[i].key;
            ls[i].state=EXP_FIXTURE_SCHEDULED;
            es[i]=(ExpandedScheduledEvent){
                fs[i].event_id,0u,fs[i].day_index,10u,
                EXP_EVENT_FIXTURE,EXP_EVENT_PENDING};
        }
        bulk.fixture_count=N;
        bulk.ledger.count=N;
        bulk.scheduler.count=N;
        bulk.fixtures_sorted_by_event=1u;
        assert(ExpandedUniversal_ValidateRestore(&bulk));
        assert(ExpandedUniversal_Find(&bulk,1000u)==&fs[0]);
        assert(ExpandedUniversal_Find(&bulk,1511u)==&fs[N-1]);
        assert(!ExpandedUniversal_Find(&bulk,999u));
        assert(!ExpandedUniversal_Find(&bulk,1512u));
        /* Negative: duplicate event ID, ledger key and club/date each fail. */
        es[17].id=es[16].id;
        assert(!ExpandedUniversal_ValidateRestore(&bulk));
        es[17].id=fs[17].event_id;
        ls[17].key=ls[16].key;
        assert(!ExpandedUniversal_ValidateRestore(&bulk));
        ls[17].key=fs[17].key;
        fs[17].away=fs[16].home;
        assert(!ExpandedUniversal_ValidateRestore(&bulk));
        fs[17].away=(ExpandedClubKey){WORLD_2627,36u};
        assert(ExpandedUniversal_ValidateRestore(&bulk));
        /* Full matchday snapshot: prior played, next still pending. */
        bulk.scheduler.clock.day_index=a;
        es[0].state=EXP_EVENT_COMPLETE;
        ls[0].state=EXP_FIXTURE_PLAYED;
        assert(ExpandedUniversal_ValidateRestore(&bulk));
        assert(ExpandedUniversal_Peek(&bulk,a,&found)==EXP_UNIVERSAL_DUE);
        assert(found && found->event_id==1001u && bulk.pending_hint==1u);
        /* A CLAIMED/PENDING CAR is legitimate but never replayable. */
        ls[1].state=EXP_FIXTURE_CLAIMED;
        assert(ExpandedUniversal_ValidateRestore(&bulk));
        assert(ExpandedUniversal_Peek(&bulk,a,&found)==EXP_UNIVERSAL_RECOVERY);
        free(fs);free(ls);free(es);
    }
    puts("POINTS_1_2_3_REGISTRY_DISPATCH_ISOLATED=PASS");
    puts("OG_50HZ_GAMEPLAY_AND_ATOMIC_CAR=NOT_TESTED");
    return 0;
}
