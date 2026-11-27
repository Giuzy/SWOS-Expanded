/* International windows table (v00.13.354). See header. Pure data reader. */
#include "expanded_international_windows.h"
#include "expanded_world_time.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {ExpIntlWindow w[EXP_INTL_MAX];uint32_t n;} s_intl;

int ExpIntl_ParseMask(const char *t,uint8_t *mask)
{
    static const struct {const char *n;uint8_t b;} k[]={
        {"FIFA",EXP_INTL_FIFA},{"AFC",EXP_INTL_AFC},{"CAF",EXP_INTL_CAF},
        {"CONCACAF",EXP_INTL_CONCACAF},{"CONMEBOL",EXP_INTL_CONMEBOL},
        {"OFC",EXP_INTL_OFC},{"UEFA",EXP_INTL_UEFA}};
    uint8_t m=0u;
    if(!t || !mask)return 0;
    while(*t) {
        size_t len=strcspn(t,","),i;
        int f=0;
        for(i=0;i<sizeof k/sizeof k[0];++i)
            if(strlen(k[i].n)==len && !strncmp(t,k[i].n,len)){m=(uint8_t)(m|k[i].b);f=1;}
        if(!f)return 0;
        t+=len;
        if(*t==',')++t;
    }
    *mask=m;
    return 1;
}
static int32_t s_generated_from;
void ExpIntl_Reset(void){memset(&s_intl,0,sizeof s_intl);s_generated_from=0;}
uint32_t ExpIntl_Count(void){return s_intl.n;}
static int parse_date(const char *t,int32_t *out)
{
    unsigned y,m,d;
    ExpandedWorldDate x;
    if(!t || strlen(t)!=10u || sscanf(t,"%4u-%2u-%2u",&y,&m,&d)!=3)return 0;
    x=(ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d};
    return ExpandedWorld_DateToIndex(x,out);
}
int ExpIntl_LoadJson(const char *j,int len)
{
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt;
    double schema=0;
    char a[16],b[16],t[16];
    ExpIntl_Reset();
    if(!j || mjson_get_number(j,len,"$.schema",&schema)<=0 || schema!=1 ||
       mjson_find(j,len,"$.windows",&arr,&alen)!=MJSON_TOK_ARRAY)return 0;
    if(mjson_get_string(j,len,"$.generated_from",a,sizeof a)>0 &&
       !parse_date(a,&s_generated_from))return 0;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        ExpIntlWindow *w;
        uint8_t m=0u;
        if(vt!=MJSON_TOK_OBJECT || s_intl.n>=EXP_INTL_MAX)goto bad;
        w=&s_intl.w[s_intl.n];
        if(mjson_get_string(arr+vo,vl,"$.from",a,sizeof a)<=0 ||
           mjson_get_string(arr+vo,vl,"$.to",b,sizeof b)<=0 ||
           mjson_get_string(arr+vo,vl,"$.type",t,sizeof t)<=0 ||
           !parse_date(a,&w->first_day) || !parse_date(b,&w->last_day) ||
           w->last_day<w->first_day || !ExpIntl_ParseMask(t,&m) ||
           !m || (m&(m-1u)))goto bad;
        if(s_intl.n && w->first_day<s_intl.w[s_intl.n-1u].first_day)goto bad; /* chronological */
        w->type=m;
        ++s_intl.n;
    }
    if(off<0)goto bad;
    return 1;
