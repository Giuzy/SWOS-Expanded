/* Universal fixture ledger in the EXISTING atomic career CAR.
 * Only native SDL career may use this; Amiga/Classic/DIY/Season remain intact.
 * CAR section is optional: old schema 3-6 and legacy careers have no field.
 * Canonical fixed-width hex is endian/ABI independent and checksummed; the
 * existing SaveJson_WriteCareer_Impl still performs full RAM validation,
 * Zstandard level 3 roundtrip and swos_io_save_atomic.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_car.h"
#include "expanded_universal_live.h"
#include "save_format_json.h" /* SaveJson_SetNextWriteAsync */
#include "world.h"
#include "platform_sdl.h" /* monotonic CPU-forward CAR stage timings */
#include "expanded_car_compact.h"
#include "expanded_edition_archive.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum { EXP_CAR_HEX_HEAD=24, EXP_CAR_HEX_ROW=134, EXP_CAR_HEX_CRC=8 };
typedef struct ExpandedCarWorld {
    ExpandedUniversalRegistry reg;
    ExpandedUniversalFixture *fixtures;
    ExpandedFixtureLedgerEntry *ledger;
    ExpandedScheduledEvent *events;
    ExpandedEditionArchive archive;   /* v290: synthetic edition archive */
    uint8_t active;
} ExpandedCarWorld;
static ExpandedCarWorld s_worlds[WORLD_COUNT];

