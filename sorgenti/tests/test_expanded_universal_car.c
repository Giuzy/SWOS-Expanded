/* C11 proof using the REAL universal CAR codec, mjson, Gregorian calendar,
 * scheduler and fixture ledger. OG 50Hz is mocked ONLY in this unit test.
 * No disk, gameplay, live save, or extra simulator is touched by this test.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_car.h"
#include "expanded_universal_standings.h"

static WorldId s_world=WORLD_2627;
WorldId World_Current(void) {return s_world;}
void *swos_io_realloc(void *p,uint32_t n) {return realloc(p,n);}
void *swos_io_alloc(uint32_t n) {return malloc(n);}
void swos_io_free(void *p) {free(p);}

static unsigned s_register_clock_calls;
unsigned long long Platform_NowMs(void) {return 0;}
void ExpandedUniversalLive_ResetCpuBatch(void) {}
void ExpandedUniversalLive_RegisterClock(void) {++s_register_clock_calls;}
static char s_disk[65536];
static unsigned s_writes,s_games;
static int s_fail_completion;
static int32_t s_civil_clock;
int Expanded_AutoCareer_SaveChecked(const char *reason)
{
    struct mjson_fixedbuf dst={s_disk,(int)sizeof s_disk,0};
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    if(s_fail_completion && strstr(reason,"complete"))return 0;
    assert(r);
    assert(r->scheduler.clock.day_index==s_civil_clock);
    assert(mjson_printf(mjson_print_fixed_buf,&dst,
        "{%Q:{%Q:%d}","living_world","now",
        (int)s_civil_clock)>0);
    assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst));
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"}")>0);
    assert(dst.len<(int)sizeof s_disk-1);
    assert(ExpandedUniversal_CarValidate(s_disk,dst.len));
    ++s_writes;
    return 1;
}
static int32_t date(int y,int m,int d)
{
    int32_t out=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){
        (uint16_t)y,(uint8_t)m,(uint8_t)d},&out));
    return out;
}
static ExpandedUniversalFixture fixture(uint64_t id,uint32_t tie,
                                         uint32_t home,uint32_t away,int32_t day)
{
    ExpandedUniversalFixture f={0};
    f.event_id=id;
    f.day_index=day;
    f.home=(ExpandedClubKey){WORLD_2627,home};
    f.away=(ExpandedClubKey){WORLD_2627,away};
    f.key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,450001u};
    f.key.edition.start_day=day;
    f.key.edition.sequence=1u;
    f.key.stage=1u;f.key.tie=tie;
    f.country=45u;f.slot=0u;f.competition_type=0u;
    return f;
}
static int run_original_mock(void *p,const ExpandedUniversalFixture *f,
                              ExpandedUniversalMatchWitness *w)
{
    (void)p;
    ++s_games;
    *w=(ExpandedUniversalMatchWitness){0};
    w->key=f->key;w->home=f->home;w->away=f->away;
    w->day_index=f->day_index;
    w->score=(ExpandedFixtureScore){2u,1u,0u,0u,0u};
    w->valid=1u;
    return 1;
}
static uint64_t s_rng,s_rng_log[64];
static unsigned s_rng_log_n;
static int run_original_rng(void *p,const ExpandedUniversalFixture *f,
                            ExpandedUniversalMatchWitness *w)
{
    (void)p;
    ++s_games;
    s_rng^=s_rng<<13;s_rng^=s_rng>>7;s_rng^=s_rng<<17;
    *w=(ExpandedUniversalMatchWitness){0};
    w->key=f->key;w->home=f->home;w->away=f->away;
    w->day_index=f->day_index;
    w->score=(ExpandedFixtureScore){(uint16_t)(s_rng%5u),(uint16_t)((s_rng>>8)%4u),0u,0u,0u};
    if(s_rng_log_n<64u)s_rng_log[s_rng_log_n++]=s_rng;
    w->valid=1u;
    return 1;
}
int main(void)
{
    const int32_t d=date(2026,7,1);
    const ExpandedUniversalOps ops={
        ExpandedUniversal_CarSaveClaim,run_original_mock,
        ExpandedUniversal_CarSaveCompletion,NULL,NULL,NULL};
    ExpandedUniversalRegistry *r;
    ExpandedUniversalFixture f0=fixture(UINT64_C(0x2000000000000001),1u,
                                        (45u<<16)|1u,(45u<<16)|2u,d);
    ExpandedUniversalFixture f1=fixture(UINT64_C(0x2000000000000002),2u,
                                        (45u<<16)|3u,(45u<<16)|4u,d);
    const ExpandedUniversalFixture *next=NULL;
    char corrupted[sizeof s_disk];
    s_civil_clock=d;
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},4u));
    r=ExpandedUniversal_CarRegistry();
    assert(r);
    assert(ExpandedUniversal_Add(r,f0)==EXP_UNIVERSAL_OK);
    assert(ExpandedUniversal_Add(r,f1)==EXP_UNIVERSAL_OK);
    assert(Expanded_AutoCareer_SaveChecked("initial"));
    /* v288: immutable CE snapshot. Emitting it equals emitting the live
     * registry at capture time, and it NEVER changes when the live registry
     * advances afterwards (the writer thread depends on this). */
    {
        char live_before[4096],snap_a[4096],snap_b[4096],live_after[4096];
        struct mjson_fixedbuf l0={live_before,(int)sizeof live_before,0};
        struct mjson_fixedbuf s0={snap_a,(int)sizeof snap_a,0};
        struct mjson_fixedbuf s1={snap_b,(int)sizeof snap_b,0};
        struct mjson_fixedbuf l1={live_after,(int)sizeof live_after,0};
        ExpandedUniversalCarSnapshot *snap=NULL;
        assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&l0));
        assert(ExpandedUniversal_CarSnapshotCapture(&snap) && snap);
        assert(ExpandedUniversal_CarSnapshotBytes(snap)>0u);
        assert(ExpandedUniversal_CarEmitSnapshot(snap,mjson_print_fixed_buf,&s0));
        assert(s0.len==l0.len && memcmp(snap_a,live_before,(size_t)l0.len)==0);
        /* the live registry memory is changed behind the snapshot's back:
         * club id, a score and an event priority (reverted right after) */
        {
            const uint32_t club=r->fixtures[0].home.stable_club;
            const uint16_t sc=r->ledger.entries[0].score.home;
            const uint32_t pr=r->scheduler.events[0].priority;
            r->fixtures[0].home.stable_club^=0x55u;
            r->ledger.entries[0].score.home=(uint16_t)(sc+9u);
            r->scheduler.events[0].priority=pr+7u;
            assert(ExpandedUniversal_CarEmitSnapshot(snap,mjson_print_fixed_buf,&l1));
            assert(l1.len==l0.len && memcmp(live_after,live_before,(size_t)l0.len)==0);
            r->fixtures[0].home.stable_club=club;
            r->ledger.entries[0].score.home=sc;
            r->scheduler.events[0].priority=pr;
        }
        assert(ExpandedUniversal_CarEmitSnapshot(snap,mjson_print_fixed_buf,&s1));
        assert(s1.len==s0.len && memcmp(snap_b,snap_a,(size_t)s0.len)==0);
        ExpandedUniversal_CarSnapshotRelease(snap);
        /* inactive registry: capture reports nothing to snapshot */
        {
            ExpandedUniversalCarSnapshot *none=(ExpandedUniversalCarSnapshot *)1;
            s_world=WORLD_9697;
            assert(ExpandedUniversal_CarSnapshotCapture(&none) && none==NULL);
            s_world=WORLD_2627;
        }
    }
    /* Field checksum must reject any tampering, without mutating runtime. */
    strcpy(corrupted,s_disk);
    {
        char *p=strstr(corrupted,"UE02:");
        assert(p);
        p[5+40]=(p[5+40]=='A')?'B':'A';
    }
    assert(!ExpandedUniversal_CarValidate(corrupted,(int)strlen(corrupted)));
    assert(ExpandedUniversal_CarRegistry()==r);
    /* v290: a LEGACY UE01 (hex) CAR must still validate and restore, and a
     * UE02 save of the restored registry must be byte-identical to the one
     * saved before (migration is lossless). */
    {
        static char legacy[65536],hex[32768],again[65536];
        char *at=legacy;
        const char *key="\"universal_engine\":\"";
        const char *k0=strstr(s_disk,key),*k1;
        uint32_t i,j;
        uint32_t crc=2166136261u;
        size_t hl=0u,pre,k;
        char *hp=hex;
        struct mjson_fixedbuf dst2={again,(int)sizeof again,0};
        assert(k0);k0+=strlen(key);k1=strchr(k0,'"');assert(k1);
        pre=(size_t)(k0-s_disk);
#define HX(val,dig) do { uint64_t q_=(uint64_t)(val);int d_; \
            for(d_=(int)(dig)-1;d_>=0;--d_) \
                *hp++="0123456789abcdef"[(q_>>(4*d_))&15u]; } while(0)
        memcpy(hp,"UE01",4);hp+=4;
        HX(r->world,4);HX((uint32_t)r->scheduler.clock.day_index,8);
        HX(r->fixture_count,8);
        for(i=0;i<r->fixture_count;++i) {
            const ExpandedUniversalFixture *f=&r->fixtures[i];
            const ExpandedFixtureLedgerEntry *cl=&r->ledger.entries[i];
            const ExpandedScheduledEvent *e=NULL;
            for(j=0;j<r->scheduler.count;++j)
                if(r->scheduler.events[j].id==f->event_id)
                    e=&r->scheduler.events[j];
            assert(e);
            HX(f->event_id,16);HX(e->requires_id,16);
            HX(f->key.edition.competition.stable_competition,8);
            HX((uint32_t)f->key.edition.start_day,8);
            HX(f->key.edition.sequence,4);HX(f->key.stage,8);
            HX(f->key.tie,8);HX(f->key.leg,4);HX(f->key.replay,4);
            HX(f->home.stable_club,8);HX(f->away.stable_club,8);
            HX((uint32_t)f->day_index,8);HX(f->country,4);HX(f->slot,2);
            HX(f->competition_type,2);HX(cl->state,2);
            HX(cl->score.home,4);HX(cl->score.away,4);
            HX(cl->score.penalties_home,4);HX(cl->score.penalties_away,4);
            HX(cl->score.flags,2);HX(e->state,2);HX(e->priority,4);
        }
        hl=(size_t)(hp-hex);
        for(k=0;k<hl;++k){crc^=(uint8_t)hex[k];crc*=16777619u;}
        HX(crc,8);*hp=0;
