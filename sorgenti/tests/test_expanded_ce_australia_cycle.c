/* v00.13.341 RG-T: multi-year cycle of the AUSTRALIAN competitions through the SAME generic
 * Competition Engine: the REAL definitions of data_json/worlds/2627/competitions (leagues.json,
 * league_calendar.json, ce_competitions.json, bottom_pools.json, test_clubs.json, league_start_history)
 * built by the runtime's own ExpCe_BuildWorldDefinitions, real club sets of AUSTRALIA.json
 * (A-League 12, NPL NSW 16, Victoria 14, Queensland 12 + pools), real registry/RunDue + CAR codec.
 * MOCKED: the OG match (deterministic scores), the disk, the first editions (synthetic seasons that
 * start after 1 July 2026: the real 2026 history is not replayed). NOT gameplay, NOT Windows. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_car.h"
#include "expanded_ce_engine.h"
#include "expanded_ce_admin.h"
#include "expanded_ce_table.h"
#include "expanded_universal_pairings.h"
#include "expanded_universal_json_leagues.h"
#include "expanded_universal_world_catalog.h"
#include "expanded_universal_live.h"
#include "world.h"
#include "expanded_ce_shared_pool_data.h"

TeamFileEx *ExpandedUniversalCatalog_Roster(ExpandedClubKey k)
{
    static TeamFileEx t;
    memset(&t,0,sizeof t);
    snprintf((char *)t.team.teamName,sizeof t.team.teamName,"C%u-%u",
             (unsigned)(k.stable_club>>16),(unsigned)(k.stable_club&0xffffu));
    return &t;
}
int ExpandedUniversalLive_ClubNameCompare(void *u,ExpandedClubKey a,ExpandedClubKey b)
{(void)u;return a.stable_club<b.stable_club?-1:a.stable_club>b.stable_club;}
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
void *swos_io_alloc(uint32_t n){return malloc(n);}
void swos_io_free(void *p){free(p);}
WorldId World_Current(void){return WORLD_2627;}
uint16_t _teamsCountryNumbers[256]={[255]=2313u};
uint8_t World_FileCountry(uint8_t f){return f;}
uint16_t World_Slot(uint8_t f,uint8_t o){(void)f;return o;}
uint16_t World_SlotTeam(uint8_t c,uint16_t slot){return (uint16_t)(((uint16_t)c<<8)|slot);}
uint16_t World_FirstYear(void){return 2026u;}
const char *swos_data_path(const char *rel,char *buf,uint32_t sz)
{snprintf(buf,sz,"../%s",rel);return buf;}
unsigned long long Platform_NowMs(void){return 0;}
void ExpandedUniversalLive_ResetCpuBatch(void){}
void ExpandedUniversalLive_RegisterClock(void){}
int CarWriter_TakeFailure(void){return 0;}

static char s_disk[1<<22];
static int s_disk_len;
int Expanded_AutoCareer_SaveChecked(const char *reason)
{
    struct mjson_fixedbuf dst={s_disk,(int)sizeof s_disk,0};
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    (void)reason;
    assert(r);
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"{%Q:{%Q:%d}","living_world",
        "now",(int)r->scheduler.clock.day_index)>0);
    assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst));
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"}")>0);
    assert(dst.len<(int)sizeof s_disk-1);
    assert(ExpandedUniversal_CarValidate(s_disk,dst.len));
    s_disk_len=dst.len;
    return 1;
}
static int h_ins(void *u,const ExpandedUniversalFixture *f,uint32_t n){(void)u;return ExpandedUniversal_CarInsertFixtures(f,n);}
static int h_prune(void *u,ExpandedEditionKey e){(void)u;return ExpandedUniversal_CarPruneEdition(e);}
static int h_hold(void *u,ExpandedEditionKey e,int h){(void)u;return ExpandedUniversal_CarHoldEdition(e,h);}
static int h_res(void *u,uint64_t id,int32_t d){(void)u;return ExpandedUniversal_CarRescheduleEvent(id,d);}
static int h_arch(void *u,ExpandedEditionKey e,uint32_t c,uint32_t r,int32_t d)
{(void)u;return ExpandedUniversal_ArchiveCloseEdition(e,c,r,d);}
static int h_club(void *u,ExpandedClubKey k){(void)u;return k.stable_club!=0u;}
static uint32_t h_div(void *u,uint8_t c,uint8_t d,ExpandedClubKey *o,uint32_t cap)
{(void)u;(void)c;(void)d;(void)o;(void)cap;return 0u;}
static const ExpCeHost H={h_ins,h_prune,h_hold,h_res,h_arch,h_club,NULL,NULL,h_div};
static int32_t D(int y,int m,int d)
{
    int32_t out=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d},&out));
    return out;
}
static ExpandedUniversalRegistry *R(void){return ExpandedUniversal_CarRegistry();}

/* ---- mock OG: deterministic, varied scores; level knockout ties go to extra time / penalties ---- */
static uint64_t s_ran[40000];
static uint32_t s_ran_n,s_dupes,s_admin;
static uint64_t mix(uint64_t x){x^=x>>33;x*=UINT64_C(0xff51afd7ed558ccd);x^=x>>33;return x;}
static int og_mock(void *user,const ExpandedUniversalFixture *f,ExpandedUniversalMatchWitness *w)
{
    ExpCeMatchRules mr;
    uint8_t decides=0;
    uint16_t h1=0,a1=0;
    const uint64_t v=mix(f->event_id);
    uint16_t home=(uint16_t)(v%4u),away=(uint16_t)((v>>8)%3u),ph=0,pa=0;
    uint8_t flags=0;
    ExpandedFixtureScore admin;
    const char *why=NULL;
    uint32_t i;
    (void)user;
    for(i=0;i<s_ran_n;++i)if(s_ran[i]==f->event_id)++s_dupes;
    if(s_ran_n<sizeof s_ran/sizeof *s_ran)s_ran[s_ran_n++]=f->event_id;
    memset(w,0,sizeof *w);
    w->key=f->key;w->home=f->home;w->away=f->away;w->day_index=f->day_index;
    if(ExpCeAdmin_Decide(f,&admin,&why)){++s_admin;w->score=admin;w->valid=1u;return 1;}
    assert(ExpCe_RulesForFixture(f,&mr,NULL));
    assert(ExpCeTie_MatchContext(R(),f,&mr,&decides,&h1,&a1));
    if(decides && (v>>16)%2u==0u) {
        if(a1>=h1){home=(uint16_t)(1u+a1-h1);away=1u;}
        else {home=1u;away=(uint16_t)(1u+h1-a1);}
    }
    if(decides) {
        int level=(uint32_t)home+h1==(uint32_t)away+a1;
        if(level && mr.legs==2u && mr.away_goals && h1!=away)level=0;
        if(level && mr.extra_time)flags|=EXP_CE_SCORE_EXTRA_TIME;
        if(level && mr.penalties) {
            flags|=EXP_CE_SCORE_PENALTIES;
            if((v>>24)&1u){ph=5u;pa=4u;}else{ph=3u;pa=5u;}
        }
    }
    w->score=(ExpandedFixtureScore){home,away,ph,pa,flags};
    w->valid=1u;
    return 1;
}
static int ok_claim(void *u,const ExpandedUniversalRegistry *r,const ExpandedUniversalFixture *f)
{(void)u;(void)r;(void)f;return 1;}
static int ok_done(void *u,const ExpandedUniversalRegistry *r,
                   const ExpandedUniversalFixture *f,const ExpandedUniversalCompletion *p)
{(void)u;(void)r;(void)f;(void)p;return 1;}
static const ExpandedUniversalOps OPS={ok_claim,og_mock,ok_done,NULL,NULL,NULL};
static void run_until(int32_t until)
{
    for(;;) {
        ExpandedUniversalRegistry *r=R();
        const ExpandedUniversalFixture *f=NULL;
        const int32_t today=r->scheduler.clock.day_index;
        ExpandedUniversalResult res;
        assert(ExpCe_Tick(r,&H,today)>=0);
        r=R();
        res=ExpandedUniversal_Peek(r,today,&f);
        if(res==EXP_UNIVERSAL_DUE) {
            assert(ExpandedUniversal_RunDue(r,today,&OPS)==EXP_UNIVERSAL_OK);
            continue;
        }
        assert(res==EXP_UNIVERSAL_EMPTY);
        if(today>=until)return;
        r->scheduler.clock.day_index=today+1;
    }
}

