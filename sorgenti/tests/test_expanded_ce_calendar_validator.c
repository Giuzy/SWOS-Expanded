/* Validator of CE calendars: positive cases from the REAL pairing kernel
 * (even/odd clubs, 1..3 cycles, extra rounds) and deliberately broken
 * calendars (negative cases). Isolated C: NOT a Windows/gameplay test. */
#include "expanded_ce_calendar_validator.h"
#include "expanded_universal_pairings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(c) do{ if(!(c)){fprintf(stderr,"FAIL %s:%d %s\n",__FILE__,__LINE__,#c);++fails;} }while(0)

typedef struct Buf {ExpandedUniversalFixture f[4096];uint32_t n;uint32_t clubs[64];uint32_t rpc;} Buf;
static const ExpandedEditionKey ED={{1,999},46000,0};
static int emit(void *u,uint32_t cycle,uint32_t round,uint32_t serial,uint32_t h,uint32_t a)
{
    Buf *b=(Buf *)u;
    ExpandedUniversalFixture *x=&b->f[b->n];
    memset(x,0,sizeof *x);
    x->key.edition=ED;x->key.stage=1;x->key.tie=b->n+1u;x->key.leg=(uint16_t)cycle;
    x->home=(ExpandedClubKey){1,b->clubs[h],0};x->away=(ExpandedClubKey){1,b->clubs[a],0};
    x->day_index=46000+7*(int32_t)(cycle*b->rpc+round);
    x->event_id=1000u+b->n;(void)serial;
    ++b->n;
    return 1;
}
static void build(Buf *b,uint32_t n,uint8_t cycles,const ExpPairExtras *x)
{
    uint32_t i;
    memset(b,0,sizeof *b);
    for(i=0;i<n;++i)b->clubs[i]=100u+i;
    b->rpc=(n&1u)?n:n-1u;
    if(x)CHECK(ExpandedUniversal_GenerateLeague(n,cycles,b->clubs,x,7u,emit,b));
    else CHECK(ExpandedUniversal_GenerateRoundRobin(n,cycles,emit,b));
}
static ExpCevReport check(const Buf *b,uint32_t n,uint8_t cycles,uint8_t extra,uint8_t gap,uint8_t run,uint8_t mirror)
{
    ExpCevReport r;
    ExpCevRoundRobin e={b->clubs,n,cycles,extra,gap,run,mirror};
    ExpCev_Init(&r);
    ExpCev_RoundRobin(b->f,b->n,ED,1,&e,&r);
    ExpCev_Global(b->f,b->n,7,&r);
    return r;
}

