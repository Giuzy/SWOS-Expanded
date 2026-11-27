/* FIFA men's ranking state (WORLD 2627). See include/expanded_fifa_ranking.h.
 * Pure C: no OG, SDL or CE engine dependency (isolated tests link it alone). */
#include "expanded_fifa_ranking.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

typedef struct ExpFifaSnapshot {
    uint32_t competition, phase;
    int32_t start_day, day;
    uint16_t sequence, n;
    uint32_t clubs[EXP_FIFA_SNAPSHOT_CLUBS];
    double points[EXP_FIFA_SNAPSHOT_CLUBS];
} ExpFifaSnapshot;

typedef struct ExpFifaState {
    uint8_t active;
    uint32_t n;
    ExpFifaNation nations[EXP_FIFA_NATIONS];
    uint16_t order[EXP_FIFA_NATIONS];   /* rank order -> nation index */
    uint16_t pos[EXP_FIFA_NATIONS];     /* nation index -> shared position */
    int32_t week;                       /* civil week of week_points/week_pos */
    uint64_t *events;                   /* sorted applied CE event ids */
    uint32_t nevents, capevents;
    ExpFifaSnapshot *snaps;
    uint32_t nsnaps;
} ExpFifaState;
static ExpFifaState s_fifa;

static int32_t week_of(int32_t day){return day>=0?day/7:-((-day+6)/7);}
static long long cents(double p){return llround(p*100.0);}

static void free_state(ExpFifaState *s)
{
    free(s->events);free(s->snaps);
    memset(s,0,sizeof *s);
}
void ExpFifa_Reset(void){free_state(&s_fifa);}
int ExpFifa_Active(void){return s_fifa.active;}
uint32_t ExpFifa_Count(void){return s_fifa.active?s_fifa.n:0u;}
int32_t ExpFifa_WeekId(void){return s_fifa.week;}
uint32_t ExpFifa_EventCount(void){return s_fifa.nevents;}
uint32_t ExpFifa_SnapshotCount(void){return s_fifa.nsnaps;}
const ExpFifaNation *ExpFifa_Nation(uint32_t i)
{return s_fifa.active&&i<s_fifa.n?&s_fifa.nations[i]:NULL;}
int ExpFifa_ImportanceValid(int v)
{return v==5||v==10||v==15||v==25||v==35||v==40||v==50||v==60;}

/* Ranking order: points rounded to the displayed 0.01 decide the position;
 * equal displayed points share it (1,2,2,4). Inside a tie the raw value,
 * then the seed order, give a stable display order. */
static const ExpFifaState *s_sort_ctx;
static int rank_cmp(const void *a,const void *b)
{
    const ExpFifaNation *x=&s_sort_ctx->nations[*(const uint16_t *)a];
    const ExpFifaNation *y=&s_sort_ctx->nations[*(const uint16_t *)b];
    const long long cx=cents(x->points),cy=cents(y->points);
    if(cx!=cy)return cx>cy?-1:1;
    if(x->points!=y->points)return x->points>y->points?-1:1;
    return (int)x->seed_index-(int)y->seed_index;
}
static void resort(ExpFifaState *s)
{
    uint32_t i;
    for(i=0;i<s->n;++i)s->order[i]=(uint16_t)i;
    s_sort_ctx=s;
    qsort(s->order,s->n,sizeof s->order[0],rank_cmp);
    s_sort_ctx=NULL;
    for(i=0;i<s->n;++i) {
        const uint16_t k=s->order[i];
        if(i && cents(s->nations[k].points)==cents(s->nations[s->order[i-1u]].points))
            s->pos[k]=s->pos[s->order[i-1u]];
        else s->pos[k]=(uint16_t)(i+1u);
    }
}
/* Freeze the reference of a NEW week (positions and points unchanged). */
static void roll_week(ExpFifaState *s,int32_t day)
{
    const int32_t w=week_of(day);
    uint32_t i;
    if(w<=s->week)return;
    for(i=0;i<s->n;++i) {
        s->nations[i].week_points=s->nations[i].points;
        s->nations[i].week_pos=s->pos[i];
    }
    s->week=w;
}