static void exp_car_clear(ExpandedCarWorld *w)
{
    if(!w)return;
    free(w->fixtures);free(w->ledger);free(w->events);
    ExpArchive_Free(&w->archive);
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
    if(World_Current()==WORLD_2627)ExpandedUniversalLive_ResetCpuBatch();
}
int ExpandedUniversal_CarBegin(ExpandedWorldDate date,uint32_t capacity)
{
    const WorldId world=World_Current();
    if(world>=WORLD_COUNT || !capacity || s_worlds[world].active)return 0;
    if(!exp_car_alloc(&s_worlds[world],(uint16_t)world,capacity,date))
        return 0;
    if(world==WORLD_2627)ExpandedUniversalLive_RegisterClock();
    return 1;
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
typedef struct ExpandedCarEventIndex {
    uint64_t id;
    const ExpandedScheduledEvent *event;
} ExpandedCarEventIndex;
static int exp_car_event_index_cmp(const void *aa,const void *bb)
{
    const ExpandedCarEventIndex *a=(const ExpandedCarEventIndex *)aa,
        *b=(const ExpandedCarEventIndex *)bb;
    return a->id<b->id?-1:a->id>b->id?1:0;
}
static const ExpandedScheduledEvent *exp_car_event(
    const ExpandedCarEventIndex *events,uint32_t count,uint64_t id)
{
    uint32_t lo=0u,hi=count;
    while(lo<hi) {
        const uint32_t mid=lo+(hi-lo)/2u;
        if(events[mid].id<id)lo=mid+1u;
        else hi=mid;
    }
    return lo<count && events[lo].id==id?events[lo].event:NULL;
}
static int exp_car_chrono_cmp(const void *aa,const void *bb)
{
    const ExpandedScheduledEvent *a=(const ExpandedScheduledEvent *)aa,
        *b=(const ExpandedScheduledEvent *)bb;
    return ExpandedEvent_Before(a,b)?-1:ExpandedEvent_Before(b,a)?1:0;
}
/* Emit core: reads ONLY through r (live registry or an owned snapshot copy). */
typedef struct ExpCarRowCtx {
    const ExpandedUniversalRegistry *r;
    const ExpandedCarEventIndex *events;
} ExpCarRowCtx;
static int exp_car_row_get(uint32_t i,uint64_t *v,void *user)
{
    const ExpCarRowCtx *x=(const ExpCarRowCtx *)user;
    const ExpandedUniversalFixture *f=&x->r->fixtures[i];
    const ExpandedFixtureLedgerEntry *claim=&x->r->ledger.entries[i];
    const ExpandedScheduledEvent *e=exp_car_event(
        x->events,x->r->scheduler.count,f->event_id);
    /* Rows stay aligned fixture/ledger/event; a misaligned ledger fails
     * closed instead of writing another fixture's result. */
    if(!ExpandedFixtureKey_Equal(claim->key,f->key) ||
       !e || e->kind!=EXP_EVENT_FIXTURE)return 0;
    v[0]=f->event_id;v[1]=e->requires_id;
    v[2]=f->key.edition.competition.stable_competition;
    v[3]=(uint32_t)f->key.edition.start_day;
    v[4]=f->key.edition.sequence;v[5]=f->key.stage;v[6]=f->key.tie;
    v[7]=f->key.leg;v[8]=f->key.replay;
    v[9]=f->home.stable_club;v[10]=f->away.stable_club;
    v[11]=(uint32_t)f->day_index;v[12]=f->country;v[13]=f->slot;
    v[14]=f->competition_type;v[15]=claim->state;
    v[16]=claim->score.home;v[17]=claim->score.away;
    v[18]=claim->score.penalties_home;v[19]=claim->score.penalties_away;
    v[20]=claim->score.flags;v[21]=e->state;v[22]=e->priority;
    return 1;
}
/* Emit core: reads ONLY through r (live registry or an owned snapshot copy).
 * v290: compact "UE02:" columnar binary (hex text in the same JSON field). */
static int exp_car_emit_registry(ExpandedUniversalRegistry *r,
                                 uint16_t expected_world,
                                 const ExpandedEditionArchive *archive,
                                 mjson_print_fn_t output,void *user)
{
    ExpandedCarEventIndex *events=NULL;
    uint32_t i;
    int result=0;
    const Uint64 started_ms=Platform_NowMs();
    Uint64 validated_ms;
    uint8_t *bin=NULL;
    char *text=NULL;
    size_t binlen=0u,hexlen;
    ExpCarRowCtx ctx;
    if(!output || !r)return 0;
    if(r->world!=expected_world ||
       !ExpandedUniversal_ValidateRestore(r))return 0;
    validated_ms=Platform_NowMs();
#if SIZE_MAX < UINT64_MAX
    if((size_t)r->scheduler.count>SIZE_MAX/sizeof *events)return 0;
#endif
    events=(ExpandedCarEventIndex *)malloc(
        ((size_t)r->scheduler.count?(size_t)r->scheduler.count:1u)*sizeof *events);
    if(!events)return 0;
    for(i=0;i<r->scheduler.count;++i)
        events[i]=(ExpandedCarEventIndex){
            r->scheduler.events[i].id,&r->scheduler.events[i]};
    qsort(events,(size_t)r->scheduler.count,sizeof *events,
          exp_car_event_index_cmp);
    ctx.r=r;ctx.events=events;
    bin=ExpCarCompact_Encode(r->world,r->scheduler.clock.day_index,
        r->fixture_count,exp_car_row_get,&ctx,&binlen);
    free(events);
    if(!bin)return 0;
    hexlen=ExpCarCompact_HexEncode(bin,binlen,NULL,0u);
    if(hexlen==(size_t)-1 || hexlen>(size_t)INT_MAX-16u) {free(bin);return 0;}
    text=(char *)malloc(5u+hexlen+1u);
    if(!text) {free(bin);return 0;}
    memcpy(text,"UE02:",5u);
    if(ExpCarCompact_HexEncode(bin,binlen,text+5,hexlen)!=hexlen) {
        free(bin);free(text);return 0;
    }
    text[5u+hexlen]=0;
    result=mjson_printf(output,user,",\n  %Q:%Q","universal_engine",text);
    if(result>0 && archive && archive->count) {
        /* v290: edition archive (absent field = empty archive) */
        size_t alen=0u,ahex;
        uint8_t *ab=ExpArchive_Encode(archive,r->world,&alen);
        char *at=NULL;
        result=0;
        if(ab) {
            ahex=ExpCarCompact_HexEncode(ab,alen,NULL,0u);
            at=(char *)malloc(5u+ahex+1u);
            if(at) {
                memcpy(at,"UA01:",5u);
                if(ExpCarCompact_HexEncode(ab,alen,at+5,ahex)==ahex) {
                    at[5u+ahex]=0;
                    result=mjson_printf(output,user,",\n  %Q:%Q",
                                        "universal_archive",at);
                }
            }
        }
        free(ab);free(at);
    }
    fprintf(stderr,"[universal-perf] CAR_EMIT_TIMING format=UE02 "
        "fixtures=%lu binary_bytes=%lu json_bytes=%lu validate_ms=%llu "
        "encode_ms=%llu status=%s\n",
        (unsigned long)r->fixture_count,(unsigned long)binlen,
        (unsigned long)(5u+hexlen),
        (unsigned long long)(validated_ms-started_ms),
        (unsigned long long)(Platform_NowMs()-validated_ms),
        result>0?"OK":"FAILED");
    free(bin);free(text);
    return result>0;
}
int ExpandedUniversal_CarEmit(mjson_print_fn_t output,void *user)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    if(!output)return 0;
    if(!r)
        return mjson_printf(output,user,",\n  %Q:null","universal_engine")>0;
    return exp_car_emit_registry(r,(uint16_t)World_Current(),
        &s_worlds[World_Current()].archive,output,user);
}
/* v288: IMMUTABLE, SELF-SUFFICIENT copy of the CE registry (fixtures, ledger,
 * events, clock, flags). The emitter below reads only this copy, so it may
 * run on the CarWriter thread while the game keeps simulating. */
