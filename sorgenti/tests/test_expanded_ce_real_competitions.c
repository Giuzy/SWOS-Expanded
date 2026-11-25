/* v00.13.301 RG-T: the REAL ce_competitions.json definitions of the approved
 * competitions run through the SAME generic Competition Engine (no C code
 * per tournament). Isolated test: real engine/registry/CAR codec, MOCKED OG
 * match (deterministic) and disk. Not gameplay, not Windows. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_car.h"
#include "expanded_ce_engine.h"
#include "expanded_ce_admin.h"
#include "expanded_ce_table.h"

static WorldId s_world=WORLD_2627;
WorldId World_Current(void){return s_world;}
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
void *swos_io_alloc(uint32_t n){return malloc(n);}
void swos_io_free(void *p){free(p);}
unsigned long long Platform_NowMs(void){return 0;}
void ExpandedUniversalLive_ResetCpuBatch(void){}
void ExpandedUniversalLive_RegisterClock(void){}
int CarWriter_TakeFailure(void){return 0;}
int Expanded_AutoCareer_SaveChecked(const char *reason)
{
    static char buf[1<<22];
    struct mjson_fixedbuf dst={buf,(int)sizeof buf,0};
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    (void)reason;
    assert(r);
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"{%Q:{%Q:%d}","living_world",
        "now",(int)r->scheduler.clock.day_index)>0);
    assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst));
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"}")>0);
    assert(ExpandedUniversal_CarValidate(buf,dst.len));
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
static unsigned s_games,s_admin,s_real;
/* OG mock: a level 1-1 after 120 minutes decided on penalties, as the OG
 * adapters do for a single-leg knockout */
