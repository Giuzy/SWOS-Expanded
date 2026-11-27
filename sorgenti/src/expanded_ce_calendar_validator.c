/* Independent CE calendar validator. See include/expanded_ce_calendar_validator.h.
 * Only reads fixtures; derives everything (pairs, sequences, days) itself. */
#include "expanded_ce_calendar_validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static void set(ExpCevReport *r,unsigned id,ExpCevStatus st,int hard,
                int32_t expected,int32_t found,uint64_t ev)
{
    ExpCevItem *c;
    if(!r || id<1u || id>EXP_CEV_CRITERIA)return;
    c=&r->c[id];
    /* a FAIL is never overwritten by a later PASS of the same criterion */
    if(c->status==EXP_CEV_FAIL && st!=EXP_CEV_FAIL)return;
    c->status=(uint8_t)st;c->hard=(uint8_t)(hard?1:0);
    c->expected=expected;c->found=found;
    if(st==EXP_CEV_FAIL) {
        if(hard)++r->hard_failures;
        if(!r->first_bad_event)r->first_bad_event=ev;
    }
}
void ExpCev_Init(ExpCevReport *r){if(r)memset(r,0,sizeof *r);}
const char *ExpCev_StatusName(uint8_t s)
{
    return s==EXP_CEV_PASS?"PASS":s==EXP_CEV_FAIL?"FAIL":s==EXP_CEV_UNKNOWN?"UNKNOWN":"N/A";
}
static int club_index(const uint32_t *clubs,uint32_t n,uint32_t c)
{
    uint32_t i;
    for(i=0;i<n;++i)if(clubs[i]==c)return (int)i;
    return -1;
}
typedef struct CevRef {int32_t day;uint64_t ev;uint32_t serial;} CevRef;
static int ref_cmp(const void *a,const void *b)
{
    const CevRef *x=(const CevRef *)a,*y=(const CevRef *)b;
    if(x->day!=y->day)return x->day<y->day?-1:1;
    return x->serial<y->serial?-1:x->serial>y->serial?1:0;
}

