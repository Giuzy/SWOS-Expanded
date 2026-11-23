/* Portable CAR container around upstream Zstandard v1.5.7.
 * Two versioned containers share ONE reader (SaveCarZstd_Decode):
 *  - 'SWOSZC3A' single frame (v286..): 24-byte header: magic, raw length LE32,
 *    compressed length LE32, raw FNV64 LE, then ONE Zstandard frame.
 *  - 'SWOSZC4A' INDEXED BLOCKS (v292): header {magic, raw_total LE32,
 *    block_count LE32, whole-image XXH64}, then block_count index entries
 *    {raw_len LE32, comp_len LE32, raw XXH64}, then the block frames, then a
 *    XXH64 trailer over everything before it. Every block is an independent
 *    Zstandard frame, so an UNCHANGED block (same length + FNV64 of its raw
 *    bytes) is reused from the in-memory block cache instead of being
 *    compressed again. The decoder NEVER trusts the index: sizes are summed
 *    with 64-bit checks, each block decodes to exactly its declared length
 *    and matches its hash, and the concatenation matches the whole hash.
 * An 8-byte magic prevents old JSON saves from being misread as compressed.
 * All fields are verified BEFORE JSON restore; no decompression to game state.
 * ZSTD level 3 is explicit, not a runtime tuneable gameplay parameter.
 * Source/license: docs/ZSTANDARD/. */
#include "save_car_zstd.h"
#if defined(SWOS_SDL_BUILD) && defined(SWOS_CAR_ZSTD)
#include "swos_io.h"
#include "../third_party/zstd/lib/zstd.h"
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static const uint8_t s_car_magic[8]={'S','W','O','S','Z','C','3','A'};
static const uint8_t s_car4_magic[8]={'S','W','O','S','Z','C','4','A'};
enum {CAR_ZSTD_HEAD=24,CAR_ZSTD_LEVEL=3,CAR4_HEAD=24,CAR4_ENTRY=16,
      CAR4_MAX_BLOCKS=1u<<20,CAR_CACHE_CAP_BYTES=64u*1024u*1024u};
