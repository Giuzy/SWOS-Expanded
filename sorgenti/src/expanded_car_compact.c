#include "expanded_car_compact.h"
#include <stdlib.h>
#include <string.h>

const uint8_t ExpCarCompact_Bits[EXP_CCOL]={
    64,64,32,32,16,32,32,16,16,32,32,32,16,8,8,8,16,16,16,16,8,8,16};
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
    memcpy(out,"UE02",4u);
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
    size_t at,c,k;
    uint64_t h=0u;
    if(!r || !b)return 0;
    memset(r,0,sizeof *r);
    if(len<HEAD+8u || memcmp(b,"UE02",4u))return 0;
    for(k=0;k<8;++k)h|=(uint64_t)b[len-8u+k]<<(8*k);
    if(h!=fnv64(b,len-8u))return 0;
    r->world=(uint16_t)(b[4]|(b[5]<<8));
    r->day=(int32_t)get32(b+6);
    r->n=get32(b+10);
    at=HEAD;
    for(c=0;c<EXP_CCOL;++c) {
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
    for(c=0;c<EXP_CCOL;++c) {
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
    for(c=0;c<EXP_CCOL;++c)if(r->col[c]!=r->end[c])return 0;
    return 1;
}
static const char B64[]=
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
size_t ExpCarCompact_B64Encode(const uint8_t *in,size_t n,char *out,size_t cap)
{
    size_t i,o=0u,need;
    if(n>((size_t)-1)/2u)return (size_t)-1;
    need=(n+2u)/3u*4u;
    if(!out)return need;
    if(cap<need)return (size_t)-1;
    for(i=0;i+2<n;i+=3) {
        const uint32_t x=((uint32_t)in[i]<<16)|((uint32_t)in[i+1]<<8)|in[i+2];
        out[o++]=B64[x>>18];out[o++]=B64[(x>>12)&63];
        out[o++]=B64[(x>>6)&63];out[o++]=B64[x&63];
    }
    if(n-i==1) {
        const uint32_t x=(uint32_t)in[i]<<16;
        out[o++]=B64[x>>18];out[o++]=B64[(x>>12)&63];
        out[o++]='=';out[o++]='=';
    } else if(n-i==2) {
        const uint32_t x=((uint32_t)in[i]<<16)|((uint32_t)in[i+1]<<8);
        out[o++]=B64[x>>18];out[o++]=B64[(x>>12)&63];
        out[o++]=B64[(x>>6)&63];out[o++]='=';
    }
    return o;
}
static int b64v(char c)
{
    if(c>='A' && c<='Z')return c-'A';
    if(c>='a' && c<='z')return c-'a'+26;
    if(c>='0' && c<='9')return c-'0'+52;
    if(c=='+')return 62;
    if(c=='/')return 63;
    return -1;
}
size_t ExpCarCompact_B64Decode(const char *in,size_t n,uint8_t *out,size_t cap)
{
    size_t i,o=0u,pad=0u;
    if(n%4u)return (size_t)-1;
    if(n && in[n-1]=='=')pad=(n>=2u && in[n-2]=='=')?2u:1u;
    if(!out)return n/4u*3u-pad;
    if(cap<n/4u*3u-pad)return (size_t)-1;
    for(i=0;i<n;i+=4) {
        const int last=(i+4==n);
        const int a=b64v(in[i]),b=b64v(in[i+1]);
        const int c=in[i+2]=='='?-2:b64v(in[i+2]),
                  d=in[i+3]=='='?-2:b64v(in[i+3]);
        uint32_t x;
        if(a<0 || b<0 || c==-1 || d==-1)return (size_t)-1;
        if((c==-2 && d!=-2) || ((c==-2 || d==-2) && !last))return (size_t)-1;
        x=((uint32_t)a<<18)|((uint32_t)b<<12)|((uint32_t)(c<0?0:c)<<6)|
          (uint32_t)(d<0?0:d);
        out[o++]=(uint8_t)(x>>16);
        if(c!=-2)out[o++]=(uint8_t)(x>>8);
        if(d!=-2)out[o++]=(uint8_t)x;
    }
    return o;
}
