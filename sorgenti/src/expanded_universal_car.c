/* Universal fixture ledger in the EXISTING atomic career CAR.
 * Only native SDL career may use this; Amiga/Classic/DIY/Season remain intact.
 * CAR section is optional: old schema 3-6 and legacy careers have no field.
 * Canonical fixed-width hex is endian/ABI independent and checksummed; the
 * existing SaveJson_WriteCareer_Impl still performs full RAM validation,
 * Zstandard level 3 roundtrip and swos_io_save_atomic.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_car.h"
#include "world.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum { EXP_CAR_HEX_HEAD=24, EXP_CAR_HEX_ROW=134, EXP_CAR_HEX_CRC=8 };
typedef struct ExpandedCarWorld {
    ExpandedUniversalRegistry reg;
    ExpandedUniversalFixture *fixtures;
    ExpandedFixtureLedgerEntry *ledger;
    ExpandedScheduledEvent *events;
    uint8_t active;
} ExpandedCarWorld;
static ExpandedCarWorld s_worlds[WORLD_COUNT];

static void exp_car_clear(ExpandedCarWorld *w)
{
    if(!w)return;
    free(w->fixtures);free(w->ledger);free(w->events);
    memset(w,0,sizeof *w);
}
static int exp_car_alloc(ExpandedCarWorld *w,uint16_t world,
                         uint32_t requested,ExpandedWorldDate start)
{
    const uint32_t n=requested?requested:1u;
    if(!w || world>=WORLD_COUNT)return 0;
#if SIZE_MAX < UINT64_MAX
    /* Only a 32-bit size_t can overflow while allocating UINT32_COUNT
     * objects here. On 64-bit GCC an always-false comparison is -Werror. */
    if((size_t)n>SIZE_MAX/sizeof *w->fixtures ||
       (size_t)n>SIZE_MAX/sizeof *w->ledger ||
       (size_t)n>SIZE_MAX/sizeof *w->events)return 0;
#endif
    memset(w,0,sizeof *w);
    w->fixtures=(ExpandedUniversalFixture *)calloc(n,sizeof *w->fixtures);
    w->ledger=(ExpandedFixtureLedgerEntry *)calloc(n,sizeof *w->ledger);
    w->events=(ExpandedScheduledEvent *)calloc(n,sizeof *w->events);
    if(!w->fixtures || !w->ledger || !w->events ||
       ExpandedUniversal_Init(&w->reg,w->fixtures,w->ledger,w->events,
                              world,n,start)!=EXP_UNIVERSAL_OK) {
        exp_car_clear(w);return 0;
    }
    w->active=1u;
    return 1;
}
ExpandedUniversalRegistry *ExpandedUniversal_CarRegistry(void)
{
    const WorldId world=World_Current();
    if(world>=WORLD_COUNT || !s_worlds[world].active)return NULL;
    return &s_worlds[world].reg;
}
void ExpandedUniversal_CarResetCurrent(void)
{
    if(World_Current()<WORLD_COUNT)exp_car_clear(&s_worlds[World_Current()]);
}
int ExpandedUniversal_CarBegin(ExpandedWorldDate date,uint32_t capacity)
{
    const WorldId world=World_Current();
    if(world>=WORLD_COUNT || !capacity || s_worlds[world].active)return 0;
    return exp_car_alloc(&s_worlds[world],(uint16_t)world,capacity,date);
}