struct ExpandedUniversalCarSnapshot {
    ExpandedUniversalRegistry reg;
    ExpandedUniversalFixture *fixtures;
    ExpandedFixtureLedgerEntry *ledger;
    ExpandedScheduledEvent *events;
    size_t bytes;
    uint16_t world;   /* captured on the main thread: the worker never asks */
    ExpandedEditionArchive archive;   /* owned copy */
};
int ExpandedUniversal_CarSnapshotCapture(ExpandedUniversalCarSnapshot **out)
{
    ExpandedUniversalRegistry *const r=ExpandedUniversal_CarRegistry();
    ExpandedUniversalCarSnapshot *s;
    uint32_t nf,nl,ne;
    if(!out)return 0;
    *out=NULL;
    if(!r)return 1; /* inactive registry: the caller emits null */
    if(r->world!=(uint16_t)World_Current())return 0;
    nf=r->fixture_count;nl=r->ledger.count;ne=r->scheduler.count;
    if(nf==0u || nl!=nf || ne!=nf || !r->fixtures || !r->ledger.entries ||
       !r->scheduler.events)return 0;
    s=(ExpandedUniversalCarSnapshot *)calloc(1u,sizeof *s);
    if(!s)return 0;
    s->fixtures=(ExpandedUniversalFixture *)malloc((size_t)nf*sizeof *s->fixtures);
    s->ledger=(ExpandedFixtureLedgerEntry *)malloc((size_t)nl*sizeof *s->ledger);
    s->events=(ExpandedScheduledEvent *)malloc((size_t)ne*sizeof *s->events);
    if(!s->fixtures || !s->ledger || !s->events ||
       !ExpArchive_Copy(&s->archive,&s_worlds[World_Current()].archive)) {
        free(s->fixtures);free(s->ledger);free(s->events);
        ExpArchive_Free(&s->archive);free(s);
        return 0;
    }
    memcpy(s->fixtures,r->fixtures,(size_t)nf*sizeof *s->fixtures);
    memcpy(s->ledger,r->ledger.entries,(size_t)nl*sizeof *s->ledger);
    memcpy(s->events,r->scheduler.events,(size_t)ne*sizeof *s->events);
    s->world=r->world;
    s->reg=*r;
    s->reg.fixtures=s->fixtures;s->reg.fixture_capacity=nf;
    s->reg.ledger.entries=s->ledger;s->reg.ledger.capacity=nl;
    s->reg.scheduler.events=s->events;s->reg.scheduler.capacity=ne;
    s->bytes=sizeof *s+(size_t)nf*sizeof *s->fixtures+
        (size_t)nl*sizeof *s->ledger+(size_t)ne*sizeof *s->events;
    *out=s;
    return 1;
}
size_t ExpandedUniversal_CarSnapshotBytes(const ExpandedUniversalCarSnapshot *s)
{
    return s?s->bytes:0u;
}
int ExpandedUniversal_CarEmitSnapshot(ExpandedUniversalCarSnapshot *s,
                                      mjson_print_fn_t output,void *user)
{
    if(!s)return mjson_printf(output,user,",\n  %Q:null","universal_engine")>0;
    return exp_car_emit_registry(&s->reg,s->world,&s->archive,output,user);
}
uint16_t ExpandedUniversal_CarSnapshotWorld(const ExpandedUniversalCarSnapshot *s)
{
    return s?s->world:0xffffu;
}
void ExpandedUniversal_CarSnapshotRelease(ExpandedUniversalCarSnapshot *s)
{
    if(!s)return;
    free(s->fixtures);free(s->ledger);free(s->events);
    ExpArchive_Free(&s->archive);free(s);
}
/* Parse a complete candidate in TEMPORARY heap memory. No current world
 * state changes on failed CRC, world, civil clock, event or ledger proof.
 * Nor is a missing optional section an invitation to invent fixtures. */
