/* v00.13.287 RG-T: the ordered CAR writer (real car_writer.c + real threads,
 * mock encode/write/verify on real files). Isolated; not gameplay.
 * Proves: single writer, ordered generations, no old save overwriting a new
 * one, sync saves wait for async ones, byte-exact verification (truncated,
 * corrupted, missing), I/O and encode failure leave the previous file intact,
 * async failure is reported once, the job buffer is freed exactly once, and
 * Shutdown drains the pending job. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "car_writer.h"

static char s_dir[512];
static int s_free_json,s_free_frame,s_delay_ms,s_fail_encode,s_fail_write,
           s_truncate,s_corrupt,s_writes;
static char s_last_written[64];
static void path_of(const char *name,char *out,size_t n)
{
    snprintf(out,n,"%s/%s",s_dir,name);
}
static int read_file(const char *name,uint8_t **buf,uint32_t *len)
{
    char path[640];
    FILE *f;
    long n;
    path_of(name,path,sizeof path);
    f=fopen(path,"rb");
    if(!f)return 0;
    fseek(f,0,SEEK_END);n=ftell(f);fseek(f,0,SEEK_SET);
    *buf=(uint8_t *)malloc((size_t)n+1u);
    if(fread(*buf,1,(size_t)n,f)!=(size_t)n){fclose(f);free(*buf);return 0;}
    fclose(f);(*buf)[n]=0;*len=(uint32_t)n;
    return 1;
}
static void put_file(const char *name,const char *text)
{
    char path[640];
    FILE *f;
    path_of(name,path,sizeof path);
    f=fopen(path,"wb");assert(f);
    fwrite(text,1,strlen(text),f);fclose(f);
}
/* mock container: 'F' + json (a frame is NOT the json: decode strips it) */
static int m_encode(const uint8_t *json,uint32_t len,uint8_t **frame,uint32_t *fl)
{
    if(s_fail_encode)return 0;
    *frame=(uint8_t *)malloc(len+1u);
    (*frame)[0]='F';memcpy(*frame+1,json,len);*fl=len+1u;
    return 1;
}
static void m_free_frame(uint8_t *f){++s_free_frame;free(f);}
static void m_free_json(uint8_t *j){++s_free_json;free(j);}
static int m_write(const char *name,const uint8_t *frame,uint32_t n)
{
    char path[640],tmp[660];
    FILE *f;
    if(s_delay_ms)SDL_Delay((Uint32)s_delay_ms);
    if(s_fail_write)return 1;
    path_of(name,path,sizeof path);
    snprintf(tmp,sizeof tmp,"%s.tmp",path);
    f=fopen(tmp,"wb");if(!f)return 1;
    {
        uint32_t put=s_truncate?n/2u:n;
        fwrite(frame,1,put,f);
    }
    fclose(f);
    if(s_corrupt) { /* flip a byte after the file is complete */
        f=fopen(tmp,"r+b");fseek(f,(long)(n/2u),SEEK_SET);
        fputc('Z',f);fclose(f);
    }
    remove(path);
    if(rename(tmp,path)!=0)return 1;
    ++s_writes;
    snprintf(s_last_written,sizeof s_last_written,"%.60s",(const char *)frame+1);
    return 0;
}
static int m_verify(const char *name,const uint8_t *json,uint32_t len)
{
    uint8_t *buf=NULL;
    uint32_t n=0u;
    int ok;
    if(!read_file(name,&buf,&n))return 0;
    ok=n==len+1u && buf[0]=='F' && memcmp(buf+1,json,len)==0;
    free(buf);
    return ok;
}
static const CarWriterOps OPS={m_encode,m_free_frame,m_free_json,m_write,m_verify,NULL,NULL};
/* snapshot-job mocks */
typedef struct PCtx {char text[32];int delay_ms;} PCtx;
static int s_prepare_calls,s_free_ctx;
static SDL_threadID s_prepare_thread,s_main_thread;
static int p_prepare(void *ctx,uint8_t **json,uint32_t *len)
{
    const PCtx *c=(const PCtx *)ctx;
    char text[80];
    ++s_prepare_calls;
    s_prepare_thread=SDL_ThreadID();
    if(c->delay_ms)SDL_Delay((Uint32)c->delay_ms);
    if(!strcmp(c->text,"FAIL"))return 0;
    snprintf(text,sizeof text,"prep:%s",c->text);
    *json=(uint8_t *)malloc(strlen(text)+1u);
    memcpy(*json,text,strlen(text)+1u);*len=(uint32_t)strlen(text);
    return 1;
}
static void p_free_ctx(void *ctx){++s_free_ctx;free(ctx);}
static void *make_ctx(const char *text,int delay_ms)
{
    PCtx *c=(PCtx *)calloc(1u,sizeof *c);
    snprintf(c->text,sizeof c->text,"%s",text);c->delay_ms=delay_ms;
    return c;
}
static uint8_t *dupjson(const char *text)
{
    uint8_t *p=(uint8_t *)malloc(strlen(text)+1u);
    memcpy(p,text,strlen(text)+1u);return p;
}
static int submit(const char *name,const char *body,int async,uint64_t *gen)
{
    return CarWriter_Submit(name,dupjson(body),(uint32_t)strlen(body),async,gen);
}
static void reset_flags(void)
{
    s_fail_encode=s_fail_write=s_truncate=s_corrupt=0;s_delay_ms=0;
}
static void expect_file(const char *name,const char *body)
{
    uint8_t *buf=NULL;uint32_t n=0u;
    assert(read_file(name,&buf,&n));
    assert(n==strlen(body)+1u && buf[0]=='F' && memcmp(buf+1,body,n-1u)==0);
    free(buf);
}
int main(int argc,char **argv)
{
    uint64_t g1=0,g2=0,g3=0;
    CarWriterStats st;
    Uint32 t0;
    assert(argc>1);
    snprintf(s_dir,sizeof s_dir,"%s",argv[1]);
    /* no ops installed: refuse, never crash */
    assert(submit("A.CAR","x",0,NULL)==CAR_WRITE_BAD_ARGUMENT);
    CarWriter_SetOps(&OPS);
    assert(CarWriter_Wait()==1 && !CarWriter_Busy());

    /* 1. synchronous, durable and byte-verified when Submit returns */
    assert(submit("A.CAR","{\"v\":1}",0,&g1)==CAR_WRITE_OK);
    expect_file("A.CAR","{\"v\":1}");
    assert(g1==1u && s_free_json==1 && s_free_frame==1);

    /* 2. async returns at once, one job in flight, ordered, newest wins */
    s_delay_ms=120;
    t0=SDL_GetTicks();
    assert(submit("A.CAR","{\"v\":2}",1,&g2)==CAR_WRITE_OK);
    assert(SDL_GetTicks()-t0<60u);          /* did not wait for the write */
    assert(CarWriter_Busy());
    t0=SDL_GetTicks();
    assert(submit("A.CAR","{\"v\":3}",1,&g3)==CAR_WRITE_OK); /* waits for v2 */
    assert(SDL_GetTicks()-t0>=60u);         /* backpressure, one at a time */
    assert(g2==g1+1u && g3==g2+1u);
    assert(CarWriter_Wait()==1);
    expect_file("A.CAR","{\"v\":3}");       /* the older job never overwrote */
    CarWriter_Stats(&st);
    assert(st.last_generation==g3 && st.last_ok && st.failed==0u);

    /* 3. a mandatory SYNC save waits for the async job and is the last word */
    s_delay_ms=100;
    assert(submit("A.CAR","{\"v\":4}",1,NULL)==CAR_WRITE_OK);
    s_delay_ms=0;
    assert(submit("A.CAR","{\"v\":5}",0,NULL)==CAR_WRITE_OK);
    assert(!CarWriter_Busy());
    expect_file("A.CAR","{\"v\":5}");

    /* 4. encode failure: nothing touched, buffer still freed once */
    {
        const int fj=s_free_json,w=s_writes;
        s_fail_encode=1;
        assert(submit("A.CAR","{\"v\":6}",0,NULL)==CAR_WRITE_ENCODE_FAILED);
        s_fail_encode=0;
        assert(s_writes==w && s_free_json==fj+1);
        expect_file("A.CAR","{\"v\":5}");
    }
    /* 5. I/O failure: previous file intact; sync reports it */
    s_fail_write=1;
    assert(submit("A.CAR","{\"v\":7}",0,NULL)==CAR_WRITE_IO_FAILED);
    expect_file("A.CAR","{\"v\":5}");
    /*    async I/O failure: accepted, then reported ONCE */
    assert(submit("A.CAR","{\"v\":8}",1,NULL)==CAR_WRITE_OK);
    assert(CarWriter_Wait()==0);
    assert(CarWriter_TakeFailure()==1 && CarWriter_TakeFailure()==0);
    s_fail_write=0;
    expect_file("A.CAR","{\"v\":5}");
    assert(submit("A.CAR","{\"v\":9}",0,NULL)==CAR_WRITE_OK); /* recovers */
    assert(CarWriter_Wait()==1);
    expect_file("A.CAR","{\"v\":9}");

    /* 6. byte-exact verification: truncated and corrupted files are caught */
    s_truncate=1;
    assert(submit("B.CAR","{\"payload\":\"0123456789abcdef\"}",0,NULL)==
           CAR_WRITE_VERIFY_FAILED);
    s_truncate=0;
    s_corrupt=1;
    assert(submit("B.CAR","{\"payload\":\"0123456789abcdef\"}",0,NULL)==
           CAR_WRITE_VERIFY_FAILED);
    s_corrupt=0;
    assert(submit("B.CAR","{\"payload\":\"0123456789abcdef\"}",0,NULL)==
           CAR_WRITE_OK);
    /*    a missing file after the write also fails verification */
    assert(!m_verify("MISSING.CAR",(const uint8_t *)"x",1u));

    /* 7. ownership: every submitted buffer freed exactly once */
    CarWriter_Stats(&st);
    assert((uint64_t)s_free_json==st.submitted);
    assert(s_free_frame>=1);

    /* 8. Shutdown drains the pending job before returning */
    s_delay_ms=150;
    assert(submit("C.CAR","{\"last\":true}",1,NULL)==CAR_WRITE_OK);
    CarWriter_Shutdown();
    s_delay_ms=0;
    expect_file("C.CAR","{\"last\":true}");
    assert(CarWriter_Wait()==1);
    /* and the writer works again afterwards */
    assert(submit("C.CAR","{\"again\":1}",1,NULL)==CAR_WRITE_OK);
    assert(CarWriter_Wait()==1);
    expect_file("C.CAR","{\"again\":1}");
    CarWriter_Shutdown();

    /* 9. v288 snapshot jobs: the image is built by prepare() from the job
     * context ONLY, on the executing thread; ctx freed exactly once. */
    {
        CarWriterOps ops2=OPS;
        ops2.prepare=p_prepare;ops2.free_ctx=p_free_ctx;
        CarWriter_SetOps(&ops2);
        s_main_thread=SDL_ThreadID();
        /* sync: runs inline on the caller */
        assert(CarWriter_SubmitPrepared("D.CAR",make_ctx("first",0),0,NULL)==
               CAR_WRITE_OK);
        expect_file("D.CAR","prep:first");
        assert(s_prepare_calls==1 && s_free_ctx==1 &&
               s_prepare_thread==s_main_thread);
        /* async: prepare() runs on the WORKER thread, caller not blocked */
        s_delay_ms=100;
        t0=SDL_GetTicks();
        assert(CarWriter_SubmitPrepared("D.CAR",make_ctx("second",40),1,&g1)==
               CAR_WRITE_OK);
        assert(SDL_GetTicks()-t0<50u);
        assert(CarWriter_Wait()==1);
        s_delay_ms=0;
        expect_file("D.CAR","prep:second");
        assert(s_prepare_thread!=s_main_thread && s_free_ctx==2);
        /* prepare failure: nothing written, file intact, ctx freed once,
         * reported as a failure and the writer keeps working */
        {
            const int w=s_writes;
            assert(CarWriter_SubmitPrepared("D.CAR",make_ctx("FAIL",0),0,NULL)==
                   CAR_WRITE_PREPARE_FAILED);
            assert(s_writes==w && s_free_ctx==3);
            expect_file("D.CAR","prep:second");
            assert(CarWriter_SubmitPrepared("D.CAR",make_ctx("FAIL",0),1,NULL)==
                   CAR_WRITE_OK);
            assert(CarWriter_Wait()==0 && CarWriter_TakeFailure()==1);
            assert(s_writes==w && s_free_ctx==4);
        }
        assert(CarWriter_SubmitPrepared("D.CAR",make_ctx("third",0),0,NULL)==
               CAR_WRITE_OK);
        expect_file("D.CAR","prep:third");
        /* a prepared job without prepare() ops is refused, ctx still freed */
        CarWriter_SetOps(&OPS);
        {
            void *cx=make_ctx("x",0);
            assert(CarWriter_SubmitPrepared("D.CAR",cx,0,NULL)==
                   CAR_WRITE_BAD_ARGUMENT);
            free(cx); /* no free_ctx op installed: ownership stays here */
        }
    }
    {   /* v289: splice (pure) */
        const char m[]="AAxxBByyCC";char out[64];size_t n;
        n=CarImage_Splice(m,10,2,4,6,8,"11",2,"2222",4,out,sizeof out);
        assert(n==12 && !memcmp(out,"AA11BB2222CC",12) && out[12]==0);
        assert(CarImage_Splice(m,10,2,4,6,8,"11",2,"2222",4,NULL,0)==12);
        assert(CarImage_Splice(m,10,0,0,10,10,"X",1,"Y",1,out,sizeof out)==12);
        assert(!memcmp(out,"XAAxxBByyCCY",12));
        assert(CarImage_Splice(m,10,3,3,3,3,NULL,0,NULL,0,out,sizeof out)==10);
        assert(CarImage_Splice(m,10,5,4,6,8,"",0,"",0,out,sizeof out)==(size_t)-1);
        assert(CarImage_Splice(m,10,2,4,3,8,"",0,"",0,out,sizeof out)==(size_t)-1);
        assert(CarImage_Splice(m,10,2,4,6,11,"",0,"",0,out,sizeof out)==(size_t)-1);
        assert(CarImage_Splice(m,10,2,4,6,8,"11",2,"2222",4,out,11)==(size_t)-1);
        assert(CarImage_Splice(m,10,2,4,6,8,NULL,2,"",0,out,64)==(size_t)-1);
    }
    {   /* v289: activation gate */
        CarGate g;CarGateEnv e={0,0,0};int sh;
        CarGate_Init(&g,3);
        assert(CarGate_Decide(&g,&e,&sh)==1 && sh==1);
        CarGate_Report(&g,&e,1);CarGate_Report(&g,&e,1);
        assert(g.state==CAR_GATE_VERIFYING);
        CarGate_Report(&g,&e,1);
        assert(g.state==CAR_GATE_ON);
        assert(CarGate_Decide(&g,&e,&sh)==1 && sh==0);
        CarGate_Report(&g,&e,0);          /* a mismatch always latches OFF */
        assert(g.state==CAR_GATE_OFF && CarGate_Decide(&g,&e,&sh)==0);
        CarGate_Init(&g,3);CarGate_Report(&g,&e,1);CarGate_Report(&g,&e,0);
        assert(g.state==CAR_GATE_OFF);
        { CarGateEnv f={1,0,0};CarGate_Init(&g,3);
          assert(CarGate_Decide(&g,&f,&sh)==1 && sh==0 && g.state==CAR_GATE_ON); }
        { CarGateEnv v={0,1,0};CarGate_Init(&g,3);
          assert(CarGate_Decide(&g,&v,&sh)==1 && sh==1);
          CarGate_Report(&g,&v,1);CarGate_Report(&g,&v,1);CarGate_Report(&g,&v,1);
          assert(g.state==CAR_GATE_VERIFYING); }
        { CarGateEnv o={0,0,1};CarGate_Init(&g,3);
          assert(CarGate_Decide(&g,&o,&sh)==0 && sh==0); }
    }
    CarWriter_Shutdown();
    reset_flags();
    (void)put_file;
    puts("CAR_WRITER_ORDERED_SINGLE_WRITER=PASS_ISOLATED_ONLY");
    puts("REAL_ZSTD_CONTAINER_AND_GAME_SAVE=NOT_TESTED_HERE");
    return 0;
}