static int og_mock(void *u,const ExpandedUniversalFixture *f,ExpandedUniversalMatchWitness *w)
{
    ExpandedFixtureScore admin;
    const char *why=NULL;
    (void)u;++s_games;
    memset(w,0,sizeof *w);
    w->key=f->key;w->home=f->home;w->away=f->away;w->day_index=f->day_index;
    if(ExpCeAdmin_Decide(f,&admin,&why)) {      /* same hook as the native runner */
        ++s_admin;
        w->score=admin;w->valid=1u;
        return 1;
    }
    ++s_real;
    w->score=(ExpandedFixtureScore){1u,1u,5u,4u,
        (uint8_t)(EXP_CE_SCORE_EXTRA_TIME|EXP_CE_SCORE_PENALTIES)};
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
static char *slurp(const char *path,int *len)
{
    FILE *f=fopen(path,"rb");
    char *b;long n;
    assert(f);fseek(f,0,SEEK_END);n=ftell(f);fseek(f,0,SEEK_SET);
    b=(char *)malloc((size_t)n+1u);assert(b && fread(b,1,(size_t)n,f)==(size_t)n);
    b[n]=0;fclose(f);*len=(int)n;return b;
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
int main(void)
{
    int len=0;
    char *json=slurp("../data_json/worlds/2627/competitions/ce_competitions.json",&len);
    const uint32_t SUPERCOPPA=1310978u;
    const ExpCeDef *def;
    const ExpCeEdition *e;
    uint32_t i,fx=0u;
    ExpandedUniversal_CarResetCurrent();
    ExpCe_Reset();
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
    assert(ExpCe_LoadJsonDefinitions(json,len,WORLD_2627,"ce_competitions.json")>=3);
    free(json);
    {   int tl=0;
        char *tj=slurp("../data_json/worlds/2627/competitions/test_clubs.json",&tl);
        assert(ExpCeAdmin_LoadJson(tj,tl,WORLD_2627)==12);
        free(tj);
    }
    def=ExpCe_Definition(SUPERCOPPA);
    assert(def && !def->missing[0] && def->type==1u && def->country==20u &&
           def->nphases==1u && def->nfirst==2u);
    assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));

    /* ===== Supercoppa Italiana: first competition added only through data */
    e=edition_of(SUPERCOPPA,2026);
    assert(e && !e->block);
    run_until(D(2026,12,21)); /* phase fixtures are created when the phase opens */
    for(i=0;i<R()->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&R()->fixtures[i];
        if(f->key.edition.competition.stable_competition!=SUPERCOPPA)continue;
        ++fx;
        assert(f->day_index==D(2026,12,22));                  /* template date */
        assert(f->home.stable_club==((20u<<16)|1u) &&         /* Inter */
               f->away.stable_club==((20u<<16)|9u));          /* Lazio */
        assert(f->country==20u);
    }
    assert(fx==1u);                                           /* one final */
    s_games=0u;
    run_until(D(2026,12,31));
    assert(s_games>=1u);
    e=edition_of(SUPERCOPPA,2026);
    assert(e && e->state>=EXP_CE_ED_COMPLETED && e->nfinal>=2u);
    /* the edition expires on 21/12/2027 (its own cycle, never 1 July) */
    run_until(D(2027,12,22));
    e=edition_of(SUPERCOPPA,2026);
    assert(e && e->state==EXP_CE_ED_ARCHIVED);
    assert(ExpandedUniversal_ArchiveContains(e->key));        /* archived once */
    {
        const ExpandedEditionSummary *s=NULL;
        for(i=0;i<ExpandedUniversal_ArchiveCount();++i) {
            const ExpandedEditionSummary *a=ExpandedUniversal_ArchiveAt(i);
            if(a->edition.competition.stable_competition==SUPERCOPPA)s=a;
        }
        assert(s && s->match_count==1u && s->champion==((20u<<16)|1u) &&
               s->runner_up==((20u<<16)|9u));                 /* penalties 5-4 */
    }
    /* renewal: only Serie A winner + Coppa Italia winner/runner-up per the
     * verified regulation, but Coppa Italia is not defined in the CE yet: the
     * 2027 edition must NOT invent participants */
    e=edition_of(SUPERCOPPA,2027);
    assert(e && e->block==EXP_CE_BLOCK_ENTRANTS && e->state==EXP_CE_ED_CREATED);
    assert(ExpandedUniversal_ValidateRestore(R()));
    /* ===== Australia Cup: 32 entrants (20 real + 12 TEST), 31 knockout games */
    {
        const uint32_t AC=2884866u;
        uint32_t per_round[6]={0},played=0u,adm=0u,tvr=0u,tvt=0u;
        const ExpCeDef *ad=ExpCe_Definition(AC);
        const ExpCeEdition *ae=edition_of(AC,2026);
        assert(ad && !ad->missing[0] && ad->nphases==5u && ad->nfirst==32u);
        assert(ae && ae->state==EXP_CE_ED_ARCHIVED && ae->nfinal>=2u);
        assert(ExpandedUniversal_ArchiveContains(ae->key));
        for(i=0;i<R()->fixture_count;++i) {
            const ExpandedUniversalFixture *f=&R()->fixtures[i];
            const ExpandedFixtureLedgerEntry *l=&R()->ledger.entries[i];
            int th,ta;
            if(f->key.edition.competition.stable_competition!=AC)continue;
            assert(l->state==EXP_FIXTURE_PLAYED && EXP_CE_STAGE_PHASE(f->key.stage)<=4u);
            ++played;++per_round[EXP_CE_STAGE_PHASE(f->key.stage)+1u];
            th=ExpCeAdmin_IsTest(f->home);ta=ExpCeAdmin_IsTest(f->away);
            if(th||ta) {
                assert(l->score.flags&EXP_CE_SCORE_ADMIN_TEST);
                assert(!(l->score.flags&3u));   /* decided without ET/penalties */
                ++adm;
                if(th!=ta) {                    /* the real club ALWAYS wins */
                    ++tvr;
                    assert(th?(l->score.home==0u&&l->score.away==3u)
                             :(l->score.home==3u&&l->score.away==0u));
                } else {++tvt;assert(l->score.home+l->score.away==1u);}
            } else assert(!(l->score.flags&EXP_CE_SCORE_ADMIN_TEST));
        }
        assert(played==31u && per_round[1]==16u && per_round[2]==8u &&
               per_round[3]==4u && per_round[4]==2u && per_round[5]==1u);
        assert(adm>0u && s_admin>=adm && s_real>=1u);   /* Supercoppa is a real match */
        printf("AUSTRALIA_CUP played=%u admin=%u test_vs_real=%u test_vs_test=%u\n",
               played,adm,tvr,tvt);
        /* the 2027 edition cannot invent its 32 participants */
        ae=edition_of(AC,2027);
        assert(ae && ae->block==EXP_CE_BLOCK_ENTRANTS);
    }
    puts("AUSTRALIA_CUP_REAL_JSON_32_ENTRANTS_31_GAMES_TEST_ADMIN_RESULTS=PASS");
    puts("SUPERCOPPA_ITALIANA_REAL_JSON_FIRST_EDITION_ARCHIVE_NO_INVENTED_RENEWAL=PASS");
    puts("OG_QUICK_50HZ_WINDOWS_GAMEPLAY=NOT_TESTED");
    return 0;
}
