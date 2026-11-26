/* Deterministic round-robin pairing kernel for the JSON Competition Engine.
 * Inputs: validated JSON participant count and meeting cycles, plus the
 * catalogue's already-sorted immutable club keys. NO OG schedule/TSKC/RNG.
 * The caller owns civil dates, fixture IDs, results and CAR.
 */
#ifndef EXPANDED_UNIVERSAL_PAIRINGS_H
#define EXPANDED_UNIVERSAL_PAIRINGS_H
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
typedef int (*ExpandedUniversalPairConsumer)(
    void *user, uint32_t cycle, uint32_t round, uint32_t serial,
    uint32_t home_index, uint32_t away_index);
/* 1/2/3/4 meetings, 2..N clubs, including odd N with a BYE.
 * Return 0 on invalid inputs or caller rejection.
 * No match pair is repeated WITHIN one cycle, and every odd cycle is the
 * exact reverse (home/away) of the previous one.
 *
 * v303 home/away rule (replaces the old parity walk that gave a club up to
 * n-1 consecutive away games). Circle method: clubs 0..slots-2 rotate,
 * club slots-1 is fixed (the bye when `clubs` is odd). Round r pairs i
 * with j=(r-i) mod m, m=slots-1; i==j meets the fixed club. The lower
 * index hosts when (r even) XOR (i+j>=m); the pair with the fixed club
 * is hosted by i in even rounds. For an even number of clubs every club
 * alternates home/away with the minimum possible number of breaks
 * (n-2 in total, longest run 2) and home games differ by at most 1. */
static inline int ExpandedUniversal_GenerateRoundRobin(
    uint32_t clubs, uint8_t meetings,
    ExpandedUniversalPairConsumer emit, void *user)
{
    uint32_t slots, m, cycle, round, i, serial;
    if(!emit || clubs<2u || meetings<1u || meetings>4u ||
       clubs==UINT32_MAX)return 0;
    if(clubs>UINT32_MAX/(clubs-1u) ||
       (clubs*(clubs-1u)/2u)>UINT32_MAX/meetings)
        return 0;
    slots=clubs+(clubs&1u);
    m=slots-1u;
    for(cycle=0;cycle<(uint32_t)meetings;++cycle) {
        serial=0u; /* cumulative per cycle: unique serial per fixture */
        for(round=0;round<m;++round) {
            for(i=0;i<m;++i) {
                const uint32_t j=(round+m-i)%m;
                uint32_t home,away;
                if(i>j)continue;
                if(i==j) {
                    const uint32_t fixed=slots-1u;
                    if(fixed>=clubs)continue; /* bye */
                    if((round&1u)==0u){home=i;away=fixed;}
                    else {home=fixed;away=i;}
                } else if(((((round&1u)==0u)?1u:0u)^((i+j>=m)?1u:0u))!=0u) {
                    home=i;away=j;
                } else {home=j;away=i;}
                if(cycle&1u){const uint32_t t=home;home=away;away=t;}
                if(!emit(user,cycle,round,serial++,home,away))return 0;
            }
        }
    }
    return 1;
}

/* Extra even rounds after two RR cycles. Stable club IDs define required
 * rivalries. Select distinct factors of a shuffled circle, then orient
 * the resulting even-degree graph along closed walks. The same pure
 * kernel is used at START and rollover. No fallback drops a rivalry. */