bad:
    ExpIntl_Reset();
    return 0;
}
#ifndef EXP_INTL_NO_IO
#include "swos_io.h"
int ExpIntl_Load(void)
{
    char path[1024],*buf=NULL;
    long len;
    int ok=1;
    FILE *f=fopen(swos_data_path("data_json/worlds/2627/competitions/international_windows.json",
                                 path,(uint32_t)sizeof path),"rb");
    ExpIntl_Reset();
    if(!f)return 1;                /* no table: no window (reduced data) */
    if(fseek(f,0,SEEK_END)==0 && (len=ftell(f))>0 && len<1024L*1024L &&
       fseek(f,0,SEEK_SET)==0 && (buf=(char *)malloc((size_t)len+1u))!=NULL &&
       fread(buf,1,(size_t)len,f)==(size_t)len) {
        buf[len]=0;ok=ExpIntl_LoadJson(buf,(int)len);
    } else ok=0;
    free(buf);fclose(f);
    fprintf(stderr,"[ce-intl] WINDOWS %s count=%lu\n",ok?"LOADED":"MALFORMED_NONE_APPLIED",
            (unsigned long)s_intl.n);
    return ok;
}
#endif
int32_t ExpIntl_LastCoveredDay(void){return s_intl.n?s_intl.w[s_intl.n-1u].last_day:0;}
/* From `generated_from` (JSON, 1 July 2027) the calendar generates every
 * international window itself (user 27/11/2026: official dates only for
 * 2026/27, then generated; every two years a World Cup OR a EURO).
 * GAME_TEMPLATE pattern taken from the FIFA 2023-2030 calendar, "more or
 * less" (user): window starts on the Monday nearest the reference day.
 *  every year: FIFA March 9 days (~20/03), FIFA autumn 16 days (~21/09),
 *              FIFA November 9 days (~12/11);
 *  June FIFA 9 days: ~30/05 in World Cup and EURO years (preparation),
 *              ~05/06 otherwise;
 *  World Cup  (2030, 2034, ...): FIFA 11/06-19/07 (every league that stops
 *              for FIFA windows stops);
 *  EURO       (2028, 2032, ...): UEFA 09/06-09/07; same years Copa
 *              America CONMEBOL 12/06-12/07, OFC Nations Cup 15/06-30/06,
 *              Africa Cup of Nations CAF 19/06-17/07;
 *  Gold Cup   (odd years): CONCACAF 19/06-11/07;
 *  Asian Cup  (2031, 2035, ...): AFC 07/01-05/02.
 * Official windows of the JSON keep priority (same type overlapping = the
 * generated one is dropped). */
static int32_t nearest_monday(int32_t day)
{
    const int wd=ExpandedWorld_Weekday(day);     /* Monday 0 */
    if(wd<0)return day;
    return wd<=3?day-wd:day+(7-wd);
}
/* s_generated_from: first generated day, 0 = no generation (no table) */
static int day_of(uint16_t y,uint8_t m,uint8_t d,int32_t *out)
{
    ExpandedWorldDate x={y,m,d};
    return ExpandedWorld_DateToIndex(x,out);
}
static uint32_t gen_year(uint16_t y,ExpIntlWindow *w)
{
    static const struct {uint8_t m,d,len,type,monday;} every[]={
        {3,20,8,EXP_INTL_FIFA,1},{9,21,15,EXP_INTL_FIFA,1},{11,12,8,EXP_INTL_FIFA,1}};
    const int wc=y>2026u && (y-2026u)%4u==0u;
    const int euro=y>=2028u && (y-2028u)%4u==0u;
    uint32_t k=0u,i;
    int32_t a;
    for(i=0u;i<3u;++i)
        if(day_of(y,every[i].m,every[i].d,&a)) {
            a=nearest_monday(a);w[k++]=(ExpIntlWindow){a,a+every[i].len,every[i].type};
        }
    if(day_of(y,(wc||euro)?5u:6u,(wc||euro)?30u:5u,&a)) {
        a=nearest_monday(a);w[k++]=(ExpIntlWindow){a,a+8,EXP_INTL_FIFA};
    }
#define T(m1,d1,m2,d2,ty) do{int32_t s1,s2;if(day_of(y,m1,d1,&s1)&&day_of(y,m2,d2,&s2))w[k++]=(ExpIntlWindow){s1,s2,ty};}while(0)
    if(wc)T(6,11,7,19,EXP_INTL_FIFA);
    if(euro){T(6,9,7,9,EXP_INTL_UEFA);T(6,12,7,12,EXP_INTL_CONMEBOL);
             T(6,15,6,30,EXP_INTL_OFC);T(6,19,7,17,EXP_INTL_CAF);}
    if(y&1u)T(6,19,7,11,EXP_INTL_CONCACAF);
    if(y>=2031u && (y-2031u)%4u==0u)T(1,7,2,5,EXP_INTL_AFC);
#undef T
    return k;
}
static int official_overlaps(const ExpIntlWindow *g)
{
    uint32_t i;
    for(i=0u;i<s_intl.n;++i)
        if(s_intl.w[i].type==g->type && s_intl.w[i].first_day<=g->last_day &&
           s_intl.w[i].last_day>=g->first_day)return 1;
    return 0;
}
static uint32_t generated(uint8_t mask,int32_t first,int32_t last,ExpIntlWindow *out,uint32_t cap)
{
    ExpandedWorldDate fd,ld;
    uint32_t k=0u;
    uint16_t y;
    if(!s_generated_from || last<s_generated_from)return 0u;
    if(first<s_generated_from)first=s_generated_from;
    if(!ExpandedWorld_IndexToDate(first,&fd) || !ExpandedWorld_IndexToDate(last,&ld))return 0u;
    for(y=(uint16_t)(fd.year>0u?fd.year-1u:0u);y<=(uint16_t)(ld.year+1u) && k<cap;++y) {
        ExpIntlWindow w[16];
        uint32_t n=gen_year(y,w),i;
        for(i=0u;i<n && k<cap;++i) {
            if(!(w[i].type&mask) || w[i].first_day<s_generated_from ||
               w[i].last_day<first || w[i].first_day>last || official_overlaps(&w[i]))continue;
            out[k++]=w[i];
        }
    }
    return k;
}
int32_t ExpIntl_GeneratedFrom(void){return s_generated_from;}
uint32_t ExpIntl_Overlapping(uint8_t mask,int32_t first,int32_t last,
                             ExpIntlWindow *out,uint32_t cap)
{
    uint32_t i,k=0u;
    for(i=0u;i<s_intl.n && k<cap;++i) {
        const ExpIntlWindow *w=&s_intl.w[i];
        if(!(w->type&mask) || w->last_day<first || w->first_day>last)continue;
        out[k++]=*w;
    }
    if(k<cap)k+=generated(mask,first,last,out+k,cap-k);
    {   /* chronological order for the planner and readers */
        uint32_t x,z;
        for(x=1u;x<k;++x){ExpIntlWindow t=out[x];z=x;
            while(z && out[z-1u].first_day>t.first_day){out[z]=out[z-1u];--z;}out[z]=t;}
    }
    return k;
}

