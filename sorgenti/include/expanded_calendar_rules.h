#ifndef EXPANDED_CALENDAR_RULES_H
#define EXPANDED_CALENDAR_RULES_H
/* v00.13.354 calendar rules of a round-robin phase, ONE implementation for
 * every path that creates league fixtures: the START catalog (first
 * edition) and the CE generate_phase (every later edition). Pure, data in /
 * dates out: no clock, OG, result or CAR. Absent rules = historical
 * behaviour.
 *  return_order 0 MIRROR (second cycle repeats the order) / 1 ASYMMETRIC;
 *  min_return_gap: logical rounds between two meetings (crit. 04);
 *  max_run: max consecutive home/away to validate (crit. 07);
 *  cal_draw 1: club -> calendar number drawn per edition (crit. 38);
 *  profile: matches per day around the round's planned day (crit. 15),
 *  offsets relative to the phase's ONE preferred weekday. */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "mjson.h"
#include "expanded_universal_pairings.h"
#include "expanded_universal_fixture_registry.h"
#include "expanded_world_time.h"

#define EXP_CAL_RR_MAX 128u
typedef struct ExpCalRules {
    uint8_t return_order, min_return_gap, max_run, cal_draw, nprofile;
    int8_t profile_off[7];
    uint8_t profile_n[7];
} ExpCalRules;

static inline int ExpCal_Used(const ExpCalRules *c)
{
    return c && (c->return_order || c->min_return_gap || c->max_run ||
                 c->cal_draw || c->nprofile);
}
/* {"return_order":"MIRROR"|"ASYMMETRIC","min_return_gap":N,"max_run":N,
 *  "calendar_draw":"EDITION","day_profile":[{"day":"FRI","matches":1},...]}
 * weekdays = preferred weekday mask of the phase (exactly one bit when a
 * day_profile is given). 1 ok, 0 invalid (why filled). */
static inline int ExpCal_Parse(const char *o, int ol, uint8_t weekdays,
                               ExpCalRules *c, char *why, size_t wl)
{
    static const char names[7][4]={"MON","TUE","WED","THU","FRI","SAT","SUN"};
    char t[16];
    double v=0;
    const char *arr=NULL;
    int alen=0;
    memset(c,0,sizeof *c);
    if(mjson_get_string(o,ol,"$.return_order",t,sizeof t)>0) {
        if(!strcmp(t,"MIRROR"))c->return_order=0u;
        else if(!strcmp(t,"ASYMMETRIC"))c->return_order=1u;
        else {snprintf(why,wl,"calendar.return_order");return 0;}
    }
    if(mjson_get_number(o,ol,"$.min_return_gap",&v)>0) {
        if(v<1||v>60||v!=(double)(int)v){snprintf(why,wl,"calendar.min_return_gap");return 0;}
        c->min_return_gap=(uint8_t)v;
    }
    if(mjson_get_number(o,ol,"$.max_run",&v)>0) {
        if(v<1||v>20||v!=(double)(int)v){snprintf(why,wl,"calendar.max_run");return 0;}
        c->max_run=(uint8_t)v;
    }
    if(mjson_get_string(o,ol,"$.calendar_draw",t,sizeof t)>0) {
        if(strcmp(t,"EDITION")){snprintf(why,wl,"calendar.calendar_draw");return 0;}
        c->cal_draw=1u;
    }
    if(mjson_find(o,ol,"$.day_profile",&arr,&alen)==MJSON_TOK_ARRAY) {
        int off=0,ko,kl,vo,vl,vt,anchor=-1,i;
        uint8_t a,b;
        for(i=0;i<7;++i)if(weekdays==(uint8_t)(1u<<i))anchor=i;
        if(anchor<0){snprintf(why,wl,"calendar.day_profile needs one preferred weekday");return 0;}
        while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
            int f=-1,d;
            uint8_t k;
            if(vt!=MJSON_TOK_OBJECT || c->nprofile>=7u ||
               mjson_get_string(arr+vo,vl,"$.day",t,sizeof t)<=0 ||
               mjson_get_number(arr+vo,vl,"$.matches",&v)<=0 || v<1 || v>64 ||
               v!=(double)(int)v){snprintf(why,wl,"calendar.day_profile");return 0;}
            for(i=0;i<7;++i)if(!strcmp(t,names[i]))f=i;
            if(f<0){snprintf(why,wl,"calendar.day_profile day");return 0;}
            d=f-anchor;if(d>3)d-=7;if(d<-3)d+=7;
            for(k=0u;k<c->nprofile;++k)
                if(c->profile_off[k]==d){snprintf(why,wl,"calendar.day_profile duplicate");return 0;}
            c->profile_off[c->nprofile]=(int8_t)d;c->profile_n[c->nprofile]=(uint8_t)v;
            ++c->nprofile;
        }
        if(off<0 || !c->nprofile){snprintf(why,wl,"calendar.day_profile");return 0;}
        for(a=0u;a<c->nprofile;++a)for(b=(uint8_t)(a+1u);b<c->nprofile;++b)
            if(c->profile_off[b]<c->profile_off[a]) {
                const int8_t x=c->profile_off[a];const uint8_t y=c->profile_n[a];
                c->profile_off[a]=c->profile_off[b];c->profile_n[a]=c->profile_n[b];
                c->profile_off[b]=x;c->profile_n[b]=y;
            }
    }
    return 1;
}
static inline uint64_t ExpCal_Hash(uint64_t h, const ExpCalRules *c)
{
    unsigned z;
    if(!ExpCal_Used(c))return h;
    h^=0xCAu;h*=UINT64_C(1099511628211);
    h^=c->return_order;h*=UINT64_C(1099511628211);
    h^=c->min_return_gap;h*=UINT64_C(1099511628211);
    h^=c->max_run;h*=UINT64_C(1099511628211);
    h^=c->cal_draw;h*=UINT64_C(1099511628211);
    for(z=0u;z<c->nprofile;++z){h^=(uint8_t)c->profile_off[z];h*=UINT64_C(1099511628211);
        h^=c->profile_n[z];h*=UINT64_C(1099511628211);}
    return h;
}

