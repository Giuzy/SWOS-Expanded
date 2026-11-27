#ifndef EXPANDED_CE_RANKING_SNAPSHOT_H
#define EXPANDED_CE_RANKING_SNAPSHOT_H
/* Read-only initial UEFA club ranking projection, not a sporting authority.
 * No fixture/club registration, eligibility, CAR or coefficient updates.
 * Caller owns display state; use the existing CE result writer for future
 * live rankings. This interface may be replaced by the CE live reader. */
#include "world.h"
#include "mjson.h"
#include "swos_io.h"
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ExpCeRankViewStatus {
    EXP_CE_RANK_DB_CANDIDATE=1,
    EXP_CE_RANK_ONLY_ACTIVE=2,
    EXP_CE_RANK_HISTORICAL=3
} ExpCeRankViewStatus;
typedef struct ExpCeRankViewRow {
    uint32_t source_row, own_milli, seed_milli, association_floor_milli;
    ExpCeRankViewStatus status;
    char association[4], name[128]; /* source UTF-8, renderer converts to OG font */
} ExpCeRankViewRow;
typedef struct ExpCeRankSnapshot {
    ExpCeRankViewRow *rows;
    uint32_t n;
} ExpCeRankSnapshot;

static inline void ExpCeRankSnapshot_Free(ExpCeRankSnapshot *p)
{
    if(p){free(p->rows);p->rows=NULL;p->n=0;}
}
/* JSON integer only, never interpret missing/null as zero. */
static inline int ExpCeRankSnapshot_U32(const char *j,int n,const char *key,uint32_t *out)
{
    const char *p=NULL;
    int l=0,i;
    uint32_t v=0;
    if(mjson_find(j,n,key,&p,&l)!=MJSON_TOK_NUMBER || l<=0)return 0;
    for(i=0;i<l;++i){
        unsigned d;
        if(p[i]<'0'||p[i]>'9')return 0;
        d=(unsigned)(p[i]-'0');
        if(v>(UINT32_MAX-d)/10u)return 0;
        v=v*10u+d;
    }
    *out=v;return 1;
}
static inline int ExpCeRankSnapshot_String(const char *j,int n,const char *key,
    char *out,int cap)
{
    int l=mjson_get_string(j,n,key,out,cap);
    return l>0 && l<cap && out[l]==0 && strlen(out)==(size_t)l;
}
/* out must be zero-initialized; on failure no partial rows escape. */
static inline int ExpCeRankSnapshot_Parse(WorldId world,const char *j,int len,
    ExpCeRankSnapshot *out)
{
    ExpCeRankSnapshot p={0};
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt,end,k;
    uint32_t schema,n;
    char str[32]={0};
    if(world!=WORLD_2627 || !out || out->rows || out->n || !j || len<=0)return 0;
    end=mjson(j,len,NULL,NULL);
    if(end<=0)return 0;
    for(k=end;k<len;++k)
        if(j[k]!=' '&&j[k]!='\n'&&j[k]!='\r'&&j[k]!='\t')return 0;
    if(!ExpCeRankSnapshot_U32(j,len,"$.schema",&schema)||schema!=1u||
       !ExpCeRankSnapshot_String(j,len,"$.world",str,sizeof str)||strcmp(str,"2627")||
       !ExpCeRankSnapshot_String(j,len,"$.asOf",str,sizeof str)||strcmp(str,"2026-07-01")||
       !ExpCeRankSnapshot_String(j,len,"$.mode",str,sizeof str)||strcmp(str,"STATIC_DISPLAY_ONLY")||
       !ExpCeRankSnapshot_U32(j,len,"$.expectedRows",&n)||!n||n>(uint32_t)len||
       (size_t)n>SIZE_MAX/sizeof *p.rows||
       mjson_find(j,len,"$.records",&arr,&alen)!=MJSON_TOK_ARRAY)return 0;
    p.rows=(ExpCeRankViewRow *)calloc((size_t)n,sizeof *p.rows);
    if(!p.rows)return 0;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0){
        ExpCeRankViewRow *r;
        uint32_t status;
        const char *row=arr+vo;
        if(vt!=MJSON_TOK_OBJECT||p.n>=n)goto bad;
        r=&p.rows[p.n];
        if(!ExpCeRankSnapshot_U32(row,vl,"$.sourceRow",&r->source_row)||r->source_row!=p.n+1u||
           !ExpCeRankSnapshot_U32(row,vl,"$.totalClubMilli",&r->own_milli)||
           !ExpCeRankSnapshot_U32(row,vl,"$.associationFloorMilli",&r->association_floor_milli)||
           !ExpCeRankSnapshot_U32(row,vl,"$.seedingMilli",&r->seed_milli)||
           r->seed_milli<r->own_milli||
           (p.n && r->seed_milli>p.rows[p.n-1u].seed_milli)||
           !ExpCeRankSnapshot_U32(row,vl,"$.viewStatus",&status)||status<1u||status>3u||
           !ExpCeRankSnapshot_String(row,vl,"$.association",r->association,sizeof r->association)||
           strlen(r->association)!=3u||
           !ExpCeRankSnapshot_String(row,vl,"$.displayName",r->name,sizeof r->name))goto bad;
        for(k=0;k<3;++k)if(r->association[k]<'A'||r->association[k]>'Z')goto bad;
        r->status=(ExpCeRankViewStatus)status;
        ++p.n;
    }
    if(off<0||p.n!=n)goto bad;
    *out=p;return 1;
bad:
    ExpCeRankSnapshot_Free(&p);return 0;
}
static inline int ExpCeRankSnapshot_Load(WorldId world,ExpCeRankSnapshot *out)
{
    char path[1024],*buf=NULL;
    long len;
    int ok=0;
    FILE *f;
    if(world!=WORLD_2627 || !out || out->rows || out->n)return 0;
    f=fopen(swos_data_path("data_json/worlds/2627/rankings/uefa_clubs.json",
        path,(uint32_t)sizeof path),"rb");
    if(!f)return 0;
    if(fseek(f,0,SEEK_END)==0&&(len=ftell(f))>0&&len<=INT_MAX&&
       fseek(f,0,SEEK_SET)==0&&(buf=(char *)malloc((size_t)len+1u))!=NULL&&
       fread(buf,1,(size_t)len,f)==(size_t)len){
        buf[len]=0;ok=ExpCeRankSnapshot_Parse(world,buf,(int)len,out);
    }
    free(buf);fclose(f);return ok;
}
#endif