/* v290: UE02 decoder. Same validation tail as UE01. */
static int exp_car_parse_ue02(const char *json,int size,const char *text,
    size_t len,ExpandedCarWorld *candidate,int *present,uint16_t expected_world)
{
    ExpCarCompactReader rd;
    uint8_t *bin=NULL;
    size_t binlen;
    uint64_t v[EXP_CCOL];
    uint32_t i,n;
    ExpandedWorldDate date;
    long civil_day=0;
    uint16_t world;
    if(len<5u+4u)return 0;
    binlen=ExpCarCompact_HexDecode(text+5,len-5u,NULL,0u);
    if(binlen==(size_t)-1 || !binlen)return 0;
    bin=(uint8_t *)malloc(binlen);
    if(!bin || ExpCarCompact_HexDecode(text+5,len-5u,bin,binlen)!=binlen) {
        free(bin);return 0;
    }
    if(!ExpCarCompact_Open(&rd,bin,binlen) || rd.world>=WORLD_COUNT ||
       rd.world!=expected_world || rd.day<0 ||
       !ExpandedWorld_IndexToDate(rd.day,&date) ||
       mjson_get_long(json,size,"$.living_world.now",&civil_day)<=0 ||
       civil_day!=rd.day) {free(bin);return 0;}
    world=rd.world;n=rd.n;
    /* each row needs >= EXP_CCOL bytes: bounds n against the payload */
    if((uint64_t)n*EXP_CCOL>binlen || !exp_car_alloc(candidate,world,n,date)) {
        free(bin);return 0;
    }
    candidate->reg.fixture_count=n;
    candidate->reg.ledger.count=n;
    candidate->reg.scheduler.count=n;
    for(i=0;i<n;++i) {
        ExpandedUniversalFixture *f=&candidate->fixtures[i];
        ExpandedFixtureLedgerEntry *claim=&candidate->ledger[i];
        ExpandedScheduledEvent e={0};
        if(!ExpCarCompact_Next(&rd,v))goto fail;
        f->event_id=v[0];e.id=f->event_id;e.requires_id=v[1];
        f->key.edition.competition.stable_competition=(uint32_t)v[2];
        f->key.edition.start_day=(int32_t)(uint32_t)v[3];
        f->key.edition.sequence=(uint16_t)v[4];
        f->key.stage=(uint32_t)v[5];f->key.tie=(uint32_t)v[6];
        f->key.leg=(uint16_t)v[7];f->key.replay=(uint16_t)v[8];
        f->home.stable_club=(uint32_t)v[9];f->away.stable_club=(uint32_t)v[10];
        f->day_index=(int32_t)(uint32_t)v[11];
        f->country=(uint16_t)v[12];f->slot=(uint8_t)v[13];
        f->competition_type=(uint8_t)v[14];
        claim->state=(uint8_t)v[15];
        claim->score.home=(uint16_t)v[16];claim->score.away=(uint16_t)v[17];
        claim->score.penalties_home=(uint16_t)v[18];
        claim->score.penalties_away=(uint16_t)v[19];
        claim->score.flags=(uint8_t)v[20];
        e.state=(uint8_t)v[21];e.priority=(uint16_t)v[22];
        f->key.edition.competition.world=world;
        f->home.world=world;f->away.world=world;
        claim->key=f->key;
        e.day_index=f->day_index;e.kind=EXP_EVENT_FIXTURE;
        candidate->events[i]=e;
        if(claim->state==EXP_FIXTURE_CLAIMED)
            candidate->reg.recovery_required=1u;
    }
    if(!ExpCarCompact_Finish(&rd))goto fail;
    qsort(candidate->events,n,sizeof *candidate->events,exp_car_chrono_cmp);
    candidate->reg.fixtures_sorted_by_event=1u;
    for(i=1u;i<n;++i)
        if(candidate->fixtures[i-1u].event_id>=candidate->fixtures[i].event_id) {
            candidate->reg.fixtures_sorted_by_event=0u;
            break;
        }
    if(!ExpandedUniversal_ValidateRestore(&candidate->reg))goto fail;
    free(bin);*present=1;return 1;
fail:
    free(bin);exp_car_clear(candidate);return 0;
}

