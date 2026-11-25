#ifndef EXPANDED_CE_START_HISTORY_H
#define EXPANDED_CE_START_HISTORY_H
#include "expanded_ce_shared_pool_data.h"
#include "expanded_ce_table.h"
#include "expanded_world_time.h"
typedef struct ExpCeHistoryGame {
    uint32_t round,home,away;
    int32_t day;
    uint16_t hg,ag;
    uint8_t played;
} ExpCeHistoryGame;
typedef struct ExpCeHistorySeed {
    uint32_t country,comp,div,year,n,ng,rounds,played,nd;
    int32_t start,cutoff;
    int32_t *days;
    ExpCeHistoryGame *games;
    ExpCeInitialDeduction *deductions;
} ExpCeHistorySeed;
typedef struct ExpCeHistoryData { ExpCeHistorySeed *rows;uint32_t n; } ExpCeHistoryData;
static inline void ExpCeHistory_Free(ExpCeHistoryData *p)
{
    uint32_t i;
    for(i=0;i<p->n;++i){free(p->rows[i].days);free(p->rows[i].games);free(p->rows[i].deductions);}
    free(p->rows);memset(p,0,sizeof *p);
}
static inline int ExpCeHistory_Date(const char *j,int n,const char *path,int32_t *out)
{
    char b[16];unsigned y,m,d;
    return mjson_get_string(j,n,path,b,sizeof b)>0 && strlen(b)==10u &&
        sscanf(b,"%4u-%2u-%2u",&y,&m,&d)==3 && y<=65535u && m<=12u && d<=31u &&
        ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d},out);
}
static inline int ExpCeHistory_Load(ExpCeHistoryData *out)
{
    char path[1024],*buf=NULL,world[16];FILE *f;long len=0;
    int alen=0,off=0,ko,kl,vo,vl,vt,ok=0;
    const char *arr=NULL;uint32_t v;int32_t cutoff;
    memset(out,0,sizeof *out);
    f=fopen(swos_data_path("data_json/worlds/2627/competitions/league_start_history.json",
        path,(uint32_t)sizeof path),"rb");
    if(!f)return 1;
    if(fseek(f,0,SEEK_END)!=0 || (len=ftell(f))<=0 || len>INT_MAX ||
       fseek(f,0,SEEK_SET)!=0)goto done;
    buf=(char *)malloc((size_t)len+1u);
    if(!buf || fread(buf,1,(size_t)len,f)!=(size_t)len)goto done;
    buf[len]=0;
    if(!ExpCeSharedPool_Number(buf,(int)len,"$.schema",1,&v)||v!=1 ||
       mjson_get_string(buf,(int)len,"$.world",world,sizeof world)<=0||strcmp(world,"2627")||
       !ExpCeHistory_Date(buf,(int)len,"$.cutoff",&cutoff) ||
       mjson_find(buf,(int)len,"$.leagues",&arr,&alen)!=MJSON_TOK_ARRAY)goto done;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *j=arr+vo,*ga=NULL;int gl=0,pass;
        uint32_t i;ExpCeHistorySeed *s,*p;
        if(vt!=MJSON_TOK_OBJECT)goto done;
        p=(ExpCeHistorySeed *)realloc(out->rows,(size_t)(out->n+1u)*sizeof *p);
        if(!p)goto done;
        out->rows=p;s=&p[out->n++];memset(s,0,sizeof *s);s->cutoff=cutoff;
        if(!ExpCeSharedPool_Number(j,vl,"$.country",254,&s->country) ||
           !ExpCeSharedPool_Number(j,vl,"$.comp_id",255,&s->comp) ||
           !ExpCeSharedPool_Number(j,vl,"$.division",15,&s->div) ||
           !ExpCeSharedPool_Number(j,vl,"$.year",65535,&s->year) ||
           !ExpCeSharedPool_Number(j,vl,"$.clubs",128,&s->n) ||
           s->n<2 || (s->n&1u) || !ExpCeHistory_Date(j,vl,"$.edition_start",&s->start)||
           s->start>=cutoff)goto done;
        for(i=0;i+1u<out->n;++i)if(p[i].country==s->country&&p[i].comp==s->comp&&
           p[i].div==s->div&&p[i].year==s->year)goto done;
        s->rounds=2u*(s->n-1u);
        s->days=(int32_t *)calloc(s->rounds,sizeof *s->days);
        s->games=(ExpCeHistoryGame *)calloc(s->n*(s->n-1u),sizeof *s->games);
        s->deductions=(ExpCeInitialDeduction *)calloc(s->n,sizeof *s->deductions);
        if(!s->days||!s->games||!s->deductions)goto done;
        for(i=0;i<s->rounds;++i){
            char q[64];snprintf(q,sizeof q,"$.round_dates[%u]",(unsigned)i);
            if(!ExpCeHistory_Date(j,vl,q,&s->days[i]) ||
               (i&&s->days[i]<=s->days[i-1u])||s->days[i]<s->start)goto done;
        }
        for(pass=0;pass<2;++pass){
            int g=0,k1,k2,o,l,t;
            if(mjson_find(j,vl,pass?"$.missing_first":"$.played",&ga,&gl)!=MJSON_TOK_ARRAY)goto done;
            while((g=mjson_next(ga,gl,g,&k1,&k2,&o,&l,&t))>0){
                ExpCeHistoryGame x={0};uint32_t hg=0,ag=0;const char *q=ga+o;
                if(t!=MJSON_TOK_OBJECT || s->ng>=s->n*(s->n-1u) ||
                   !ExpCeSharedPool_Number(q,l,"$.round",s->rounds,&x.round)||!x.round ||
                   !ExpCeSharedPool_Number(q,l,"$.home",UINT32_MAX,&x.home)||!x.home ||
                   !ExpCeSharedPool_Number(q,l,"$.away",UINT32_MAX,&x.away)||!x.away ||
                   x.home==x.away||(x.home>>16)!=s->country||(x.away>>16)!=s->country)goto done;
                if(!pass){
                    if(!ExpCeHistory_Date(q,l,"$.date",&x.day) || x.day>=cutoff ||x.day<s->start||
                       !ExpCeSharedPool_Number(q,l,"$.hg",31,&hg) ||
                       !ExpCeSharedPool_Number(q,l,"$.ag",31,&ag))goto done;
                    x.hg=(uint16_t)hg;x.ag=(uint16_t)ag;x.played=1u;++s->played;
                }else if(x.round>=s->n||s->days[x.round-1u]>=cutoff)goto done;
                for(i=0;i<s->ng;++i)if(s->games[i].home==x.home&&s->games[i].away==x.away)goto done;
                s->games[s->ng++]=x;
            }
            if(g<0)goto done;
        }
        if(!s->played||mjson_find(j,vl,"$.deductions",&ga,&gl)!=MJSON_TOK_ARRAY)goto done;
        {int g=0,k1,k2,o,l,t;
         while((g=mjson_next(ga,gl,g,&k1,&k2,&o,&l,&t))>0){
            ExpCeInitialDeduction d={0};
            if(t!=MJSON_TOK_OBJECT||s->nd>=s->n||
               !ExpCeSharedPool_Number(ga+o,l,"$.club",UINT32_MAX,&d.club)||
               !ExpCeSharedPool_Number(ga+o,l,"$.points",UINT32_MAX,&d.points)||
               !d.club||!d.points||(d.club>>16)!=s->country)goto done;
            for(i=0;i<s->nd;++i)if(s->deductions[i].club==d.club)goto done;
            s->deductions[s->nd++]=d;
         }if(g<0)goto done;}
    }
    ok=off==0;
done:
    free(buf);fclose(f);if(!ok)ExpCeHistory_Free(out);return ok;
}
static inline const ExpCeHistorySeed *ExpCeHistory_Find(const ExpCeHistoryData *p,
    uint8_t country,uint8_t comp,uint8_t div,uint16_t year)
{
    uint32_t i;
    for(i=0;i<p->n;++i)if(p->rows[i].country==country&&p->rows[i].comp==comp&&
        p->rows[i].div==div&&p->rows[i].year==year)return &p->rows[i];
    return NULL;
}
#endif
