/* Portable CAR container around upstream Zstandard v1.5.7.
 * An 8-byte magic prevents old JSON saves from being misread as compressed.
 * 24-byte header: magic, raw length LE32, compressed length LE32, raw FNV64 LE.
 * All fields verified BEFORE JSON restore; no decompression to game state.
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
enum {CAR_ZSTD_HEAD=24,CAR_ZSTD_LEVEL=3};
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
    return bytes && size>=8u && memcmp(bytes,s_car_magic,8u)==0;
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
int SaveCarZstd_Decode(const uint8_t *frame,uint32_t frameSize,
                       uint8_t **json,uint32_t *jsonSize) {
    if(!json || !jsonSize)return 0;
    *json=NULL;*jsonSize=0;
    if(!SaveCarZstd_IsFrame(frame,frameSize) ||
       frameSize<CAR_ZSTD_HEAD)return 0;
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
