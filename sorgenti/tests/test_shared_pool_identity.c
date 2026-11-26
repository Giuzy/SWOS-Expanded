#include <assert.h>
#include <stdio.h>
#include "expanded_ce_shared_pool_identity.h"
int main(void)
{
    uint16_t a=0,b=0;
    /* Native end from WORLD2627 baked 91 resize_country calls, v316.
     * 14 real pool IDs fit; no native global number or pool ID changes. */
    assert(EXP_CE_POOL_NATIVE_MARGIN==128u);
    for(uint32_t i=1;i<=119;++i) {
        assert(ExpCePool_GlobalSlot(2560,2313,i,&a));
        assert(a==2560-i);
        assert(ExpCePool_IsReservedGlobal(2313,a));
        if(i>1)assert(a+1u==b);
        b=a;
    }
    assert(a==2441);
    assert(!ExpCePool_IsReservedGlobal(2313,2440));
    assert(!ExpCePool_GlobalSlot(2560,2313,120,&a) && a==2441);
    assert(!ExpCePool_GlobalSlot(2560,2313,0,&a));
    assert(!ExpCePool_GlobalSlot(2560,2313,2561,&a));
    assert(!ExpCePool_GlobalSlot(2560,UINT32_MAX,1,&a));
    assert(!ExpCePool_IsReservedGlobal(UINT32_MAX,32767));
    assert(!ExpCePool_GlobalSlot(65536,0,1,&a));
    assert(!ExpCePool_GlobalSlot(2560,2313,1,NULL));
    assert(ExpCePool_GlobalSlot(2560,2313,14,&a) && a==2546);
    assert(ExpCePool_GlobalSlot(2560,2313,1,&b) && b==2559);
    /* Old fixture used 2226; the already allocated top IDs stay identical. */
    assert(ExpCePool_GlobalSlot(2560,2226,78,&a) && a==2482);
    puts("SHARED_POOL_GLOBAL_ID=PASS native_end=2313 margin=128 capacity=119");
    return 0;
}