static void add_windows(ExpandedDatePlanPolicy *p,uint8_t mask,int32_t a,int32_t b)
{
    ExpIntlWindow w[EXP_DATE_PLAN_MAX_BREAKS];
    const uint32_t k=ExpIntl_Overlapping(mask,a,b,w,EXP_DATE_PLAN_MAX_BREAKS);
    uint32_t i,q;
    for(i=0u;i<k && p->break_count<EXP_DATE_PLAN_MAX_BREAKS;++i) {
        int dup=0;
        for(q=0u;q<p->break_count;++q)
            if(p->breaks[q].first_day==w[i].first_day && p->breaks[q].last_day==w[i].last_day)dup=1;
        if(dup)continue;
        p->breaks[p->break_count].first_day=w[i].first_day;
        p->breaks[p->break_count].last_day=w[i].last_day;
        ++p->break_count;
    }
}
static int32_t stopped_days(uint8_t mask,int32_t a,int32_t b)
{
    ExpIntlWindow w[EXP_DATE_PLAN_MAX_BREAKS];
    const uint32_t k=ExpIntl_Overlapping(mask,a,b,w,EXP_DATE_PLAN_MAX_BREAKS);
    uint32_t i;
    int32_t n=0;
    for(i=0u;i<k;++i) {
        const int32_t x=w[i].first_day>a?w[i].first_day:a,y=w[i].last_day<b?w[i].last_day:b;
        if(y>=x)n+=y-x+1;
    }
    return n;
}
int ExpIntl_PlanWithWindows(ExpandedDatePlanPolicy *pol,uint8_t mask,
                            uint32_t rounds,int32_t *days,int32_t *extended)
{
    const int32_t last0=pol->last_day;
    int reason=0,step;
    if(extended)*extended=0;
    if(!mask)return ExpandedDatePlanner_Plan(pol,rounds,days,&reason);
    add_windows(pol,mask,pol->first_day,pol->last_day);
    if(ExpandedDatePlanner_Plan(pol,rounds,days,&reason))return 1;
    for(step=0;step<4;++step) {
        const int32_t need=stopped_days(mask,pol->first_day,pol->last_day);
        const int32_t target=last0+need;
        if(target<=pol->last_day || target-last0>120)return 0;
        pol->last_day=target;
        add_windows(pol,mask,pol->first_day,pol->last_day);
        if(ExpandedDatePlanner_Plan(pol,rounds,days,&reason)) {
            if(extended)*extended=pol->last_day-last0;
            return 1;
        }
    }
    return 0;
}
