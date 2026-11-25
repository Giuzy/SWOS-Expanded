#ifndef EXPANDED_CE_POOL_H
#define EXPANDED_CE_POOL_H

#include <stdint.h>

typedef struct {
    uint32_t relegated, promoted;
} ExpCePoolSwap;

enum {
    EXP_CE_POOL_INVALID = -1,
    EXP_CE_POOL_WAIT = 0,
    EXP_CE_POOL_READY = 1
};

/* Plan, never apply, an exchange at the bottom of a sporting pyramid.
 * ranked: final complete table, best first, stable club IDs.
 * pool: at least direct + playoff eligible clubs, in resolved role order.
 * Index direct is the challenger if playoff == 1; remaining clubs are standby.
 * winner must come from a completed OG tie, verified by the caller.
 * A zero winner means not decided. No synthetic matches, RNG or CAR writes.
 * Output capacity is np; output must not alias input. No partial plan on error.
 */
static inline int ExpCePool_Plan(const uint32_t *ranked, uint32_t n,
    const uint32_t *pool, uint32_t np, uint32_t direct, uint32_t playoff,
    uint32_t winner, ExpCePoolSwap *out, uint32_t cap, uint32_t *count)
{
    uint32_t i,j;
    if (!count) return EXP_CE_POOL_INVALID;
    *count=0;
    if (!ranked || !n || playoff>1u || direct>=n ||
        playoff>=n-direct || np<direct+playoff ||
        (np && !pool) || ((direct+playoff) && (!out || cap<direct+playoff)) ||
        (!playoff && winner)) return EXP_CE_POOL_INVALID;
    for (i=0;i<n;++i) {
        if (!ranked[i]) return EXP_CE_POOL_INVALID;
        for (j=0;j<i;++j)
            if (ranked[i]==ranked[j]) return EXP_CE_POOL_INVALID;
    }
    for (i=0;i<np;++i) {
        if (!pool[i]) return EXP_CE_POOL_INVALID;
        for (j=0;j<n;++j)
            if (pool[i]==ranked[j]) return EXP_CE_POOL_INVALID;
        for (j=0;j<i;++j)
            if (pool[i]==pool[j]) return EXP_CE_POOL_INVALID;
    }
    if (playoff) {
        if (!winner) return EXP_CE_POOL_WAIT;
        if (winner!=ranked[n-direct-1u] && winner!=pool[direct])
            return EXP_CE_POOL_INVALID;
    }
    for (i=0;i<direct;++i)
        out[i]=(ExpCePoolSwap){ranked[n-1u-i],pool[i]};
    *count=direct;
    if (playoff && winner==pool[direct])
        out[(*count)++]=(ExpCePoolSwap){ranked[n-direct-1u],pool[direct]};
    return EXP_CE_POOL_READY;
}
/* Counts only; caller stages all affected pyramids atomically. */
static inline int ExpCePool_Balance(uint32_t ordinary,uint32_t incoming,
    uint32_t outgoing,uint32_t *up,uint32_t *down)
{
    uint32_t a=ordinary,b=ordinary;
    if(!up || !down || up==down)return EXP_CE_POOL_INVALID;
    if(incoming>outgoing) {
        if(UINT32_MAX-b<incoming-outgoing)return EXP_CE_POOL_INVALID;
        b+=incoming-outgoing;
    } else {
        if(UINT32_MAX-a<outgoing-incoming)return EXP_CE_POOL_INVALID;
        a+=outgoing-incoming;
    }
    *up=a;*down=b;
    return EXP_CE_POOL_READY;
}
#endif
