#ifndef EXPANDED_MATCH_LINEUP_H
#define EXPANDED_MATCH_LINEUP_H
/* v00.13.320 phase (a): ONE shared match squad ("distinta") builder for manager and CPU teams.
 * Pure: it only selects which REAL roster identities take which match slot; it never creates
 * players, never touches OG structures and never reads PlayerFile bytes. The caller provides
 * the roster in nomination priority order (first the club's own XI/bench order, then the rest
 * of the real roster), the competition bench size (CE rule, 0..15) and availability.
 * Identity (roster_id) is permanent and unique; shirt number, formation slot and match slot
 * are separate fields and are never interchanged. */
#include <stdint.h>
#define EXP_LINEUP_XI 11u
#define EXP_LINEUP_MAX_BENCH 15u
#define EXP_LINEUP_MAX_SLOTS (EXP_LINEUP_XI+EXP_LINEUP_MAX_BENCH)
#define EXP_LINEUP_MAX_ROSTER 30u
typedef struct ExpLineupCandidate {
    uint16_t roster_id;      /* permanent identity within the club roster (unique) */
    uint8_t  shirt;          /* shirt number, informational */
    uint8_t  is_keeper;
    uint8_t  available;      /* 0 injured / suspended / sent off / otherwise unusable */
} ExpLineupCandidate;
typedef struct ExpLineup {
    uint16_t roster_id[EXP_LINEUP_MAX_SLOTS]; /* slots 0..10 XI, 11.. bench */
    uint8_t  filled;         /* slots actually filled (never padded) */
    uint8_t  xi_filled;
    uint8_t  bench_filled;
    uint8_t  bench_has_keeper;
} ExpLineup;
/* 1 = built (possibly with fewer bench players than requested: honest, no filler),
 * 0 = invalid input (duplicate identity, too many candidates, bench>15), out untouched. */
static inline int ExpLineup_Build(ExpLineup *out,const ExpLineupCandidate *c,unsigned n,unsigned bench)
{
    ExpLineup l; unsigned i,j;
    if(!out || (n && !c) || n>EXP_LINEUP_MAX_ROSTER || bench>EXP_LINEUP_MAX_BENCH)return 0;
    for(i=0;i<n;++i)for(j=i+1;j<n;++j)if(c[i].roster_id==c[j].roster_id)return 0;
    l=(ExpLineup){0};
    for(i=0;i<n && l.xi_filled<EXP_LINEUP_XI;++i)
        if(c[i].available){l.roster_id[l.filled++]=c[i].roster_id;++l.xi_filled;}
    {   /* bench: next available in priority order; a keeper is preferred for the first bench seat */
        uint8_t used[EXP_LINEUP_MAX_ROSTER]={0};
        unsigned k=0,seen=0;
        for(i=0;i<n && seen<l.xi_filled;++i)if(c[i].available){used[i]=1;++seen;}
        if(bench && l.xi_filled==EXP_LINEUP_XI)
            for(i=0;i<n;++i)if(!used[i] && c[i].available && c[i].is_keeper){
                l.roster_id[l.filled++]=c[i].roster_id;used[i]=1;++l.bench_filled;l.bench_has_keeper=1;break;}
        for(i=0;i<n && l.bench_filled<bench && l.xi_filled==EXP_LINEUP_XI;++i)
            if(!used[i] && c[i].available){l.roster_id[l.filled++]=c[i].roster_id;used[i]=1;++l.bench_filled;}
        (void)k;
    }
    *out=l;
    return 1;
}
#endif