static void put32(uint8_t *p,uint32_t n) {
    for(unsigned i=0;i<4u;++i)p[i]=(uint8_t)(n>>(8u*i));
}
static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|
           ((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void put64(uint8_t *p,uint64_t n) {
    for(unsigned i=0;i<8u;++i)p[i]=(uint8_t)(n>>(8u*i));
}
static uint64_t get64(const uint8_t *p) {
    uint64_t n=0;
    for(unsigned i=0;i<8u;++i)n|=(uint64_t)p[i]<<(8u*i);
    return n;
}
static uint64_t hash64(const uint8_t *p,size_t n) {
    uint64_t h=UINT64_C(14695981039346656037);
    for(size_t i=0;i<n;++i){h^=p[i];h*=UINT64_C(1099511628211);}
    return h;
}
int SaveCarZstd_IsFrame(const uint8_t *bytes,uint32_t size) {
    return bytes && size>=8u &&
           (memcmp(bytes,s_car_magic,8u)==0 || memcmp(bytes,s_car4_magic,8u)==0);
}
int SaveCarZstd_IsBlocks(const uint8_t *bytes,uint32_t size) {
    return bytes && size>=8u && memcmp(bytes,s_car4_magic,8u)==0;
}
int SaveCarZstd_Encode(const uint8_t *json,uint32_t jsonSize,
                       uint8_t **frame,uint32_t *frameSize) {
    if(!frame || !frameSize)return 0;
    *frame=NULL;*frameSize=0;
    if(!json || !jsonSize || jsonSize>INT32_MAX)return 0;
    const size_t bound=ZSTD_compressBound((size_t)jsonSize);
    if(ZSTD_isError(bound) || bound>UINT32_MAX-CAR_ZSTD_HEAD ||
       bound>SIZE_MAX-CAR_ZSTD_HEAD)return 0;
    uint8_t *buf=(uint8_t *)malloc(CAR_ZSTD_HEAD+bound);
    if(!buf)return 0;
    size_t n=ZSTD_compress(buf+CAR_ZSTD_HEAD,bound,json,jsonSize,CAR_ZSTD_LEVEL);
    if(ZSTD_isError(n) || n>UINT32_MAX-CAR_ZSTD_HEAD) {
        free(buf);return 0;
    }
    memcpy(buf,s_car_magic,8);
    put32(buf+8u,jsonSize);
    put32(buf+12u,(uint32_t)n);
    put64(buf+16u,hash64(json,jsonSize));
    *frame=buf;*frameSize=(uint32_t)n+CAR_ZSTD_HEAD;
    return 1;
}

/* XXH64 from the vendored Zstandard (built with XXH_NAMESPACE=ZSTD_): ~20x
 * faster than the byte-wise FNV that the v3 container keeps for compatibility.
 * The v4 container is new, so it owns its hash choice. */
extern unsigned long long ZSTD_XXH64(const void *input,size_t length,
                                      unsigned long long seed);
static uint64_t hash4(const uint8_t *p,size_t n) {
    return (uint64_t)ZSTD_XXH64(p,n,0u);
}
/* ---- block cache (writer side only) ------------------------------------
 * Entries are owned malloc'd compressed frames keyed by (raw_len, raw FNV64).
 * ONLY EncodeBlocks/CacheReset touch it, and the CarWriter runs one job at a
 * time, so it needs no lock. The decoder never reads or fills it: after a
 * CONTINUE or a world switch the cache is simply COLD and the next save pays
 * the full compression once. A wrong hit cannot reach disk: the caller
 * decodes the produced container and compares it with the image first. */
typedef struct CarBlockEntry {
    uint64_t hash;
    uint32_t raw_len,comp_len;
    uint8_t *comp;
} CarBlockEntry;
static CarBlockEntry *s_cache;
static uint32_t s_cache_n;
static SaveCarZstdStats s_stats;
static void cache_free(void)
{
    for(uint32_t i=0;i<s_cache_n;++i)free(s_cache[i].comp);
    free(s_cache);
    s_cache=NULL;s_cache_n=0u;
}
void SaveCarZstd_CacheReset(void) {cache_free();}
SaveCarZstdStats SaveCarZstd_Stats(void) {return s_stats;}
static const CarBlockEntry *cache_find(uint64_t h,uint32_t raw_len)
{
    for(uint32_t i=0;i<s_cache_n;++i)
        if(s_cache[i].hash==h && s_cache[i].raw_len==raw_len)return &s_cache[i];
    return NULL;
}
int SaveCarZstd_EncodeBlocks(const uint8_t *json,uint32_t jsonSize,
                             const uint32_t *cuts,uint32_t ncuts,
                             uint8_t **frame,uint32_t *frameSize) {
    CarBlockEntry *blk=NULL;
    uint8_t *out=NULL;
    uint32_t nb=ncuts+1u,prev=0u,i;
    uint64_t total=CAR4_HEAD,payload=0u;
    size_t at;
    uint32_t hits=0u,misses=0u;
    uint64_t hit_raw=0u,all_raw=0u;
    if(!frame || !frameSize)return 0;
    *frame=NULL;*frameSize=0;
    if(!json || !jsonSize || jsonSize>INT32_MAX || nb>CAR4_MAX_BLOCKS ||
       (ncuts && !cuts))return 0;
    for(i=0;i<ncuts;++i) {
        if(cuts[i]<=prev || cuts[i]>=jsonSize)return 0;
        prev=cuts[i];
    }
    blk=(CarBlockEntry *)calloc(nb,sizeof *blk);
    if(!blk)return 0;
    prev=0u;
    for(i=0;i<nb;++i) {
        const uint32_t end=i<ncuts?cuts[i]:jsonSize;
        const uint32_t len=end-prev;
        const uint8_t *src=json+prev;
        const uint64_t h=hash4(src,len);
        const CarBlockEntry *hit=cache_find(h,len);
        blk[i].hash=h;blk[i].raw_len=len;
        if(hit) {
            blk[i].comp=(uint8_t *)malloc(hit->comp_len);
            if(!blk[i].comp)goto fail;
            memcpy(blk[i].comp,hit->comp,hit->comp_len);
            blk[i].comp_len=hit->comp_len;
            ++hits;hit_raw+=len;
        } else {
            const size_t bound=ZSTD_compressBound(len);
            size_t n;
            if(ZSTD_isError(bound) || bound>UINT32_MAX)goto fail;
            blk[i].comp=(uint8_t *)malloc(bound);
            if(!blk[i].comp)goto fail;
            n=ZSTD_compress(blk[i].comp,bound,src,len,CAR_ZSTD_LEVEL);
            if(ZSTD_isError(n) || n>UINT32_MAX)goto fail;
            blk[i].comp_len=(uint32_t)n;
            ++misses;
        }
        all_raw+=len;payload+=blk[i].comp_len;
        prev=end;
    }
    total+=(uint64_t)nb*CAR4_ENTRY+payload+8u;
    if(total>UINT32_MAX || total>SIZE_MAX)goto fail;
    out=(uint8_t *)malloc((size_t)total);
    if(!out)goto fail;
    memcpy(out,s_car4_magic,8u);
    put32(out+8u,jsonSize);put32(out+12u,nb);
    put64(out+16u,hash4(json,jsonSize));
    at=CAR4_HEAD;
    for(i=0;i<nb;++i) {
        put32(out+at,blk[i].raw_len);put32(out+at+4u,blk[i].comp_len);
        put64(out+at+8u,blk[i].hash);
        at+=CAR4_ENTRY;
    }
    for(i=0;i<nb;++i) {
        memcpy(out+at,blk[i].comp,blk[i].comp_len);
        at+=blk[i].comp_len;
    }
    put64(out+at,hash4(out,at));
    at+=8u;
    if(at!=(size_t)total)goto fail;
    /* Replace the cache by THIS generation's blocks (bounded by memory). */
    cache_free();
    {
        uint64_t kept=0u;
        s_cache=(CarBlockEntry *)calloc(nb,sizeof *s_cache);
        if(s_cache) {
            for(i=0;i<nb;++i) {
                if(kept+blk[i].comp_len>CAR_CACHE_CAP_BYTES)continue;
                s_cache[s_cache_n++]=blk[i]; /* ownership moves */
                kept+=blk[i].comp_len;
                blk[i].comp=NULL;
            }
        }
        s_stats.cache_bytes=kept;
    }
    for(i=0;i<nb;++i)free(blk[i].comp);
    free(blk);
    s_stats.hits=hits;s_stats.misses=misses;
    s_stats.raw_reused=hit_raw;s_stats.raw_total=all_raw;
    s_stats.blocks=nb;
    *frame=out;*frameSize=(uint32_t)total;
    return 1;
fail:
    if(blk) {
        for(i=0;i<nb;++i)free(blk[i].comp);
        free(blk);
    }
    free(out);
    return 0;
}
static int decode_blocks(const uint8_t *frame,uint32_t frameSize,
                         uint8_t **json,uint32_t *jsonSize) {
    uint32_t i,nb;
    uint64_t raw_sum=0u,comp_sum=0u,off=0u;
    size_t at;
    uint8_t *buf=NULL;
    if(frameSize<CAR4_HEAD+CAR4_ENTRY+8u)return 0;
    {
        const uint32_t raw=get32(frame+8u);
        nb=get32(frame+12u);
        if(!raw || raw>INT32_MAX || !nb || nb>CAR4_MAX_BLOCKS)return 0;
        if((uint64_t)CAR4_HEAD+(uint64_t)nb*CAR4_ENTRY+8u>frameSize)return 0;
        /* trailer first: covers header, index and every payload byte */
        if(hash4(frame,frameSize-8u)!=get64(frame+frameSize-8u))return 0;
        at=CAR4_HEAD;
        for(i=0;i<nb;++i) {
            const uint32_t rl=get32(frame+at),cl=get32(frame+at+4u);
            if(!rl || !cl)return 0;
            raw_sum+=rl;comp_sum+=cl;at+=CAR4_ENTRY;
        }
        if(raw_sum!=raw ||
           (uint64_t)CAR4_HEAD+(uint64_t)nb*CAR4_ENTRY+comp_sum+8u!=frameSize)
            return 0;
        buf=(uint8_t *)swos_io_alloc(raw+1u);
        if(!buf)return 0;
        at=CAR4_HEAD;
        {
            size_t pay=(size_t)CAR4_HEAD+(size_t)nb*CAR4_ENTRY;
            for(i=0;i<nb;++i) {
                const uint32_t rl=get32(frame+at),cl=get32(frame+at+4u);
                const uint64_t bh=get64(frame+at+8u);
                const unsigned long long expected=
                    ZSTD_getFrameContentSize(frame+pay,cl);
                size_t actual;
                if(expected==ZSTD_CONTENTSIZE_ERROR ||
                   (expected!=ZSTD_CONTENTSIZE_UNKNOWN && expected!=rl))
                    goto bad;
                actual=ZSTD_decompress(buf+off,rl,frame+pay,cl);
                if(ZSTD_isError(actual) || actual!=rl ||
                   hash4(buf+off,rl)!=bh)goto bad;
                off+=rl;pay+=cl;at+=CAR4_ENTRY;
            }
        }
        if(off!=raw || hash4(buf,raw)!=get64(frame+16u) || buf[0]!='{')
            goto bad;
        buf[raw]=0;
        *json=buf;*jsonSize=raw;
        return 1;
    }
bad:
    swos_io_free(buf);
    return 0;
}
int SaveCarZstd_Decode(const uint8_t *frame,uint32_t frameSize,
                       uint8_t **json,uint32_t *jsonSize) {
    if(!json || !jsonSize)return 0;
    *json=NULL;*jsonSize=0;
    if(!SaveCarZstd_IsFrame(frame,frameSize))return 0;
    if(SaveCarZstd_IsBlocks(frame,frameSize))
        return decode_blocks(frame,frameSize,json,jsonSize);
    if(frameSize<CAR_ZSTD_HEAD)return 0;
    const uint32_t raw=get32(frame+8u),packed=get32(frame+12u);
    if(!raw || raw>INT32_MAX || packed!=(frameSize-CAR_ZSTD_HEAD) ||
       !packed)return 0;
    const unsigned long long expected=
        ZSTD_getFrameContentSize(frame+CAR_ZSTD_HEAD,packed);
    if(expected==ZSTD_CONTENTSIZE_ERROR ||
       (expected!=ZSTD_CONTENTSIZE_UNKNOWN && expected!=(uint64_t)raw))
        return 0;
    uint8_t *buf=(uint8_t *)swos_io_alloc(raw+1u);
    if(!buf)return 0;
    size_t actual=ZSTD_decompress(buf,(size_t)raw,
                                 frame+CAR_ZSTD_HEAD,packed);
    if(ZSTD_isError(actual) || actual!=(size_t)raw ||
       hash64(buf,raw)!=get64(frame+16u) ||
       buf[0]!='{') {
        swos_io_free(buf);return 0;
    }
    buf[raw]=0;
    *json=buf;*jsonSize=raw;
    return 1;
}
#endif