/* Each field has a fixed width in hex, no host-endian struct dumps. */
static void exp_hex_put(char **ptr,uint64_t value,unsigned digits)
{
    static const char h[]="0123456789abcdef";
    unsigned i;
    for(i=0;i<digits;++i)
        (*ptr)[i]=h[(value>>((digits-1u-i)*4u))&15u];
    *ptr+=digits;
}
static int exp_hex_get(const char **p,const char *end,
                       unsigned digits,uint64_t *out)
{
    unsigned i;
    uint64_t value=0u;
    if(!p || !*p || !out || digits>16u ||
       (size_t)(end-*p)<digits)return 0;
    for(i=0;i<digits;++i) {
        const unsigned char c=(unsigned char)(*p)[i];
        unsigned d;
        if(c>='0' && c<='9')d=c-'0';
        else if(c>='a' && c<='f')d=c-'a'+10u;
        else return 0;
        value=(value<<4u)|(uint64_t)d;
    }
    *p+=digits;*out=value;
    return 1;
}
static uint32_t exp_car_hash(const char *text,size_t bytes)
{
    uint32_t h=2166136261u;
    size_t i;
    for(i=0;i<bytes;++i) {
        h^=(uint8_t)text[i];h*=16777619u;
    }
    return h;
}
static const ExpandedScheduledEvent *exp_car_event(
    const ExpandedUniversalRegistry *r,uint64_t id)
{
    uint32_t i;
    for(i=0;i<r->scheduler.count;++i)
        if(r->scheduler.events[i].id==id)return &r->scheduler.events[i];
    return NULL;
}
int ExpandedUniversal_CarEmit(mjson_print_fn_t output,void *user)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    size_t len;
    char *buf,*p;
    uint32_t i;
    int result;
    if(!output)return 0;
    if(!r)
        return mjson_printf(output,user,",\n  %Q:null","universal_engine")>0;
    if(r->world!=(uint16_t)World_Current() ||
       !ExpandedUniversal_ValidateRestore(r))return 0;
    if(r->fixture_count>(uint32_t)((INT_MAX-EXP_CAR_HEX_HEAD-
        EXP_CAR_HEX_CRC-1)/EXP_CAR_HEX_ROW))return 0;
    len=EXP_CAR_HEX_HEAD+(size_t)r->fixture_count*EXP_CAR_HEX_ROW+
        EXP_CAR_HEX_CRC;
    if(len>INT_MAX-1)return 0;
    buf=(char *)malloc(len+1u);
    if(!buf)return 0;
    p=buf;
    memcpy(p,"UE01",4u);p+=4u;
    exp_hex_put(&p,r->world,4u);
    exp_hex_put(&p,(uint32_t)r->scheduler.clock.day_index,8u);
    exp_hex_put(&p,r->fixture_count,8u);
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *claim=
            ExpandedFixtureLedger_Find(&r->ledger,f->key);
        const ExpandedScheduledEvent *e=exp_car_event(r,f->event_id);
        if(!claim || !e || e->kind!=EXP_EVENT_FIXTURE) {
            free(buf);return 0;
        }
        exp_hex_put(&p,f->event_id,16u);
        exp_hex_put(&p,e->requires_id,16u);
        exp_hex_put(&p,f->key.edition.competition.stable_competition,8u);
        exp_hex_put(&p,(uint32_t)f->key.edition.start_day,8u);
        exp_hex_put(&p,f->key.edition.sequence,4u);
        exp_hex_put(&p,f->key.stage,8u);
        exp_hex_put(&p,f->key.tie,8u);
        exp_hex_put(&p,f->key.leg,4u);
        exp_hex_put(&p,f->key.replay,4u);
        exp_hex_put(&p,f->home.stable_club,8u);
        exp_hex_put(&p,f->away.stable_club,8u);
        exp_hex_put(&p,(uint32_t)f->day_index,8u);
        exp_hex_put(&p,f->country,4u);
        exp_hex_put(&p,f->slot,2u);
        exp_hex_put(&p,f->competition_type,2u);
        exp_hex_put(&p,claim->state,2u);
        exp_hex_put(&p,claim->score.home,4u);
        exp_hex_put(&p,claim->score.away,4u);
        exp_hex_put(&p,claim->score.penalties_home,4u);
        exp_hex_put(&p,claim->score.penalties_away,4u);
        exp_hex_put(&p,claim->score.flags,2u);
        exp_hex_put(&p,e->state,2u);
        exp_hex_put(&p,e->priority,4u);
    }
    if((size_t)(p-buf)!=len-EXP_CAR_HEX_CRC) {
        free(buf);return 0;
    }
    exp_hex_put(&p,exp_car_hash(buf,len-EXP_CAR_HEX_CRC),8u);
    *p=0;
    result=mjson_printf(output,user,",\n  %Q:%Q","universal_engine",buf);
    free(buf);
    return result>0;
}
/* Parse a complete candidate in TEMPORARY heap memory. No current world
 * state changes on failed CRC, world, civil clock, event or ledger proof.
 * Nor is a missing optional section an invitation to invent fixtures. */