void ExpCev_RoundRobin(const ExpandedUniversalFixture *f,uint32_t n,
                       ExpandedEditionKey edition,uint32_t stage,
                       const ExpCevRoundRobin *e,ExpCevReport *r)
{
    uint32_t N,i,a,b,count=0u,*pair_n=NULL,*pair_home=NULL,*pair_first=NULL;
    uint32_t *club_n=NULL,*club_h=NULL;
    CevRef **seq=NULL;
    const ExpandedUniversalFixture **sf=NULL;
    int32_t worst_run=0,worst_gap=INT32_MAX,worst_ha=0;
    uint32_t mirrored=0u,returned=0u,extra_pairs=0u;
    int bad=0;
    if(!f || !e || !r || !e->clubs || e->nclubs<2u || e->cycles<1u)return;
    N=e->nclubs;
    sf=(const ExpandedUniversalFixture **)calloc(n?n:1u,sizeof *sf);
    pair_n=(uint32_t *)calloc((size_t)N*N,sizeof *pair_n);
    pair_home=(uint32_t *)calloc((size_t)N*N,sizeof *pair_home);
    pair_first=(uint32_t *)calloc((size_t)N*N,sizeof *pair_first);
    club_n=(uint32_t *)calloc(N,sizeof *club_n);
    club_h=(uint32_t *)calloc(N,sizeof *club_h);
    seq=(CevRef **)calloc(N,sizeof *seq);
    if(!sf||!pair_n||!pair_home||!pair_first||!club_n||!club_h||!seq)goto out;
    for(i=0;i<n;++i)
        if(ExpandedEditionKey_Equal(f[i].key.edition,edition) && f[i].key.stage==stage)
            sf[count++]=&f[i];
    r->fixtures+=count;
    for(i=0;i<N;++i)if(!(seq[i]=(CevRef *)calloc(count?count:1u,sizeof **seq)))goto out;
    /* 01 participants: every side is a listed participant, never itself */
    for(i=0;i<count;++i) {
        const int h=club_index(e->clubs,N,sf[i]->home.stable_club);
        const int w=club_index(e->clubs,N,sf[i]->away.stable_club);
        if(h<0 || w<0 || h==w) {
            set(r,1,EXP_CEV_FAIL,1,0,(int32_t)(h<0||w<0?-1:h),sf[i]->event_id);bad=1;continue;
        }
        a=(uint32_t)(h<w?h:w);b=(uint32_t)(h<w?w:h);
        ++pair_n[a*N+b];
        if((uint32_t)h==a)++pair_home[a*N+b];
        ++club_n[h];++club_n[w];++club_h[h];
        seq[h][club_n[h]-1u]=(CevRef){sf[i]->day_index,sf[i]->event_id,(uint32_t)i|0x80000000u};
        seq[w][club_n[w]-1u]=(CevRef){sf[i]->day_index,sf[i]->event_id,(uint32_t)i};
    }
    if(!bad)set(r,1,EXP_CEV_PASS,1,(int32_t)N,(int32_t)N,0);
    /* 02 cardinality from the regulation (not from the generator) */
    {
        const uint64_t exp_total=(uint64_t)e->cycles*N*(N-1u)/2u+
            (uint64_t)e->extra_rounds*N/2u;
        int ok=count==exp_total;
        for(a=0;a<N && ok;++a)
            if(club_n[a]!=(uint32_t)e->cycles*(N-1u)+e->extra_rounds)ok=0;
        for(a=0;a<N;++a)for(b=a+1u;b<N;++b) {
            const uint32_t k=pair_n[a*N+b];
            if(k<e->cycles || k>(uint32_t)e->cycles+(e->extra_rounds?1u:0u))ok=0;
            if(k==(uint32_t)e->cycles+1u)++extra_pairs;
        }
        set(r,2,ok?EXP_CEV_PASS:EXP_CEV_FAIL,1,(int32_t)exp_total,(int32_t)count,
            ok?0:(count?sf[0]->event_id:0));
        /* 05 extra rounds: exactly extra_rounds*N/2 additional meetings */
        if(e->extra_rounds)
            set(r,5,extra_pairs==(uint32_t)e->extra_rounds*N/2u?EXP_CEV_PASS:EXP_CEV_FAIL,1,
                (int32_t)(e->extra_rounds*N/2u),(int32_t)extra_pairs,0);
    }
    /* 06 home/away: in an even number of meetings each side hosts half */
    {
        int ok=1;
        for(a=0;a<N;++a)for(b=a+1u;b<N;++b) {
            const uint32_t k=pair_n[a*N+b],hk=pair_home[a*N+b];
            if(k==e->cycles && !(e->cycles&1u) && hk*2u!=k)ok=0;
        }
        for(a=0;a<N;++a) {
            const int32_t d=(int32_t)(2u*club_h[a])-(int32_t)club_n[a];
            if((d<0?-d:d)>worst_ha)worst_ha=d<0?-d:d;
        }
        set(r,6,ok?EXP_CEV_PASS:EXP_CEV_FAIL,1,e->cycles&1u?(int32_t)N:0,worst_ha,0);
    }
    /* per-club chronological sequences: 27 (stage), 07 runs, 04/03 returns */
    for(a=0;a<N;++a) {
        int32_t run=0;
        int last=-1;
        uint32_t k;
        qsort(seq[a],club_n[a],sizeof **seq,ref_cmp);
        for(k=0;k<club_n[a];++k) {
            const int home=(seq[a][k].serial&0x80000000u)!=0u;
            if(k && seq[a][k].day==seq[a][k-1u].day) {
                set(r,27,EXP_CEV_FAIL,1,1,2,seq[a][k].ev);
            }
            run=home==last?run+1:1;last=home;
            if(run>worst_run)worst_run=run;
        }
        /* return distance in this club's own matches (logical rounds) */
        for(b=0;b<N;++b) {
            uint32_t p0=UINT32_MAX,p1=UINT32_MAX;
            if(a==b)continue;
            for(k=0;k<club_n[a];++k) {
                const ExpandedUniversalFixture *x=sf[seq[a][k].serial&0x7fffffffu];
                const uint32_t o=x->home.stable_club==e->clubs[a]?x->away.stable_club:x->home.stable_club;
                if(o!=e->clubs[b])continue;
                if(p0==UINT32_MAX)p0=k;else if(p1==UINT32_MAX)p1=k;
            }
            if(p1!=UINT32_MAX) {
                const int32_t g=(int32_t)(p1-p0);
                if(g<worst_gap)worst_gap=g;
                ++returned;
                if(!(N&1u) && g==(int32_t)(N-1u))++mirrored;
            }
        }
    }
    if(r->c[27].status!=EXP_CEV_FAIL)set(r,27,EXP_CEV_PASS,1,1,1,0);
    if(e->max_run)set(r,7,worst_run<=e->max_run?EXP_CEV_PASS:EXP_CEV_FAIL,1,e->max_run,worst_run,0);
    else set(r,7,EXP_CEV_NA,0,0,worst_run,0);
    if(returned) {
        if(e->min_return_gap)
            set(r,4,worst_gap>=e->min_return_gap?EXP_CEV_PASS:EXP_CEV_FAIL,1,
                e->min_return_gap,worst_gap,0);
        else set(r,4,EXP_CEV_NA,0,0,worst_gap,0);
        if((N&1u) || !e->mirror)set(r,3,EXP_CEV_NA,0,e->mirror,(int32_t)mirrored,0);
        else if(e->mirror==1u)
            set(r,3,mirrored==returned?EXP_CEV_PASS:EXP_CEV_FAIL,1,(int32_t)returned,(int32_t)mirrored,0);
        else
            set(r,3,mirrored<returned?EXP_CEV_PASS:EXP_CEV_FAIL,0,0,(int32_t)mirrored,0);
    }
out:
    if(seq)for(i=0;i<N;++i)free(seq[i]);
    free(seq);free(sf);free(pair_n);free(pair_home);free(pair_first);free(club_n);free(club_h);
}

