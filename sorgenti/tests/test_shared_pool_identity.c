#include <assert.h>
#include <stdio.h>
#include "expanded_ce_shared_pool_identity.h"
int main(void)
{
    uint16_t a=0,b=0;
    for(uint32_t i=1;i<=78;++i) {
        assert(ExpCePool_GlobalSlot(2560,2226,i,&a));
        assert(a==2560-i);
        if(i>1)assert(a+1u==b);
        b=a;
    }
    assert(a==2482);
    assert(!ExpCePool_GlobalSlot(2560,2226,79,&a) && a==2482);
    assert(!ExpCePool_GlobalSlot(2560,2226,0,&a));
    assert(!ExpCePool_GlobalSlot(2560,2226,2561,&a));
    assert(!ExpCePool_GlobalSlot(2560,UINT32_MAX,1,&a));
    assert(!ExpCePool_GlobalSlot(65536,0,1,&a));
    assert(!ExpCePool_GlobalSlot(2560,2226,1,NULL));
    assert(ExpCePool_GlobalSlot(2560,2226,10,&a) && a==2550);
    assert(ExpCePool_GlobalSlot(2560,2226,1,&b) && b==2559);
    puts("SHARED_POOL_GLOBAL_ID=PASS 78 slots, 6 negative cases, stable ordering");
    return 0;
}