static int exp_car_parse(const char *json,int size,
                          ExpandedCarWorld *candidate,int *present)
{
    const char *token=NULL,*p,*end;
    int tok_len=0,kind;
    char *text=NULL;
    uint64_t v=0u,crc=0u;
    uint32_t i,n;
    uint16_t world;
    int32_t day;
    ExpandedWorldDate date;
    size_t len;
    long civil_day=0;
    if(candidate)memset(candidate,0,sizeof *candidate);
    if(present)*present=0;
    if(!json || size<=0 || !candidate || !present)return 0;
    kind=mjson_find(json,size,"$.universal_engine",&token,&tok_len);
    if(kind==MJSON_TOK_INVALID || kind==MJSON_TOK_NULL)return 1;
    if(kind!=MJSON_TOK_STRING || tok_len<0 || tok_len>INT_MAX-1)return 0;
    text=(char *)malloc((size_t)tok_len+1u);
    if(!text)return 0;
    if(mjson_get_string(json,size,"$.universal_engine",text,tok_len+1)<=0) {
        free(text);return 0;
    }
    len=strlen(text);
    if(len<EXP_CAR_HEX_HEAD+EXP_CAR_HEX_CRC ||
       (len-EXP_CAR_HEX_HEAD-EXP_CAR_HEX_CRC)%EXP_CAR_HEX_ROW) {
        free(text);return 0;
    }
    p=text;end=text+len;
    if(memcmp(p,"UE01",4u)!=0) {free(text);return 0;}
    p+=4;
    if(!exp_hex_get(&p,end,4u,&v) || v>=WORLD_COUNT ||
       v!=(uint16_t)World_Current()) {free(text);return 0;}
    world=(uint16_t)v;
    if(!exp_hex_get(&p,end,8u,&v) || v>INT32_MAX ||
       !ExpandedWorld_IndexToDate((int32_t)v,&date)) {
        free(text);return 0;
    }
    day=(int32_t)v;
    if(!exp_hex_get(&p,end,8u,&v) || v>UINT32_MAX) {
        free(text);return 0;
    }
    n=(uint32_t)v;
    if((size_t)n!=(len-EXP_CAR_HEX_HEAD-EXP_CAR_HEX_CRC)/
                    EXP_CAR_HEX_ROW ||
       mjson_get_long(json,size,"$.living_world.now",&civil_day)<=0 ||
       civil_day!=day ||
       !exp_car_alloc(candidate,world,n,date)) {
        free(text);return 0;
    }
    candidate->reg.fixture_count=n;
    candidate->reg.ledger.count=n;
    candidate->reg.scheduler.count=n;
    for(i=0;i<n;++i) {
        ExpandedUniversalFixture *f=&candidate->fixtures[i];
        ExpandedFixtureLedgerEntry *claim=&candidate->ledger[i];
        ExpandedScheduledEvent e={0};
#define READ_HEX(width,dst) do { \
        if(!exp_hex_get(&p,end,width,&v))goto fail; \
        (dst)=v; \
    } while(0)
        READ_HEX(16u,f->event_id);
        e.id=f->event_id;
        READ_HEX(16u,e.requires_id);
        READ_HEX(8u,f->key.edition.competition.stable_competition);
        READ_HEX(8u,f->key.edition.start_day);
        READ_HEX(4u,f->key.edition.sequence);
        READ_HEX(8u,f->key.stage);
        READ_HEX(8u,f->key.tie);
        READ_HEX(4u,f->key.leg);
        READ_HEX(4u,f->key.replay);
        READ_HEX(8u,f->home.stable_club);
        READ_HEX(8u,f->away.stable_club);
        READ_HEX(8u,f->day_index);
        READ_HEX(4u,f->country);
        READ_HEX(2u,f->slot);
        READ_HEX(2u,f->competition_type);
        READ_HEX(2u,claim->state);
        READ_HEX(4u,claim->score.home);
        READ_HEX(4u,claim->score.away);
        READ_HEX(4u,claim->score.penalties_home);
        READ_HEX(4u,claim->score.penalties_away);
        READ_HEX(2u,claim->score.flags);
        READ_HEX(2u,e.state);
        READ_HEX(4u,e.priority);
#undef READ_HEX
        f->key.edition.competition.world=world;
        f->home.world=world;f->away.world=world;
        claim->key=f->key;
        e.day_index=f->day_index;e.kind=EXP_EVENT_FIXTURE;
        /* The serializer stores fixtures in edition order; events must
         * still be in date/priority/id order for scheduler restore. */
        {
            uint32_t j=i;
            while(j && ExpandedEvent_Before(&e,&candidate->events[j-1u])) {
                candidate->events[j]=candidate->events[j-1u];
                --j;
            }
            candidate->events[j]=e;
        }
        if(claim->state==EXP_FIXTURE_CLAIMED)
            candidate->reg.recovery_required=1u;
    }
    if(!exp_hex_get(&p,end,8u,&crc) || p!=end ||
       (uint32_t)crc!=exp_car_hash(text,len-EXP_CAR_HEX_CRC) ||
       !ExpandedUniversal_ValidateRestore(&candidate->reg))
        goto fail;
    free(text);*present=1;return 1;