int ExpFifa_FindClub(uint32_t club)
{
    uint32_t i;
    if(!s_fifa.active)return -1;
    for(i=0;i<s_fifa.n;++i)if(s_fifa.nations[i].stable_club==club)return (int)i;
    return -1;
}
int ExpFifa_EventApplied(uint64_t id)
{
    uint32_t lo=0,hi=s_fifa.nevents;
    while(lo<hi){const uint32_t m=(lo+hi)/2u;
        if(s_fifa.events[m]==id)return 1;
        if(s_fifa.events[m]<id)lo=m+1u;else hi=m;}
    return 0;
}
static int event_insert(ExpFifaState *s,uint64_t id)
{
    uint32_t lo=0,hi=s->nevents;
    while(lo<hi){const uint32_t m=(lo+hi)/2u;
        if(s->events[m]==id)return 0;
        if(s->events[m]<id)lo=m+1u;else hi=m;}
    if(s->nevents==s->capevents) {
        const uint32_t cap=s->capevents?s->capevents*2u:256u;
        uint64_t *e=(uint64_t *)realloc(s->events,(size_t)cap*sizeof *e);
        if(!e)return -1;
        s->events=e;s->capevents=cap;
    }
    memmove(s->events+lo+1u,s->events+lo,(size_t)(s->nevents-lo)*sizeof *s->events);
    s->events[lo]=id;++s->nevents;
    return 1;
}

ExpCeRankResult ExpFifa_ResultFromScore(unsigned h,unsigned a,unsigned ph,unsigned pa)
{
    if(h>a)return EXP_RANK_A_WIN;
    if(h<a)return EXP_RANK_B_WIN;
    if(ph!=pa)return ph>pa?EXP_RANK_A_PSO:EXP_RANK_B_PSO;
    return EXP_RANK_DRAW;
}

int ExpFifa_Apply(uint64_t event_id,int32_t day,uint32_t club_a,uint32_t club_b,
                  int importance,int protect,ExpCeRankResult result,ExpFifaUndo *undo)
{
    ExpFifaState *s=&s_fifa;
    int ia,ib;
    double na,nb;
    uint32_t i;
    if(undo)memset(undo,0,sizeof *undo);
    if(!s->active || !event_id || !ExpFifa_ImportanceValid(importance))return -1;
    ia=ExpFifa_FindClub(club_a);ib=ExpFifa_FindClub(club_b);
    if(ia<0 || ib<0 || ia==ib)return -1;
    if(ExpFifa_EventApplied(event_id))return 0;
    /* both teams use their PRE-match points */
    if(!ExpCeFifa_Sum(s->nations[ia].points,s->nations[ib].points,importance,
                      result,protect?1:0,&na,&nb))return -1;
    if(undo) {
        undo->event_id=event_id;undo->idx_a=ia;undo->idx_b=ib;
        undo->old_a=s->nations[ia].points;undo->old_b=s->nations[ib].points;
        undo->old_week=s->week;
        for(i=0;i<s->n;++i) {
            undo->old_wpos[i]=s->nations[i].week_pos;
            undo->old_wpts[i]=s->nations[i].week_points;
        }
    }
    if(event_insert(s,event_id)<0)return -1;
    roll_week(s,day);
    s->nations[ia].points=na;s->nations[ib].points=nb;
    resort(s);
    if(undo)undo->valid=1;
    return 1;
}
void ExpFifa_Undo(const ExpFifaUndo *u)
{
    ExpFifaState *s=&s_fifa;
    uint32_t i,lo=0,hi;
    if(!u || !u->valid || !s->active)return;
    s->nations[u->idx_a].points=u->old_a;
    s->nations[u->idx_b].points=u->old_b;
    s->week=u->old_week;
    for(i=0;i<s->n;++i) {
        s->nations[i].week_pos=u->old_wpos[i];
        s->nations[i].week_points=u->old_wpts[i];
    }
    hi=s->nevents;
    while(lo<hi){const uint32_t m=(lo+hi)/2u;
        if(s->events[m]==u->event_id){
            memmove(s->events+m,s->events+m+1u,(size_t)(s->nevents-m-1u)*sizeof *s->events);
            --s->nevents;break;}
        if(s->events[m]<u->event_id)lo=m+1u;else hi=m;}
    resort(s);
}

