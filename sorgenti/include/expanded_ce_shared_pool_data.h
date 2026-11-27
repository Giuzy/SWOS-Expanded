#ifndef EXPANDED_CE_SHARED_POOL_DATA_H
#define EXPANDED_CE_SHARED_POOL_DATA_H
/* Shared immutable catalogue; no sporting decisions, no CAR writes. */
#include "expanded_ce_shared_pool_identity.h"
#include "expanded_test_zero.h"
/* Pool globals stored in earlier CARs descend from the old 2560 ceiling. */
#define EXP_CE_POOL_SAVED_CAPACITY ((GLOBAL_TEAM_NUMBERS)>2560u?2560u:(GLOBAL_TEAM_NUMBERS))
#include "mjson.h"
#include "swos_io.h"
#include "game_types.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
typedef struct ExpCeSharedPoolRow {
    uint32_t id, key;
    uint16_t raw,global;
    uint8_t country,division;
    char name[18];
} ExpCeSharedPoolRow;
typedef struct ExpCeSharedPoolData {
    ExpCeSharedPoolRow *rows;
    uint32_t n;
} ExpCeSharedPoolData;
static inline void ExpCeSharedPool_Free(ExpCeSharedPoolData *p)
{free(p->rows);p->rows=NULL;p->n=0;}
static inline int ExpCeSharedPool_Number(const char *j,int n,const char *path,
    uint32_t max,uint32_t *out)
{
    double v=0;
    if(mjson_get_number(j,n,path,&v)<=0 || v<0 || v>(double)max ||
       v!=(double)(uint32_t)v)return 0;
    *out=(uint32_t)v;return 1;
}
static inline int ExpCeSharedPool_Load(ExpCeSharedPoolData *out)
{
    extern uint16_t _teamsCountryNumbers[256];
    char path[1024],*bytes=NULL;
    FILE *f;
    const char *arr=NULL;
    long size=0;
    int alen=0,off=0,ko,kl,vo,vl,vt,ok=0;
    uint32_t schema=0,cap=0,i;
    ExpCeSharedPoolData p={0};
    char world[16],model[32],player[32];
    if(!out)return 0;
    memset(out,0,sizeof *out);
    f=fopen(swos_data_path("data_json/worlds/2627/competitions/bottom_pools.json",
        path,(uint32_t)sizeof path),"rb");
    if(!f)return 1; /* optional, no pool in worlds without a definition */
    if(fseek(f,0,SEEK_END)!=0 || (size=ftell(f))<=0 || size>INT_MAX ||
       fseek(f,0,SEEK_SET)!=0)goto done;
    bytes=(char *)malloc((size_t)size+1u);
    if(!bytes || fread(bytes,1,(size_t)size,f)!=(size_t)size)goto done;
    bytes[size]=0;
    if(!ExpCeSharedPool_Number(bytes,(int)size,"$.schema",1u,&schema) || schema!=1u ||
       mjson_get_string(bytes,(int)size,"$.world",world,sizeof world)<=0 ||
       strcmp(world,"2627") ||
       mjson_get_string(bytes,(int)size,"$.roster_template.id",model,sizeof model)<=0 ||
       strcmp(model,"TEST_ZERO") ||
       mjson_get_string(bytes,(int)size,"$.roster_template.player.name",player,sizeof player)<=0 ||
       strcmp(player,"TEST") ||
       mjson_find(bytes,(int)size,"$.clubs",&arr,&alen)!=MJSON_TOK_ARRAY)
        goto done;
    {uint32_t z;char q[64];
     if(!ExpCeSharedPool_Number(bytes,(int)size,"$.roster_template.player.price",0u,&z))goto done;
     for(i=0;i<7u;++i){snprintf(q,sizeof q,"$.roster_template.player.skills[%u]",(unsigned)i);
         if(!ExpCeSharedPool_Number(bytes,(int)size,q,0u,&z))goto done;}
     for(i=0;i<4u;++i){snprintf(q,sizeof q,"$.roster_template.player.stats[%u]",(unsigned)i);
         if(!ExpCeSharedPool_Number(bytes,(int)size,q,0u,&z))goto done;}}
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *j=arr+vo,*flag=NULL;
        int flen=0;
        uint32_t id,file,ord,country,division,slots;
        char key[32],expect[32],roster[32];
        ExpCeSharedPoolRow x={0};
        if(vt!=MJSON_TOK_OBJECT ||
           !ExpCeSharedPool_Number(j,vl,"$.pool_id",GLOBAL_TEAM_NUMBERS,&id) ||
           !ExpCeSharedPool_Number(j,vl,"$.country",254u,&country) ||
           !ExpCeSharedPool_Number(j,vl,"$.proposed_native_file",254u,&file) ||
           !ExpCeSharedPool_Number(j,vl,"$.proposed_native_ordinal",255u,&ord) ||
           !ExpCeSharedPool_Number(j,vl,"$.home_pyramid_division",15u,&division) ||
           !ExpCeSharedPool_Number(j,vl,"$.roster.base_slots",16u,&slots) || slots!=16u ||
           mjson_get_string(j,vl,"$.roster.template",roster,sizeof roster)<=0 ||
           strcmp(roster,"TEST_ZERO") ||
           mjson_get_string(j,vl,"$.native_name",x.name,sizeof x.name)<=0 ||
           strlen(x.name)>17u ||
           mjson_get_string(j,vl,"$.key",key,sizeof key)<=0 ||
           mjson_find(j,vl,"$.administrative_test",&flag,&flen)==MJSON_TOK_INVALID ||
           flen!=5 || memcmp(flag,"false",5u) ||
           (file>=80u && file<=85u))goto done;
        if(World_FileCountry((uint8_t)file)!=country)goto done;
        x.raw=(uint16_t)((file<<8)|ord);
        {uint16_t slot=World_Slot((uint8_t)file,(uint8_t)ord);
         if(World_SlotTeam((uint8_t)country,slot)!=x.raw)goto done;
         x.key=(country<<16)|(slot+1u);
         snprintf(expect,sizeof expect,"%u:%u",(unsigned)country,(unsigned)slot+1u);}
        if(strcmp(key,expect) ||
           !ExpCePool_GlobalSlot(EXP_CE_POOL_SAVED_CAPACITY,_teamsCountryNumbers[255],id,&x.global))
            goto done;
        x.id=id;x.country=(uint8_t)country;x.division=(uint8_t)division;
        for(i=0;i<p.n;++i)
            if(p.rows[i].id==id || p.rows[i].raw==x.raw || p.rows[i].key==x.key)goto done;
        if(p.n==cap) {
            uint32_t next=cap?cap*2u:16u;
            ExpCeSharedPoolRow *q=(ExpCeSharedPoolRow *)realloc(p.rows,(size_t)next*sizeof *q);
            if(!q)goto done;
            p.rows=q;cap=next;
        }
        p.rows[p.n++]=x;
    }
    if(off<0)goto done;
    ok=1;*out=p;