fail:
    free(text);exp_car_clear(candidate);return 0;
}
int ExpandedUniversal_CarValidate(const char *json,int size)
{
    ExpandedCarWorld candidate={0};
    int present=0;
    const int ok=exp_car_parse(json,size,&candidate,&present);
    exp_car_clear(&candidate);
    return ok;
}
int ExpandedUniversal_CarRestore(const char *json,int size)
{
    ExpandedCarWorld candidate={0};
    int present=0;
    const WorldId world=World_Current();
    if(world>=WORLD_COUNT ||
       !exp_car_parse(json,size,&candidate,&present))return 0;
    exp_car_clear(&s_worlds[world]);
    if(present)s_worlds[world]=candidate;
    return 1;
}
/* The only permitted durable transaction boundary for this registry.
 * SaveChecked calls existing SaveJson_WriteCareer_Impl, not a sidecar.
 * Never claim success if the whole OG/civil/CAR prewrite rejected. */
extern int Expanded_AutoCareer_SaveChecked(const char *reason);
int ExpandedUniversal_CarSaveClaim(void *user,
    const ExpandedUniversalRegistry *r,const ExpandedUniversalFixture *f)
{
    ExpandedFixtureLedgerEntry *entry;
    (void)user;
    if(!r || r!=ExpandedUniversal_CarRegistry() || !f ||
       !ExpandedUniversal_ValidateRestore((ExpandedUniversalRegistry *)r))
        return 0;
    entry=ExpandedFixtureLedger_Find(
        (ExpandedFixtureLedger *)&r->ledger,f->key);
    if(!entry || entry->state!=EXP_FIXTURE_CLAIMED)return 0;
    return Expanded_AutoCareer_SaveChecked("universal-fixture-claimed");
}
int ExpandedUniversal_CarSaveCompletion(void *user,
    const ExpandedUniversalRegistry *r,const ExpandedUniversalFixture *f,
    const ExpandedUniversalCompletion *proposal)
{
    ExpandedFixtureLedgerEntry *entry,saved;
    ExpandedScheduledEvent *event=NULL;
    ExpandedScheduledEvent old_event;
    uint32_t i;
    int ok;
    (void)user;
    if(!r || r!=ExpandedUniversal_CarRegistry() || !f || !proposal ||
       !proposal->witness.valid ||
       !ExpandedFixtureKey_Equal(proposal->witness.key,f->key) ||
       !ExpandedClubKey_Equal(proposal->witness.home,f->home) ||
       !ExpandedClubKey_Equal(proposal->witness.away,f->away) ||
       proposal->witness.day_index!=f->day_index ||
       !ExpandedFixtureScore_Equal(proposal->witness.score,
                                    proposal->played.score))
        return 0;
    entry=ExpandedFixtureLedger_Find(
        (ExpandedFixtureLedger *)&r->ledger,f->key);
    for(i=0;i<r->scheduler.count;++i)
        if(r->scheduler.events[i].id==f->event_id) {
            event=&r->scheduler.events[i];break;
        }
    if(!entry || entry->state!=EXP_FIXTURE_CLAIMED ||
       !event || event->state!=EXP_EVENT_PENDING ||
       !ExpandedFixtureKey_Equal(proposal->played.key,f->key) ||
       proposal->played.state!=EXP_FIXTURE_PLAYED ||
       proposal->completed.id!=f->event_id ||
       proposal->completed.state!=EXP_EVENT_COMPLETE)
        return 0;
    saved=*entry;old_event=*event;
    /* Temporarily expose EXACT proposed result + event ACK to the one
     * atomic SaveJson transaction, including current OG/RNG/club state.
     * Then restore CLAIMED so the core can Finish/Complete exactly once. */
    *entry=proposal->played;
    *event=proposal->completed;
    /* A proposed snapshot must itself be structurally recoverable BEFORE
     * any disk write; a rejected proposal preserves the previous CAR. */
    ok=ExpandedUniversal_ValidateRestore((ExpandedUniversalRegistry *)r);
    if(ok)ok=Expanded_AutoCareer_SaveChecked("universal-fixture-complete");
    *entry=saved;*event=old_event;
    return ok;
}
#endif