/* ---- first (synthetic) seasons of the four Australian leagues with the real club sets ---- */
typedef struct Em{ExpandedUniversalFixture *out;uint32_t n,clubs;uint32_t slot0;uint8_t div;
    uint32_t comp;int32_t start,day0,step;} Em;
static int em_rr(void *u,uint32_t cycle,uint32_t round,uint32_t serial,uint32_t h,uint32_t a)
{
    Em *x=(Em *)u;
    ExpandedUniversalFixture *f=&x->out[x->n++];
    const uint32_t per=(x->clubs&1u)?x->clubs:x->clubs-1u;
    memset(f,0,sizeof *f);
    f->home=(ExpandedClubKey){WORLD_2627,(44u<<16)|(x->slot0+h),EXP_PARTICIPANT_CLUB};
    f->away=(ExpandedClubKey){WORLD_2627,(44u<<16)|(x->slot0+a),EXP_PARTICIPANT_CLUB};
    f->day_index=x->day0+x->step*(int32_t)(cycle*per+round);
    /* the synthetic NPL rounds never share a day with the Australia Cup rounds of 2026 (12 and 18 August) */
    if(x->step==2 && (f->day_index==D(2026,8,12) || f->day_index==D(2026,8,18)))++f->day_index;
    f->key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,x->comp};
    f->key.edition.start_day=x->start;f->key.stage=1u;f->key.tie=serial+1u;
    f->key.leg=(uint16_t)cycle;f->country=44u;f->slot=x->div;
    f->event_id=((uint64_t)WORLD_2627<<60)|((uint64_t)x->comp<<20)|((uint64_t)cycle<<12)|(serial+1u);
    return 1;
}
static void first_season(uint8_t div,uint32_t slot0,uint32_t clubs,int32_t start,int32_t day0,int32_t step)
{
    static ExpandedUniversalFixture fx[1024];
    Em e;
    memset(&e,0,sizeof e);
    e.out=fx;e.clubs=clubs;e.slot0=slot0;e.div=div;
    e.comp=ExpCe_LeagueCompetitionId(44u,88u,div);e.start=start;e.day0=day0;e.step=step;
    {   /* the first season as the catalog emits it: double round robin + the definition's extra rounds */
        const ExpCeDef *d=ExpCe_Definition(e.comp);
        ExpPairExtras ex;
        uint32_t keys[EXP_PAIR_MAX_CLUBS],j;
        assert(d && clubs<=EXP_PAIR_MAX_CLUBS);
        ex=d->phases[0].extras;
        for(j=0u;j<clubs;++j)keys[j]=(44u<<16)|(slot0+j);
        assert(ExpandedUniversal_GenerateLeague(clubs,d->phases[0].meetings,ex.rounds?keys:NULL,&ex,
                                                ExpPair_Seed(d->id,start),em_rr,&e));
        assert(e.n==clubs*(clubs-1u)/2u*d->phases[0].meetings+clubs*ex.rounds/2u);
    }
    assert(ExpandedUniversal_CarInsertFixtures(fx,e.n));
}
static const ExpCeEdition *edition_of(uint32_t comp,uint16_t year)
{
    uint32_t i;
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *e=ExpCe_EditionAt(i);
        if(e->key.competition.stable_competition==comp && e->year==year)return e;
    }
    return NULL;
}
static int has(const ExpCeEdition *e,uint32_t slot)
{
    uint32_t i;
    for(i=0;e && i<e->nentrants;++i)if(e->entrants[i].club.stable_club==((44u<<16)|slot))return 1;
    return 0;
}
static void setup(void)
{
    ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
    s_ran_n=s_dupes=s_admin=0u;
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},8192u));
    assert(ExpandedUniversalJsonLeagues_Load());
    assert(ExpCe_BuildWorldDefinitions(WORLD_2627)>0);
    first_season(0u,1u,12u,D(2026,10,16),D(2026,10,17),7);     /* A-League: Saturdays from the real start */
    first_season(1u,13u,16u,D(2026,7,2),D(2026,7,3),2);        /* NPL NSW 30 rounds, ends 30 Aug  */
    first_season(2u,29u,14u,D(2026,7,2),D(2026,7,3),2);        /* NPL Victoria 26 rounds          */
    first_season(3u,43u,12u,D(2026,7,2),D(2026,7,3),2);        /* NPL Queensland 22 rounds        */
    assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
}

