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
extern unsigned long long ZSTD_XXH64(const void *,size_t,unsigned long long);
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
    /* v292: indexed-block container 'SWOSZC4A' + block cache. */
    {
        enum {N=300000};
        uint8_t *img=(uint8_t *)malloc(N);
        uint32_t ksz=0;
        uint32_t cuts[5]={50000u,100000u,100001u,200000u,250000u},bsz=0,rsz=0;
        uint8_t *b1=NULL,*b2=NULL,*dec=NULL;
        SaveCarZstdStats st;
        size_t k;
        assert(img);
        img[0]='{';
        for(size_t i=1;i<N-1;++i)img[i]=(uint8_t)('a'+((i*i)%13u));
        img[N-1]='}';
        SaveCarZstd_CacheReset();
        assert(SaveCarZstd_EncodeBlocks(img,N,cuts,5,&b1,&bsz) && b1);
        assert(SaveCarZstd_IsFrame(b1,bsz) && SaveCarZstd_IsBlocks(b1,bsz));
        assert(SaveCarZstd_Decode(b1,bsz,&dec,&rsz) && rsz==N && !memcmp(dec,img,N));
        swos_io_free(dec);dec=NULL;
        st=SaveCarZstd_Stats();
        assert(st.blocks==6 && st.hits==0 && st.misses==6);
        /* warm cache: identical image -> every block reused, identical bytes */
        assert(SaveCarZstd_EncodeBlocks(img,N,cuts,5,&b2,&rsz) && rsz==bsz &&
               !memcmp(b1,b2,bsz));
        st=SaveCarZstd_Stats();
        assert(st.hits==6 && st.misses==0 && st.raw_reused==N);
        free(b2);b2=NULL;
        /* one changed block (same size) -> only that block is recompressed */
        img[120000]^=0x11;
        assert(SaveCarZstd_EncodeBlocks(img,N,cuts,5,&b2,&rsz));
        st=SaveCarZstd_Stats();
        assert(st.hits==5 && st.misses==1);
        assert(SaveCarZstd_Decode(b2,rsz,&dec,&ksz) && ksz==N && !memcmp(dec,img,N));
        swos_io_free(dec);dec=NULL;free(b2);b2=NULL;
        /* cold cache after a reset: full compression again, same content */
        SaveCarZstd_CacheReset();
        assert(SaveCarZstd_EncodeBlocks(img,N,cuts,5,&b2,&rsz));
        st=SaveCarZstd_Stats();
        assert(st.hits==0 && st.misses==6);
        free(b2);b2=NULL;
        /* invalid cut tables are refused, nothing allocated for the caller */
        { uint32_t bad1[2]={100u,100u},bad2[1]={0u},bad3[1]={N},bad4[2]={9u,5u};
          assert(!SaveCarZstd_EncodeBlocks(img,N,bad1,2,&b2,&rsz) && !b2);
          assert(!SaveCarZstd_EncodeBlocks(img,N,bad2,1,&b2,&rsz) && !b2);
          assert(!SaveCarZstd_EncodeBlocks(img,N,bad3,1,&b2,&rsz) && !b2);
          assert(!SaveCarZstd_EncodeBlocks(img,N,bad4,2,&b2,&rsz) && !b2);
          assert(!SaveCarZstd_EncodeBlocks(img,N,NULL,1,&b2,&rsz) && !b2); }
        /* zero cuts = one block, still the indexed container */
        assert(SaveCarZstd_EncodeBlocks(img,N,NULL,0,&b2,&rsz));
        assert(SaveCarZstd_Decode(b2,rsz,&dec,&ksz) && ksz==N && !memcmp(dec,img,N));
        swos_io_free(dec);dec=NULL;free(b2);b2=NULL;
        /* damage anywhere is rejected: header, index, payload, trailer */
        for(k=0;k<bsz;k+=(k<400u?1u:bsz/233u+1u)) {
            b1[k]^=0x21u;
            assert(!SaveCarZstd_Decode(b1,bsz,&dec,&rsz) && !dec);
            b1[k]^=0x21u;
        }
        assert(SaveCarZstd_Decode(b1,bsz,&dec,&rsz));swos_io_free(dec);dec=NULL;
        for(k=1;k<64u;++k)assert(!SaveCarZstd_Decode(b1,bsz-(uint32_t)k,&dec,&rsz));
        assert(!SaveCarZstd_Decode(b1,bsz/2u,&dec,&rsz));
        assert(!SaveCarZstd_Decode(b1,8u,&dec,&rsz));
        { /* forged but self-consistent trailer over a lying index entry */
          uint8_t *f=(uint8_t *)malloc(bsz);uint64_t h;
          size_t i;memcpy(f,b1,bsz);f[24]^=1u; /* raw_len of block 0 */
          h=(uint64_t)ZSTD_XXH64(f,bsz-8u,0u);
          for(i=0;i<8u;++i)f[bsz-8u+i]=(uint8_t)(h>>(8u*i));
          assert(!SaveCarZstd_Decode(f,bsz,&dec,&rsz));free(f); }
        free(b1);free(img);
        SaveCarZstd_CacheReset();
    }
    free(raw);free(out);
    puts("CAR_ZSTD_LEVEL3_ROUNDTRIP_LEGACY_DETECT_CORRUPTION=PASS_ISOLATED_ONLY");
    return 0;
}
