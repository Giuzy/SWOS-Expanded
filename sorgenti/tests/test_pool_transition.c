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
    exercise(1,1);exercise(3,0);exercise(2,0);exercise(0,0);
    puts("BOTTOM_POOL_TRANSITION_ISOLATED=PASS 4000 cycles");
    return 0;
}
