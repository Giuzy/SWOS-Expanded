/* Runtime JSON league rules. Definitions/participants come from JSON;
 * this module NEVER asks the old OG resident about club order or fixtures.
 * A missing/malformed source is a closed gate, not permission to invent.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_json_leagues.h"
#include "mjson.h"
#include "swos_io.h" /* swos_data_path only: load_alloc is SAVE-ONLY */
#include "world.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
    ExpandedUniversalJsonLeague *rows;
    uint32_t count,capacity;
    uint8_t ready;
} s_json;

void ExpandedUniversalJsonLeagues_Reset(void)
{
    free(s_json.rows);
    memset(&s_json,0,sizeof s_json);
}
uint32_t ExpandedUniversalJsonLeagues_Count(void)
{
    return s_json.ready?s_json.count:0u;
}
int ExpandedUniversalJsonLeagues_Find(uint8_t country, uint8_t comp_id,
    uint8_t division, ExpandedUniversalJsonLeague *out)
{
    uint32_t i;
    if(!out || !s_json.ready)return 0;
    for(i=0;i<s_json.count;++i) {
        const ExpandedUniversalJsonLeague *r=&s_json.rows[i];
        if(r->country==country && r->comp_id==comp_id &&
           r->division==division) {
            *out=*r;return 1;
        }
    }
    return 0;
}
static int json_append(ExpandedUniversalJsonLeague row)
{
    uint32_t i;
    ExpandedUniversalJsonLeague *grow;
    uint32_t next;
    for(i=0;i<s_json.count;++i)
        if(s_json.rows[i].country==row.country &&
           s_json.rows[i].comp_id==row.comp_id &&
           s_json.rows[i].division==row.division)
            return 0;
    if(s_json.count==s_json.capacity) {
        next=s_json.capacity?2u*s_json.capacity:128u;
        if(next<s_json.capacity || next>UINT32_MAX/sizeof *grow)
            return 0;
        grow=(ExpandedUniversalJsonLeague *)realloc(
            s_json.rows,(size_t)next*sizeof *grow);
        if(!grow)return 0;
        s_json.rows=grow;s_json.capacity=next;
    }
    s_json.rows[s_json.count++]=row;
    return 1;
}
static int json_long(const char *row,int len,const char *key,
                     long lower,long upper,long *out)
{
    long v=0;
    if(mjson_get_long(row,len,key,&v)<=0 || v<lower || v>upper)
        return 0;
    *out=v;
    return 1;
}
int ExpandedUniversalJsonLeagues_ParseMonthDay(const char *text,
    uint8_t *month, uint8_t *day)
{
    static const uint8_t max_day[12]={31,29,31,30,31,30,31,31,30,31,30,31};
    unsigned m,d;
    if(!text || !month || !day ||
       text[0]<'0' || text[0]>'9' || text[1]<'0' || text[1]>'9' ||
       text[2]!='-' ||
       text[3]<'0' || text[3]>'9' || text[4]<'0' || text[4]>'9' ||
       text[5]!=0)return 0;
    m=(unsigned)(text[0]-'0')*10u+(unsigned)(text[1]-'0');
    d=(unsigned)(text[3]-'0')*10u+(unsigned)(text[4]-'0');
    if(m<1u || m>12u || d<1u || d>max_day[m-1u])return 0;
    *month=(uint8_t)m;*day=(uint8_t)d;
    return 1;
}
int ExpandedUniversalJsonLeagues_ParseWeekdays(const char *text,
    uint8_t *mask)
{
    static const char names[7][4]={"MON","TUE","WED","THU","FRI","SAT","SUN"};
    uint8_t out=0u;
    if(!text || !mask)return 0;
    if(!*text) { *mask=0u; return 1; } /* empty = none (extra days only) */
    for(;;) {
        int i,found=-1;
        for(i=0;i<7;++i)
            if(text[0]==names[i][0] && text[1]==names[i][1] &&
               text[2]==names[i][2])found=i;
        if(found<0 || (out&(1u<<found)))return 0;
        out=(uint8_t)(out|(1u<<found));
        text+=3;
        if(!*text)break;
        if(*text!=',')return 0;
        ++text;
    }
    *mask=out;
    return 1;
}
/* league_calendar.json -> date window/policy for every division of the
 * matching (team_number, comp_id). Fails closed on any malformed row. */