static int exp_car_parse_registry(const char *json,int size,
                          ExpandedCarWorld *candidate,int *present,
                          uint16_t expected_world)
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
    if(len>=5u && memcmp(text,"UE02:",5u)==0) {
        const int ok=exp_car_parse_ue02(json,size,text,len,candidate,
                                        present,expected_world);
        free(text);
        return ok;
    }
    if(len<EXP_CAR_HEX_HEAD+EXP_CAR_HEX_CRC ||
       (len-EXP_CAR_HEX_HEAD-EXP_CAR_HEX_CRC)%EXP_CAR_HEX_ROW) {
        free(text);return 0;
    }
    p=text;end=text+len;
    if(memcmp(p,"UE01",4u)!=0) {free(text);return 0;}
    p+=4;
    if(!exp_hex_get(&p,end,4u,&v) || v>=WORLD_COUNT ||
       v!=expected_world) {free(text);return 0;}
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
        /* Order only once after decoding all rows. Insertion sorting each
         * event made CAR restore quadratic for 27k scheduled fixtures. */
        candidate->events[i]=e;
        if(claim->state==EXP_FIXTURE_CLAIMED)
            candidate->reg.recovery_required=1u;
    }
    qsort(candidate->events,n,sizeof *candidate->events,
          exp_car_chrono_cmp);
    candidate->reg.fixtures_sorted_by_event=1u;
    for(i=1u;i<n;++i)
        if(candidate->fixtures[i-1u].event_id>=candidate->fixtures[i].event_id) {
            candidate->reg.fixtures_sorted_by_event=0u;
            break;
        }
    if(!exp_hex_get(&p,end,8u,&crc) || p!=end ||
       (uint32_t)crc!=exp_car_hash(text,len-EXP_CAR_HEX_CRC) ||
       !ExpandedUniversal_ValidateRestore(&candidate->reg))
        goto fail;
    free(text);*present=1;return 1;
