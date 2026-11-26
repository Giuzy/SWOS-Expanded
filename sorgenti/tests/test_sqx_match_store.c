#include <assert.h>
#include <stdio.h>
#include "sqx_match_store.h"

int main(void)
{
    SqxMatchStore a={0},restored={0};
    PlayerFile p={0};
    memcpy(p.playerName,"REAL EXTRA",11);
    p.shirtNumber=27;p.cardsInjuries=0xa2;p.goalStats[0]=3;p.form=-17;
    assert(!SqxMatch_Insert(&a,1,15,&p));
    assert(!SqxMatch_Insert(&a,1,30,&p));
    SqxMatchRecord *first=SqxMatch_Ensure(&a,65535,29,&p);
    assert(first && first->bytes+76==(uint8_t *)&first->player);
    first->player.goalStats[0]=10;
    for(unsigned team=0;team<1024;team++)
        for(unsigned idx=16;idx<30;idx++)
            assert(SqxMatch_Ensure(&a,(uint16_t)team,(uint16_t)idx,&p));
    assert(a.count==14337);
    assert(SqxMatch_Ensure(&a,65535,29,&p)==first);
    assert(first->player.goalStats[0]==10);
    assert(!SqxMatch_Insert(&a,65535,29,&p));
    PlayerFile other=p;other.playerName[0]='X';
    assert(!SqxMatch_Ensure(&a,65535,29,&other));
    for(size_t i=0;i<a.count;i++) {
        SqxMatchRecord *r=a.rows[i];
        if(i)assert(SqxMatch_Key(a.rows[i-1]->team,a.rows[i-1]->index)<SqxMatch_Key(r->team,r->index));
        char h[SQX_MATCH_HEX_LEN+1];PlayerFile q={0};
        SqxMatch_Encode(&r->player,h);
        assert(SqxMatch_Decode(h,strlen(h),&q));
        assert(!memcmp(&q,&r->player,sizeof q));
        assert(SqxMatch_Insert(&restored,r->team,r->index,&q));
    }
    char h[SQX_MATCH_HEX_LEN+1];PlayerFile q=p;
    SqxMatch_Encode(&p,h);h[0]='z';
    assert(!SqxMatch_Decode(h,strlen(h),&q) && !memcmp(&p,&q,sizeof p));
    assert(!SqxMatch_Decode(h,strlen(h)-1,&q));
    memset(other.playerName,'A',sizeof other.playerName);
    assert(!SqxMatch_Insert(&a,3000,16,&other));
    SqxMatch_Encode(&other,h);assert(!SqxMatch_Decode(h,strlen(h),&q));
    assert(SqxMatch_Find(&restored,65535,29)->player.goalStats[0]==10);
    assert(!SqxMatch_Find(&restored,65534,29));
    SqxMatch_Clear(&a);SqxMatch_Clear(&restored);
    assert(!a.count && !a.rows && !a.capacity && !restored.count);
    puts("SQX_MATCH_STORE_ISOLATED=PASS");return 0;
}