static int json_load_calendar(void)
{
    char *bytes=NULL;
    uint32_t size=0u,attached=0u;
    FILE *file=NULL;
    long file_size=0,declared=0;
    const char *array=NULL;
    int array_len=0,off=0,ko,kl,vo,vl,vt,rows=0;
    char path[1024];
    const char *rel="data_json/worlds/2627/competitions/league_calendar.json";
    file=fopen(swos_data_path(rel,path,(uint32_t)sizeof path),"rb");
    if(!file || fseek(file,0,SEEK_END)!=0)goto failure;
    file_size=ftell(file);
    if(file_size<=0 || file_size>INT_MAX ||
       fseek(file,0,SEEK_SET)!=0)goto failure;
    size=(uint32_t)file_size;
    bytes=(char *)malloc((size_t)size+1u);
    if(!bytes || fread(bytes,1,size,file)!=size)goto failure;
    bytes[size]=0;
    fclose(file);file=NULL;
    if(!json_long(bytes,(int)size,"$.schema",1,1,&declared) ||
       !json_long(bytes,(int)size,"$.count",1,65535,&declared) ||
       mjson_find(bytes,(int)size,"$.leagues",&array,&array_len)
            !=MJSON_TOK_ARRAY)goto failure;
    while((off=mjson_next(array,array_len,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *entry=array+vo;
        const char *breaks=NULL;
        int brk_len=0,boff=0,bk,bl,bv,blen,bt;
        long country=0,comp=0,offset=0,rest=0;
        char text[32];
        ExpandedUniversalJsonLeague cal;
        uint32_t i,hit=0u;
        memset(&cal,0,sizeof cal);
        if(vt!=MJSON_TOK_OBJECT ||
           !json_long(entry,vl,"$.team_number",0,255,&country) ||
           !json_long(entry,vl,"$.comp_id",0,255,&comp) ||
           !json_long(entry,vl,"$.last_match_year_offset",0,2,&offset) ||
           !json_long(entry,vl,"$.min_rest_days",0,14,&rest) ||
           mjson_get_string(entry,vl,"$.first_match",text,sizeof text)<=0 ||
           !ExpandedUniversalJsonLeagues_ParseMonthDay(
               text,&cal.first_month,&cal.first_day) ||
           mjson_get_string(entry,vl,"$.last_match",text,sizeof text)<=0 ||
           !ExpandedUniversalJsonLeagues_ParseMonthDay(
               text,&cal.last_month,&cal.last_day) ||
           mjson_get_string(entry,vl,"$.match_weekdays",text,sizeof text)<=0 ||
           !ExpandedUniversalJsonLeagues_ParseWeekdays(
               text,&cal.match_weekdays) || !cal.match_weekdays)
            goto failure;
        if(mjson_get_string(entry,vl,"$.extra_weekdays",text,sizeof text)<0 ||
           !ExpandedUniversalJsonLeagues_ParseWeekdays(
               text,&cal.extra_weekdays))
            goto failure;
        if(mjson_find(entry,vl,"$.breaks",&breaks,&brk_len)
                !=MJSON_TOK_ARRAY)goto failure;
        while((boff=mjson_next(breaks,brk_len,boff,&bk,&bl,&bv,&blen,&bt))>0) {
            ExpandedUniversalJsonLeagueBreak *b;
            if(bt!=MJSON_TOK_OBJECT ||
               cal.break_count>=EXP_JSON_LEAGUE_MAX_BREAKS)goto failure;
            b=&cal.breaks[cal.break_count++];
            if(mjson_get_string(breaks+bv,blen,"$.from",text,sizeof text)<=0 ||
               !ExpandedUniversalJsonLeagues_ParseMonthDay(
                   text,&b->from_month,&b->from_day) ||
               mjson_get_string(breaks+bv,blen,"$.to",text,sizeof text)<=0 ||
               !ExpandedUniversalJsonLeagues_ParseMonthDay(
                   text,&b->to_month,&b->to_day))goto failure;
        }
        if(boff<0)goto failure;
        cal.last_year_offset=(uint8_t)offset;
        cal.min_rest_days=(uint8_t)rest;
        for(i=0;i<s_json.count;++i) {
            ExpandedUniversalJsonLeague *r=&s_json.rows[i];
            if(r->country!=(uint8_t)country || r->comp_id!=(uint8_t)comp)
                continue;
            if(r->has_calendar)goto failure; /* duplicate calendar row */
            r->has_calendar=1u;
            r->first_month=cal.first_month;r->first_day=cal.first_day;
            r->last_month=cal.last_month;r->last_day=cal.last_day;
            r->last_year_offset=cal.last_year_offset;
            r->match_weekdays=cal.match_weekdays;
            r->extra_weekdays=cal.extra_weekdays;
            r->min_rest_days=cal.min_rest_days;
            r->break_count=cal.break_count;
            memcpy(r->breaks,cal.breaks,sizeof r->breaks);
            ++hit;
        }
        if(!hit)goto failure; /* calendar for an undeclared league */
        attached+=hit;
        ++rows;
    }
    if(off<0 || rows!=(int)declared)goto failure;
    free(bytes);
    fprintf(stderr,"[universal-json] league-calendar rows=%d divisions=%lu "
        "date-source=CE_JSON_PLANNER og-phase-dates=0 status=READY\n",
        rows,(unsigned long)attached);
    return 1;
failure:
    if(file)fclose(file);
    free(bytes);
    for(off=0;off<(int)s_json.count;++off)s_json.rows[off].has_calendar=0u;
    fprintf(stderr,"[universal-json] LEAGUE_CALENDAR_UNAVAILABLE source=%s "
        "owner=CE_DATE_PLANNER no-OG-phase-fallback=1 fail-closed=1\n",rel);
    return 0;
}
int ExpandedUniversalJsonLeagues_Load(void)
{
    char *bytes=NULL;
    uint32_t size=0u,definitions=0u;
    FILE *file=NULL;
    long file_size=0;
    const char *array=NULL;
    int array_len=0,off=0,ko,kl,vo,vl,vt;
    long declared=0;
    char path[1024];
    const char *rel="data_json/worlds/2627/competitions/leagues.json";
    ExpandedUniversalJsonLeagues_Reset();
    if(World_Current()!=WORLD_2627)return 1;
    file=fopen(swos_data_path(rel,path,(uint32_t)sizeof path),"rb");
    if(!file || fseek(file,0,SEEK_END)!=0)goto failure;
    file_size=ftell(file);
    if(file_size<=0 || file_size>INT_MAX ||
       fseek(file,0,SEEK_SET)!=0)goto failure;
    size=(uint32_t)file_size;
    bytes=(char *)malloc((size_t)size+1u);
    if(!bytes || fread(bytes,1,size,file)!=size)goto failure;
    bytes[size]=0;
    fclose(file);file=NULL;
    if(!json_long((const char *)bytes,(int)size,"$.count",1,65535,&declared) ||
       mjson_find((const char *)bytes,(int)size,"$.leagues",
                  &array,&array_len)!=MJSON_TOK_ARRAY)goto failure;
    while((off=mjson_next(array,array_len,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *entry=array+vo;
        const char *divisions=NULL;
        int div_len=0,divoff=0,dk,dl,dv,dlen,dt;
        long country=0,comp=0,type=0,begin=0,end=0,cycles=0,count=0;
        uint32_t d=0;
        if(vt!=MJSON_TOK_OBJECT ||
           !json_long(entry,vl,"$.comp.team_number",0,255,&country) ||
           !json_long(entry,vl,"$.comp.comp_id",0,255,&comp) ||
           !json_long(entry,vl,"$.comp.type",0,0,&type) ||
           !json_long(entry,vl,"$.comp.begin_month",0,254,&begin) ||
           !json_long(entry,vl,"$.comp.end_month",0,254,&end) ||
           !json_long(entry,vl,"$.rounds",1,4,&cycles) ||
           !json_long(entry,vl,"$.divisions_count",1,16,&count) ||
           mjson_find(entry,vl,"$.divisions",&divisions,&div_len)
                !=MJSON_TOK_ARRAY)goto failure;
        while((divoff=mjson_next(divisions,div_len,divoff,
                                  &dk,&dl,&dv,&dlen,&dt))>0) {
            long teams=0;
            ExpandedUniversalJsonLeague row;
            memset(&row,0,sizeof row);
            if(dt!=MJSON_TOK_OBJECT || d>=(uint32_t)count ||
               !json_long(divisions+dv,dlen,"$.teams",2,512,&teams))
                goto failure;
            row.country=(uint8_t)country;row.comp_id=(uint8_t)comp;
            row.division=(uint8_t)d++;
            row.begin_phase=(uint8_t)begin;row.end_phase=(uint8_t)end;
            row.meetings=(uint8_t)cycles;
            row.declared_clubs=(uint16_t)teams;
            if(!json_append(row))goto failure;
        }
        if(divoff<0 || d!=(uint32_t)count)goto failure;
        ++definitions;
    }
    if(off<0 || definitions!=(uint32_t)declared || !s_json.count)goto failure;
    free(bytes);
    s_json.ready=1u;
    fprintf(stderr,"[universal-json] league-definitions=%lu divisions=%lu "
        "pair-source=JSON_ONLY seed-source=IMMUTABLE_CLUB_KEYS status=READY\n",
        (unsigned long)definitions,(unsigned long)s_json.count);
    /* Structure stays usable; leagues without a calendar row fail closed
     * individually in the catalog (no OG phase date fallback). */
    (void)json_load_calendar();
    return 1;
failure:
    if(file)fclose(file);
    free(bytes);
    ExpandedUniversalJsonLeagues_Reset();
    fprintf(stderr,"[universal-json] LEAGUE_RULES_UNAVAILABLE "
       "source=%s owner=JSON fail-closed=1\n",rel);
    return 0;
}
#endif
