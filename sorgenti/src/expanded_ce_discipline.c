/* Team disciplinary points per CE fixture. See header. */
#include "expanded_ce_discipline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DiscRow {uint64_t id;uint16_t h,a;} DiscRow;
static DiscRow *s_rows;
static uint32_t s_n,s_cap;

static uint32_t find_pos(uint64_t id,int *hit)
{
    uint32_t lo=0,hi=s_n;
    *hit=0;
    while(lo<hi){const uint32_t m=(lo+hi)/2u;
        if(s_rows[m].id==id){*hit=1;return m;}
        if(s_rows[m].id<id)lo=m+1u;else hi=m;}
    return lo;
}
int ExpCeDisc_Record(uint64_t id,uint32_t h,uint32_t a)
{
    int hit;
    uint32_t p;
    if(!id || h>0xffffu || a>0xffffu)return 0;
    p=find_pos(id,&hit);
    if(hit)return 0;                          /* never twice */
    if(s_n==s_cap){
        const uint32_t cap=s_cap?s_cap*2u:256u;
        DiscRow *r=(DiscRow *)realloc(s_rows,(size_t)cap*sizeof *r);
        if(!r)return 0;
        s_rows=r;s_cap=cap;
    }
    memmove(s_rows+p+1u,s_rows+p,(size_t)(s_n-p)*sizeof *s_rows);
    s_rows[p]=(DiscRow){id,(uint16_t)h,(uint16_t)a};
    ++s_n;
    return 1;
}
int ExpCeDisc_Get(uint64_t id,uint32_t *h,uint32_t *a)
{
    int hit;
    const uint32_t p=find_pos(id,&hit);
    if(!hit)return 0;
    if(h)*h=s_rows[p].h;
    if(a)*a=s_rows[p].a;
    return 1;
}
void ExpCeDisc_Undo(uint64_t id)
{
    int hit;
    const uint32_t p=find_pos(id,&hit);
    if(!hit)return;
    memmove(s_rows+p,s_rows+p+1u,(size_t)(s_n-p-1u)*sizeof *s_rows);
    --s_n;
}
void ExpCeDisc_Reset(void){free(s_rows);s_rows=NULL;s_n=s_cap=0u;}
uint32_t ExpCeDisc_Count(void){return s_n;}
char *ExpCeDisc_Encode(void)
{
    char *t;
    size_t cap,len=0;
    uint32_t i;
    if(!s_n)return NULL;
    cap=16u+(size_t)s_n*32u;
    t=(char *)malloc(cap);
    if(!t)return NULL;
    len=(size_t)snprintf(t,cap,"DS01;%lu",(unsigned long)s_n);
    for(i=0;i<s_n;++i)
        len+=(size_t)snprintf(t+len,cap-len,";%llx,%u,%u",(unsigned long long)s_rows[i].id,
                              (unsigned)s_rows[i].h,(unsigned)s_rows[i].a);
    return t;
}
static int decode(const char *t,DiscRow **out,uint32_t *n)
{
    char *e=NULL;
    unsigned long cnt;
    uint32_t i;
    DiscRow *rows;
    *out=NULL;*n=0;
    if(!t || strncmp(t,"DS01;",5))return 0;
    t+=5;
    cnt=strtoul(t,&e,10);
    if(e==t || cnt>0x1000000ul)return 0;
    t=e;
    rows=(DiscRow *)calloc(cnt?cnt:1u,sizeof *rows);
    if(!rows)return 0;
    for(i=0;i<cnt;++i) {
        unsigned long long id;
        unsigned long h,a;
        if(*t!=';'){free(rows);return 0;}
        ++t;
        id=strtoull(t,&e,16);if(e==t||*e!=','){free(rows);return 0;}t=e+1;
        h=strtoul(t,&e,10);if(e==t||*e!=','||h>0xffffu){free(rows);return 0;}t=e+1;
        a=strtoul(t,&e,10);if(e==t||a>0xffffu){free(rows);return 0;}t=e;
        if(!id || (i && id<=rows[i-1u].id)){free(rows);return 0;}
        rows[i]=(DiscRow){(uint64_t)id,(uint16_t)h,(uint16_t)a};
    }
    if(*t){free(rows);return 0;}
    *out=rows;*n=(uint32_t)cnt;
    return 1;
}
int ExpCeDisc_Validate(const char *text)
{
    DiscRow *r;uint32_t n;
    const int ok=decode(text,&r,&n);
    free(r);
    return ok;
}
int ExpCeDisc_Decode(const char *text)
{
    DiscRow *r;uint32_t n;
    if(!decode(text,&r,&n))return 0;
    ExpCeDisc_Reset();
    s_rows=r;s_n=s_cap=n;
    if(!n){free(r);s_rows=NULL;s_cap=0u;}
    return 1;
}