done:
    if(!ok)ExpCeSharedPool_Free(&p);
    free(bytes);fclose(f);return ok;
}
static inline const ExpCeSharedPoolRow *ExpCeSharedPool_Find(const ExpCeSharedPoolData *p,
    uint16_t raw)
{
    uint32_t i;
    for(i=0;i<p->n;++i)if(p->rows[i].raw==raw)return &p->rows[i];
    return NULL;
}
/* Existing rosters remain untouched; reserve globals must map to the
 * exact stable raw key. No regeneration of a saved roster. */
static inline int ExpCeSharedPool_SavedRosterOK(const ExpCeSharedPoolData *p,
    const TeamFileEx *t)
{
    extern uint16_t _teamsCountryNumbers[256];
    uint16_t raw=(uint16_t)(((uint16_t)t->team.countryNumber<<8)|t->team.teamOrdinal);
    const ExpCeSharedPoolRow *x=ExpCeSharedPool_Find(p,raw);
    uint32_t g=(uint16_t)t->team.globalTeamNumber;
    if(x)return g==x->global;
    return !ExpCePool_IsReservedGlobal(_teamsCountryNumbers[255],g);
}
static inline void ExpCeSharedPool_Materialize(const ExpCeSharedPoolRow *x,TeamFileEx *t)
{ExpCeTestZero_Materialize(x->raw,x->global,x->name,t);}
#endif