int ExpFifa_ViewRow(uint32_t rank,int32_t today,ExpFifaViewRow *out)
{
    const ExpFifaState *s=&s_fifa;
    const ExpFifaNation *n;
    uint16_t k;
    if(!out || !s->active || rank>=s->n)return 0;
    k=s->order[rank];n=&s->nations[k];
    out->nation=n;out->pos=s->pos[k];
    if(week_of(today)>s->week) {      /* new week, nothing played yet */
        out->move=0;out->week_delta=0.0;
    } else {
        out->move=(int16_t)((int)n->week_pos-(int)s->pos[k]);
        out->week_delta=n->points-n->week_points;
    }
    return 1;
}

/* ---- snapshots for draws ---- */
static ExpFifaSnapshot *snap_find(uint32_t comp,int32_t sd,uint16_t seq,uint32_t ph)
{
    uint32_t i;
    for(i=0;i<s_fifa.nsnaps;++i) {
        ExpFifaSnapshot *p=&s_fifa.snaps[i];
        if(p->competition==comp && p->start_day==sd && p->sequence==seq && p->phase==ph)
            return p;
    }
    return NULL;
}
static double snap_points(const ExpFifaSnapshot *p,uint32_t club,int *ok)
{
    uint32_t i;
    for(i=0;i<p->n;++i)if(p->clubs[i]==club){*ok=1;return p->points[i];}
    *ok=0;return 0.0;
}
int ExpFifa_SeedOrder(uint32_t comp,int32_t sd,uint16_t seq,uint32_t ph,int32_t day,
                      uint32_t *clubs,uint32_t n)
{
    ExpFifaSnapshot *p;
    uint32_t i,j;
    if(!s_fifa.active || !clubs || n==0u || n>EXP_FIFA_SNAPSHOT_CLUBS)return 0;
    for(i=0;i<n;++i)if(ExpFifa_FindClub(clubs[i])<0)return 0;
    p=snap_find(comp,sd,seq,ph);
    if(!p) {
        ExpFifaSnapshot *a;
        if(s_fifa.nsnaps>=EXP_FIFA_MAX_SNAPSHOTS) {
            /* oldest draw (already used) leaves the window */
            memmove(s_fifa.snaps,s_fifa.snaps+1,
                    (size_t)(s_fifa.nsnaps-1u)*sizeof *s_fifa.snaps);
            --s_fifa.nsnaps;
        }
        a=(ExpFifaSnapshot *)realloc(s_fifa.snaps,
            (size_t)(s_fifa.nsnaps+1u)*sizeof *a);
        if(!a)return 0;
        s_fifa.snaps=a;p=&a[s_fifa.nsnaps++];
        memset(p,0,sizeof *p);
        p->competition=comp;p->start_day=sd;p->sequence=seq;p->phase=ph;
        p->day=day;p->n=(uint16_t)n;
        for(i=0;i<n;++i){p->clubs[i]=clubs[i];
            p->points[i]=s_fifa.nations[ExpFifa_FindClub(clubs[i])].points;}
    }
    /* insertion sort, best first; equal points keep the incoming order */
    for(i=1;i<n;++i) {
        const uint32_t c=clubs[i];
        int ok;
        const double pc=snap_points(p,c,&ok);
        if(!ok)return 0;
        j=i;
        while(j>0) {
            int okp;
            const double pp=snap_points(p,clubs[j-1u],&okp);
            if(!okp)return 0;
            if(pp>=pc)break;
            clubs[j]=clubs[j-1u];--j;
        }
        clubs[j]=c;
    }
    return 1;
}