fail:
    free(text);exp_car_clear(candidate);return 0;
}
/* v290: registry + optional edition archive, both fail-closed. */
static int exp_car_parse(const char *json,int size,
                         ExpandedCarWorld *candidate,int *present,
                         uint16_t expected_world)
{
    const char *tok=NULL;
    int tlen=0,kind;
    if(!exp_car_parse_registry(json,size,candidate,present,expected_world))
        return 0;
    kind=mjson_find(json,size,"$.universal_archive",&tok,&tlen);
    if(kind==MJSON_TOK_INVALID || kind==MJSON_TOK_NULL) {
        if(*present && !ExpArchive_AuditFixtures(&candidate->archive,
               candidate->fixtures,candidate->ledger,0u))goto fail;
        return 1; /* old CAR / no closed edition yet */
    }
    if(!*present || kind!=MJSON_TOK_STRING || tlen<5+8 || tlen>INT_MAX-1)
        goto fail;
    {
        char *text=(char *)malloc((size_t)tlen+1u);
        uint8_t *bin=NULL;
        size_t blen,tl;
        int ok=0;
        if(!text)goto fail;
        if(mjson_get_string(json,size,"$.universal_archive",text,tlen+1)>0) {
            tl=strlen(text);
            if(tl>5u && memcmp(text,"UA01:",5u)==0 &&
               (blen=ExpCarCompact_HexDecode(text+5,tl-5u,NULL,0u))!=(size_t)-1 &&
               blen>0u && (bin=(uint8_t *)malloc(blen))!=NULL &&
               ExpCarCompact_HexDecode(text+5,tl-5u,bin,blen)==blen &&
               ExpArchive_Decode(&candidate->archive,bin,blen,expected_world) &&
               ExpArchive_AuditFixtures(&candidate->archive,candidate->fixtures,
                   candidate->ledger,candidate->reg.fixture_count))ok=1;
        }
        free(bin);free(text);
        if(ok)return 1;
    }
fail:
    exp_car_clear(candidate);
    *present=0;
    return 0;
}
int ExpandedUniversal_CarValidateFor(const char *json,int size,uint16_t world)
{
    ExpandedCarWorld candidate={0};
    int present=0;
    const int ok=exp_car_parse(json,size,&candidate,&present,world);
    exp_car_clear(&candidate);
    return ok;
}
int ExpandedUniversal_CarValidate(const char *json,int size)
{
    return ExpandedUniversal_CarValidateFor(json,size,(uint16_t)World_Current());
}
int ExpandedUniversal_CarPeekClockFor(const char *json,int size,uint16_t world,
                                      int32_t *day)
{
    ExpandedCarWorld candidate={0};
    int present=0,ok;
    ok=day && exp_car_parse(json,size,&candidate,&present,world) && present;
    if(ok)*day=candidate.reg.scheduler.clock.day_index;
    exp_car_clear(&candidate);
    return ok;
}
int ExpandedUniversal_CarPeekClock(const char *json,int size,int32_t *day)
{
    return ExpandedUniversal_CarPeekClockFor(json,size,(uint16_t)World_Current(),day);
}
static int exp_key_cmp(const void *aa,const void *bb)
{
    const uint32_t a=*(const uint32_t *)aa,b=*(const uint32_t *)bb;
    return a<b?-1:a>b?1:0;
}
int ExpandedUniversal_CarCoversClubs(const char *json,int size,
    const uint32_t *sorted_keys,uint32_t n)
{
    ExpandedCarWorld candidate={0};
    int present=0,ok;
    uint32_t i;
    ok=sorted_keys && n && exp_car_parse(json,size,&candidate,&present,(uint16_t)World_Current()) &&
       present;
    for(i=0;ok && i<candidate.reg.fixture_count;++i) {
        const ExpandedUniversalFixture *f=&candidate.reg.fixtures[i];
        if(!bsearch(&f->home.stable_club,sorted_keys,n,sizeof *sorted_keys,
                    exp_key_cmp) ||
           !bsearch(&f->away.stable_club,sorted_keys,n,sizeof *sorted_keys,
                    exp_key_cmp))ok=0;
    }
    exp_car_clear(&candidate);
    return ok;
}
void ExpandedUniversal_CarMarkRecovery(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    if(r)r->recovery_required=1u;
}
int ExpandedUniversal_CarRestore(const char *json,int size)
{
    ExpandedCarWorld candidate={0};
    int present=0;
    const WorldId world=World_Current();
    if(world>=WORLD_COUNT ||
       !exp_car_parse(json,size,&candidate,&present,(uint16_t)World_Current()))return 0;
    exp_car_clear(&s_worlds[world]);
    if(world==WORLD_2627)ExpandedUniversalLive_ResetCpuBatch();
    if(present) {
        uint32_t i,scheduled=0u,claimed=0u,completed=0u;
        s_worlds[world]=candidate;
        if(world==WORLD_2627)ExpandedUniversalLive_RegisterClock();
        for(i=0;i<candidate.reg.ledger.count;++i) {
            const ExpandedFixtureLedgerEntry *e=&candidate.ledger[i];
            if(e->state==EXP_FIXTURE_SCHEDULED)++scheduled;
            else if(e->state==EXP_FIXTURE_CLAIMED)++claimed;
            else if(e->state==EXP_FIXTURE_PLAYED)++completed;
        }
        fprintf(stderr,"[universal-car] CONTINUE_RESTORED "
            "world=%u scheduled=%lu claimed=%lu completed=%lu "
            "recovery=%u replay=FORBIDDEN_ON_CLAIMED\n",
            (unsigned)world,(unsigned long)scheduled,
            (unsigned long)claimed,(unsigned long)completed,
            (unsigned)candidate.reg.recovery_required);
    }
    return 1;
}
/* The only permitted durable transaction boundary for this registry.
 * SaveChecked calls existing SaveJson_WriteCareer_Impl, not a sidecar.
 * Never claim success if the whole OG/civil/CAR prewrite rejected. */
