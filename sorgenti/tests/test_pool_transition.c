#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_ce_pool_transition.h"

static void exercise(uint32_t direct,uint32_t playoff)
{
    const uint32_t universe[]={101,102,103,104,105,106,107,108};
    uint32_t members[]={101,102,103,104},pool[]={105,106,107,108};
    const uint32_t np=direct+playoff+1u;
    uint32_t next[4],reserve[4],recovered[4],rank[4];
    ExpCePoolSwap work[4];
    unsigned season,i,j;
    for(season=0;season<1000;++season){
        uint32_t winner=0;
        for(i=0;i<4;++i)rank[i]=members[(i+season)%4];
        if(playoff)winner=(season&1u)?pool[direct]:rank[4-direct-1];
        assert(ExpCePool_Transition(1,1,members,rank,4,pool,np,direct,
            playoff,winner,work,4,next,reserve)==EXP_CE_POOL_READY);
        assert(ExpCePool_Recover(universe,4+np,next,4,recovered,np)==EXP_CE_POOL_READY);
        for(i=0;i<np;++i){
            for(j=0;j<np && recovered[j]!=reserve[i];++j){}
            assert(j<np);
        }
        memcpy(members,next,sizeof members);
        memcpy(pool,reserve,np*sizeof *pool);
    }
}
static void cross_region(void)
{
    uint32_t upper[]={90,91},next_upper[]={999,999};
    uint32_t a[]={1,2,3,4},ap[]={5,6,7},b[]={11,12,13,14},bp[]={15,16,17,18};
    uint32_t ar[4],br[4],an[4],bn[4],apn[8],bpn[8];
    ExpCePoolRegion regions[2]={
        {a,ar,ap,4,3,1,1,0,1,an,apn,8,999},
        {b,br,bp,4,4,2,0,0,1,bn,bpn,8,999}
    };
    uint32_t i;
    memcpy(ar,a,sizeof a);memcpy(br,b,sizeof b);
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,1,regions,2,next_upper)==EXP_CE_POOL_WAIT);
    assert(next_upper[0]==999 && regions[0].next_np==999);
    regions[0].winner=6;
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,1,regions,2,next_upper)==EXP_CE_POOL_READY);
    assert(next_upper[0]==1 && next_upper[1]==91);
    assert(regions[0].next_np==2 && regions[1].next_np==5);
    assert(ExpCePool_Has(an,4,7) && ExpCePool_Has(an,4,6) && ExpCePool_Has(an,4,5));
    assert(ExpCePool_Has(bn,4,90) && ExpCePool_Has(bpn,5,12));
    regions[1].pool_cap=4;
    for(i=0;i<4;++i)an[i]=999;
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,1,regions,2,next_upper)==EXP_CE_POOL_INVALID);
    assert(an[0]==999 && an[3]==999 && regions[1].next_np==5);
    regions[1].pool_cap=8;
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,0,regions,2,next_upper)==EXP_CE_POOL_READY);
    assert(regions[0].next_np==3 && regions[1].next_np==4 && ExpCePool_Has(an,4,90));
    assert(ExpCePool_CrossRegion(upper,2,4,90,0,1,regions,2,next_upper)==EXP_CE_POOL_INVALID);
    regions[0].winner=3; /* OG incumbent wins: challenger not promoted */
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,1,regions,2,next_upper)==EXP_CE_POOL_READY);
    assert(ExpCePool_Has(an,4,3) && !ExpCePool_Has(an,4,6) && ExpCePool_Has(an,4,7));
    regions[0].np=2; /* local standby exhausted: borrow from B */
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,1,regions,2,next_upper)==EXP_CE_POOL_READY);
    assert(regions[0].next_np==2 && regions[1].next_np==4);
    assert(ExpCePool_Has(an,4,17) && !ExpCePool_Has(bpn,4,17));
    assert(ExpCePool_Has(an,4,3) && !ExpCePool_Has(an,4,6));
    regions[1].np=2; /* both regions have only reserved roles */
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,1,regions,2,next_upper)==EXP_CE_POOL_INVALID);
    assert(regions[0].next_np==2 && regions[1].next_np==4);
    regions[1].np=4;regions[0].np=3;
    bp[0]=1; /* duplicated global identity */
    assert(ExpCePool_CrossRegion(upper,2,1,90,0,1,regions,2,next_upper)==EXP_CE_POOL_INVALID);
}

