#ifndef EXPANDED_CE_POOL_TRANSITION_H
#define EXPANDED_CE_POOL_TRANSITION_H
#include "expanded_ce_pool.h"

/* Caller supplies the immutable population of ONE bottom division and its
 * initial reserve clubs. Recover the inactive set from durable CE membership;
 * the order is catalogue order, NOT a sporting qualification or role order.
 * All buffers must be disjoint. No writes on invalid input. */
static inline int ExpCePool_Recover(const uint32_t *universe,uint32_t nu,
    const uint32_t *members,uint32_t nm,uint32_t *pool,uint32_t np)
{
    uint32_t i,j,k=0;
    if(!universe || !members || !nm || nm>nu || nu-nm!=np ||
       (np && !pool))return EXP_CE_POOL_INVALID;
    for(i=0;i<nu;++i){
        if(!universe[i])return EXP_CE_POOL_INVALID;
        for(j=0;j<i;++j)if(universe[j]==universe[i])return EXP_CE_POOL_INVALID;
    }
    for(i=0;i<nm;++i){
        for(j=0;j<i;++j)if(members[j]==members[i])return EXP_CE_POOL_INVALID;
        for(j=0;j<nu && universe[j]!=members[i];++j){}
        if(j==nu)return EXP_CE_POOL_INVALID;
    }
    for(i=0;i<nu;++i){
        for(j=0;j<nm && members[j]!=universe[i];++j){}
        if(j==nm)pool[k++]=universe[i];
    }
    return EXP_CE_POOL_READY;
}

/* Stage a complete next membership + reciprocal reserve set. The caller
 * must establish bottom-division scope, complete real CE final ranking,
 * and a durable OG playout winner (if required). The input pool is already
 * in the approved role order; this code never chooses challengers. The
 * caller commits the successor once through CE/CAR; this is not a writer.
 * All input/output/work buffers must be disjoint, sized as declared.
 * WAIT/INVALID leave both output arrays unchanged. */
static inline int ExpCePool_Transition(int bottom,int complete,
    const uint32_t *members,const uint32_t *ranked,uint32_t nm,
    const uint32_t *pool,uint32_t np,uint32_t direct,uint32_t playoff,
    uint32_t winner,ExpCePoolSwap *work,uint32_t cap,
    uint32_t *next_members,uint32_t *next_pool)
{
    uint32_t i,j,k,n=0;
    int rc;
    if(!bottom || !members || !ranked || !nm || !next_members ||
       (np && !next_pool))return EXP_CE_POOL_INVALID;
    if(!complete)return EXP_CE_POOL_WAIT;
    for(i=0;i<nm;++i){
        for(j=0;j<i;++j)if(members[j]==members[i])return EXP_CE_POOL_INVALID;
        for(j=0;j<nm && ranked[j]!=members[i];++j){}
        if(j==nm)return EXP_CE_POOL_INVALID;
    }
    rc=ExpCePool_Plan(ranked,nm,pool,np,direct,playoff,winner,work,cap,&n);
    if(rc!=EXP_CE_POOL_READY)return rc;
    for(i=0;i<nm;++i){
        next_members[i]=members[i];
        for(j=0;j<n;++j)if(work[j].relegated==members[i]){
            next_members[i]=work[j].promoted;break;
        }
    }
    for(k=0;k<np;++k){
        next_pool[k]=pool[k];
        for(j=0;j<n;++j)if(work[j].promoted==pool[k]){
            next_pool[k]=work[j].relegated;break;
        }
    }
    return EXP_CE_POOL_READY;
}
#endif
