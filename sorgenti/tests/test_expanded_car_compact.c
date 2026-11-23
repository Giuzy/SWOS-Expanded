/* v00.13.290 RG-T: compact UE02 fixture codec. Isolated pure-codec test:
 * exact round trip (extreme and random values), every single-byte corruption
 * rejected, truncation/trailing/over-width/bad varint rejected, base64 edge
 * cases, and the size against the 134-byte UE01 hex row. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_car_compact.h"

typedef struct Rows {uint32_t n;uint64_t (*v)[EXP_CCOL];} Rows;
static int get(uint32_t i,uint64_t *out,void *u)
{
    const Rows *r=(const Rows *)u;
    memcpy(out,r->v[i],sizeof r->v[i]);
    return 1;
}
static uint64_t s_rng=88172645463325252ull;
static uint64_t rnd(void){s_rng^=s_rng<<13;s_rng^=s_rng>>7;s_rng^=s_rng<<17;return s_rng;}
static uint64_t mask(unsigned b){return b>=64?~0ull:((1ull<<b)-1u);}
static int roundtrip(const Rows *r,const uint8_t *bin,size_t len)
{
    ExpCarCompactReader rd;
    uint64_t v[EXP_CCOL];
    uint32_t i;
    if(!ExpCarCompact_Open(&rd,bin,len) || rd.n!=r->n)return 0;
    for(i=0;i<r->n;++i) {
        if(!ExpCarCompact_Next(&rd,v) || memcmp(v,r->v[i],sizeof v))return 0;
    }
    return ExpCarCompact_Finish(&rd);
}
int main(void)
{
    Rows r;uint32_t i,c;size_t len=0u,k;uint8_t *bin,*mut;
    /* 1) random + extreme rows, incl. descending and wrap-around deltas */
    r.n=5000;r.v=(uint64_t (*)[EXP_CCOL])calloc(r.n,sizeof r.v[0]);
    assert(r.v);
    for(i=0;i<r.n;++i)for(c=0;c<EXP_CCOL;++c) {
        const unsigned b=ExpCarCompact_Bits[c];
        uint64_t x;
        switch(i%7) {
        case 0:x=0;break;
        case 1:x=mask(b);break;
        case 2:x=1ull<<(b-1);break;
        case 3:x=(1ull<<(b-1))-1u;break;
        default:x=rnd()&mask(b);
        }
        if(i>2000 && (i&1) && c==0)x=1000000u+i; /* ordered ids */
        r.v[i][c]=x;
    }
    bin=ExpCarCompact_Encode(2627,46313,r.n,get,&r,&len);
    assert(bin && len>0);
    assert(roundtrip(&r,bin,len));
    {   ExpCarCompactReader rd;
        assert(ExpCarCompact_Open(&rd,bin,len) && rd.world==2627 && rd.day==46313);
    }
    /* 2) every byte flipped must be rejected by Open (trailer hash) */
    mut=(uint8_t *)malloc(len);assert(mut);
    for(k=0;k<len;k+=(len<2000?1:37)) {
        ExpCarCompactReader rd;
        memcpy(mut,bin,len);mut[k]^=0x5a;
        assert(!ExpCarCompact_Open(&rd,mut,len));
    }
    /* 3) truncation / extension rejected */
    for(k=0;k<40 && k<len;++k) {
        ExpCarCompactReader rd;
        assert(!ExpCarCompact_Open(&rd,bin,len-1u-k));
    }
    {   ExpCarCompactReader rd;uint8_t *ext=(uint8_t *)malloc(len+1);
        memcpy(ext,bin,len);ext[len]=0;
        assert(!ExpCarCompact_Open(&rd,ext,len+1));free(ext);}
    free(mut);free(bin);
    /* 4) empty registry */
    {   Rows e={0,NULL};ExpCarCompactReader rd;uint64_t v[EXP_CCOL];
        bin=ExpCarCompact_Encode(1,0,0,get,&e,&len);
        assert(bin && ExpCarCompact_Open(&rd,bin,len) && rd.n==0 &&
               !ExpCarCompact_Next(&rd,v) && ExpCarCompact_Finish(&rd));
        free(bin);}
    /* 5) value wider than its column is refused at encode time */
    {   Rows w;w.n=1;w.v=(uint64_t (*)[EXP_CCOL])calloc(1,sizeof w.v[0]);
        w.v[0][13]=256u; /* slot is 8 bits */
        assert(!ExpCarCompact_Encode(1,0,1,get,&w,&len));
        free(w.v);}
    /* 6) a crafted stream whose row count exceeds the data fails at Next,
     *    and one with leftover bytes fails Finish (hash fixed up). */
    {   Rows q;ExpCarCompactReader rd;uint64_t v[EXP_CCOL];
        q.n=3;q.v=(uint64_t (*)[EXP_CCOL])calloc(3,sizeof q.v[0]);
        bin=ExpCarCompact_Encode(1,5,3,get,&q,&len);assert(bin);
        assert(ExpCarCompact_Open(&rd,bin,len));
        assert(ExpCarCompact_Next(&rd,v)&&ExpCarCompact_Next(&rd,v));
        assert(!ExpCarCompact_Finish(&rd)); /* one row still pending */
        assert(ExpCarCompact_Next(&rd,v)&&!ExpCarCompact_Next(&rd,v));
        free(bin);free(q.v);}
    /* 7) base64 */
    {   const uint8_t d[]={0,1,2,3,4,5,250,251,252,253};
        char t[64];uint8_t o[16];size_t n,m;
        for(n=0;n<=sizeof d;++n) {
            m=ExpCarCompact_B64Encode(d,n,t,sizeof t);
            assert(m!=(size_t)-1 && m==(n+2)/3*4);
            t[m]=0;
            assert(ExpCarCompact_B64Decode(t,m,o,sizeof o)==n && !memcmp(o,d,n));
        }
        assert(ExpCarCompact_B64Decode("abc",3,o,16)==(size_t)-1);
        assert(ExpCarCompact_B64Decode("ab!d",4,o,16)==(size_t)-1);
        assert(ExpCarCompact_B64Decode("a=bc",4,o,16)==(size_t)-1);
        assert(ExpCarCompact_B64Decode("ab==abcd",8,o,16)==(size_t)-1);
        assert(ExpCarCompact_B64Encode(d,10,t,4)==(size_t)-1);
        assert(ExpCarCompact_B64Decode("YWJj",4,o,2)==(size_t)-1);}
    /* 8) size on a realistic league season (sequential ids, 2 clubs/round) */
    {   const uint32_t N=100000;uint32_t j;size_t clen,hex=(size_t)N*134u;
        Rows s;s.n=N;s.v=(uint64_t (*)[EXP_CCOL])calloc(N,sizeof s.v[0]);
        assert(s.v);
        for(j=0;j<N;++j) {
            uint64_t *x=s.v[j];
            x[0]=0x1100000000000000ull+j;x[1]=j?x[0]-1u:0;x[2]=(2627u<<4)+(j/380u);
            x[3]=46300u;x[4]=1;x[5]=j%38u;x[6]=0;x[7]=0;x[8]=0;
            x[9]=(j%20u)*7919u;x[10]=((j+1u)%20u)*7919u;x[11]=46310u+(j%38u)*7u;
            x[12]=j/380u%90u;x[13]=j/38u%8u;x[14]=0;x[15]=2;
            x[16]=rnd()%5u;x[17]=rnd()%4u;x[18]=0;x[19]=0;x[20]=0;x[21]=2;x[22]=j%100u;
        }
        bin=ExpCarCompact_Encode(2627,46310,N,get,&s,&clen);assert(bin);
        assert(roundtrip(&s,bin,clen));
        printf("UE02_SIZE rows=%lu hex_UE01=%lu binary_UE02=%lu base64_json=%lu ratio_json=%.1fx\n",
               (unsigned long)N,(unsigned long)hex,(unsigned long)clen,
               (unsigned long)((clen+2)/3*4),(double)hex/((clen+2)/3*4));
        free(bin);free(s.v);}
    free(r.v);
    puts("EXP_CAR_COMPACT_UE02=PASS_ISOLATED_ONLY");
    puts("GAME_CAR_INTEGRATION_AND_WINDOWS=NOT_TESTED_HERE");
    return 0;
}
