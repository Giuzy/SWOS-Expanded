#ifndef EXPANDED_CE_SHARED_POOL_IDENTITY_H
#define EXPANDED_CE_SHARED_POOL_IDENTITY_H
#include <stdint.h>
/* Reserve from the high end of the existing global table. Preserve the
 * user-approved 128-slot margin after the native range; actual native
 * raw/global collisions are also rejected by the catalog import. */
#define EXP_CE_POOL_NATIVE_MARGIN 128u
static inline int ExpCePool_IsReservedGlobal(uint32_t native_end,uint32_t global)
{
    return native_end<=UINT32_MAX-EXP_CE_POOL_NATIVE_MARGIN &&
        global>=native_end+EXP_CE_POOL_NATIVE_MARGIN;
}
static inline int ExpCePool_GlobalSlot(uint32_t capacity,uint32_t native_end,
    uint32_t pool_id,uint16_t *out)
{
    uint32_t slot;
    if(!out || !pool_id || pool_id>capacity || native_end>UINT32_MAX-EXP_CE_POOL_NATIVE_MARGIN)
        return 0;
    slot=capacity-pool_id;
    if(!ExpCePool_IsReservedGlobal(native_end,slot) || slot>32767u)return 0;
    *out=(uint16_t)slot;
    return 1;
}
#endif
