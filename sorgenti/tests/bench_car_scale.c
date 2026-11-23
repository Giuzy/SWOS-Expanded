/* v00.13.288 SCALE BENCHMARK (not a QA gate): cost of the CE snapshot, its
 * pure serialisation, validation and zstd compression with 100k / 500k /
 * 1.2M fixtures, plus the roster copy for the matching club count.
 * Real expanded_universal_car.c code; synthetic but VALID registry.
 * Prints RAM (snapshot) and times; says nothing about gameplay or disk. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <zstd.h>
#include "expanded_universal_car.h"
#include "game_types.h"

static WorldId s_world=WORLD_2627;
WorldId World_Current(void) {return s_world;}
void *swos_io_realloc(void *p,uint32_t n) {return realloc(p,n);}
void *swos_io_alloc(uint32_t n) {return malloc(n);}
void swos_io_free(void *p) {free(p);}
unsigned long long Platform_NowMs(void)
{
    return (unsigned long long)(SDL_GetPerformanceCounter()*1000u/
                                SDL_GetPerformanceFrequency());
}
void ExpandedUniversalLive_ResetCpuBatch(void) {}
void ExpandedUniversalLive_RegisterClock(void) {}
int Expanded_AutoCareer_SaveChecked(const char *r) {(void)r;return 1;}
void SaveJson_SetNextWriteAsync(int a) {(void)a;}

typedef struct Buf {char *p;size_t len,cap;} Buf;
static int sink(const char *c,int n,void *u)
{
    Buf *b=(Buf *)u;
    if(b->len+(size_t)n+1u>b->cap) {
        size_t w=b->cap?b->cap:65536u;
        while(w<b->len+(size_t)n+1u)w*=2u;
        b->p=(char *)realloc(b->p,w);b->cap=w;
    }
    memcpy(b->p+b->len,c,(size_t)n);b->len+=(size_t)n;
    return n;
}
static double ms_now(void)
{
    return 1000.0*(double)SDL_GetPerformanceCounter()/
           (double)SDL_GetPerformanceFrequency();
}
static void bench(uint32_t n)
{
    enum {CLUBS=2000};
    const uint32_t per_day=CLUBS/2u;
    ExpandedUniversalRegistry *r;
    ExpandedUniversalCarSnapshot *snap=NULL;
    Buf out={0};
    double t0,t1,t2,t3,t4;
    size_t bound,cs;
    void *zbuf;
    TeamFileEx *rost;
    int32_t base=0;
    uint32_t i;
    s_world=WORLD_2627;
    ExpandedUniversal_CarResetCurrent();
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2026,7,1},&base));
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},n));
    r=ExpandedUniversal_CarRegistry();assert(r);
    for(i=0;i<n;++i) {
        const uint32_t day=i/per_day,k=i%per_day;
        ExpandedUniversalFixture *f=&r->fixtures[i];
        ExpandedFixtureLedgerEntry *l=&r->ledger.entries[i];
        ExpandedScheduledEvent *e=&r->scheduler.events[i];
        memset(f,0,sizeof *f);
        f->event_id=UINT64_C(0x2000000000000000)+i+1u;
        f->day_index=base+(int32_t)(day*3u);   /* 3-day rounds */
        f->home=(ExpandedClubKey){WORLD_2627,(45u<<16)|(2u*k+1u)};
        f->away=(ExpandedClubKey){WORLD_2627,(45u<<16)|(2u*k+2u)};
        f->key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,450001u};
        f->key.edition.start_day=base;
        f->key.edition.sequence=1u;
        f->key.stage=1u;f->key.tie=i+1u;
        f->country=45u;f->slot=0u;f->competition_type=0u;
        memset(l,0,sizeof *l);
        l->key=f->key;l->state=EXP_FIXTURE_SCHEDULED;
        *e=(ExpandedScheduledEvent){f->event_id,0u,f->day_index,10u,
            EXP_EVENT_FIXTURE,EXP_EVENT_PENDING};
    }
    r->fixture_count=r->ledger.count=r->scheduler.count=n;
    r->fixtures_sorted_by_event=1u;r->pending_hint=0u;
    t0=ms_now();
    if(!ExpandedUniversal_ValidateRestore(r)) {
        printf("n=%u SYNTHETIC_REGISTRY_INVALID (bench cannot continue)\n",n);
        return;
    }
    t1=ms_now();
    assert(ExpandedUniversal_CarSnapshotCapture(&snap) && snap);
    t2=ms_now();
    assert(ExpandedUniversal_CarEmitSnapshot(snap,sink,&out));
    t3=ms_now();
    {   /* v290: decode + full validation of the section (CONTINUE cost) */
        char head[96];
        const int hl=snprintf(head,sizeof head,"{\"living_world\":{\"now\":%ld}",(long)r->scheduler.clock.day_index);
        char *js=(char *)malloc((size_t)hl+out.len+2u);
        double d0,d1;
        assert(js);memcpy(js,head,(size_t)hl);memcpy(js+hl,out.p,out.len);
        js[hl+out.len]='}';js[hl+out.len+1u]=0;
        d0=ms_now();
        assert(ExpandedUniversal_CarValidate(js,(int)(hl+out.len+1u)));
        d1=ms_now();
        printf("n=%-8u decode+validate(CONTINUE section)=%7.1f ms\n",n,d1-d0);
        free(js);
    }
    bound=ZSTD_compressBound(out.len);zbuf=malloc(bound);assert(zbuf);
    cs=ZSTD_compress(zbuf,bound,out.p,out.len,3);
    t4=ms_now();
    rost=(TeamFileEx *)malloc((size_t)(n/60u+1u)*sizeof *rost);
    {   /* roster copy for ~n/60 clubs (about 2000 clubs per 120k fixtures) */
        const uint32_t clubs=n/60u+1u;
        const double c0=ms_now();
        TeamFileEx *dst=(TeamFileEx *)malloc((size_t)clubs*sizeof *dst);
        memset(rost,0,(size_t)clubs*sizeof *rost);
        memcpy(dst,rost,(size_t)clubs*sizeof *dst);
        printf("n=%-8u snapshot_ram=%7.1f MB  capture=%6.1f ms  "
               "emit(validate+hex)=%7.1f ms  raw=%7.1f MB  zstd3=%6.1f ms "
               "-> %6.1f MB   [validate-live=%6.1f ms]  roster_copy(%u clubs, "
               "%.1f MB)=%5.1f ms\n",
               n,
               (double)ExpandedUniversal_CarSnapshotBytes(snap)/1048576.0,
               t2-t1,t3-t2,(double)out.len/1048576.0,t4-t3,
               (double)cs/1048576.0,t1-t0,clubs,
               (double)clubs*sizeof *dst/1048576.0,ms_now()-c0);
        free(dst);
    }
    free(rost);free(zbuf);free(out.p);
    ExpandedUniversal_CarSnapshotRelease(snap);
    ExpandedUniversal_CarResetCurrent();
}
int main(int argc,char **argv)
{
    uint32_t sizes[3]={100000u,500000u,1200000u};
    int i;
    (void)argc;(void)argv;
    for(i=0;i<3;++i)bench(sizes[i]);
    return 0;
}
