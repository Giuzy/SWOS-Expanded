#ifndef EXPANDED_CE_SHARED_POOL_IDENTITY_H
#define EXPANDED_CE_SHARED_POOL_IDENTITY_H
#include <stdint.h>
/* Reserve from the high end of the existing global table. The complete
 * legacy base range, including any 8-bit raw ordinal, is excluded. Never
 * shift existing global IDs or silently reuse a pool id. */
static inline int ExpCePool_GlobalSlot(uint32_t capacity,uint32_t native_end,
    uint32_t pool_id,uint16_t *out)
{
    uint32_t slot;
    if(!out || !pool_id || pool_id>capacity || native_end>UINT32_MAX-256u)
        return 0;
    slot=capacity-pool_id;
    if(slot<native_end+256u || slot>32767u)return 0;
    *out=(uint16_t)slot;
    return 1;
}
#endif
