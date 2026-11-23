#include "expanded_edition_archive.h"
#include <stdlib.h>
#include <string.h>

/* total order used by the sorted array */
static int ed_cmp(ExpandedEditionKey a,ExpandedEditionKey b)
{
    if(a.competition.world!=b.competition.world)
        return a.competition.world<b.competition.world?-1:1;
    if(a.competition.stable_competition!=b.competition.stable_competition)
        return a.competition.stable_competition<b.competition.stable_competition?-1:1;
    if(a.start_day!=b.start_day)return a.start_day<b.start_day?-1:1;
    if(a.sequence!=b.sequence)return a.sequence<b.sequence?-1:1;
    return 0;
}
void ExpArchive_Free(ExpandedEditionArchive *a)
{
    if(!a)return;
    free(a->items);
    memset(a,0,sizeof *a);
}
static uint32_t lower_bound(const ExpandedEditionArchive *a,ExpandedEditionKey e)
{
    uint32_t lo=0u,hi=a->count;
    while(lo<hi) {
        const uint32_t mid=lo+(hi-lo)/2u;
        if(ed_cmp(a->items[mid].edition,e)<0)lo=mid+1u;
        else hi=mid;
    }
    return lo;
}
const ExpandedEditionSummary *ExpArchive_Find(const ExpandedEditionArchive *a,
                                              ExpandedEditionKey e)
{
    uint32_t i;
    if(!a || !a->items)return NULL;
    i=lower_bound(a,e);
    return i<a->count && ed_cmp(a->items[i].edition,e)==0?&a->items[i]:NULL;
}
static int summary_ok(const ExpandedEditionSummary *s)
{
    return s && s->edition.competition.stable_competition!=0u &&
           s->closed_day>=s->edition.start_day && s->champion!=0u &&
           s->runner_up!=s->champion && s->match_count>0u &&
           s->qualification_facts_secured<=1u;
}
ExpandedArchiveResult ExpArchive_Insert(ExpandedEditionArchive *a,
                                        const ExpandedEditionSummary *s)
{
    uint32_t i;
    if(!a || !summary_ok(s))return EXP_ARCHIVE_INVALID;
    i=lower_bound(a,s->edition);
    if(i<a->count && ed_cmp(a->items[i].edition,s->edition)==0)
        return memcmp(&a->items[i],s,sizeof *s)==0?EXP_ARCHIVE_ALREADY
                                                  :EXP_ARCHIVE_CONFLICT;
    if(a->count==a->cap) {
        const uint32_t nc=a->cap?a->cap*2u:16u;
        ExpandedEditionSummary *n;
        if(nc<a->cap)return EXP_ARCHIVE_FULL;
        n=(ExpandedEditionSummary *)realloc(a->items,(size_t)nc*sizeof *n);
        if(!n)return EXP_ARCHIVE_FULL;
        a->items=n;a->cap=nc;
    }
    memmove(&a->items[i+1u],&a->items[i],(size_t)(a->count-i)*sizeof *a->items);
    a->items[i]=*s;
    ++a->count;
    return EXP_ARCHIVE_OK;
}
ExpandedArchiveResult ExpArchive_CloseFromRegistry(ExpandedEditionArchive *a,
    const ExpandedUniversalFixture *fx,const ExpandedFixtureLedgerEntry *led,
    uint32_t n,ExpandedEditionKey edition,uint32_t champion,uint32_t runner_up,
    int32_t closed_day)
{
    uint32_t i,matches=0u;
    int champ_in=0,runner_in=(runner_up==0u);
    int32_t last_day=edition.start_day;
    ExpandedEditionSummary s;
    if(!a || !fx || !led || !champion || runner_up==champion)
        return EXP_ARCHIVE_INVALID;
    for(i=0u;i<n;++i) {
        if(!ExpandedEditionKey_Equal(fx[i].key.edition,edition))continue;
        if(!ExpandedFixtureKey_Equal(led[i].key,fx[i].key))
            return EXP_ARCHIVE_INVALID;
        if(led[i].state!=EXP_FIXTURE_PLAYED)return EXP_ARCHIVE_OPEN;
        ++matches;
        if(fx[i].day_index>last_day)last_day=fx[i].day_index;
        if(fx[i].home.stable_club==champion || fx[i].away.stable_club==champion)
            champ_in=1;
        if(runner_up &&
           (fx[i].home.stable_club==runner_up || fx[i].away.stable_club==runner_up))
            runner_in=1;
    }
    if(!matches)return EXP_ARCHIVE_UNKNOWN;
    if(!champ_in || !runner_in || closed_day<last_day)return EXP_ARCHIVE_INVALID;
    memset(&s,0,sizeof s);
    s.edition=edition;s.closed_day=closed_day;s.champion=champion;
    s.runner_up=runner_up;s.match_count=matches;
    s.qualification_facts_secured=0u;
    return ExpArchive_Insert(a,&s);
}
int ExpArchive_AuditFixtures(const ExpandedEditionArchive *a,
    const ExpandedUniversalFixture *fx,const ExpandedFixtureLedgerEntry *led,
    uint32_t n)
{
    uint32_t i;
    if(!a || !a->count)return 1;
    if(!fx || !led)return 0;
    for(i=0u;i<n;++i)
        if(ExpArchive_Find(a,fx[i].key.edition) &&
           led[i].state!=EXP_FIXTURE_PLAYED)return 0;
    return 1;
}
int ExpArchive_Validate(const ExpandedEditionArchive *a,uint16_t world)
{
    uint32_t i;
    if(!a || (a->count && !a->items))return 0;
    for(i=0u;i<a->count;++i) {
        if(!summary_ok(&a->items[i]) ||
           a->items[i].edition.competition.world!=world)return 0;
        if(i && ed_cmp(a->items[i-1u].edition,a->items[i].edition)>=0)return 0;
    }
    return 1;
}
enum { UA_HEAD=4+2+4, UA_REC=4+4+2+4+4+4+4+1 };
static void w32(uint8_t *p,uint32_t v)
{p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static uint32_t r32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint64_t fnv(const uint8_t *p,size_t n)
{
    uint64_t h=1469598103934665603ull;
    size_t i;
    for(i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}
    return h;
}
uint8_t *ExpArchive_Encode(const ExpandedEditionArchive *a,uint16_t world,
                           size_t *len)
{
    size_t total,at,k;
    uint32_t i;
    uint8_t *b;
    if(!a || !len || !ExpArchive_Validate(a,world))return NULL;
    total=UA_HEAD+(size_t)a->count*UA_REC+8u;
    b=(uint8_t *)malloc(total);
    if(!b)return NULL;
    memcpy(b,"UA01",4u);b[4]=(uint8_t)world;b[5]=(uint8_t)(world>>8);
    w32(b+6,a->count);
    at=UA_HEAD;
    for(i=0u;i<a->count;++i) {
        const ExpandedEditionSummary *s=&a->items[i];
        w32(b+at,s->edition.competition.stable_competition);
        w32(b+at+4,(uint32_t)s->edition.start_day);
        b[at+8]=(uint8_t)s->edition.sequence;
        b[at+9]=(uint8_t)(s->edition.sequence>>8);
        w32(b+at+10,(uint32_t)s->closed_day);
        w32(b+at+14,s->champion);w32(b+at+18,s->runner_up);
        w32(b+at+22,s->match_count);b[at+26]=s->qualification_facts_secured;
        at+=UA_REC;
    }
    {const uint64_t h=fnv(b,at);for(k=0;k<8;++k)b[at+k]=(uint8_t)(h>>(8*k));}
    *len=at+8u;
    return b;
}
int ExpArchive_Decode(ExpandedEditionArchive *a,const uint8_t *b,size_t len,
                      uint16_t expected_world)
{
    size_t at,k;
    uint32_t i,n;
    uint64_t h=0u;
    ExpandedEditionArchive tmp={0};
    if(!a || !b)return 0;
    ExpArchive_Free(a);
    if(len<UA_HEAD+8u || memcmp(b,"UA01",4u))return 0;
    for(k=0;k<8;++k)h|=(uint64_t)b[len-8u+k]<<(8*k);
    if(h!=fnv(b,len-8u) || (uint16_t)(b[4]|(b[5]<<8))!=expected_world)return 0;
    n=r32(b+6);
    if((len-UA_HEAD-8u)!=(size_t)n*UA_REC)return 0;
    if(n) {
        tmp.items=(ExpandedEditionSummary *)calloc(n,sizeof *tmp.items);
        if(!tmp.items)return 0;
        tmp.cap=n;
    }
    at=UA_HEAD;
    for(i=0u;i<n;++i) {
        ExpandedEditionSummary *s=&tmp.items[i];
        s->edition.competition.world=expected_world;
        s->edition.competition.stable_competition=r32(b+at);
        s->edition.start_day=(int32_t)r32(b+at+4);
        s->edition.sequence=(uint16_t)(b[at+8]|(b[at+9]<<8));
        s->closed_day=(int32_t)r32(b+at+10);
        s->champion=r32(b+at+14);s->runner_up=r32(b+at+18);
        s->match_count=r32(b+at+22);s->qualification_facts_secured=b[at+26];
        at+=UA_REC;
    }
    tmp.count=n;
    if(!ExpArchive_Validate(&tmp,expected_world)) {ExpArchive_Free(&tmp);return 0;}
    *a=tmp;
    return 1;
}
int ExpArchive_Copy(ExpandedEditionArchive *dst,const ExpandedEditionArchive *src)
{
    if(!dst || !src)return 0;
    memset(dst,0,sizeof *dst);
    if(!src->count)return 1;
    dst->items=(ExpandedEditionSummary *)malloc((size_t)src->count*sizeof *dst->items);
    if(!dst->items)return 0;
    memcpy(dst->items,src->items,(size_t)src->count*sizeof *dst->items);
    dst->count=dst->cap=src->count;
    return 1;
}