int main(void)
{
    static Buf b;
    ExpCevReport r;
    uint32_t n;
    uint8_t c;
    /* positive: even and odd, 1..3 cycles, mirror second cycle (OG default) */
    for(n=2;n<=21;++n)for(c=1;c<=3;++c) {
        build(&b,n,c,NULL);
        /* user rule 27/11: never more than 2 consecutive H or A (the
         * rotated circle order holds it for every size except 4 clubs in
         * a double round robin, where 3 is unavoidable) */
        r=check(&b,n,c,0,0,(n==4u && c>1u)?0:2,1);
        CHECK(r.hard_failures==0u && r.c[7].found<=((n==4u && c>1u)?3:2));
        CHECK(r.c[1].status==EXP_CEV_PASS && r.c[2].status==EXP_CEV_PASS &&
              r.c[27].status==EXP_CEV_PASS && r.c[21].status==EXP_CEV_PASS);
        CHECK(r.c[2].found==(int32_t)(c*n*(n-1u)/2u));
        if(c==2u && !(n&1u))CHECK(r.c[6].status==EXP_CEV_PASS && r.c[3].status==EXP_CEV_PASS);
        if(r.hard_failures)fprintf(stderr,"  n=%u c=%u\n",n,c),(void)ExpCev_Log(&r,"rr");
    }
    /* extra rounds (derbies) with rivalries */
    {
        ExpPairExtras x;
        memset(&x,0,sizeof x);
        x.rounds=2;x.nrivalries=1;x.rivalries[0][0]=100;x.rivalries[0][1]=101;
        build(&b,12,2,&x);
        r=check(&b,12,2,2,0,0,1);
        CHECK(r.hard_failures==0u && r.c[5].status==EXP_CEV_PASS);
    }
    /* ---- negative cases: the validator must catch every one ---- */
    build(&b,10,2,NULL);
    {   Buf *m=(Buf *)malloc(sizeof *m);
        *m=b;m->f[0].away=m->f[0].home;                 /* club v itself */
        r=check(m,10,2,0,0,0,1);CHECK(r.c[1].status==EXP_CEV_FAIL && r.hard_failures);
        *m=b;m->f[0].home.stable_club=999;               /* non participant */
        r=check(m,10,2,0,0,0,1);CHECK(r.c[1].status==EXP_CEV_FAIL);
        *m=b;m->f[m->n]=m->f[0];m->f[m->n].event_id=77777;m->f[m->n].day_index+=3;++m->n; /* pair too often */
        r=check(m,10,2,0,0,0,1);CHECK(r.c[2].status==EXP_CEV_FAIL);
        *m=b;--m->n;                                     /* missing fixture */
        r=check(m,10,2,0,0,0,1);CHECK(r.c[2].status==EXP_CEV_FAIL);
        *m=b;{ExpandedClubKey t=m->f[50].home;m->f[50].home=m->f[50].away;m->f[50].away=t;} /* return at the wrong ground */
        r=check(m,10,2,0,0,0,1);CHECK(r.c[6].status==EXP_CEV_FAIL);
        *m=b;m->f[1].day_index=m->f[0].day_index;m->f[1].home=m->f[0].home; /* same club same day */
        r=check(m,10,2,0,0,0,1);CHECK(r.c[27].status==EXP_CEV_FAIL);
        *m=b;m->f[1].event_id=m->f[0].event_id;           /* identity duplicated */
        r=check(m,10,2,0,0,0,1);CHECK(r.c[21].status==EXP_CEV_FAIL);
        *m=b;                                            /* rest impossible: 2 days */
        { ExpCevReport rr;ExpCev_Init(&rr);m->f[5].day_index=m->f[0].day_index+2;
          m->f[5].home=m->f[0].home;ExpCev_Global(m->f,m->n,3,&rr);CHECK(rr.c[28].status==EXP_CEV_FAIL);}
        *m=b;                                            /* return gap regulated 12 > 9 */
        r=check(m,10,2,0,12,0,1);CHECK(r.c[4].status==EXP_CEV_FAIL);
        *m=b;                                            /* mirror required but asymmetric claimed */
        r=check(m,10,2,0,0,0,2);CHECK(r.c[3].status==EXP_CEV_FAIL);
        *m=b;                                            /* max run 1 is impossible */
        r=check(m,10,2,0,0,1,1);CHECK(r.c[7].status==EXP_CEV_FAIL);
        free(m);
    }
    /* weekday profile: 1 FRI 3 SAT 5 SUN 1 MON (Monday=0) */
    {
        ExpandedUniversalFixture f[10];
        const uint8_t exp[7]={1,0,0,0,1,3,5};
        const int32_t sunday=46010; /* 46010 % 7 == 6: Sunday */
        uint32_t i;
        memset(f,0,sizeof f);
        for(i=0;i<10;++i)f[i].day_index=sunday;
        f[0].day_index=sunday-2;f[1].day_index=f[2].day_index=f[3].day_index=sunday-1;f[9].day_index=sunday+1;
        ExpCev_Init(&r);ExpCev_DayProfile(f,10,exp,&r);CHECK(r.c[15].status==EXP_CEV_PASS);
        f[9].day_index=sunday;ExpCev_Init(&r);ExpCev_DayProfile(f,10,exp,&r);CHECK(r.c[15].status==EXP_CEV_FAIL);
    }
    if(fails){fprintf(stderr,"CE_CALENDAR_VALIDATOR FAIL (%d)\n",fails);return 1;}
    puts("CE_CALENDAR_VALIDATOR_POSITIVE_NEGATIVE=PASS");
    return 0;
}