/* ---- seed ---- */
static int str_field(const char *j,int n,const char *k,char *out,int cap)
{
    const int l=mjson_get_string(j,n,k,out,cap);
    return l>0 && l<cap;
}
int ExpFifa_LoadSeedJson(const char *j,int len,int32_t start_day)
{
    ExpFifaState t;
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt;
    double v=0;
    char str[160];
    memset(&t,0,sizeof t);
    if(!j || len<=0 || mjson_get_number(j,len,"$.schema",&v)<=0 || v!=1.0 ||
       !str_field(j,len,"$.world",str,sizeof str) || strcmp(str,"2627") ||
       !str_field(j,len,"$.mode",str,sizeof str) || strcmp(str,"FIFA_SUM_SEED") ||
       mjson_get_number(j,len,"$.expectedRows",&v)<=0 || v!=(double)EXP_FIFA_NATIONS ||
       mjson_find(j,len,"$.records",&arr,&alen)!=MJSON_TOK_ARRAY)return 0;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        ExpFifaNation *nn;
        const char *r=arr+vo;
        double sc=0;
        char *end=NULL;
        uint32_t i;
        if(vt!=MJSON_TOK_OBJECT || t.n>=EXP_FIFA_NATIONS)return 0;
        nn=&t.nations[t.n];
        if(!str_field(r,vl,"$.code",nn->code,sizeof nn->code) || strlen(nn->code)!=3u ||
           !str_field(r,vl,"$.name",nn->name,sizeof nn->name) ||
           mjson_get_number(r,vl,"$.stableClub",&sc)<=0 || sc<1 || sc>65536 ||
           !str_field(r,vl,"$.points",str,sizeof str) ||
           mjson_get_string(r,vl,"$.flag",nn->flag,sizeof nn->flag)!=
               EXP_FIFA_FLAG_W*EXP_FIFA_FLAG_H)return 0;
        nn->points=strtod(str,&end);
        if(!end || *end || !isfinite(nn->points) || nn->points<=0)return 0;
        nn->stable_club=(uint32_t)sc;nn->seed_index=(uint16_t)t.n;
        for(i=0;i<t.n;++i)
            if(t.nations[i].stable_club==nn->stable_club ||
               !strcmp(t.nations[i].code,nn->code))return 0;
        nn->week_points=nn->points;
        ++t.n;
    }
    if(off<0 || t.n!=EXP_FIFA_NATIONS)return 0;
    resort(&t);
    {uint32_t i;for(i=0;i<t.n;++i)t.nations[i].week_pos=t.pos[i];}
    t.week=week_of(start_day);
    t.active=1;
    free_state(&s_fifa);
    s_fifa=t;
    return 1;
}

/* ---- persistence: "FR01;" text, %.17g round-trips every double ----
 * FR01;week;n;CODE,club,points,week_points,week_pos;...;E,count,id,...;
 * S,count,comp,start,seq,phase,day,n,club:points,...;...;#crc */
typedef struct Buf {char *p;size_t n,cap;int bad;} Buf;
static void bput(Buf *b,const char *fmt,...)
    __attribute__((format(printf,2,3)));