#undef HX
        memcpy(at,s_disk,pre);at+=pre;
        memcpy(at,hex,strlen(hex));at+=strlen(hex);
        strcpy(at,k1);
        assert(ExpandedUniversal_CarValidate(legacy,(int)strlen(legacy)));
        /* tamper with the legacy hex: rejected as before */
        legacy[pre+30]=(legacy[pre+30]=='0')?'1':'0';
        assert(!ExpandedUniversal_CarValidate(legacy,(int)strlen(legacy)));
        legacy[pre+30]=(legacy[pre+30]=='0')?'1':'0';
        assert(ExpandedUniversal_CarRestore(legacy,(int)strlen(legacy)));
        r=ExpandedUniversal_CarRegistry();
        assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst2));
        {   const char *a0=strstr(again,"UE02:");
            assert(a0 && (size_t)(strchr(a0,'"')-a0)==(size_t)(k1-k0) &&
                   !memcmp(a0,k0,(size_t)(k1-k0)));   /* lossless migration */
        }
    }
    /* Cross-world CAR cannot load or erase the 2627 state. */
    s_world=WORLD_9697;
    assert(!ExpandedUniversal_CarValidate(s_disk,(int)strlen(s_disk)));
    s_world=WORLD_2627;
    /* v284 side-effect-free helpers used by the transactional CONTINUE. */
    {
        const int len=(int)strlen(s_disk);
        int32_t peeked=-1;
        uint32_t keys[4]={(45u<<16)|1u,(45u<<16)|2u,(45u<<16)|3u,(45u<<16)|4u};
        uint32_t partial[3]={(45u<<16)|1u,(45u<<16)|2u,(45u<<16)|3u};
        assert(ExpandedUniversal_CarPeekClock(s_disk,len,&peeked) && peeked==d);
        assert(ExpandedUniversal_CarCoversClubs(s_disk,len,keys,4u));
        assert(!ExpandedUniversal_CarCoversClubs(s_disk,len,partial,3u));
        assert(!ExpandedUniversal_CarCoversClubs(s_disk,len,NULL,0u));
        assert(!ExpandedUniversal_CarPeekClock(corrupted,(int)strlen(corrupted),
                                               &peeked));
        assert(!ExpandedUniversal_CarCoversClubs(corrupted,
                   (int)strlen(corrupted),keys,4u));
        s_world=WORLD_9697;
        assert(!ExpandedUniversal_CarPeekClock(s_disk,len,&peeked));
        s_world=WORLD_2627;
        /* none of the helpers touched or replaced the live registry */
        assert(ExpandedUniversal_CarRegistry()==r);
        assert(ExpandedUniversal_ValidateRestore(r) && !r->recovery_required);
        assert(s_register_clock_calls>=1u); /* CarBegin registered the clock */
    }
    /* One real transaction in the test uses mock OG result, but REAL
     * production CAR Claim/Completion callbacks and codec. */
    assert(ExpandedUniversal_RunDue(r,d,&ops)==EXP_UNIVERSAL_OK);
    assert(s_games==1u && s_writes==3u);
    /* A single original OG witness becomes exactly one table update.
     * The second fixture is still scheduled and must not produce points. */
    {
        ExpandedUniversalStanding rows[4];
        ExpandedClubKey winner={0};
        uint32_t count=0u, played=0u, points=0u;
        assert(ExpandedUniversal_Standings(
            r,f0.key.edition,1u,3u,rows,4u,&count));
        assert(count==4u);
        for(uint32_t i=0;i<count;++i) {
            played+=rows[i].played;
            points+=rows[i].points;
        }
        assert(played==2u && points==3u);
        assert(ExpandedUniversal_KnockoutWinner(
            r,f0.key.edition,1u,1u,&winner));
        assert(ExpandedClubKey_Equal(winner,f0.home));
        assert(!ExpandedUniversal_KnockoutWinner(
            r,f1.key.edition,1u,2u,&winner));
    }
    assert(ExpandedUniversal_CarValidate(s_disk,(int)strlen(s_disk)));
    assert(ExpandedUniversal_CarRestore(s_disk,(int)strlen(s_disk)));
    r=ExpandedUniversal_CarRegistry();
    assert(r && ExpandedUniversal_ValidateRestore(r));
    assert(ExpandedUniversal_Peek(r,d,&next)==EXP_UNIVERSAL_DUE);
    assert(next && next->event_id==f1.event_id);
    /* Failed restore past the point of no return: lock instead of playing a
     * half-restored career; a later valid restore clears the lock. */
    {
        const unsigned before=s_register_clock_calls;
        ExpandedUniversal_CarMarkRecovery();
        assert(r->recovery_required);
        assert(ExpandedUniversal_Peek(r,d,&next)==EXP_UNIVERSAL_RECOVERY);
        assert(ExpandedUniversal_CarRestore(s_disk,(int)strlen(s_disk)));
        r=ExpandedUniversal_CarRegistry();
        assert(r && !r->recovery_required);
        assert(s_register_clock_calls==before+1u);
        assert(ExpandedUniversal_Peek(r,d,&next)==EXP_UNIVERSAL_DUE);
        /* repeated CONTINUE of the same CAR: identical, no replay */
        assert(ExpandedUniversal_CarRestore(s_disk,(int)strlen(s_disk)));
        r=ExpandedUniversal_CarRegistry();
        assert(r && !r->recovery_required &&
               ExpandedUniversal_Peek(r,d,&next)==EXP_UNIVERSAL_DUE &&
               next && next->event_id==f1.event_id);
    }
    /* Save completion failure must leave durable CLAIMED and block retry. */
    s_fail_completion=1;
    assert(ExpandedUniversal_RunDue(r,d,&ops)==EXP_UNIVERSAL_RECOVERY);
    assert(s_games==2u);
    assert(ExpandedUniversal_RunDue(r,d,&ops)==EXP_UNIVERSAL_RECOVERY);
    assert(s_games==2u);
    assert(ExpandedUniversal_CarValidate(s_disk,(int)strlen(s_disk)));
    assert(ExpandedUniversal_CarRestore(s_disk,(int)strlen(s_disk)));
    r=ExpandedUniversal_CarRegistry();
    assert(r && r->recovery_required);
    assert(ExpandedUniversal_Peek(r,d,&next)==EXP_UNIVERSAL_RECOVERY);
    /* NULL section restores old-career compatibility without scores. */
    {
        char old[200];
        const int n=snprintf(old,sizeof old,
            "{\"living_world\":{\"now\":%d},\"universal_engine\":null}",
            (int)d);
        assert(n>0 && n<(int)sizeof old);
        assert(ExpandedUniversal_CarValidate(old,n));
        assert(ExpandedUniversal_CarRestore(old,n));
        assert(!ExpandedUniversal_CarRegistry());
    }
    /* v290: synthetic edition archive in the CAR. */
    {
        const uint32_t c1=(45u<<16)|1u,c3=(45u<<16)|3u;
        ExpandedUniversalFixture a0=fixture(UINT64_C(0x2000000000000011),1u,
            c1,(45u<<16)|2u,d);
        ExpandedUniversalFixture a1=fixture(UINT64_C(0x2000000000000012),2u,
            c3,(45u<<16)|4u,d);
        ExpandedEditionKey ed=a0.key.edition;
        char *p,*q;
        static char live_s[65536],snap_s[65536];
        struct mjson_fixedbuf ls={live_s,(int)sizeof live_s,0};
        struct mjson_fixedbuf ss={snap_s,(int)sizeof snap_s,0};
        ExpandedUniversalCarSnapshot *sn=NULL;
        ExpandedUniversal_CarResetCurrent();
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},4u));
        r=ExpandedUniversal_CarRegistry();
        assert(ExpandedUniversal_Add(r,a0)==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_Add(r,a1)==EXP_UNIVERSAL_OK);
        assert(ExpandedUniversal_ArchiveCount()==0u);
        /* nothing played: the gate refuses */
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,c1,0u,d)==EXP_ARCHIVE_OPEN);
        s_fail_completion=0;
        assert(ExpandedUniversal_RunDue(r,d,&ops)!=EXP_UNIVERSAL_RECOVERY);
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,c1,0u,d)==EXP_ARCHIVE_OPEN);
        assert(ExpandedUniversal_RunDue(r,d,&ops)!=EXP_UNIVERSAL_RECOVERY);
        /* both PLAYED: wrong champion / bad day / unknown edition refused */
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,(45u<<16)|9u,0u,d)==EXP_ARCHIVE_INVALID);
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,c1,c1,d)==EXP_ARCHIVE_INVALID);
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,c1,c3,d-1)==EXP_ARCHIVE_INVALID);
        { ExpandedEditionKey nx=ed;nx.sequence=77u;
          assert(ExpandedUniversal_ArchiveCloseEdition(nx,c1,0u,d)==EXP_ARCHIVE_UNKNOWN); }
        assert(ExpandedUniversal_ArchiveCount()==0u);
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,c1,c3,d)==EXP_ARCHIVE_OK);
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,c1,c3,d)==EXP_ARCHIVE_ALREADY);
        assert(ExpandedUniversal_ArchiveCloseEdition(ed,c3,c1,d)==EXP_ARCHIVE_CONFLICT);
        assert(ExpandedUniversal_ArchiveCount()==1u && ExpandedUniversal_ArchiveContains(ed));
        assert(ExpandedUniversal_ArchiveAt(0)->match_count==2u &&
               ExpandedUniversal_ArchiveAt(0)->champion==c1 &&
               ExpandedUniversal_ArchiveAt(0)->runner_up==c3);
        /* persisted together with the registry, in the SAME CAR */
        assert(Expanded_AutoCareer_SaveChecked("archive"));
        assert(strstr(s_disk,"\"universal_archive\":\"UA01:"));
        assert(ExpandedUniversal_CarValidate(s_disk,(int)strlen(s_disk)));
        /* snapshot carries an owned copy and emits identical bytes */
        assert(ExpandedUniversal_CarSnapshotCapture(&sn) && sn);
        assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&ls));
        assert(ExpandedUniversal_CarEmitSnapshot(sn,mjson_print_fixed_buf,&ss));
        assert(ls.len==ss.len && memcmp(live_s,snap_s,(size_t)ls.len)==0);
        ExpandedUniversal_CarSnapshotRelease(sn);
        /* CONTINUE restores the archive */
        ExpandedUniversal_CarResetCurrent();
        assert(ExpandedUniversal_ArchiveCount()==0u);
        assert(ExpandedUniversal_CarRestore(s_disk,(int)strlen(s_disk)));
        assert(ExpandedUniversal_ArchiveCount()==1u && ExpandedUniversal_ArchiveContains(ed));
        assert(ExpandedUniversal_ArchiveAt(0)->closed_day==d);
        /* tampered archive text is rejected, live state untouched */
        strcpy(corrupted,s_disk);
        p=strstr(corrupted,"\"universal_archive\":\"UA01:");assert(p);
        p+=strlen("\"universal_archive\":\"UA01:")+20;
        *p=(*p=='0')?'1':'0';
        assert(!ExpandedUniversal_CarValidate(corrupted,(int)strlen(corrupted)));
        assert(!ExpandedUniversal_CarRestore(corrupted,(int)strlen(corrupted)));
        assert(ExpandedUniversal_ArchiveCount()==1u);
        /* an archive without a registry, or a non-string archive: rejected */
        strcpy(corrupted,s_disk);
        q=strstr(corrupted,"\"universal_archive\":\"");assert(q);
        memcpy(q+strlen("\"universal_archive\":"),"null",4);
        memmove(q+strlen("\"universal_archive\":")+4,
                strchr(q+strlen("\"universal_archive\":")+1,'"')+1,
                strlen(strchr(q+strlen("\"universal_archive\":")+1,'"')+1)+1);
        /* null archive == an old CAR: valid, restores with an empty archive */
        assert(ExpandedUniversal_CarValidate(corrupted,(int)strlen(corrupted)));
        assert(ExpandedUniversal_CarRestore(corrupted,(int)strlen(corrupted)));
        assert(ExpandedUniversal_ArchiveCount()==0u);
    }
    /* v292: BACKLOG RECOVERY (controlled crash). CAR A persisted after two
     * results; four more results are produced WITHOUT being persisted; the
     * "process" dies; CONTINUE from A recomputes exactly those four in the same
     * order with the same persisted RNG state and reaches IDENTICAL results;
     * the two durable results are never replayed. The RNG is a stateful mock of
     * the OG score source (the real fields random_seed/seed_2/... are persisted
     * in the CAR); proving the OG scorer itself needs gameplay. */
    {
        static char carA[65536];
        uint64_t rngA,digest1=0u,digest2=0u,res1[4],res2[4];
        unsigned k,gamesA;
        ExpandedUniversalFixture bf[6];
        ExpandedUniversalOps bops={ExpandedUniversal_CarSaveClaim,
            run_original_rng,ExpandedUniversal_CarSaveCompletion,NULL,NULL,NULL};
        ExpandedUniversal_CarResetCurrent();
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},8u));
        r=ExpandedUniversal_CarRegistry();
        for(k=0;k<6u;++k) {
            bf[k]=fixture(UINT64_C(0x2000000000000100)+k,k+1u,
                (45u<<16)|(2u*k+1u),(45u<<16)|(2u*k+2u),d);
            bf[k].key.edition.competition.stable_competition=450002u;
            assert(ExpandedUniversal_Add(r,bf[k])==EXP_UNIVERSAL_OK);
        }
        s_civil_clock=d;s_rng=UINT64_C(0x9e3779b97f4a7c15);s_rng_log_n=0u;
        assert(Expanded_AutoCareer_SaveChecked("initial-b"));
        for(k=0;k<2u;++k)
            assert(ExpandedUniversal_RunDue(r,d,&bops)==EXP_UNIVERSAL_OK);
        memcpy(carA,s_disk,sizeof carA);rngA=s_rng;gamesA=s_games;
        for(k=0;k<4u;++k) {
            assert(ExpandedUniversal_RunDue(r,d,&bops)==EXP_UNIVERSAL_OK);
            res1[k]=s_rng_log[s_rng_log_n-1u];digest1=digest1*1099511628211ull+res1[k]+1u;
        }
        assert(s_games==gamesA+4u);
        /* crash: only CAR A survives */
        assert(ExpandedUniversal_CarRestore(carA,(int)strlen(carA)));
        r=ExpandedUniversal_CarRegistry();
        s_rng=rngA;
        for(k=0;k<4u;++k) {
            assert(ExpandedUniversal_RunDue(r,d,&bops)==EXP_UNIVERSAL_OK);
            res2[k]=s_rng_log[s_rng_log_n-1u];digest2=digest2*1099511628211ull+res2[k]+1u;
        }
        assert(s_games==gamesA+8u);                  /* 2 durable never replayed */
        assert(digest1==digest2 && !memcmp(res1,res2,sizeof res1));
        assert(ExpandedUniversal_RunDue(r,d,&bops)!=EXP_UNIVERSAL_OK ||
               s_games==gamesA+8u);                  /* nothing left to play */
        /* a WRONG RNG after CONTINUE would diverge: the guard is persisting it */
        assert(ExpandedUniversal_CarRestore(carA,(int)strlen(carA)));
        r=ExpandedUniversal_CarRegistry();
        s_rng=rngA^1u;
        assert(ExpandedUniversal_RunDue(r,d,&bops)==EXP_UNIVERSAL_OK);
        assert(s_rng_log[s_rng_log_n-1u]!=res1[0]);
    }
    printf("UNIVERSAL_CAR_CODEC_REAL_C11=PASS claims and ACK, CRC, world, rollback, legacy\n");
    printf("OG_50HZ_GAMEPLAY=NOT_TESTED LIVE_CUTOVER=NOT_IMPLEMENTED\n");
    return 0;
}
