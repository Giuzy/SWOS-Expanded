/* One ordered CAR writer. See include/car_writer.h for the contract. */
#ifdef SWOS_SDL_BUILD
#include "car_writer.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Job {
    char name[128];
    uint8_t *json;
    uint32_t len;
    void *ctx;              /* snapshot job context (prepare path) */
    uint64_t generation;
} Job;

static struct {
    SDL_mutex *lock;
    SDL_cond *wake,*done;
    SDL_Thread *thread;
    CarWriterOps ops;
    int ops_set,busy,pending,stopping,failure_flag,atexit_registered;
    Job job;
    uint64_t next_generation;
    CarWriterStats stats;
} W;

static uint32_t now_ms(void)
{
    return (uint32_t)(SDL_GetPerformanceCounter()*1000u/
                      SDL_GetPerformanceFrequency());
}
static int writer_init(void)
{
    if(W.lock)return 1;
    W.lock=SDL_CreateMutex();
    W.wake=SDL_CreateCond();
    W.done=SDL_CreateCond();
    if(!W.lock || !W.wake || !W.done) {
        if(W.lock)SDL_DestroyMutex(W.lock);
        if(W.wake)SDL_DestroyCond(W.wake);
        if(W.done)SDL_DestroyCond(W.done);
        W.lock=NULL;W.wake=W.done=NULL;
        return 0;
    }
    return 1;
}
void CarWriter_SetOps(const CarWriterOps *ops)
{
    if(!writer_init())return;
    SDL_LockMutex(W.lock);
    if(ops) {W.ops=*ops;W.ops_set=ops->encode && ops->write && ops->verify &&
        ops->free_json && ops->free_frame &&
        (!ops->prepare || ops->free_ctx);}
    else {memset(&W.ops,0,sizeof W.ops);W.ops_set=0;}
    SDL_UnlockMutex(W.lock);
}
/* Runs on the thread executing the job; touches only the job and the ops. */
static int run_job(Job *j,const CarWriterOps *ops,uint32_t *prep_ms,
                   uint32_t *enc_ms,uint32_t *wr_ms,uint32_t *ver_ms)
{
    uint8_t *frame=NULL;
    uint32_t frame_len=0u,tp,t0,t1,t2,t3;
    int result=CAR_WRITE_OK;
    *prep_ms=*enc_ms=*wr_ms=*ver_ms=0u;
    tp=now_ms();
    if(j->ctx) {
        /* snapshot job: build + validate the image from the snapshot ONLY */
        uint8_t *json=NULL;uint32_t len=0u;
        const int ok=ops->prepare && ops->prepare(j->ctx,&json,&len) &&
                     json && len;
        ops->free_ctx(j->ctx);j->ctx=NULL;
        if(!ok) {
            if(json)ops->free_json(json);
            *prep_ms=now_ms()-tp;
            return CAR_WRITE_PREPARE_FAILED;
        }
        j->json=json;j->len=len;
    }
    *prep_ms=now_ms()-tp;
    t0=now_ms();
    if(!ops->encode(j->json,j->len,&frame,&frame_len) || !frame ||
       !frame_len) {
        result=CAR_WRITE_ENCODE_FAILED;
        t1=t2=t3=now_ms();
    } else {
        t1=now_ms();
        if(ops->write(j->name,frame,frame_len)!=0) {
            result=CAR_WRITE_IO_FAILED;
            t2=t3=now_ms();
        } else {
            t2=now_ms();
            if(!ops->verify(j->name,j->json,j->len))
                result=CAR_WRITE_VERIFY_FAILED;
            t3=now_ms();
        }
    }
    *enc_ms=t1-t0;*wr_ms=t2-t1;*ver_ms=t3-t2;
    if(frame)ops->free_frame(frame);
    ops->free_json(j->json);
    return result;
}
/* caller holds the lock */
static void finish_job(uint64_t generation,int result,int async,
                       uint32_t prep,uint32_t enc,uint32_t wr,uint32_t ver)
{
    W.busy=0;
    ++W.stats.completed;
    W.stats.last_generation=generation;
    W.stats.last_ok=result==CAR_WRITE_OK;
    W.stats.last_prepare_ms=prep;
    W.stats.last_encode_ms=enc;W.stats.last_write_ms=wr;
    W.stats.last_verify_ms=ver;W.stats.last_total_ms=prep+enc+wr+ver;
    if(result!=CAR_WRITE_OK) {
        ++W.stats.failed;
        if(async)W.failure_flag=1;
    }
    fprintf(stderr,"[car-writer] generation=%llu mode=%s result=%d "
        "prepare_ms=%u encode_ms=%u write_ms=%u verify_ms=%u "
        "worker_total_ms=%u status=%s\n",
        (unsigned long long)generation,async?"ASYNC":"SYNC",result,
        (unsigned)prep,(unsigned)enc,(unsigned)wr,(unsigned)ver,
        (unsigned)(prep+enc+wr+ver),
        result==CAR_WRITE_OK?"DURABLE_AND_BYTE_VERIFIED":"FAILED");
    SDL_CondBroadcast(W.done);
}
static int worker_main(void *unused)
{
    (void)unused;
    SDL_LockMutex(W.lock);
    for(;;) {
        Job job;
        CarWriterOps ops;
        uint32_t prep,enc,wr,ver;
        int result;
        while(!W.pending && !W.stopping)SDL_CondWait(W.wake,W.lock);
        if(!W.pending)break; /* stopping and nothing left to write */
        job=W.job;ops=W.ops;W.pending=0;
        SDL_UnlockMutex(W.lock);
        result=run_job(&job,&ops,&prep,&enc,&wr,&ver);
        SDL_LockMutex(W.lock);
        finish_job(job.generation,result,1,prep,enc,wr,ver);
    }
    SDL_UnlockMutex(W.lock);
    return 0;
}
static int submit_job(const char *name,uint8_t *json_owned,uint32_t len,
                      void *ctx,int async,uint64_t *generation)
{
    Job job;
    CarWriterOps ops;
    uint32_t prep,enc,wr,ver;
    int result;
    if(generation)*generation=0u;
    if(!writer_init() || !W.ops_set || !name ||
       (ctx ? (json_owned!=NULL) : (!json_owned || !len)) ||
       (ctx && !W.ops.prepare) || strlen(name)>=sizeof job.name) {
        if(W.ops_set) {
            if(json_owned)W.ops.free_json(json_owned);
            if(ctx && W.ops.free_ctx)W.ops.free_ctx(ctx);
        }
        return CAR_WRITE_BAD_ARGUMENT;
    }
    SDL_LockMutex(W.lock);
    while(W.busy)SDL_CondWait(W.done,W.lock); /* one job in flight, ordered */
    memset(&job,0,sizeof job);
    snprintf(job.name,sizeof job.name,"%s",name);
    job.json=json_owned;job.len=len;job.ctx=ctx;
    job.generation=++W.next_generation;
    ++W.stats.submitted;
    if(generation)*generation=job.generation;
    ops=W.ops;
    W.busy=1;
    if(async) {
        if(!W.thread) {
            W.stopping=0;
            W.thread=SDL_CreateThread(worker_main,"car-writer",NULL);
            if(W.thread && !W.atexit_registered) {
                W.atexit_registered=1;
                atexit(CarWriter_Shutdown);
            }
        }
        if(W.thread) {
            W.job=job;W.pending=1;
            SDL_CondSignal(W.wake);
            SDL_UnlockMutex(W.lock);
            return CAR_WRITE_OK;
        }
        /* no thread available: degrade to the inline path, still ordered */
        async=0;
    }
    SDL_UnlockMutex(W.lock);
    result=run_job(&job,&ops,&prep,&enc,&wr,&ver);
    SDL_LockMutex(W.lock);
    finish_job(job.generation,result,0,prep,enc,wr,ver);
    SDL_UnlockMutex(W.lock);
    return result;
}
int CarWriter_Submit(const char *name,uint8_t *json_owned,uint32_t len,
                     int async,uint64_t *generation)
{
    return submit_job(name,json_owned,len,NULL,async,generation);
}
int CarWriter_SubmitPrepared(const char *name,void *ctx,int async,
                             uint64_t *generation)
{
    return submit_job(name,NULL,0u,ctx,async,generation);
}
int CarWriter_Wait(void)
{
    int ok;
    if(!W.lock)return 1;
    SDL_LockMutex(W.lock);
    while(W.busy)SDL_CondWait(W.done,W.lock);
    ok=W.stats.completed==0u || W.stats.last_ok;
    SDL_UnlockMutex(W.lock);
    return ok;
}
int CarWriter_Busy(void)
{
    int busy;
    if(!W.lock)return 0;
    SDL_LockMutex(W.lock);
    busy=W.busy;
    SDL_UnlockMutex(W.lock);
    return busy;
}
int CarWriter_TakeFailure(void)
{
    int flag;
    if(!W.lock)return 0;
    SDL_LockMutex(W.lock);
    flag=W.failure_flag;W.failure_flag=0;
    SDL_UnlockMutex(W.lock);
    return flag;
}
void CarWriter_Stats(CarWriterStats *out)
{
    if(!out)return;
    memset(out,0,sizeof *out);
    if(!W.lock)return;
    SDL_LockMutex(W.lock);
    *out=W.stats;
    SDL_UnlockMutex(W.lock);
}
void CarWriter_Shutdown(void)
{
    SDL_Thread *thread;
    if(!W.lock)return;
    SDL_LockMutex(W.lock);
    thread=W.thread;
    W.stopping=1;
    SDL_CondSignal(W.wake);
    SDL_UnlockMutex(W.lock);
    if(thread)SDL_WaitThread(thread,NULL); /* drains a pending job first */
    SDL_LockMutex(W.lock);
    W.thread=NULL;W.stopping=0;
    SDL_UnlockMutex(W.lock);
}