#define EXP_PAIR_MAX_CLUBS 128u
#define EXP_PAIR_MAX_RIVALRIES 32u
typedef struct ExpPairExtras {
    uint8_t rounds, nrivalries;
    uint32_t rivalries[EXP_PAIR_MAX_RIVALRIES][2];
} ExpPairExtras;
static inline uint64_t ExpPair_Next(uint64_t *s)
{ *s^=*s<<13;*s^=*s>>7;*s^=*s<<17;return *s; }
static inline uint64_t ExpPair_Seed(uint32_t comp,int32_t start)
{
    uint64_t s=((uint64_t)comp<<32)^(uint32_t)start;
    return s?s:1u;
}
static inline int ExpPair_ValidExtras(uint32_t clubs,uint8_t meetings,
                                      const ExpPairExtras *x)
{
    uint32_t i,j;
    if(!x)return 0;
    if(!x->rounds)return x->nrivalries==0u;
    if(clubs<4u||clubs>EXP_PAIR_MAX_CLUBS||(clubs&1u)||meetings!=2u||
       (x->rounds&1u)||x->rounds>=clubs||x->nrivalries>EXP_PAIR_MAX_RIVALRIES)return 0;
    for(i=0u;i<x->nrivalries;++i) {
        uint32_t a=x->rivalries[i][0],b=x->rivalries[i][1];
        if(!a||!b||a==b)return 0;
        for(j=0u;j<i;++j)
            if((a==x->rivalries[j][0]&&b==x->rivalries[j][1])||
               (a==x->rivalries[j][1]&&b==x->rivalries[j][0]))return 0;
    }
    return 1;
}
static inline int ExpandedUniversal_GenerateLeague(
    uint32_t clubs,uint8_t meetings,const uint32_t *keys,
    const ExpPairExtras *x,uint64_t seed,
    ExpandedUniversalPairConsumer emit,void *user)
{
    uint8_t order[EXP_PAIR_MAX_CLUBS],pos[EXP_PAIR_MAX_CLUBS];
    uint8_t chosen[EXP_PAIR_MAX_CLUBS],factors[EXP_PAIR_MAX_CLUBS];
    uint8_t ra[EXP_PAIR_MAX_RIVALRIES],rb[EXP_PAIR_MAX_RIVALRIES];
    uint8_t degree[EXP_PAIR_MAX_CLUBS]={0};
    uint8_t home[EXP_PAIR_MAX_CLUBS*(EXP_PAIR_MAX_CLUBS-1u)/2u];
    uint8_t away[sizeof home],used[sizeof home];
    uint32_t i,j,k,attempt,nchosen=0u,edges=0u,m,nr=0u;
    if(!emit||!ExpPair_ValidExtras(clubs,meetings,x))return 0;
    if(!x->rounds)return ExpandedUniversal_GenerateRoundRobin(clubs,meetings,emit,user);
    if(!keys)return 0;
    if(!seed)seed=1u;
    m=clubs-1u;
    for(i=0u;i<clubs;++i) {
        if(!keys[i])return 0;
        for(j=0u;j<i;++j)if(keys[i]==keys[j])return 0;
    }
    for(k=0u;k<x->nrivalries;++k) {
        for(i=0u;i<clubs&&keys[i]!=x->rivalries[k][0];++i){}
        for(j=0u;j<clubs&&keys[j]!=x->rivalries[k][1];++j){}
        /* v341: a rivalry whose club is not in THIS edition (relegated or replaced by the league
         * exchange) is not played this season; the other extra rounds come from the seeded fill */
        if(i==clubs||j==clubs)continue;
        if(++degree[i]>x->rounds||++degree[j]>x->rounds)return 0;
        ra[nr]=(uint8_t)i;rb[nr]=(uint8_t)j;++nr;
    }
    for(attempt=0u;attempt<65536u;++attempt) {
        for(i=0u;i<clubs;++i)order[i]=(uint8_t)i;
        for(i=clubs;i>1u;--i) {
            uint8_t q=order[i-1u];j=(uint32_t)(ExpPair_Next(&seed)%i);
            order[i-1u]=order[j];order[j]=q;
        }
        for(i=0u;i<clubs;++i)pos[order[i]]=(uint8_t)i;
        for(i=0u;i<m;++i)chosen[i]=0u;
        nchosen=0u;
        for(k=0u;k<nr;++k) {
            uint32_t r;
            i=pos[ra[k]];j=pos[rb[k]];
            r=(i==m?2u*j:j==m?2u*i:i+j)%m;
            if(!chosen[r]){chosen[r]=1u;factors[nchosen++]=(uint8_t)r;}
        }
        if(nchosen<=x->rounds)break;
    }
    if(attempt==65536u)return 0;
    while(nchosen<x->rounds) {
        uint32_t start=(uint32_t)(ExpPair_Next(&seed)%m);
        for(i=0u;i<m;++i) {
            j=(start+i)%m;
            if(!chosen[j]){chosen[j]=1u;factors[nchosen++]=(uint8_t)j;break;}
        }
    }
    for(i=nchosen;i>1u;--i) {
        uint8_t q=factors[i-1u];j=(uint32_t)(ExpPair_Next(&seed)%i);
        factors[i-1u]=factors[j];factors[j]=q;
    }
    for(k=0u;k<nchosen;++k) {
        uint32_t r=factors[k];
        for(i=0u;i<m;++i) {
            j=(r+m-i)%m;
            if(i>j)continue;
            home[edges]=order[i];away[edges]=order[i==j?m:j];used[edges++]=0u;
        }
    }
    if(edges!=clubs*x->rounds/2u)return 0;
    for(k=0u;k<edges;++k)if(!used[k]) {
        uint32_t first=home[k],at=first;
        do {
            for(i=0u;i<edges;++i)
                if(!used[i]&&(home[i]==at||away[i]==at))break;
            if(i==edges)return 0;
            if(away[i]==at){uint8_t q=home[i];home[i]=away[i];away[i]=q;}
            used[i]=1u;at=away[i];
        }while(at!=first);
    }
    if(!ExpandedUniversal_GenerateRoundRobin(clubs,meetings,emit,user))return 0;
    for(i=0u;i<edges;++i)
        if(!emit(user,meetings,i/(clubs/2u),i,home[i],away[i]))return 0;
    return 1;
}

#endif