/* ---- calendar draw: deterministic shuffle of the club order ---- */
static inline void ExpCal_DrawOrder(ExpandedClubKey *clubs, uint32_t n, uint64_t seed)
{
    uint32_t k;
    if(!seed)seed=1u;
    for(k=n;k>1u;--k) {
        const uint32_t j=(uint32_t)(ExpPair_Next(&seed)%k);
        const ExpandedClubKey t=clubs[k-1u];
        clubs[k-1u]=clubs[j];clubs[j]=t;
    }
}

/* ---- ASYMMETRIC return order ---- */
typedef struct ExpCalBase {
    uint32_t n, rpc;
    int8_t ha[EXP_CAL_RR_MAX][EXP_CAL_RR_MAX];   /* [club][round] 1 H, -1 A, 0 bye */
} ExpCalBase;
static inline int ExpCal_BaseEmit(void *user, uint32_t cycle, uint32_t round,
                                  uint32_t serial, uint32_t home, uint32_t away)
{
    ExpCalBase *b=(ExpCalBase *)user;
    (void)serial;
    if(cycle)return 1;
    if(round>=EXP_CAL_RR_MAX || home>=b->n || away>=b->n)return 0;
    b->ha[home][round]=1;b->ha[away][round]=-1;
    return 1;
}
/* games beyond 2 consecutive H/A (user limit 27/11/2026) and longest run,
 * over cycle 0 then cycle 1 reversed in map order */
static inline uint32_t ExpCal_Runs(const ExpCalBase *b, const uint8_t *map, int *worst)
{
    uint32_t c,k,pos,excess=0u;
    uint8_t inv[EXP_CAL_RR_MAX];
    *worst=0;
    for(k=0;k<b->rpc;++k)inv[map[k]]=(uint8_t)k;
    for(c=0;c<b->n;++c) {
        int last=0,run=0;
        for(pos=0;pos<2u*b->rpc;++pos) {
            const int v=pos<b->rpc?b->ha[c][pos]:-b->ha[c][inv[pos-b->rpc]];
            if(!v)continue;
            run=v==last?run+1:1;last=v;
            if(run>2)++excess;
            if(run>*worst)*worst=run;
        }
    }
    return excess;
}
/* round r of the first cycle is replayed (H/A reversed) at return position
 * map[r], (rpc - r) + map[r] >= gap for every round; a seeded valid map is
 * improved by a deterministic swap search minimising the games beyond 2
 * consecutive, then the longest run. 1 ok, 0 impossible (gap > rounds). */
static inline int ExpCal_ReturnMap(uint32_t n, uint32_t rpc, uint32_t gap,
                                   uint64_t seed, uint8_t *best, int *best_run)
{
    static ExpCalBase b;
    uint8_t map[EXP_CAL_RR_MAX],used[EXP_CAL_RR_MAX];
    uint32_t it,excess;
    int32_t r;
    int worst;
    if(n<2u || n>EXP_CAL_RR_MAX || rpc<2u || rpc>EXP_CAL_RR_MAX || gap>rpc)return 0;
    memset(&b,0,sizeof b);b.n=n;b.rpc=rpc;
    if(!ExpandedUniversal_GenerateRoundRobin(n,1u,ExpCal_BaseEmit,&b))return 0;
    if(!seed)seed=1u;
    memset(used,0,sizeof used);
    for(r=(int32_t)rpc-1;r>=0;--r) {
        const int32_t lo=(int32_t)gap-(int32_t)rpc+r>0?(int32_t)gap-(int32_t)rpc+r:0;
        uint32_t free_n=0u,k,pick;
        for(k=(uint32_t)lo;k<rpc;++k)if(!used[k])++free_n;
        if(!free_n)return 0;
        pick=(uint32_t)(ExpPair_Next(&seed)%free_n);
        for(k=(uint32_t)lo;k<rpc;++k)if(!used[k]){if(!pick)break;--pick;}
        used[k]=1u;map[r]=(uint8_t)k;
    }
    excess=ExpCal_Runs(&b,map,&worst);
    for(it=0u;it<200000u && excess;++it) {
        const uint32_t i=(uint32_t)(ExpPair_Next(&seed)%rpc),j=(uint32_t)(ExpPair_Next(&seed)%rpc);
        uint8_t t;
        uint32_t ex;
        int w;
        if(i==j || rpc-i+map[j]<gap || rpc-j+map[i]<gap)continue;
        t=map[i];map[i]=map[j];map[j]=t;
        ex=ExpCal_Runs(&b,map,&w);
        if(ex<excess || (ex==excess && w<=worst)){excess=ex;worst=w;}
        else {t=map[i];map[i]=map[j];map[j]=t;}
    }
    memcpy(best,map,rpc);
    *best_run=worst;
    return 1;
}