static void cross_repeat(void)
{
    uint32_t upper[]={90,91},un[2],a[]={1,2,3,4},b[]={11,12,13,14};
    uint32_t ap[8]={5,6,7},bp[8]={15,16,17,18},an[4],bn[4],apn[8],bpn[8];
    ExpCePoolRegion p[2]={
        {a,a,ap,4,3,1,0,0,1,an,apn,8,0},
        {b,b,bp,4,4,1,0,0,1,bn,bpn,8,0}
    };
    unsigned s;
    for(s=0;s<1000u;++s) {
        const uint32_t from=s&1u,to=1u-from;
        /* upper[0] is the B-origin club initially; alternating transfers
         * make its region of origin alternate without changing its ID. */
        assert(ExpCePool_CrossRegion(upper,2,p[from].members[0],upper[0],
            from,to,p,2,un)==EXP_CE_POOL_READY);
        assert(p[0].next_np==((s&1u)?3u:2u));
        assert(p[1].next_np==((s&1u)?4u:5u));
        memcpy(upper,un,sizeof upper);
        memcpy(a,an,sizeof a);memcpy(b,bn,sizeof b);
        memcpy(ap,apn,p[0].next_np*sizeof *ap);
        memcpy(bp,bpn,p[1].next_np*sizeof *bp);
        p[0].np=p[0].next_np;p[1].np=p[1].next_np;
    }
}

static void cross_consecutive(void)
{
    uint32_t upper[]={90,91,92,93},un[4],a[]={1,2,3,4},b[]={11,12,13,14};
    uint32_t ap[8]={5,6},bp[8]={15,16,17,18},an[4],bn[4],apn[8],bpn[8];
    /* Donor first in array: test order independence of removal/insertion. */
    ExpCePoolRegion p[2]={
        {b,b,bp,4,4,2,0,0,1,bn,bpn,8,0},
        {a,a,ap,4,2,1,1,0,1,an,apn,8,0}
    };
    unsigned s;
    for(s=0;s<4u;++s) {
        uint32_t borrowed=bp[2];
        p[1].winner=ap[1]; /* actual OG result would supply this witness */
        assert(ExpCePool_CrossRegion(upper,4,a[0],upper[s],1,0,p,2,un)==EXP_CE_POOL_READY);
        assert(p[0].next_np==4u && p[1].next_np==2u);
        assert(ExpCePool_Has(an,4,borrowed) && !ExpCePool_Has(bpn,4,borrowed));
        memcpy(upper,un,sizeof upper);
        memcpy(a,an,sizeof a);memcpy(b,bn,sizeof b);
        memcpy(ap,apn,p[1].next_np*sizeof *ap);
        memcpy(bp,bpn,p[0].next_np*sizeof *bp);
        p[0].np=p[0].next_np;p[1].np=p[1].next_np;
    }
}

int main(void)
{
    uint32_t members[]={1,2,3,4},rank[]={2,1,4,3},pool[]={5,6};
    uint32_t next[]={99,99,99,99},reserve[]={99,99};
    uint32_t universe[]={1,2,3,4,5,6};
    ExpCePoolSwap work[2];
    assert(ExpCePool_Transition(0,1,members,rank,4,pool,2,1,1,6,work,2,next,reserve)==EXP_CE_POOL_INVALID);
    assert(ExpCePool_Transition(1,0,members,rank,4,pool,2,1,1,6,work,2,next,reserve)==EXP_CE_POOL_WAIT);
    assert(ExpCePool_Transition(1,1,members,rank,4,pool,2,1,1,0,work,2,next,reserve)==EXP_CE_POOL_WAIT);
    assert(ExpCePool_Transition(1,1,members,rank,4,pool,2,1,1,5,work,2,next,reserve)==EXP_CE_POOL_INVALID);
    rank[0]=9;
    assert(ExpCePool_Transition(1,1,members,rank,4,pool,2,1,1,6,work,2,next,reserve)==EXP_CE_POOL_INVALID);
    assert(next[0]==99 && next[1]==99 && next[2]==99 && next[3]==99 && reserve[0]==99 && reserve[1]==99);
    assert(ExpCePool_Recover(universe,6,rank,4,reserve,2)==EXP_CE_POOL_INVALID);
    assert(reserve[0]==99 && reserve[1]==99);
    rank[0]=2;
    assert(ExpCePool_Transition(1,1,members,rank,4,pool,2,1,1,6,work,2,next,reserve)==EXP_CE_POOL_READY);
    assert(next[0]==1 && next[1]==2 && next[2]==5 && next[3]==6 && reserve[0]==3 && reserve[1]==4);
    { uint32_t up=0,down=0;
      assert(ExpCePool_Balance(3,0,1,&up,&down)==EXP_CE_POOL_READY && up==4 && down==3);
      assert(ExpCePool_Balance(3,1,0,&up,&down)==EXP_CE_POOL_READY && up==3 && down==4);
      assert(ExpCePool_Balance(2,1,1,&up,&down)==EXP_CE_POOL_READY && up==2 && down==2);
      assert(ExpCePool_Balance(UINT32_MAX,1,0,&up,&down)==EXP_CE_POOL_INVALID && up==2 && down==2);
    }
    cross_region();cross_repeat();cross_consecutive();
    exercise(1,1);exercise(3,0);exercise(2,0);exercise(0,0);
    puts("BOTTOM_POOL_TRANSITION_ISOLATED=PASS 4000 ordinary + 1000 cross-region cycles + consecutive borrowing");
    return 0;
}