extern int Expanded_AutoCareer_SaveChecked(const char *reason);

/* Commit a completed group of ORIGINAL OG quick scores at one atomic
 * boundary. Until this succeeds, the whole unsaved group is speculative:
 * on process loss CONTINUE loads the previous complete CAR and recomputes it.
 * The caller must never invoke this while a fixture remains CLAIMED. */
/* v287: async=1 lets the CarWriter thread encode/write/verify the already
 * validated image while the game continues; the outcome is reported later by
 * CarWriter_TakeFailure(). async=0 returns only after the file is durable. */
int ExpandedUniversal_CarSaveCpuBatchEx(uint32_t completed_count,int async)
{
    ExpandedUniversalRegistry *const r=ExpandedUniversal_CarRegistry();
    const Uint64 started_ms=Platform_NowMs();
    Uint64 validated_ms;
    int ok;
    uint32_t i;
    if(!completed_count || !r || r->recovery_required)return 0;
    /* Structural validation is O(N) and therefore happens PER BATCH,
     * never before every CPU quick match. v288: an ASYNC batch is validated
     * by the writer thread on its immutable snapshot (the CE emitter always
     * validates what it serialises; measured 53 ms per 100k fixtures, 684 ms
     * per 1.2M, so it must not stay on the FORWARD thread). A failure there
     * is reported through CarWriter_TakeFailure and the next flush is
     * synchronous, i.e. validated here. */
    if(!async && !ExpandedUniversal_ValidateRestore(r)) {
        r->recovery_required=1u;
        fprintf(stderr,"[universal-perf] CPU_BATCH_REJECTED reason=registry\n");
        return 0;
    }
    for(i=0;i<r->ledger.count;++i)
        if(r->ledger.entries[i].state==EXP_FIXTURE_CLAIMED) {
            r->recovery_required=1u;
            fprintf(stderr,"[universal-perf] CPU_BATCH_REJECTED reason=claimed\n");
            return 0;
        }
    validated_ms=Platform_NowMs();
#ifdef SWOS_CAR_ZSTD
    SaveJson_SetNextWriteAsync(async);
#else
    (void)async;
#endif
    ok=Expanded_AutoCareer_SaveChecked("universal-cpu-batch");
#ifdef SWOS_CAR_ZSTD
    SaveJson_SetNextWriteAsync(0);
#endif
    fprintf(stderr,"[universal-perf] CPU_BATCH_CAR count=%lu "
        "validate_ms=%llu full_save_ms=%llu status=%s\n",
        (unsigned long)completed_count,
        (unsigned long long)(validated_ms-started_ms),
        (unsigned long long)(Platform_NowMs()-validated_ms),
        ok?"SAVED":"REJECTED");
    if(!ok)r->recovery_required=1u;
    return ok;
}
int ExpandedUniversal_CarSaveCpuBatch(uint32_t completed_count)
{
    return ExpandedUniversal_CarSaveCpuBatchEx(completed_count,0);
}

