/* Isolated real-C codec round-trip; no SDL, career state or gameplay.
 * The test links the vendored Zstandard 1.5.7 sources, not a new DLL. */
#include "save_car_zstd.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* Provide only the allocation symbols used by the codec; the full Windows
 * runtime uses the real swos_io.c memory manager instead. */
void *swos_io_alloc(uint32_t n) {return malloc((size_t)n);}
void swos_io_free(void *p) {free(p);}
int main(void)
{
    enum {BYTES=150000};
    uint8_t *raw=(uint8_t *)malloc(BYTES);
    assert(raw);
    raw[0]='{';raw[1]='"';
    for(size_t i=2u;i<BYTES-2u;++i)
        raw[i]=(uint8_t)('a'+(i%7u));
    raw[BYTES-2u]='"';raw[BYTES-1u]='}';
    uint8_t *out=NULL;uint32_t stored=0;
    assert(SaveCarZstd_Encode(raw,BYTES,&out,&stored));
    assert(stored<BYTES);
    assert(SaveCarZstd_IsFrame(out,stored));
    assert(!SaveCarZstd_IsFrame(raw,BYTES)); /* legacy plain JSON */
    uint8_t *decoded=NULL;uint32_t size=0;
    assert(SaveCarZstd_Decode(out,stored,&decoded,&size));
    assert(size==BYTES && memcmp(raw,decoded,BYTES)==0);
    swos_io_free(decoded);
    decoded=NULL;
    out[16]^=0x40u; /* checksum corruption must fail closed */
    assert(!SaveCarZstd_Decode(out,stored,&decoded,&size));
    out[16]^=0x40u;
    out[12]^=1u; /* forged compressed length */
    assert(!SaveCarZstd_Decode(out,stored,&decoded,&size));
    out[12]^=1u;
    out[24]^=1u; /* damaged Zstandard magic */
    assert(!SaveCarZstd_Decode(out,stored,&decoded,&size));
    out[24]^=1u;
    assert(!SaveCarZstd_Decode(out,stored-1u,&decoded,&size));
    /* v287: truncation anywhere, header-only, empty and a damaged payload byte
     * must all fail closed (the byte-for-byte read-back relies on this). */
    assert(!SaveCarZstd_Decode(out,stored/2u,&decoded,&size));
    assert(!SaveCarZstd_Decode(out,24u,&decoded,&size));
    assert(!SaveCarZstd_Decode(out,8u,&decoded,&size));
    assert(!SaveCarZstd_Decode(out,0u,&decoded,&size));
    out[24u+(stored-24u)/2u]^=0x5au;
    assert(!SaveCarZstd_Decode(out,stored,&decoded,&size));
    out[24u+(stored-24u)/2u]^=0x5au;
    assert(SaveCarZstd_Decode(out,stored,&decoded,&size) && size==BYTES &&
           memcmp(raw,decoded,BYTES)==0); /* restored bytes decode again */
    swos_io_free(decoded);decoded=NULL;
    free(raw);free(out);
    puts("CAR_ZSTD_LEVEL3_ROUNDTRIP_LEGACY_DETECT_CORRUPTION=PASS_ISOLATED_ONLY");
    return 0;
}