static int au_comp(uint32_t c,uint32_t AL,uint32_t NSW,uint32_t VIC,uint32_t QLD,uint32_t CH,uint32_t AC)
{return c==AL||c==NSW||c==VIC||c==QLD||c==CH||c==AC;}
/* signature of the whole competition state of the six Australian definitions */
static uint64_t sig(uint32_t AL,uint32_t NSW,uint32_t VIC,uint32_t QLD,uint32_t CH,uint32_t AC)
{
    uint64_t h=UINT64_C(14695981039346656037);
    uint32_t i,k;
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *x=ExpCe_EditionAt(i);
        if(!au_comp(x->key.competition.stable_competition,AL,NSW,VIC,QLD,CH,AC))continue;
#define MIX(v) do{h^=(uint64_t)(v);h*=UINT64_C(1099511628211);}while(0)
        MIX(x->key.competition.stable_competition);MIX(x->year);MIX(x->state);MIX(x->block);
        MIX(x->nentrants);MIX(x->nfinal);MIX(x->phase);MIX(x->successor);MIX(x->expiry_day);
        for(k=0;k<x->nentrants;++k)MIX(x->entrants[k].club.stable_club);
        for(k=0;k<x->nfinal;++k)MIX(x->final_order[k]);
#undef MIX
    }
    return h;
}
/* save, forget the process, rebuild the definitions, restore: the state must be identical */
static void save_restore(uint32_t AL,uint32_t NSW,uint32_t VIC,uint32_t QLD,uint32_t CH,uint32_t AC)
{
    static char disk[1<<22];
    int len;
    const uint64_t before=sig(AL,NSW,VIC,QLD,CH,AC);
    const int32_t today=R()->scheduler.clock.day_index;
    assert(Expanded_AutoCareer_SaveChecked("au-cycle"));
    memcpy(disk,s_disk,(size_t)s_disk_len);len=s_disk_len;disk[len]=0;
    ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
    assert(ExpandedUniversalJsonLeagues_Load());
    assert(ExpCe_BuildWorldDefinitions(WORLD_2627)>0);
    assert(ExpandedUniversal_CarValidate(disk,len));
    assert(ExpandedUniversal_CarRestore(disk,len));
    {   char *hex=ExpandedUniversal_CarTakeLifecycle();
        assert(hex && ExpCe_DecodeHex(hex,strlen(hex),WORLD_2627));free(hex);
    }
    assert(ExpCe_ValidateAfterRestore(R(),&H)>=0);
    assert(R()->scheduler.clock.day_index==today);
    assert(sig(AL,NSW,VIC,QLD,CH,AC)==before);
}
static unsigned s_checks,s_restores,s_exchanges,s_dyn,s_champ_blocked;
/* v00.13.342: the extra rounds keep 26 games per club (156 in total) and 13 home / 13 away even when some
 * rivalry clubs are absent from the edition (relegated / exchanged). Pure pairing kernel, 3000 club sets. */