int ExpandedUniversal_CarSaveClaim(void *user,
    const ExpandedUniversalRegistry *r,const ExpandedUniversalFixture *f)
{
    ExpandedFixtureLedgerEntry *entry;
    const Uint64 started_ms=Platform_NowMs();
    Uint64 validated_ms;
    (void)user;
    if(!r || r!=ExpandedUniversal_CarRegistry() || !f ||
       !ExpandedUniversal_ValidateRestore((ExpandedUniversalRegistry *)r))
        return 0;
    validated_ms=Platform_NowMs();
    entry=ExpandedUniversal_EntryFor((ExpandedUniversalRegistry *)r,f);
    if(!entry || entry->state!=EXP_FIXTURE_CLAIMED)return 0;
    {
        const int ok=Expanded_AutoCareer_SaveChecked(
            "universal-fixture-claimed");
        fprintf(stderr,"[universal-perf] CAR_CLAIM_TIMING "
            "event=%llu validate_ms=%llu full_save_ms=%llu status=%s\n",
            (unsigned long long)f->event_id,
            (unsigned long long)(validated_ms-started_ms),
            (unsigned long long)(Platform_NowMs()-validated_ms),
            ok?"SAVED":"REJECTED");
        if(ok)fprintf(stderr,"[universal-car] CLAIMED_PERSISTED "
            "event=%llu replay=FORBIDDEN\n",
            (unsigned long long)f->event_id);
        return ok;
    }
}
int ExpandedUniversal_CarSaveRelease(void *user,
    const ExpandedUniversalRegistry *r,const ExpandedUniversalFixture *f)
{
    ExpandedFixtureLedgerEntry *entry;
    (void)user;
    if(!r || r!=ExpandedUniversal_CarRegistry() || !f ||
       !ExpandedUniversal_ValidateRestore((ExpandedUniversalRegistry *)r))
        return 0;
    entry=ExpandedUniversal_EntryFor((ExpandedUniversalRegistry *)r,f);
    /* The caller already restored SCHEDULED in RAM; nothing was played. */
    if(!entry || entry->state!=EXP_FIXTURE_SCHEDULED)return 0;
    if(!Expanded_AutoCareer_SaveChecked("universal-fixture-abandoned")) {
        fprintf(stderr,"[universal-car] RELEASE_REJECTED event=%llu\n",
            (unsigned long long)f->event_id);
        return 0;
    }
    fprintf(stderr,"[universal-car] RELEASED_PERSISTED event=%llu "
        "state=SCHEDULED OG-abort=BACK_TO_PRE_MATCH\n",
        (unsigned long long)f->event_id);
    return 1;
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
    const Uint64 started_ms=Platform_NowMs();
    Uint64 validated_ms,saved_ms;
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
    entry=ExpandedUniversal_EntryFor((ExpandedUniversalRegistry *)r,f);
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
    validated_ms=Platform_NowMs();
    if(ok)ok=Expanded_AutoCareer_SaveChecked("universal-fixture-complete");
    saved_ms=Platform_NowMs();
    *entry=saved;*event=old_event;
    fprintf(stderr,"[universal-perf] CAR_COMPLETE_TIMING "
        "event=%llu precheck_ms=%llu full_save_ms=%llu status=%s\n",
        (unsigned long long)f->event_id,
        (unsigned long long)(validated_ms-started_ms),
        (unsigned long long)(saved_ms-validated_ms),
        ok?"SAVED":"REJECTED");
    if(ok)fprintf(stderr,"[universal-car] COMPLETED_PERSISTED "
        "event=%llu score=%u:%u state=COMPLETED "
        "one-atomic-CAR=1\n",
        (unsigned long long)f->event_id,
        (unsigned)proposal->played.score.home,
        (unsigned)proposal->played.score.away);
    return ok;
}
/* ---- v290 edition archive API ---- */
int ExpandedUniversal_ArchiveCloseEdition(ExpandedEditionKey edition,
    uint32_t champion,uint32_t runner_up,int32_t closed_day)
{
    ExpandedCarWorld *w;
    if(World_Current()>=WORLD_COUNT)return EXP_ARCHIVE_INVALID;
    w=&s_worlds[World_Current()];
    if(!w->active || edition.competition.world!=w->reg.world)
        return EXP_ARCHIVE_INVALID;
    return (int)ExpArchive_CloseFromRegistry(&w->archive,w->fixtures,w->ledger,
        w->reg.fixture_count,edition,champion,runner_up,closed_day);
}
uint32_t ExpandedUniversal_ArchiveCount(void)
{
    return World_Current()<WORLD_COUNT?s_worlds[World_Current()].archive.count:0u;
}
const ExpandedEditionSummary *ExpandedUniversal_ArchiveAt(uint32_t index)
{
    const ExpandedEditionArchive *a;
    if(World_Current()>=WORLD_COUNT)return NULL;
    a=&s_worlds[World_Current()].archive;
    return index<a->count?&a->items[index]:NULL;
}
int ExpandedUniversal_ArchiveContains(ExpandedEditionKey edition)
{
    return World_Current()<WORLD_COUNT &&
           ExpArchive_Find(&s_worlds[World_Current()].archive,edition)!=NULL;
}

#endif
