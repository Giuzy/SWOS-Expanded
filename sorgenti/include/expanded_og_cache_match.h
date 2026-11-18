/* SWOS Expanded - read-only manager-keycache match lookup.
 * Does not execute matches, mutate OG state, or claim to reconcile a world.
 * Team IDs in keycache match records are BE; stride and score are host-native.
 */
#ifndef EXPANDED_OG_CACHE_MATCH_H
#define EXPANDED_OG_CACHE_MATCH_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    EXP_OG_CACHE_NOT_FOUND=0,
    EXP_OG_CACHE_SCHEDULED=1,
    EXP_OG_CACHE_PLAYED=2,
    EXP_OG_CACHE_AMBIGUOUS=3,
    EXP_OG_CACHE_CORRUPT=4
} ExpandedOGCacheStatus;

typedef struct {
    uint8_t comp_slot, phase, flags;
    uint16_t home, away, packed_goals;
    unsigned match_count;
    size_t record_end, used_bytes;
} ExpandedOGCacheMatch;

/* Validate the occupied OG stream before SERIALIZATION or after RESTORE.
 * The historical 1870-byte CAR blob and the full 8192-byte buffer use
 * identical entry layouts. The save writer must reject, not truncate, an
 * invalid or overflowing stream. Compiles on host/Amiga without SDL. */
#define EXP_OG_CACHE_CAPACITY 8192u
#define EXP_OG_CACHE_LEGACY_SIZE 1870u
static inline int ExpandedOGCache_Measure(const uint8_t *bytes,size_t capacity,
                                           uint16_t count,size_t *used)
{
    size_t off=0u;
    if(!bytes || !used || capacity<10u || count>capacity/10u)return 0;
    for(uint16_t i=0;i<count;i++) {
        uint16_t stride=0u;
        if(off>capacity || capacity-off<10u)return 0;
        memcpy(&stride,bytes+off+2u,sizeof stride);
        if(stride<10u || (size_t)stride>capacity-off ||
           bytes[off+5u]>(size_t)(stride-10u)/12u)return 0;
        off+=stride;
    }
    *used=off;
    return 1;
}

/* The caller supplies the actual occupied stream size (not an arbitrary
 * number of entries in untrusted memory). A duplicate key fails closed:
 * world+edition identity must be checked separately by the caller.
 * No inference from rank, club name or home/away alone is permitted. */
static inline ExpandedOGCacheStatus ExpandedOGCache_Find(
    const uint8_t *bytes,size_t capacity,uint16_t entry_count,
    uint8_t comp_slot,uint8_t phase,uint16_t home,uint16_t away,
    ExpandedOGCacheMatch *out)
{
    size_t off=0u;
    unsigned found=0u;
    ExpandedOGCacheMatch candidate={0};
    if(!bytes || !out || capacity<10u || entry_count>capacity/10u)
        return EXP_OG_CACHE_CORRUPT;
    memset(out,0,sizeof(*out));
    for(uint16_t i=0;i<entry_count;++i){
        uint16_t stride=0u;
        if(off>capacity || capacity-off<10u)return EXP_OG_CACHE_CORRUPT;
        memcpy(&stride,bytes+off+2u,sizeof stride);
        if(stride<10u || stride>capacity-off ||
           bytes[off+5u]>(size_t)(stride-10u)/12u)
            return EXP_OG_CACHE_CORRUPT;
        if(bytes[off]==comp_slot && bytes[off+1u]==phase){
            const unsigned n=bytes[off+5u];
            for(unsigned j=0;j<n;++j){
                const uint8_t *s=bytes+off+10u+(size_t)j*12u;
                const uint16_t h=(uint16_t)(((uint16_t)s[2]<<8)|s[3]);
                const uint16_t a=(uint16_t)(((uint16_t)s[4]<<8)|s[5]);
                if(h!=home || a!=away)continue;
                if(++found>1u)return EXP_OG_CACHE_AMBIGUOUS;
                candidate.comp_slot=comp_slot;
                candidate.phase=phase;
                candidate.home=home;
                candidate.away=away;
                candidate.flags=s[1];
                candidate.record_end=off+10u+(size_t)(j+1u)*12u;
                memcpy(&candidate.packed_goals,s+6u,sizeof candidate.packed_goals);
            }
        }
        off+=(size_t)stride;
    }
    if(!found){out->used_bytes=off;return EXP_OG_CACHE_NOT_FOUND;}
    candidate.match_count=found;
    candidate.used_bytes=off;
    *out=candidate;
    return candidate.flags&0x80u?EXP_OG_CACHE_PLAYED:EXP_OG_CACHE_SCHEDULED;
}
#endif