/* ---- v289: pure helpers --------------------------------------------------- */
size_t CarImage_Splice(const char *main,size_t main_len,size_t p1s,size_t p1e,
                       size_t p2s,size_t p2e,const char *a,size_t alen,
                       const char *b,size_t blen,char *out,size_t cap)
{
    size_t total,at=0u;
    if(!main || (!a && alen) || (!b && blen) || p1s>p1e || p1e>p2s ||
       p2s>p2e || p2e>main_len)return (size_t)-1;
    total=main_len-(p1e-p1s)-(p2e-p2s);
    if(alen>(size_t)-1-total || blen>(size_t)-1-total-alen)return (size_t)-1;
    total+=alen+blen;
    if(!out)return total;
    if(cap<total)return (size_t)-1;
    memcpy(out+at,main,p1s);at+=p1s;
    if(alen) {memcpy(out+at,a,alen);}
    at+=alen;
    memcpy(out+at,main+p1e,p2s-p1e);at+=p2s-p1e;
    if(blen) {memcpy(out+at,b,blen);}
    at+=blen;
    memcpy(out+at,main+p2e,main_len-p2e);at+=main_len-p2e;
    if(cap>at)out[at]=0;
    return at;
}
void CarGate_Init(CarGate *g,int proofs_needed)
{
    if(!g)return;
    g->state=CAR_GATE_VERIFYING;g->proofs=0;
    g->proofs_needed=proofs_needed>0?proofs_needed:1;
}
int CarGate_Decide(CarGate *g,const CarGateEnv *env,int *shadow)
{
    if(shadow)*shadow=0;
    if(!g || (env && env->force_off) || g->state==CAR_GATE_OFF)return 0;
    if(env && env->force_on) {g->state=CAR_GATE_ON;return 1;}
    if(env && env->verify_only) {if(shadow)*shadow=1;return 1;}
    if(g->state==CAR_GATE_ON)return 1;
    if(shadow)*shadow=1; /* still proving equivalence on this career */
    return 1;
}
void CarGate_Report(CarGate *g,const CarGateEnv *env,int equal)
{
    if(!g || (env && env->verify_only))return;
    if(!equal) {g->state=CAR_GATE_OFF;return;}
    if(++g->proofs>=g->proofs_needed && g->state==CAR_GATE_VERIFYING)
        g->state=CAR_GATE_ON;
}

#endif
