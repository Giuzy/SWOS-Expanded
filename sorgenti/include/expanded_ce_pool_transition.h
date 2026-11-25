#ifndef EXPANDED_CE_POOL_TRANSITION_H
#define EXPANDED_CE_POOL_TRANSITION_H
#include "expanded_ce_pool.h"
#include <stdlib.h>
#include <stddef.h>

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
/* Cross-pyramid transaction planner. No country-specific rules, scores,
 * RNG or publication to CE/CAR. Caller provides a complete regular rank,
 * resolved persistent pool roles and certified OG winners, plus the two
 * upper-level transfers. Region input/output buffers must be disjoint.
 * The pool size may vary; total population across all regions is conserved.
 * The additional promotion uses the first unused local standby, or a free
 * standby in another eligible region if none is local. Never take a direct
 * entrant or a playout challenger. Caller supplies eligible regions in
 * persisted CE selection order, not catalogue order. Additional relegation
 * uses the lowest remaining old member. The borrowed club keeps its ID.
 * Conflicting ordinary/upper movements fail closed, not double-counted. */
typedef struct ExpCePoolRegion {
    const uint32_t *members,*ranked,*pool;
    uint32_t nm,np,direct,playoff,winner;
    int complete;
    uint32_t *next_members,*next_pool;
    uint32_t pool_cap,next_np;
} ExpCePoolRegion;

static inline int ExpCePool_Has(const uint32_t *a,uint32_t n,uint32_t id)
{
    uint32_t i;
    for(i=0;i<n;++i)if(a[i]==id)return 1;
    return 0;
}

/* A single promoted/relegated pair, possibly routed to different regions.
 * On WAIT/INVALID no caller output or next_np is modified. */