typedef struct CevDay {uint32_t club;int32_t day;uint64_t ev;} CevDay;
static int cday_cmp(const void *a,const void *b)
{
    const CevDay *x=(const CevDay *)a,*y=(const CevDay *)b;
    if(x->club!=y->club)return x->club<y->club?-1:1;
    return x->day<y->day?-1:x->day>y->day?1:0;
}
static int u64_cmp(const void *a,const void *b)
{
    const uint64_t x=*(const uint64_t *)a,y=*(const uint64_t *)b;
    return x<y?-1:x>y?1:0;
}
void ExpCev_Global(const ExpandedUniversalFixture *f,uint32_t n,uint32_t min_rest,
                   ExpCevReport *r)
{
    CevDay *d;
    uint64_t *ids;
    uint32_t i,k=0u;
    int32_t min_gap=INT32_MAX;
    int coll=0,dup=0;
    uint64_t bad=0;
    if(!f || !r || !n)return;
    d=(CevDay *)malloc((size_t)n*2u*sizeof *d);
    ids=(uint64_t *)malloc((size_t)n*sizeof *ids);
    if(!d || !ids){free(d);free(ids);return;}
    for(i=0;i<n;++i) {
        d[k++]=(CevDay){f[i].home.stable_club|((uint32_t)f[i].home.kind<<31),f[i].day_index,f[i].event_id};
        d[k++]=(CevDay){f[i].away.stable_club|((uint32_t)f[i].away.kind<<31),f[i].day_index,f[i].event_id};
        ids[i]=f[i].event_id;
    }
    qsort(d,k,sizeof *d,cday_cmp);
    for(i=1;i<k;++i)
        if(d[i].club==d[i-1u].club) {
            const int32_t g=d[i].day-d[i-1u].day;
            if(!g){coll=1;if(!bad)bad=d[i].ev;}
            else if(g<min_gap)min_gap=g;
        }
    qsort(ids,n,sizeof *ids,u64_cmp);
    for(i=1;i<n;++i)if(ids[i]==ids[i-1u])dup=1;
    set(r,27,coll?EXP_CEV_FAIL:EXP_CEV_PASS,1,1,coll?2:1,bad);
    if(min_gap==INT32_MAX)set(r,28,EXP_CEV_NA,0,(int32_t)min_rest,0,0); /* no club plays twice */
    else if(min_rest)set(r,28,min_gap>=(int32_t)min_rest?EXP_CEV_PASS:EXP_CEV_FAIL,1,(int32_t)min_rest,min_gap,0);
    else set(r,28,EXP_CEV_NA,0,0,min_gap,0);
    set(r,21,dup?EXP_CEV_FAIL:EXP_CEV_PASS,1,0,dup,0);   /* identity unique */
    free(d);free(ids);
}

void ExpCev_DayProfile(const ExpandedUniversalFixture *f,uint32_t n,
                       const uint8_t expected[7],ExpCevReport *r)
{
    uint32_t got[7]={0},i;
    int ok=1;
    if(!f || !r || !expected)return;
    for(i=0;i<n;++i)++got[(uint32_t)f[i].day_index%7u]; /* 1900-01-01 Monday */
    for(i=0;i<7u;++i)if(got[i]!=expected[i])ok=0;
    set(r,15,ok?EXP_CEV_PASS:EXP_CEV_FAIL,0,
        (int32_t)(expected[4]*1000+expected[5]*100+expected[6]*10+expected[0]),
        (int32_t)(got[4]*1000+got[5]*100+got[6]*10+got[0]),0);
}

uint32_t ExpCev_Log(const ExpCevReport *r,const char *tag)
{
    unsigned id;
    if(!r)return 0;
    for(id=1;id<=EXP_CEV_CRITERIA;++id) {
        const ExpCevItem *c=&r->c[id];
        if(c->status==EXP_CEV_NA && !c->found)continue;
        if(c->status==EXP_CEV_PASS && !c->hard)continue;
        if(c->status!=EXP_CEV_FAIL && c->status!=EXP_CEV_UNKNOWN)continue;
        fprintf(stderr,"[ce-validate] %s criterion=%02u %s %s expected=%ld found=%ld event=%llu\n",
            tag?tag:"-",id,ExpCev_StatusName(c->status),c->hard?"HARD":"SOFT",
            (long)c->expected,(long)c->found,(unsigned long long)r->first_bad_event);
    }
    return r->hard_failures;
}
