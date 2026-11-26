#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_match_bench.h"

static int selectable(void *p,unsigned i)
{
    assert(i>=1 && i<=EXP_BENCH_MAX);
    return (*(uint32_t *)p & (UINT32_C(1)<<i))!=0;
}
static unsigned legacy(unsigned i,int down,uint32_t mask)
{
    if(down) {
        if(i==5)return i;
        do { ++i; if(i==6)break; if(mask&(1u<<i))return i; } while(1);
    } else if(i==0)return 0;
    do { --i; if(i==0)return 0; if(mask&(1u<<i))return i; } while(1);
}
int main(void)
{
    assert(sizeof(PlayerInfo)==61 && sizeof(TeamGame)==1018);
    for(uint32_t mask=0;mask<64;++mask)
        for(unsigned i=0;i<=5;++i)
            for(int down=0;down<=1;++down)
                assert(ExpBench_Move(i,5,down,selectable,&mask)==legacy(i,down,mask));
    uint32_t mask=(1u<<1)|(1u<<7)|(1u<<15);
    assert(ExpBench_Move(1,15,1,selectable,&mask)==7);
    assert(ExpBench_Move(7,15,1,selectable,&mask)==15);
    assert(ExpBench_Move(15,15,0,selectable,&mask)==7);
    assert(ExpBench_Move(1,15,0,selectable,&mask)==0);
    assert(ExpBench_Move(15,15,1,selectable,&mask)==15);
    mask=0;
    assert(ExpBench_Move(5,15,1,selectable,&mask)==0);
    assert(ExpBench_Move(16,15,0,selectable,&mask)==0);
    assert(ExpBench_Move(1,16,0,selectable,&mask)==0);
    TeamGame native={0},opponent={0}; ExpMatchBench b={0},other={0};
    PlayerInfo extra[10]={{0}};
    for(unsigned i=0;i<16;++i)native.players[i].playerIndex=(int8_t)i;
    for(unsigned i=0;i<10;++i)extra[i].playerIndex=(int8_t)(16+i);
    for(unsigned n=0;n<=10;++n) {
        assert(ExpBench_Init(&b,&native,extra,n));
        assert(ExpBench_Count(&b)==5+n);
        assert(!ExpBench_Player(&native,&b,16+n));
        assert(ExpBench_Player(&native,&b,15+n)->playerIndex==(int)(15+n));
    }
    extra[9].playerIndex=15;
    assert(!ExpBench_Init(&b,&native,extra,10) && ExpBench_Count(&b)==15);
    extra[9].playerIndex=-1;
    assert(!ExpBench_Init(&b,&native,extra,10));
    extra[9].playerIndex=25;
    assert(!ExpBench_Init(&b,&native,extra,11));
    assert(!ExpBench_Init(&b,&native,NULL,1));
    assert(ExpBench_Init(&b,&native,extra,10));
    assert(ExpBench_Count(&other)==5 && !ExpBench_Player(&opponent,&other,16));
    native.players[4].goalsScored=2; native.players[4].numYellowCards=1;
    b.extra[9].shirtNumber=26; b.extra[9].fullName[0]='A';
    PlayerInfo entering=b.extra[9],leaving=native.players[4];
    assert(ExpBench_TrySwap(&native,&b,25,4));
    leaving.position=-1;
    assert(!memcmp(&native.players[4],&entering,sizeof entering));
    assert(!memcmp(&b.extra[9],&leaving,sizeof leaving));
    assert(!ExpBench_TrySwap(&native,&b,25,5));
    assert(!ExpBench_TrySwap(&native,&b,26,5));
    b.extra[8].skip=1; assert(!ExpBench_TrySwap(&native,&b,24,5));
    b.extra[7].numYellowCards=2; assert(!ExpBench_TrySwap(&native,&b,23,5));
    assert(!ExpBench_TrySwap(&native,&b,22,11));
    assert(ExpBench_TrySwap(&native,&b,22,5));
    assert(ExpBench_SeatY(374,15,1,15)==ExpBench_SeatY(374,5,1,5));
    assert(ExpBench_SeatY(470,15,0,0)==ExpBench_SeatY(470,5,0,0));
    assert(ExpBench_SeatY(374,15,1,0)==304);
    assert(ExpBench_SeatY(470,15,0,15)==575);
    assert(ExpBench_AnchorStart(360,42,112,1)==290);
    assert(ExpBench_AnchorStart(456,42,112,0)==456);
    puts("EXPANDED_MATCH_BENCH_ISOLATED=PASS");
    return 0;
}