#include <stdarg.h>
static void bput(Buf *b,const char *fmt,...)
{
    va_list ap;
    int k;
    if(b->bad)return;
    for(;;) {
        const size_t room=b->cap-b->n;
        va_start(ap,fmt);k=vsnprintf(b->p?b->p+b->n:NULL,room,fmt,ap);va_end(ap);
        if(k<0){b->bad=1;return;}
        if((size_t)k<room){b->n+=(size_t)k;return;}
        {
            size_t cap=b->cap?b->cap*2u:65536u;
            char *q;
            while(cap<b->n+(size_t)k+1u)cap*=2u;
            q=(char *)realloc(b->p,cap);
            if(!q){b->bad=1;return;}
            b->p=q;b->cap=cap;
        }
    }
}
static uint32_t fnv(const char *p,size_t n)
{
    uint32_t h=2166136261u;
    while(n--){h^=(uint8_t)*p++;h*=16777619u;}
    return h;
}
char *ExpFifa_Encode(void)
{
    const ExpFifaState *s=&s_fifa;
    Buf b={0};
    uint32_t i,k;
    if(!s->active)return NULL;
    bput(&b,"FR01;%ld;%lu;",(long)s->week,(unsigned long)s->n);
    for(i=0;i<s->n;++i) {
        const ExpFifaNation *n=&s->nations[i];
        bput(&b,"%s,%lu,%.17g,%.17g,%u;",n->code,(unsigned long)n->stable_club,
             n->points,n->week_points,(unsigned)n->week_pos);
    }
    bput(&b,"E,%lu",(unsigned long)s->nevents);
    for(i=0;i<s->nevents;++i)bput(&b,",%llx",(unsigned long long)s->events[i]);
    bput(&b,";S,%lu",(unsigned long)s->nsnaps);
    for(i=0;i<s->nsnaps;++i) {
        const ExpFifaSnapshot *p=&s->snaps[i];
        bput(&b,";%lu,%ld,%u,%lu,%ld,%u",(unsigned long)p->competition,(long)p->start_day,
             (unsigned)p->sequence,(unsigned long)p->phase,(long)p->day,(unsigned)p->n);
        for(k=0;k<p->n;++k)bput(&b,",%lu:%.17g",(unsigned long)p->clubs[k],p->points[k]);
    }
    bput(&b,";");
    if(b.bad){free(b.p);return NULL;}
    {
        const uint32_t h=fnv(b.p,b.n);
        bput(&b,"#%08lx",(unsigned long)h);
    }
    if(b.bad){free(b.p);return NULL;}
    return b.p;
}
/* strict tokenizer over the encoded text */
typedef struct Rd {const char *p,*end;} Rd;
static int rd_long(Rd *r,long *out,int base,char stop)
{
    char *e=NULL;
    long long v;
    if(r->p>=r->end)return 0;
    v=strtoll(r->p,&e,base);
    if(e==r->p || e>=r->end+1 || *e!=stop || v<LONG_MIN || v>LONG_MAX)return 0;
    *out=(long)v;r->p=e+1;return 1;
}
static int rd_u64(Rd *r,uint64_t *out,char stop1,char stop2)
{
    char *e=NULL;
    unsigned long long v;
    if(r->p>=r->end)return 0;
    v=strtoull(r->p,&e,16);
    if(e==r->p || (*e!=stop1 && *e!=stop2))return 0;
    *out=(uint64_t)v;r->p=e;return 1;
}
static int rd_dbl(Rd *r,double *out,char stop)
{
    char *e=NULL;
    double v;
    if(r->p>=r->end)return 0;
    v=strtod(r->p,&e);
    if(e==r->p || *e!=stop || !isfinite(v))return 0;
    *out=v;r->p=e+1;return 1;
}
static int decode_into(const char *text,ExpFifaState *t)
{
    Rd r;
    long v,cnt,i;
    size_t len;
    const char *hash;
    unsigned long want=0;
    memset(t,0,sizeof *t);
    if(!text || strncmp(text,"FR01;",5))return 0;
    len=strlen(text);
    hash=strrchr(text,'#');
    if(!hash || (size_t)(text+len-hash)!=9u ||
       sscanf(hash+1,"%8lx",&want)!=1 ||
       fnv(text,(size_t)(hash-text))!=(uint32_t)want)return 0;
    r.p=text+5;r.end=hash;
    if(!rd_long(&r,&v,10,';'))return 0;
    t->week=(int32_t)v;
    if(!rd_long(&r,&cnt,10,';') || cnt!=(long)EXP_FIFA_NATIONS)return 0;
    for(i=0;i<cnt;++i) {
        ExpFifaNation *n=&t->nations[i];
        long sc,wp;
        if(r.end-r.p<4 || r.p[3]!=',')return 0;
        memcpy(n->code,r.p,3);n->code[3]=0;r.p+=4;
        if(!rd_long(&r,&sc,10,',') || sc<1 || sc>65536 ||
           !rd_dbl(&r,&n->points,',') || !rd_dbl(&r,&n->week_points,',') ||
           !rd_long(&r,&wp,10,';') || wp<1 || wp>(long)EXP_FIFA_NATIONS)return 0;
        n->stable_club=(uint32_t)sc;n->week_pos=(uint16_t)wp;
    }
    t->n=(uint32_t)cnt;
    if(r.end-r.p<2 || r.p[0]!='E' || r.p[1]!=',')return 0;
    r.p+=2;
    {
        char *e=NULL;
        const unsigned long ne=strtoul(r.p,&e,10);
        if(e==r.p || ne>0x4000000ul)return 0;
        r.p=e;
        if(ne) {
            t->events=(uint64_t *)malloc(ne*sizeof *t->events);
            if(!t->events)return 0;
            t->capevents=(uint32_t)ne;
        }
        for(i=0;(unsigned long)i<ne;++i) {
            if(*r.p!=',')return 0;
            ++r.p;
            if(!rd_u64(&r,&t->events[i],',',';') || !t->events[i] ||
               (i && t->events[i]<=t->events[i-1]))return 0;
        }
        t->nevents=(uint32_t)ne;
        if(*r.p!=';')return 0;
        ++r.p;
    }
    if(r.end-r.p<2 || r.p[0]!='S' || r.p[1]!=',')return 0;
    r.p+=2;
    {
        char *e=NULL;
        const unsigned long ns=strtoul(r.p,&e,10);
        if(e==r.p || ns>EXP_FIFA_MAX_SNAPSHOTS)return 0;
        r.p=e;
        if(ns) {
            t->snaps=(ExpFifaSnapshot *)calloc(ns,sizeof *t->snaps);
            if(!t->snaps)return 0;
        }
        for(i=0;(unsigned long)i<ns;++i) {
            ExpFifaSnapshot *p=&t->snaps[i];
            long a,b,c,d,f,g,k;
            if(*r.p!=';')return 0;
            ++r.p;
            if(!rd_long(&r,&a,10,',') || !rd_long(&r,&b,10,',') || !rd_long(&r,&c,10,',') ||
               !rd_long(&r,&d,10,',') || !rd_long(&r,&f,10,','))return 0;
            {
                char *e2=NULL;
                g=strtol(r.p,&e2,10);
                if(e2==r.p || g<1 || g>(long)EXP_FIFA_SNAPSHOT_CLUBS)return 0;
                r.p=e2;
            }
            p->competition=(uint32_t)a;p->start_day=(int32_t)b;p->sequence=(uint16_t)c;
            p->phase=(uint32_t)d;p->day=(int32_t)f;p->n=(uint16_t)g;
            for(k=0;k<g;++k) {
                long club;
                if(*r.p!=',')return 0;
                ++r.p;
                if(!rd_long(&r,&club,10,':') || club<1)return 0;
                {
                    char *e3=NULL;
                    p->points[k]=strtod(r.p,&e3);
                    if(e3==r.p || !isfinite(p->points[k]))return 0;
                    r.p=e3;
                }
                p->clubs[k]=(uint32_t)club;
            }
        }
        t->nsnaps=(uint32_t)ns;
        if(*r.p!=';' || r.p+1!=r.end)return 0;
    }
    return 1;
}
/* Identity check against the seed: same 211 codes and stable clubs.
 * Names and flags come from the seed (data, not save state). */