/* ---- day profile (crit. 15) on the fixtures of ONE phase ----
 * days[0..rounds-1]: planned civil day of each logical round (strictly
 * increasing); a fixture belongs to round L when its day == days[L].
 * Constraints: rest from each club's previous fixture, before the next
 * round's first possible day, >= earliest, and free_day() (other
 * competitions / already placed fixtures). A fixture that fits nowhere
 * keeps the planned day (counted in kept). */
typedef int (*ExpCalFreeDay)(void *ctx, const ExpandedUniversalFixture *self,
                             const ExpandedUniversalFixture *all, uint32_t n,
                             int32_t day);
static inline void ExpCal_ApplyDayProfile(ExpandedUniversalFixture *f, uint32_t n,
    const ExpCalRules *c, uint8_t weekdays, uint32_t min_rest,
    const int32_t *days, uint32_t rounds, int32_t earliest, uint64_t seed,
    ExpCalFreeDay free_day, void *ctx,
    uint32_t *moved, uint32_t *kept, uint32_t *skipped)
{
    const int32_t rest=min_rest>1u?(int32_t)min_rest:1;
    int8_t min_off=0;
    uint32_t L,i,k,cap_total=0u,nl=0u;
    static uint32_t lclub[2u*EXP_CAL_RR_MAX];
    static int32_t lday[2u*EXP_CAL_RR_MAX];
    static uint32_t idx[EXP_CAL_RR_MAX];
    if(!seed)seed=1u;
    for(k=0u;k<c->nprofile;++k){cap_total+=c->profile_n[k];if(c->profile_off[k]<min_off)min_off=c->profile_off[k];}
    for(L=0u;L<rounds;++L) {
        const int32_t D=days[L];
        const int wd=ExpandedWorld_Weekday(D);
        int32_t next_min=INT32_MAX;
        uint32_t ni=0u,cap[7];
        uint64_t s=seed^((uint64_t)(L+1u)*UINT64_C(0x9E3779B97F4A7C15));
        if(!s)s=1u;
        for(i=0u;i<n && ni<EXP_CAL_RR_MAX;++i)if(f[i].day_index==D)idx[ni++]=i;
        if(L+1u<rounds) {
            const int nwd=ExpandedWorld_Weekday(days[L+1u]);
            next_min=days[L+1u]+((nwd>=0 && (weekdays&(1u<<nwd)))?min_off:0);
        }
        if(wd<0 || !(weekdays&(1u<<wd)) || ni!=cap_total) {
            if(ni)++*skipped;
        } else {
            for(k=0u;k<c->nprofile;++k)cap[k]=c->profile_n[k];
            for(i=ni;i>1u;--i){const uint32_t j=(uint32_t)(ExpPair_Next(&s)%i);const uint32_t t=idx[i-1u];idx[i-1u]=idx[j];idx[j]=t;}
            for(i=0u;i<ni;++i)f[idx[i]].day_index=INT32_MIN;
            for(i=0u;i<ni;++i) {
                ExpandedUniversalFixture *x=&f[idx[i]];
                int32_t lh=INT32_MIN/2,la=INT32_MIN/2;
                uint32_t q;
                int placed=0;
                for(q=0u;q<nl;++q){if(lclub[q]==x->home.stable_club)lh=lday[q];if(lclub[q]==x->away.stable_club)la=lday[q];}
                for(k=0u;k<c->nprofile && !placed;++k) {
                    const int32_t day=D+c->profile_off[k];
                    if(!cap[k] || day<earliest || day-lh<rest || day-la<rest || next_min-day<rest ||
                       (free_day && !free_day(ctx,x,f,n,day)))continue;
                    x->day_index=day;--cap[k];placed=1;
                    if(day!=D)++*moved;
                }
                if(!placed){x->day_index=D;++*kept;}
            }
        }
        for(i=0u;i<ni;++i) {
            const ExpandedUniversalFixture *x=&f[idx[i]];
            const uint32_t c2[2]={x->home.stable_club,x->away.stable_club};
            uint32_t sd,q;
            for(sd=0u;sd<2u;++sd) {
                for(q=0u;q<nl && lclub[q]!=c2[sd];++q){}
                if(q==nl){if(nl>=2u*EXP_CAL_RR_MAX)continue;lclub[nl++]=c2[sd];}
                lday[q]=x->day_index;
            }
        }
    }
}
#endif
