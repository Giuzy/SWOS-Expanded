#include "expanded_car_compact.h"
#include <stdlib.h>
#include <string.h>

const uint8_t ExpCarCompact_Bits[EXP_CCOL]={
    64,64,32,32,16,32,32,16,16,32,32,32,16,8,8,8,16,16,16,16,8,8,16,8,8};
typedef struct Buf {uint8_t *p;size_t n,cap;} Buf;
static int buf_put(Buf *b,uint64_t v)
{
    if(b->n+10u>b->cap) {
        const size_t nc=b->cap?b->cap*2u:4096u;
        uint8_t *q=(uint8_t *)realloc(b->p,nc);
        if(!q || nc<b->cap)return 0;
        b->p=q;b->cap=nc;
    }
    while(v>=0x80u) {b->p[b->n++]=(uint8_t)(v|0x80u);v>>=7;}
    b->p[b->n++]=(uint8_t)v;
    return 1;
}
static uint64_t mask_of(unsigned bits){return bits>=64u?~0ull:((1ull<<bits)-1u);}
/* delta modulo 2^bits, zig-zag inside that width */
static uint64_t zz(uint64_t d,unsigned bits)
{
    const uint64_t m=mask_of(bits),x=d&m,sign=1ull<<(bits-1u);
    const int neg=(x&sign)!=0u;
    const uint64_t mag=neg?((~x+1u)&m):x;
    return neg?(((mag-1u)<<1)|1u):(mag<<1);
}
static uint64_t unzz(uint64_t z,unsigned bits)
{
    const uint64_t m=mask_of(bits);
    if(z&1u)return (~((z>>1)+1u)+1u)&m;
    return (z>>1)&m;
}
static uint64_t fnv64(const uint8_t *p,size_t n)
{
    uint64_t h=1469598103934665603ull;
    size_t i;
    for(i=0;i<n;++i) {h^=p[i];h*=1099511628211ull;}
    return h;
}
static void put32(uint8_t *p,uint32_t v)
{p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static uint32_t get32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static size_t head_of(size_t ncol){return 4u+2u+4u+4u+4u*ncol;}
enum { HEAD=4+2+4+4+4*EXP_CCOL };

uint8_t *ExpCarCompact_Encode(uint16_t world,int32_t day,uint32_t n,
    int (*row)(uint32_t,uint64_t *,void *),void *user,size_t *len)
{
    Buf col[EXP_CCOL];
    uint64_t prev[EXP_CCOL]={0},v[EXP_CCOL];
    uint8_t *out=NULL;
    size_t total=HEAD+8u,at,c,k;
    uint32_t i;
    memset(col,0,sizeof col);
    if(!row || !len)return NULL;
    for(i=0;i<n;++i) {
        if(!row(i,v,user))goto fail;
        for(c=0;c<EXP_CCOL;++c) {
            const uint64_t x=v[c]&mask_of(ExpCarCompact_Bits[c]);
            if(x!=v[c])goto fail; /* value wider than its column */
            if(!buf_put(&col[c],zz(x-prev[c],ExpCarCompact_Bits[c])))goto fail;
            prev[c]=x;
        }
    }
    for(c=0;c<EXP_CCOL;++c) {
        if(col[c].n>UINT32_MAX || total+col[c].n<total)goto fail;
        total+=col[c].n;
    }
    out=(uint8_t *)malloc(total);
    if(!out)goto fail;
    memcpy(out,"UE03",4u);
    out[4]=(uint8_t)world;out[5]=(uint8_t)(world>>8);
    put32(out+6,(uint32_t)day);put32(out+10,n);
    for(c=0;c<EXP_CCOL;++c)put32(out+14+4*c,(uint32_t)col[c].n);
    at=HEAD;
    for(c=0;c<EXP_CCOL;++c) {
        if(col[c].n)memcpy(out+at,col[c].p,col[c].n);
        at+=col[c].n;
    }
    {
        const uint64_t h=fnv64(out,at);
        for(k=0;k<8;++k)out[at+k]=(uint8_t)(h>>(8*k));
    }
    at+=8u;
    for(c=0;c<EXP_CCOL;++c)free(col[c].p);
    *len=at;
    return out;
fail:
    for(c=0;c<EXP_CCOL;++c)free(col[c].p);
    free(out);
    return NULL;
}
int ExpCarCompact_Open(ExpCarCompactReader *r,const uint8_t *b,size_t len)
{
    size_t at,c,k,ncol,head;
    uint64_t h=0u;
    if(!r || !b)return 0;
    memset(r,0,sizeof *r);
    if(len<4u)return 0;
    if(!memcmp(b,"UE03",4u))ncol=EXP_CCOL;
    else if(!memcmp(b,"UE02",4u))ncol=EXP_CCOL_UE02;
    else return 0;
    head=head_of(ncol);
    if(len<head+8u)return 0;
    for(k=0;k<8;++k)h|=(uint64_t)b[len-8u+k]<<(8*k);
    if(h!=fnv64(b,len-8u))return 0;
    r->ncol=(uint32_t)ncol;
    r->world=(uint16_t)(b[4]|(b[5]<<8));
    r->day=(int32_t)get32(b+6);
    r->n=get32(b+10);
    at=head;
    for(c=0;c<ncol;++c) {
        const uint32_t l=get32(b+14+4*c);
        if(l>len-8u-at)return 0;
        r->col[c]=b+at;r->end[c]=b+at+l;at+=l;
    }
    return at==len-8u;
}
int ExpCarCompact_Next(ExpCarCompactReader *r,uint64_t *vals)
{
    size_t c;
    if(!r || !vals || r->next>=r->n)return 0;
    for(c=r->ncol;c<EXP_CCOL;++c)vals[c]=0u; /* UE02: kinds = CLUB */
    for(c=0;c<r->ncol;++c) {
        const unsigned bits=ExpCarCompact_Bits[c];
        uint64_t z=0u;
        unsigned sh=0u;
        for(;;) {
            uint8_t byte;
            if(r->col[c]>=r->end[c] || sh>63u)return 0;
            byte=*r->col[c]++;
            if(sh==63u && (byte&0x7eu))return 0;
            z|=(uint64_t)(byte&0x7fu)<<sh;
            sh+=7u;
            if(!(byte&0x80u))break;
        }
        if(bits<64u && (z>>bits))return 0;
        vals[c]=(r->prev[c]+unzz(z,bits))&mask_of(bits);
        r->prev[c]=vals[c];
    }
    r->next++;
    return 1;
}
int ExpCarCompact_Finish(const ExpCarCompactReader *r)
{
    size_t c;
    if(!r || r->next!=r->n)return 0;
    for(c=0;c<r->ncol;++c)if(r->col[c]!=r->end[c])return 0;
    return 1;
}
size_t ExpCarCompact_HexEncode(const uint8_t *in,size_t n,char *out,size_t cap)
{
    static const char H[]="0123456789abcdef";
    size_t i;
    if(n>((size_t)-1)/2u)return (size_t)-1;
    if(!out)return n*2u;
    if(cap<n*2u)return (size_t)-1;
    for(i=0;i<n;++i) {
        out[2u*i]=H[in[i]>>4];
        out[2u*i+1u]=H[in[i]&15u];
    }
    return n*2u;
}
size_t ExpCarCompact_HexDecode(const char *in,size_t n,uint8_t *out,size_t cap)
{
    size_t i;
    if(n&1u)return (size_t)-1;
    if(!out)return n/2u;
    if(cap<n/2u)return (size_t)-1;
    for(i=0;i<n;i+=2) {
        int v[2],k;
        for(k=0;k<2;++k) {
            const char ch=in[i+(size_t)k];
            if(ch>='0' && ch<='9')v[k]=ch-'0';
            else if(ch>='a' && ch<='f')v[k]=ch-'a'+10;
            else return (size_t)-1;
        }
        out[i/2u]=(uint8_t)((v[0]<<4)|v[1]);
    }
    return n/2u;
}