typedef struct PairCount { uint32_t games[12],home[12],total; } PairCount;
static int pair_emit(void *u,uint32_t cycle,uint32_t round,uint32_t serial,uint32_t h,uint32_t a)
{
    PairCount *c=(PairCount *)u;
    (void)cycle;(void)round;(void)serial;
    assert(h<12u && a<12u && h!=a);
    ++c->games[h];++c->games[a];++c->home[h];++c->total;
    return 1;
}
static void test_extra_rounds_keep_26_games(void)
{
    ExpPairExtras x;
    uint32_t seed,i,j;
    const uint32_t riv[6][2]={{5,12},{4,6},{2,10},{1,3},{8,11},{7,9}};   /* same shape as the six documented pairs */
    memset(&x,0,sizeof x);
    x.rounds=4u;x.nrivalries=6u;
    for(seed=0u;seed<3000u;++seed) {
        uint32_t keys[12],absent=seed%5u;     /* 0..4 clubs replaced by strangers */
        PairCount c;
        for(i=0u;i<12u;++i)keys[i]=i+1u;
        for(i=0u;i<absent;++i)keys[(seed/5u+i*3u)%12u]=13u+i+(seed%7u);   /* a rivalry key replaced */
        for(i=0u;i<12u;++i)for(j=0u;j<i;++j)assert(keys[i]!=keys[j]);
        for(i=0u;i<6u;++i){x.rivalries[i][0]=riv[i][0];x.rivalries[i][1]=riv[i][1];}
        memset(&c,0,sizeof c);
        assert(ExpandedUniversal_GenerateLeague(12u,2u,keys,&x,seed+1u,pair_emit,&c));
        assert(c.total==156u);
        for(i=0u;i<12u;++i)assert(c.games[i]==26u && c.home[i]==13u);
    }
}
int main(void)
{
    const uint32_t AL=ExpCe_LeagueCompetitionId(44u,88u,0u),NSW=ExpCe_LeagueCompetitionId(44u,88u,1u),
                   VIC=ExpCe_LeagueCompetitionId(44u,88u,2u),QLD=ExpCe_LeagueCompetitionId(44u,88u,3u),
                   CH=2884098u,AC=2884866u;
    const uint32_t lg[4]={AL,NSW,VIC,QLD},want[4]={12u,16u,14u,12u};
    uint32_t i,k,y,m;
    uint32_t seen_clubs[256],nseen=0u;
    setup();
    assert(ExpCe_Definition(AL) && ExpCe_Definition(AL)->xch_on && ExpCe_Definition(CH) && ExpCe_Definition(AC));
    /* a ceased historical competition is never regenerated: no NSL Cup definition exists */
    for(i=0;i<ExpCe_DefinitionCount();++i) {
        const ExpCeDef *d=ExpCe_DefinitionAt(i);
        assert(!strstr(d->name,"NSL"));
    }
    /* month by month from 1 July 2026 for five full years */
    for(y=2026u;y<=2031u;++y)
      for(m=(y==2026u?8u:1u);m<=12u && !(y==2031u && m>7u);++m) {
        uint32_t li;
        run_until(D((int)y,(int)m,1));
        /* ---- integrity of every Australian league at this date ---- */
        for(li=0;li<4u;++li) {
            const ExpCeEdition *latest=NULL;
            for(i=0;i<ExpCe_EditionCount();++i) {
                const ExpCeEdition *x=ExpCe_EditionAt(i);
                if(x->key.competition.stable_competition==lg[li] && (!latest || x->key.start_day>latest->key.start_day))latest=x;
            }
            if(latest && latest->block)printf("[cycle] BLOCKED comp=%u year=%u block=%d why=%s at %u-%02u\n",(unsigned)lg[li],(unsigned)latest->year,(int)latest->block,latest->why,(unsigned)y,(unsigned)m);
            if(latest && latest->block) {
                uint32_t pl=0u,sc=0u,ot=0u,j;
                for(j=0u;j<R()->fixture_count;++j) {
                    const ExpandedUniversalFixture *f=&R()->fixtures[j];
                    if(!ExpandedEditionKey_Equal(f->key.edition,latest->key))continue;
                    if(R()->ledger.entries[j].state==EXP_FIXTURE_PLAYED)++pl;
                    else if(R()->ledger.entries[j].state==EXP_FIXTURE_SCHEDULED){++sc;if(sc<4u)printf("[cycle] unplayed day=%d stage=%u tie=%u leg=%u home=%u away=%u\n",(int)f->day_index,(unsigned)f->key.stage,(unsigned)f->key.tie,(unsigned)f->key.leg,(unsigned)f->home.stable_club,(unsigned)f->away.stable_club);}
                    else ++ot;
                }
                printf("[cycle] edition fixtures played=%u scheduled=%u other=%u today=%d\n",(unsigned)pl,(unsigned)sc,(unsigned)ot,(int)R()->scheduler.clock.day_index);
            }
            assert(latest && !latest->block);
            assert(latest->nentrants==want[li]);
            for(k=0;k<latest->nentrants;++k) {   /* club inside a league once; memorise the universe */
                const uint32_t c=latest->entrants[k].club.stable_club;
                uint32_t q,dup=0u;
                for(q=0;q<k;++q)if(latest->entrants[q].club.stable_club==c)dup=1u;
                assert(!dup && (c>>16)==44u);
                for(q=0;q<nseen;++q)if(seen_clubs[q]==c)dup=1u;
                if(!dup){assert(nseen<256u);seen_clubs[nseen++]=c;}
            }
        }
        if(m==1u) {       /* settled date (January): no club in two leagues; in July a promoted club may still finish its region season */
            uint32_t a,b;
            for(a=0;a<4u;++a)for(b=a+1u;b<4u;++b) {
                const ExpCeEdition *ea=NULL,*eb=NULL;
                for(i=0;i<ExpCe_EditionCount();++i) {
                    const ExpCeEdition *x=ExpCe_EditionAt(i);
                    if(x->key.competition.stable_competition==lg[a] && (!ea || x->key.start_day>ea->key.start_day))ea=x;
                    if(x->key.competition.stable_competition==lg[b] && (!eb || x->key.start_day>eb->key.start_day))eb=x;
                }
                for(k=0;k<ea->nentrants;++k)
                    if(has(eb,ea->entrants[k].club.stable_club&0xffffu))
                        printf("[cycle] CLUB IN TWO LEAGUES club=%u leagues=%u/%u at %u-%02u\n",
                               (unsigned)(ea->entrants[k].club.stable_club&0xffffu),(unsigned)a,(unsigned)b,(unsigned)y,(unsigned)m);
                for(k=0;k<ea->nentrants;++k)
                    assert(!has(eb,ea->entrants[k].club.stable_club&0xffffu));
            }
        }
        assert(ExpandedUniversal_ValidateRestore(R()) && s_dupes==0u);
        ++s_checks;
        save_restore(AL,NSW,VIC,QLD,CH,AC);++s_restores;
    }
    /* the universe of club identities never grows: 54 league clubs + the 10 free clubs of the pools */
    printf("[cycle] distinct league clubs seen over 5 years: %u\n",(unsigned)nseen);
    {   uint32_t q,out=0u;
        for(q=0;q<nseen;++q)if((seen_clubs[q]&0xffffu)>54u){assert((seen_clubs[q]&0xffffu)>=80u && (seen_clubs[q]&0xffffu)<=93u);++out;printf("[cycle] club outside the 54 league clubs: slot %u\n",(unsigned)(seen_clubs[q]&0xffffu));}
        printf("[cycle] slots 55+ seen in league editions: %u\n",(unsigned)out);
    }
    assert(nseen<=68u);     /* 54 league clubs + the 14 standby clubs of bottom_pools.json (slots 80..93): nothing invented */
    /* every Australian edition that ended: champion in the archive = final order, trophies kept */
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *x=ExpCe_EditionAt(i);
        if(!au_comp(x->key.competition.stable_competition,AL,NSW,VIC,QLD,CH,AC) || x->state<EXP_CE_ED_COMPLETED)continue;
        assert(!x->block && x->nfinal>=2u);
        if(x->state==EXP_CE_ED_ARCHIVED) {
            const ExpandedEditionSummary *sm=NULL;
            uint32_t q;
            for(q=0;q<ExpandedUniversal_ArchiveCount();++q) {
                const ExpandedEditionSummary *c=ExpandedUniversal_ArchiveAt(q);
                if(ExpandedEditionKey_Equal(c->edition,x->key))sm=c;
            }
            assert(sm && sm->champion==x->final_order[0] && sm->runner_up==x->final_order[1]);
            ++s_exchanges;
        }
    }
    /* ===== league exchange year by year: Championship winner <-> last of the A-League =====
     * A Championship edition that is BLOCKED (the documented collision of a club that plays both the
     * A-League and the Championship) gives no winner: the A-League must then stay as it was. */
    for(y=2026u;y<=2029u;++y) {
        const ExpCeEdition *a=edition_of(AL,(uint16_t)y),*an=edition_of(AL,(uint16_t)(y+1u)),*c=edition_of(CH,(uint16_t)y);
        uint32_t w,x,j;
        assert(a && an && an->nentrants==12u);
        if(!c || c->state<EXP_CE_ED_COMPLETED) {
            assert(!c || c->block==EXP_CE_BLOCK_COLLISION || c->block==EXP_CE_BLOCK_DATE_PLAN);   /* no free day for a club in both leagues */
            for(j=0u;j<12u;++j)assert(an->entrants[j].club.stable_club==a->entrants[j].club.stable_club);
            ++s_champ_blocked;
            printf("[cycle] year %u: Championship %s, A-League unchanged\n",(unsigned)y,c?"BLOCKED by a calendar collision":"not played");
            continue;
        }
        assert(c->nfinal>=1u);
        w=c->final_order[0];
        {   int win_in=0;
            for(j=0u;j<a->nentrants;++j)if(a->entrants[j].club.stable_club==w)win_in=1;
            if(win_in) {                                   /* winner already in the A-League: nothing moves */
                for(j=0u;j<12u;++j)assert(an->entrants[j].club.stable_club==a->entrants[j].club.stable_club);
                printf("[cycle] year %u: Championship winner %u already in the A-League, no movement\n",(unsigned)y,(unsigned)(w&0xffffu));
            } else {
                int w_in=0,x_in=0;
                x=a->nregular?a->regular_order[11]:a->final_order[11];
                for(j=0u;j<12u;++j){if(an->entrants[j].club.stable_club==w)w_in=1;if(an->entrants[j].club.stable_club==x)x_in=1;}
                assert(w_in && !x_in);
                ++s_dyn;
                printf("[cycle] year %u: promoted %u (Championship winner), relegated %u (12th)\n",(unsigned)y,(unsigned)(w&0xffffu),(unsigned)(x&0xffffu));
            }
        }
    }
    /* ===== Championship: 16 distinct entrants, the dynamic ones are the premiers of the SAME season ===== */
    for(y=2026u;y<=2030u;++y) {
        const ExpCeEdition *c=edition_of(CH,(uint16_t)y);
        uint32_t j,q;
        if(!c) {                                   /* only after a blocked edition (no successor is created) */
            {   uint32_t py;int any_blocked=0;             /* a blocked edition never gets a successor */
                for(py=2026u;py<y;++py) {
                    const ExpCeEdition *pc=edition_of(CH,(uint16_t)py);
                    if(pc && pc->block)any_blocked=1;
                }
                assert(y>2026u && any_blocked);
            }
            continue;
        }
        if(c->block)continue;                       /* counted above; its cause is a calendar collision */
        assert(c->nentrants==16u);
        for(j=0u;j<c->nentrants;++j)for(q=j+1u;q<c->nentrants;++q)
            assert(c->entrants[j].club.stable_club!=c->entrants[q].club.stable_club);
        for(j=0u;j<c->nentrants;++j) {
            const ExpCeEntrant *en=&c->entrants[j];
            if(en->source==EXP_CE_SRC_LEAGUE_RANK || en->source==EXP_CE_SRC_REALLOCATED) {
                const uint32_t lgs[3]={NSW,VIC,QLD};
                int from_same_season=0;
                uint32_t li;
                for(li=0u;li<3u;++li) {
                    const ExpCeEdition *l=edition_of(lgs[li],(uint16_t)y);
                    uint32_t r;
                    if(!l)continue;
                    for(r=0u;r<l->nentrants;++r)
                        if(l->entrants[r].club.stable_club==en->club.stable_club)from_same_season=1;
                }
                assert(from_same_season);              /* a premier of the season, not of an older one */
            }
        }
    }
    /* ===== Australia Cup: 32 distinct entrants every year, no NZ club, one champion slot ===== */
    for(y=2026u;y<=2030u;++y) {
        const ExpCeEdition *c=edition_of(AC,(uint16_t)y);
        uint32_t j,q,champ=0u;
        assert(c && c->nentrants==32u && !c->block);
        for(j=0u;j<32u;++j) {
            assert(c->entrants[j].club.stable_club!=((44u<<16)|3u) && c->entrants[j].club.stable_club!=((44u<<16)|8u));
            for(q=j+1u;q<32u;++q)assert(c->entrants[j].club.stable_club!=c->entrants[q].club.stable_club);
            if(c->entrants[j].source==EXP_CE_SRC_CUP_WINNER)++champ;
        }
        assert(y==2026u || champ==1u);
    }
    printf("[cycle] exchange years with a promotion/relegation: %u, years with the Championship blocked by a collision: %u\n",(unsigned)s_dyn,(unsigned)s_champ_blocked);
    printf("[cycle] checks=%u restores=%u archived_with_trophy=%u dupes=%u admin_results=%u\n",
           s_checks,s_restores,s_exchanges,s_dupes,s_admin);
    test_extra_rounds_keep_26_games();
    puts("EXTRA_ROUNDS_156_GAMES_26_PER_CLUB_13_HOME_13_AWAY_WITH_ABSENT_RIVALS=PASS");
    puts("AUSTRALIA_CYCLE_FIVE_YEARS_REAL_DEFINITIONS_INTEGRITY_RESTORE_TROPHIES=PASS");
    return 0;
}