static inline int ExpCePool_CrossRegion(const uint32_t *upper,uint32_t nu,
    uint32_t promoted,uint32_t relegated,uint32_t from,uint32_t to,
    ExpCePoolRegion *regions,uint32_t nr,uint32_t *next_upper)
{
    uint32_t i,j,k,total=nu,pos=0u,off;
    uint32_t donor=UINT32_MAX,extra_club=0u;
    int q;
    uint32_t *all=NULL,*stage=NULL,*sizes=NULL;
    int rc=EXP_CE_POOL_INVALID;
    if(!upper || !nu || !regions || !nr || !next_upper || from>=nr || to>=nr ||
       !promoted || !relegated || promoted==relegated ||
       !ExpCePool_Has(upper,nu,relegated))return rc;
    for(i=0;i<nr;++i) {
        const ExpCePoolRegion *p=&regions[i];
        if(!p->complete)return EXP_CE_POOL_WAIT;
        if(!p->members || !p->ranked || !p->nm || !p->next_members ||
           (p->np && !p->pool) || (p->pool_cap && !p->next_pool) ||
           p->direct>=p->nm || p->playoff>1u ||
           p->playoff>=p->nm-p->direct || p->np<p->direct+p->playoff ||
           p->nm>UINT32_MAX-total || p->np>UINT32_MAX-total-p->nm)
            return rc;
        total+=p->nm+p->np;
    }
    if(!ExpCePool_Has(regions[from].members,regions[from].nm,promoted))return rc;
#if SIZE_MAX <= UINT32_MAX
    if(total>SIZE_MAX/sizeof *all || nr>SIZE_MAX/sizeof *sizes)return rc;
#endif
    all=malloc((size_t)total*sizeof *all);
    stage=malloc((size_t)total*sizeof *stage);
    sizes=malloc((size_t)nr*sizeof *sizes);
    if(!all || !stage || !sizes)goto done;
    for(i=0;i<nu;++i)all[pos++]=upper[i];
    for(i=0;i<nr;++i) {
        const ExpCePoolRegion *p=&regions[i];
        for(j=0;j<p->nm;++j)all[pos++]=p->members[j];
        for(j=0;j<p->np;++j)all[pos++]=p->pool[j];
    }
    for(i=0;i<total;++i)
        if(!all[i] || ExpCePool_Has(all,i,all[i]))goto done;
    /* Resolve compensation before any staging, so the donor can precede
     * the recipient in the region array. A borrowed club is removed only
     * from the donor's reserve set and inserted in the recipient league. */
    if(from!=to) {
        const ExpCePoolRegion *p=&regions[from];
        j=p->direct+p->playoff;
        if(j<p->np)donor=from;
        else for(i=0;i<nr;++i) {
            const ExpCePoolRegion *candidate=&regions[i];
            if(i!=from && candidate->np>candidate->direct+candidate->playoff) {
                donor=i;break;
            }
        }
        if(donor==UINT32_MAX)goto done;
        p=&regions[donor];
        extra_club=p->pool[p->direct+p->playoff];
    }
    pos=0u;
    for(i=0;i<nu;++i)stage[pos++]=upper[i]==relegated?promoted:upper[i];
    for(i=0;i<nr;++i) {
        const ExpCePoolRegion *p=&regions[i];
        const uint32_t in=(i==to),out=(i==from);
        uint32_t ordinary=0u,up=0u,down=0u,extra_up=0u,extra_down=0u;
        const uint32_t reserve_used=(i==donor);
        uint32_t extra_in=0u,extra_out=0u,nm=0u,np=0u;
        uint32_t *members=stage+pos,*pool;
        ExpCePoolSwap *swaps=NULL;
        if(p->direct>=p->nm || p->playoff>1u ||
           p->playoff>=p->nm-p->direct || p->np<p->direct+p->playoff)
            goto done;
        for(j=0;j<p->nm;++j)
            if(!ExpCePool_Has(p->members,p->nm,p->ranked[j]) ||
               ExpCePool_Has(p->ranked,j,p->ranked[j]))goto done;
        if(p->direct+p->playoff) {
            swaps=malloc((size_t)(p->direct+p->playoff)*sizeof *swaps);
            if(!swaps)goto done;
        }
        q=ExpCePool_Plan(p->ranked,p->nm,p->pool,p->np,p->direct,p->playoff,
            p->winner,swaps,p->direct+p->playoff,&ordinary);
        if(q!=EXP_CE_POOL_READY) {
            free(swaps);
            if(q==EXP_CE_POOL_WAIT)rc=EXP_CE_POOL_WAIT;
            goto done;
        }
        if(ExpCePool_Balance(ordinary,in,out,&up,&down)!=EXP_CE_POOL_READY) {
            free(swaps);goto done;
        }
        extra_up=up-ordinary;extra_down=down-ordinary;
        for(j=0;j<ordinary;++j)
            if(out && swaps[j].relegated==promoted){free(swaps);goto done;}
        if(extra_up) {
            if(!extra_club){free(swaps);goto done;}
            extra_in=extra_club;
        }
        if(extra_down) {
            for(j=p->nm;j>0u;--j) {
                const uint32_t c=p->ranked[j-1u];
                if(out && c==promoted)continue;
                for(k=0;k<ordinary && swaps[k].relegated!=c;++k){}
                if(k==ordinary){extra_out=c;break;}
            }
            if(!extra_out){free(swaps);goto done;}
        }
        if(reserve_used>p->np || extra_down>UINT32_MAX-(p->np-reserve_used)) {
            free(swaps);goto done;
        }
        sizes[i]=p->np-reserve_used+extra_down;
        if(sizes[i]>p->pool_cap || p->nm>total-pos ||
           sizes[i]>total-pos-p->nm){free(swaps);goto done;}
        pool=members+p->nm;
        for(j=0;j<p->nm;++j) {
            uint32_t c=p->members[j];
            if((out && c==promoted) || c==extra_out)continue;
            for(k=0;k<ordinary && swaps[k].relegated!=c;++k){}
            if(k<ordinary)c=swaps[k].promoted;
            members[nm++]=c;
        }
        if(in)members[nm++]=relegated;
        if(extra_in)members[nm++]=extra_in;
        for(j=0;j<p->np;++j) {
            uint32_t c=p->pool[j];
            if(reserve_used && c==extra_club)continue;
            for(k=0;k<ordinary && swaps[k].promoted!=c;++k){}
            if(k<ordinary)c=swaps[k].relegated;
            pool[np++]=c;
        }
        if(extra_out)pool[np++]=extra_out;
        free(swaps);
        if(nm!=p->nm || np!=sizes[i])goto done;
        pos+=nm+np;
    }
    if(pos!=total)goto done;
    for(i=0;i<total;++i)
        if(!stage[i] || ExpCePool_Has(stage,i,stage[i]) ||
           !ExpCePool_Has(all,total,stage[i]))goto done;
    /* Only now publish the complete candidate into caller-owned buffers. */
    for(i=0;i<nu;++i)next_upper[i]=stage[i];
    off=nu;
    for(i=0;i<nr;++i) {
        ExpCePoolRegion *p=&regions[i];
        for(j=0;j<p->nm;++j)p->next_members[j]=stage[off++];
        for(j=0;j<sizes[i];++j)p->next_pool[j]=stage[off++];
        p->next_np=sizes[i];
    }
    rc=EXP_CE_POOL_READY;
done:
    free(all);free(stage);free(sizes);return rc;
}

#endif
