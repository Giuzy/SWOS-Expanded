/* SWOS Expanded OG fixture-journal identity index (pure, endian-neutral).
 * Capacity scales with observed matches. This code ONLY checks identifiers,
 * it never schedules or simulates matches.
 */
#ifndef EXPANDED_OG_FIXTURE_HASH_H
#define EXPANDED_OG_FIXTURE_HASH_H
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define EXP_OG_HASH_RECORD_SIZE 20u
static inline uint32_t ExpandedOGFixture_HashKey(const uint8_t *record)
{
    uint32_t h=2166136261u;
    for(unsigned i=0;i<12u;++i){h^=record[i];h*=16777619u;}
    h^=record[14];h*=16777619u;
    return h;
}
static inline int ExpandedOGFixture_KeySame(const uint8_t *a,
                                             const uint8_t *b)
{
    return memcmp(a,b,12u)==0 && a[14]==b[14];
}
/* 1 duplicate, 0 free bucket, -1 invalid/full index. */
static inline int ExpandedOGFixture_Locate(
    const uint8_t *wire,const uint32_t *buckets,size_t capacity,
    const uint8_t *record,size_t *position)
{
    if(!capacity || !buckets || !record || !position ||
       (capacity&(capacity-1u)))return -1;
    size_t pos=(size_t)ExpandedOGFixture_HashKey(record)&(capacity-1u);
    for(size_t k=0;k<capacity;++k) {
        const uint32_t entry=buckets[pos];
        if(!entry){*position=pos;return 0;}
        if(!wire || ExpandedOGFixture_KeySame(wire+
           ((size_t)entry-1u)*EXP_OG_HASH_RECORD_SIZE,record)) {
            *position=pos;return 1;
        }
        pos=(pos+1u)&(capacity-1u);
    }
    return -1;
}
static inline int ExpandedOGFixture_BuildIndex(
    const uint8_t *wire,size_t count,uint32_t *buckets,size_t capacity)
{
    if(!buckets || !capacity || (capacity&(capacity-1u)) ||
       count>UINT32_MAX || count>capacity/2u ||
       (count && !wire) || capacity>SIZE_MAX/sizeof(*buckets))
        return 0;
    memset(buckets,0,capacity*sizeof(*buckets));
    for(size_t i=0;i<count;++i) {
        size_t pos=0u;
        if(ExpandedOGFixture_Locate(wire,buckets,capacity,
              wire+i*EXP_OG_HASH_RECORD_SIZE,&pos)!=0)return 0;
        buckets[pos]=(uint32_t)(i+1u);
    }
    return 1;
}
static inline int ExpandedOGFixture_MakeIndex(
    const uint8_t *wire,size_t count,
    uint32_t **out,size_t *capacity)
{
    if(!out || !capacity)return 0;
    *out=NULL;*capacity=0u;
    if(count>UINT32_MAX || count>(SIZE_MAX-2u)/2u ||
       (count && !wire))return 0;
    const size_t required=(count+1u)*2u;
    size_t cap=2048u;
    while(cap<required) {
        if(cap>SIZE_MAX/2u || cap>SIZE_MAX/(2u*sizeof(uint32_t)))
            return 0;
        cap*=2u;
    }
    if(cap>SIZE_MAX/sizeof(uint32_t))return 0;
    uint32_t *scratch=(uint32_t *)calloc(cap,sizeof(uint32_t));
    if(!scratch)return 0;
    if(!ExpandedOGFixture_BuildIndex(wire,count,scratch,cap)) {
        free(scratch);return 0;
    }
    *out=scratch;*capacity=cap;
    return 1;
}
#endif /* EXPANDED_OG_FIXTURE_HASH_H */
