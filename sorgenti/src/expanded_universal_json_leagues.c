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