static int adopt_identity(ExpFifaState *t,const ExpFifaState *seed)
{
    uint32_t i,k;
    if(!seed->active || seed->n!=t->n)return 0;
    for(i=0;i<t->n;++i) {
        for(k=0;k<seed->n;++k)
            if(seed->nations[k].stable_club==t->nations[i].stable_club)break;
        if(k==seed->n || strcmp(seed->nations[k].code,t->nations[i].code))return 0;
        memcpy(t->nations[i].name,seed->nations[k].name,sizeof t->nations[i].name);
        memcpy(t->nations[i].flag,seed->nations[k].flag,sizeof t->nations[i].flag);
        t->nations[i].seed_index=seed->nations[k].seed_index;
    }
    return 1;
}
int ExpFifa_Validate(const char *text)
{
    ExpFifaState t;
    const int ok=decode_into(text,&t);
    free_state(&t);
    return ok;
}
/* Requires the seed to be loaded first (identity, names, flags). */
int ExpFifa_Decode(const char *text)
{
    ExpFifaState t;
    if(!decode_into(text,&t) || !adopt_identity(&t,&s_fifa)) {
        free_state(&t);return 0;
    }
    resort(&t);
    t.active=1;
    free_state(&s_fifa);
    s_fifa=t;
    return 1;
}
