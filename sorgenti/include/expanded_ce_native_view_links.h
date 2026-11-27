#ifndef EXPANDED_CE_NATIVE_VIEW_LINKS_H
#define EXPANDED_CE_NATIVE_VIEW_LINKS_H
/* Read-only presentation adapter included by diy_competition.c.
 * No CE rules, scheduling, result, roster or CAR writer. The UI links
 * file identifies display roles only; an empty phase array is pending.
 * A phase index is never a fixture id. The original cup menu supplies
 * its six-row renderer and scrolling; no fake packed cup is created. */
#include "expanded_ce_viewer.h"
#include "mjson.h"
#include "swos_io.h"
#include <stdio.h>
#include <limits.h>

enum { CE_X_QUAL,CE_X_SPAREGGI,CE_X_PLAYOFF,CE_X_PLAYOUT,CE_X_POOL,CE_X_COUNT };
static const char *const ce_x_name[CE_X_COUNT]={
    "QUAL","SPAREGGI","PLAYOFF","PLAYOUT","POOL"};
static const char *const ce_x_path[CE_X_POOL]={
    "$.qual","$.spareggi","$.playoff","$.playout"};
static const unsigned ce_x_width[CE_X_COUNT]={30u,50u,44u,44u,30u};
typedef struct CeExtraLinks {
    ExpandedEditionKey edition;
    uint8_t visible[CE_X_COUNT],phases[CE_X_POOL];
    uint32_t pool[64],npool;
    char title[40];
} CeExtraLinks;
static CeExtraLinks s_ceExtra;
typedef struct CeNativeMatchRow {
    char home[18],away[18],detail[56];
    uint16_t gh,ga;
    uint8_t played;
    int32_t day;
    uint32_t stage,tie;
    uint16_t leg,replay;
} CeNativeMatchRow;
static struct {
    CeNativeMatchRow *rows;
    uint32_t n,scroll;
    int active;
    ExpandedEditionKey edition;
    char title[64],status[80];
} s_ceNativeCup;

static char *ce_x_read(const char *rel,int *len)
{
    char path[1024],*p=NULL;
    FILE *f=fopen(swos_data_path(rel,path,(uint32_t)sizeof path),"rb");
    long n=0;
    *len=0;
    if(!f)return NULL;
    if(fseek(f,0,SEEK_END)==0 && (n=ftell(f))>0 && n<INT_MAX &&
       fseek(f,0,SEEK_SET)==0 && (p=(char *)malloc((size_t)n+1u))!=NULL) {
        if(fread(p,1,(size_t)n,f)!=(size_t)n){free(p);p=NULL;}
        else {p[n]=0;*len=(int)n;}
    }
    fclose(f);return p;
}
static int ce_x_num(const char *j,int n,const char *path,unsigned max,unsigned *v)
{
    double d=0;
    if(mjson_get_number(j,n,path,&d)<=0 || d<0 || d>(double)max ||
       d!=(double)(unsigned)d)return 0;
    *v=(unsigned)d;return 1;
}
static int ce_x_row(const char *j,int n,const ExpCeDef *d)
{
    unsigned c=0,k=0,v=0;
    return ce_x_num(j,n,"$.country",254u,&c) &&
           ce_x_num(j,n,"$.comp_id",255u,&k) &&
           ce_x_num(j,n,"$.division",255u,&v) &&
           c==d->country && k==d->comp_id && v==d->division;
}
static int ce_x_role_data(CeExtraLinks *x,const ExpCeDef *d)
{
    int n=0,an=0,o=0,k,kl,v,vl,t,found=0,ok=1;
    char *j=ce_x_read("data_json/worlds/2627/competitions/ce_viewer_links.json",&n);
    const char *a=NULL;
    unsigned schema=0;
    if(!j)return 1;
    if(!ce_x_num(j,n,"$.schema",1u,&schema) || schema!=1u ||
       mjson_find(j,n,"$.leagues",&a,&an)!=MJSON_TOK_ARRAY){free(j);return 0;}
    while((o=mjson_next(a,an,o,&k,&kl,&v,&vl,&t))>0) {
        const char *row=a+v;
        if(t!=MJSON_TOK_OBJECT || !ce_x_row(row,vl,d))continue;
        if(found++){ok=0;break;}
        for(unsigned r=0;r<CE_X_POOL;++r) {
            const char *p=NULL;int pn=0,po=0,pk,pkl,pv,pvl,pt;
            int token=mjson_find(row,vl,ce_x_path[r],&p,&pn);
            if(token==MJSON_TOK_INVALID)continue;
            if(token!=MJSON_TOK_ARRAY){ok=0;break;}
            x->visible[r]=1;
            while((po=mjson_next(p,pn,po,&pk,&pkl,&pv,&pvl,&pt))>0) {
                unsigned ph=0;
                if(pt!=MJSON_TOK_NUMBER ||
                   !ce_x_num(p+pv,pvl,"$",EXP_CE_MAX_PHASES-1u,&ph) ||
                   ph==0u || ph>=d->nphases ||
                   d->phases[ph].kind!=EXP_CE_PHASE_KNOCKOUT ||
                   (x->phases[r]&(1u<<ph))){ok=0;break;}
                x->phases[r]|=(uint8_t)(1u<<ph);
            }
            if(po<0 || !ok){ok=0;break;}
        }
        if(!ok)break;
        /* A sporting phase must never be offered twice under two labels. */
        for(unsigned r=0;r<CE_X_POOL;++r)
            for(unsigned q=r+1u;q<CE_X_POOL;++q)
                if(x->phases[r]&x->phases[q])ok=0;
    }
    free(j);return ok && o>=0;
}
/* Read the canonical pool scope and keys, not the native TEAM league field.
 * Until CE exposes durable cross-pyramid reserve membership this adapter
 * can show only the initial pool, and must refuse any later edition. */
static int ce_x_pool_data(CeExtraLinks *x,const ExpCeDef *d)
{
    int n=0,an=0,o=0,k,kl,v,vl,t,found=0,ok=1;
    char *j=ce_x_read("data_json/worlds/2627/competitions/bottom_pools.json",&n);
    const char *a=NULL;
    unsigned schema=0;
    if(!j)return 1;
    if(!ce_x_num(j,n,"$.schema",1u,&schema) || schema!=1u ||
       mjson_find(j,n,"$.pyramids",&a,&an)!=MJSON_TOK_ARRAY){free(j);return 0;}
    while((o=mjson_next(a,an,o,&k,&kl,&v,&vl,&t))>0) {
        const char *row=a+v,*p=NULL,*b=NULL;
        int pn=0,bn=0,po=0,pk,pkl,pv,pvl,pt;
        if(t!=MJSON_TOK_OBJECT || !ce_x_row(row,vl,d))continue;
        if(found++){ok=0;break;}
        if(mjson_find(row,vl,"$.bottom",&b,&bn)==MJSON_TOK_INVALID ||
           bn!=4 || memcmp(b,"true",4u))continue;
        if(mjson_find(row,vl,"$.game.pool",&p,&pn)!=MJSON_TOK_ARRAY){ok=0;break;}
        while((po=mjson_next(p,pn,po,&pk,&pkl,&pv,&pvl,&pt))>0) {
            char id[24],tail;
            unsigned country,slot;
            uint32_t stable;
            if(pt!=MJSON_TOK_STRING || pvl<5 || pvl>(int)sizeof id ||
               x->npool>=64u){ok=0;break;}
            memcpy(id,p+pv+1,(size_t)pvl-2u);id[pvl-2]=0;
            if(sscanf(id,"%u:%u%c",&country,&slot,&tail)!=2 ||
               country>254u || !slot || slot>65535u){ok=0;break;}
            stable=(country<<16)|slot;
            for(unsigned z=0;z<x->npool;++z)if(x->pool[z]==stable)ok=0;
            if(!ok)break;
            x->pool[x->npool++]=stable;
        }
        if(po<0 || !ok){ok=0;break;}
        x->visible[CE_X_POOL]=(uint8_t)(x->npool!=0u);
    }
    free(j);return ok && o>=0;
}
static void ce_board_extra_load(ExpandedEditionKey key,const char *label)
{
    const ExpCeDef *d=ExpCe_Definition(key.competition.stable_competition);
    CeExtraLinks tmp={0};
    memset(&s_ceExtra,0,sizeof s_ceExtra);
    if(!d || d->world!=key.competition.world || d->world!=WORLD_2627 ||
       d->origin!=EXP_CE_ORIGIN_LEAGUE)return;
    tmp.edition=key;
    snprintf(tmp.title,sizeof tmp.title,"%.39s",label?label:d->name);
    if(!ce_x_role_data(&tmp,d) || !ce_x_pool_data(&tmp,d)) {
        fprintf(stderr,"[ce-native-links] INVALID_UI_LINKS def=%u readonly=1\n",(unsigned)d->id);
        return;
    }
    s_ceExtra=tmp;
}
static void ce_native_cup_rules(MenuEntry *entry)
{
    (void)entry;
    if(s_ceNativeCup.active)(void)ExpCeViewer_OpenBasicRules(s_ceNativeCup.edition);
    DrawMenu();
}
static void ce_native_cup_draw(void)
{
    unsigned i,k;
    _menuEntries[0].fg=(uintptr_t)s_ceNativeCup.title;
    for(i=1u;i<=15u;++i)_menuEntries[i].isInvisible=1;
    for(i=16u;i<=17u;++i) {
        _menuEntries[i].isInvisible=(uint16_t)(s_ceNativeCup.n<=6u);
        _menuEntries[i].rightEntry=10;
    }
    _menuEntries[10].isInvisible=0;
    _menuEntries[9].fg=(uintptr_t)"RULES";
    _menuEntries[9].onSelect=ce_native_cup_rules;
    _menuEntries[9].isInvisible=_menuEntries[9].isDisabled=0;
    _menuEntries[9].leftEntry=s_ceNativeCup.n>6u?17:-1;
    _menuEntries[9].rightEntry=10;
    _menuEntries[9].skipLeftEntry=_menuEntries[9].skipRightEntry=-1;
    _menuEntries[10].leftEntry=9;
    _menuEntries[10].upEntry=s_ceNativeCup.n>6u?17:-1;
    for(i=0;i<6u;++i) {
        MenuEntry *e=&_menuEntries[18u+i*8u];
        const uint32_t at=s_ceNativeCup.scroll+i;
        for(k=0;k<8u;++k)e[k].isInvisible=1;
        if(at>=s_ceNativeCup.n)continue;
        const CeNativeMatchRow *m=&s_ceNativeCup.rows[at];
        e[0].fg=(uintptr_t)(m->played?"-":"V");
        e[1].fg=(uintptr_t)m->home;e[3].fg=(uintptr_t)m->away;
        e[7].fg=(uintptr_t)m->detail;
        e[0].isInvisible=e[1].isInvisible=e[3].isInvisible=e[7].isInvisible=0;
        if(m->played) {
            e[5].fgWord=m->gh;e[6].fgWord=m->ga;
            e[5].isInvisible=e[6].isInvisible=0;
        }
    }
    if(!s_ceNativeCup.n) {
        _menuEntries[25].fg=(uintptr_t)s_ceNativeCup.status;
        _menuEntries[25].isInvisible=0;
    }
}
static int ce_native_cup_cmp(const void *a,const void *b)
{
    const CeNativeMatchRow *x=(const CeNativeMatchRow *)a,*y=(const CeNativeMatchRow *)b;
#define CE_X_CMP(k) if(x->k!=y->k)return x->k<y->k?-1:1
    CE_X_CMP(day);CE_X_CMP(stage);CE_X_CMP(tie);CE_X_CMP(leg);CE_X_CMP(replay);
#undef CE_X_CMP
    return 0;
}
static void ce_x_error(const char *msg)
{
    extern void ShowErrorMenu(uint8_t *);
    static uint8_t text[96];
    snprintf((char *)text,sizeof text,"%s",msg);
    ShowErrorMenu(text);
}
static void ce_native_cup_open(ExpandedEditionKey key,uint8_t mask,
                              const char *label,const char *role)
{
    const ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    const ExpCeDef *d=ExpCe_Definition(key.competition.stable_competition);
    const ExpCeEdition *ed=ExpCe_Edition(key);
    uint32_t i,n=0;
    CeNativeMatchRow *rows=NULL;
    if(s_ceNativeCup.active || !r || r->recovery_required ||
       r->fixture_count!=r->ledger.count || !d || !ed)return;
    memset(&s_ceNativeCup,0,sizeof s_ceNativeCup);
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        unsigned p=EXP_CE_STAGE_PHASE(f->key.stage);
        if(ExpandedEditionKey_Equal(f->key.edition,key) &&
           p<d->nphases && p<EXP_CE_MAX_PHASES && (mask&(1u<<p)))++n;
    }
    if(n) {
        rows=(CeNativeMatchRow *)calloc(n,sizeof *rows);
        if(!rows){ce_x_error("FIXTURE DATA UNAVAILABLE");return;}
    }
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        unsigned p=EXP_CE_STAGE_PHASE(f->key.stage);
        if(!ExpandedEditionKey_Equal(f->key.edition,key) ||
           p>=d->nphases || p>=EXP_CE_MAX_PHASES || !(mask&(1u<<p)))continue;
        const ExpandedFixtureLedgerEntry *e=&r->ledger.entries[i];
        const TeamFileEx *h=ExpandedUniversalCatalog_Roster(f->home);
        const TeamFileEx *a=ExpandedUniversalCatalog_Roster(f->away);
        ExpandedWorldDate date={0};
        CeNativeMatchRow *m=&rows[s_ceNativeCup.n++];
        if(!ExpandedFixtureKey_Equal(e->key,f->key) || !h || !a ||
           !ExpandedWorld_IndexToDate(f->day_index,&date)) {
            free(rows);memset(&s_ceNativeCup,0,sizeof s_ceNativeCup);
            ce_x_error("FIXTURE DATA UNAVAILABLE");return;
        }
        snprintf(m->home,sizeof m->home,"%.17s",(const char *)h->team.teamName);
        snprintf(m->away,sizeof m->away,"%.17s",(const char *)a->team.teamName);
        m->day=f->day_index;m->stage=f->key.stage;m->tie=f->key.tie;
        m->leg=f->key.leg;m->replay=f->key.replay;
        m->played=(uint8_t)(e->state==EXP_FIXTURE_PLAYED);
        if(m->played){m->gh=e->score.home;m->ga=e->score.away;}
        snprintf(m->detail,sizeof m->detail,"%02u/%02u/%04u R%u L%u",
                 (unsigned)date.day,(unsigned)date.month,(unsigned)date.year,
                 p+1u,(unsigned)f->key.leg+1u);
        if(m->played && (e->score.flags&EXP_CE_SCORE_EXTRA_TIME))
            snprintf(m->detail+strlen(m->detail),sizeof m->detail-strlen(m->detail)," ET");
        if(m->played && (e->score.flags&EXP_CE_SCORE_PENALTIES))
            snprintf(m->detail+strlen(m->detail),sizeof m->detail-strlen(m->detail),
                     " P %u-%u",(unsigned)e->score.penalties_home,(unsigned)e->score.penalties_away);
    }
    if(n>1u)qsort(rows,n,sizeof *rows,ce_native_cup_cmp);
    s_ceNativeCup.rows=rows;
    s_ceNativeCup.edition=key;
    if(role && *role)
        snprintf(s_ceNativeCup.title,sizeof s_ceNativeCup.title,"%.26s %.9s %u",
                 label,role,(unsigned)ed->year);
    else snprintf(s_ceNativeCup.title,sizeof s_ceNativeCup.title,"%.47s",label);
    snprintf(s_ceNativeCup.status,sizeof s_ceNativeCup.status,"%s",
        !mask || ed->block || ed->state>=EXP_CE_ED_COMPLETED?
        "FIXTURE DATA UNAVAILABLE":"COMPETITION NOT STARTED - START DATE UNAVAILABLE");
    s_ceNativeCup.active=1;
    ShowMenu(_diyCupMenu);
    s_ceNativeCup.active=0;
    free(rows);memset(&s_ceNativeCup,0,sizeof s_ceNativeCup);
}
/* ------------------------------------------------------------------ */
/* v00.13.342 (user 26/11/2026): ORIGINAL OG CUP BOARD for the manager   */
/* knockout fixture (cups, super cups, league finals/playoffs). Same flow */
/* as OG: PLAY MATCH -> cup round (results so far, "V" still to play) -> */
/* MATCH = OG team screen + 50 Hz, RESULT = OG team screen with VIEW     */
/* RESULT + OG quick; back on the board with the result; NEXT MATCH walks */
/* the remaining ties of the day (VIEW RESULT, OG quick). The CE stays the */
/* only writer: every action is an existing ExpandedUniversalLive call.   */
/* ------------------------------------------------------------------ */
static struct {
    int active,committed,after;
    uint64_t manager,cur;
    uint64_t seq[64];
    unsigned nseq;
    ExpandedEditionKey edition;
    uint32_t stage;
    uint16_t leg;
    char t1[18],t2[18];
    uint16_t s1,s2;
} s_ceCupBoard;
static int ce_cup_board_is_active(void){return s_ceCupBoard.active;}
extern char _aStats[];
static void ce_cup_board_stats_select(MenuEntry *e);
static int ce_cup_board_rows(void)
{
    const ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    uint32_t i,n=0u;
    CeNativeMatchRow *rows;
    if(!r || r->fixture_count!=r->ledger.count)return 0;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        if(ExpandedEditionKey_Equal(f->key.edition,s_ceCupBoard.edition) &&
           f->key.stage==s_ceCupBoard.stage && f->key.leg==s_ceCupBoard.leg)++n;
    }
    rows=n?(CeNativeMatchRow *)calloc(n,sizeof *rows):NULL;
    if(n && !rows)return 0;
    n=0u;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *e=&r->ledger.entries[i];
        const TeamFileEx *h,*a;
        CeNativeMatchRow *m;
        if(!ExpandedEditionKey_Equal(f->key.edition,s_ceCupBoard.edition) ||
           f->key.stage!=s_ceCupBoard.stage || f->key.leg!=s_ceCupBoard.leg)continue;
        h=ExpandedUniversalCatalog_Roster(f->home);a=ExpandedUniversalCatalog_Roster(f->away);
        if(!h || !a){free(rows);return 0;}
        m=&rows[n++];
        snprintf(m->home,sizeof m->home,"%.17s",(const char *)h->team.teamName);
        snprintf(m->away,sizeof m->away,"%.17s",(const char *)a->team.teamName);
        m->day=f->day_index;m->stage=f->key.stage;m->tie=f->key.tie;
        m->leg=f->key.leg;m->replay=f->key.replay;
        m->played=(uint8_t)(e->state==EXP_FIXTURE_PLAYED);
        if(m->played){m->gh=e->score.home;m->ga=e->score.away;}
    }
    if(n>1u)qsort(rows,n,sizeof *rows,ce_native_cup_cmp);
    free(s_ceNativeCup.rows);
    s_ceNativeCup.rows=rows;s_ceNativeCup.n=n;
    if(s_ceNativeCup.scroll+6u>n)s_ceNativeCup.scroll=n>6u?n-6u:0u;
    return 1;
}
static void ce_cup_board_names(uint64_t id)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    const ExpandedUniversalFixture *f=r?ExpandedUniversal_Find(r,id):NULL;
    const TeamFileEx *h=f?ExpandedUniversalCatalog_Roster(f->home):NULL;
    const TeamFileEx *a=f?ExpandedUniversalCatalog_Roster(f->away):NULL;
    snprintf(s_ceCupBoard.t1,sizeof s_ceCupBoard.t1,"%.17s",h?(const char *)h->team.teamName:"");
    snprintf(s_ceCupBoard.t2,sizeof s_ceCupBoard.t2,"%.17s",a?(const char *)a->team.teamName:"");
}
static void ce_cup_board_draw(void)
{
    unsigned i;
    ce_native_cup_draw();                       /* rows, title, scroll, EXIT */
    for(i=0;i<6u;++i)_menuEntries[18u+i*8u+7u].isInvisible=1; /* OG rows: no detail line */
    /* OG STATS: original competition stats menu on the clubs of the CE edition */
    _menuEntries[9].fg=(uintptr_t)_aStats;
    _menuEntries[9].onSelect=ce_cup_board_stats_select;
    _menuEntries[9].isInvisible=_menuEntries[9].isDisabled=0;
    _menuEntries[9].leftEntry=-1;_menuEntries[9].rightEntry=10;
    _menuEntries[10].leftEntry=9;
    _menuEntries[11].isInvisible=_menuEntries[12].isInvisible=_menuEntries[13].isInvisible=0;
    _menuEntries[12].fg=(uintptr_t)s_ceCupBoard.t1;
    _menuEntries[13].fg=(uintptr_t)s_ceCupBoard.t2;
    if(s_ceCupBoard.after) {                    /* OG result header + NEXT MATCH */
        _menuEntries[11].fg=(uintptr_t)_aMinus;
        _menuEntries[14].fgWord=s_ceCupBoard.s1;_menuEntries[15].fgWord=s_ceCupBoard.s2;
        _menuEntries[14].isInvisible=_menuEntries[15].isInvisible=0;
        _menuEntries[1].fg=(uintptr_t)_aNextMatch;
        _menuEntries[1].isInvisible=0;
        SetCurrentEntry(1u);
        return;
    }
    _menuEntries[11].fg=(uintptr_t)_aV;
    _menuEntries[14].isInvisible=_menuEntries[15].isInvisible=1;
    if(s_ceCupBoard.cur==s_ceCupBoard.manager) {/* OG human tie: MATCH + RESULT */
        _menuEntries[3].isInvisible=0;
        _menuEntries[5].isInvisible=0;
        SetCurrentEntry(3u);
    } else {                                    /* OG CPU tie: VIEW RESULT only */
        _menuEntries[8].isInvisible=0;
        _menuEntries[8].x=_menuEntries[6].x;
        SetCurrentEntry(8u);
    }
}
static void ce_cup_board_redraw(void)
{
    unsigned i;
    for(i=1u;i<=15u;++i)_menuEntries[i].isInvisible=1;
    ce_cup_board_draw();
    DrawMenu();
}
static void ce_cup_board_action(unsigned mode)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    const ExpandedUniversalFixture *f;
    const ExpandedFixtureLedgerEntry *e;
    int rc;
    if(!s_ceCupBoard.active || s_ceCupBoard.after || !r)return;
    if(s_ceCupBoard.cur==s_ceCupBoard.manager) {
        if(mode==2u)rc=ExpandedUniversalLive_PlayManager();
        else if(mode==0u)rc=ExpandedUniversalLive_ViewManagerResult();
        else return;
        if(rc==2)s_ceCupBoard.committed=1;
    } else {
        if(mode!=0u)return;
        rc=ExpandedUniversalLive_ViewRoundCpuResult(s_ceCupBoard.cur);
    }
    f=ExpandedUniversal_Find(r,s_ceCupBoard.cur);
    e=f?ExpandedUniversal_EntryFor(r,f):NULL;
    if(e && e->state==EXP_FIXTURE_PLAYED) {
        s_ceCupBoard.after=1;
        s_ceCupBoard.s1=e->score.home;s_ceCupBoard.s2=e->score.away;
    }
    fprintf(stderr,"[universal-og-cup] ACTION mode=%u event=%llu rc=%d played=%d\n",
            mode,(unsigned long long)s_ceCupBoard.cur,rc,s_ceCupBoard.after);
    (void)ce_cup_board_rows();
    ce_cup_board_redraw();
}
static void ce_cup_board_next(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    unsigned i;
    if(!s_ceCupBoard.active || !r)return;
    for(i=0u;i<s_ceCupBoard.nseq;++i) {
        const ExpandedUniversalFixture *f=ExpandedUniversal_Find(r,s_ceCupBoard.seq[i]);
        const ExpandedFixtureLedgerEntry *e=f?ExpandedUniversal_EntryFor(r,f):NULL;
        if(e && e->state==EXP_FIXTURE_SCHEDULED) {
            s_ceCupBoard.cur=f->event_id;s_ceCupBoard.after=0;
            ce_cup_board_names(f->event_id);
            ce_cup_board_redraw();
            return;
        }
    }
    SetExitMenuFlag();                          /* OG: the round of the day is over */
}
/* OG STATS of the cup board: the ORIGINAL competition stats menu, fed with the
 * clubs of the CE edition (same as the CE league board does with its table).
 * Only the DIY fields read by SetupDIYCompetitionStatsMenu are written, on the
 * copy that ExpandedUniversal_OGCupBoardPresent restores when the board closes. */
static void ce_cup_board_stats(void)
{
    const ExpCeEdition *ed=ExpCe_Edition(s_ceCupBoard.edition);
    const ExpCeDef *d=ExpCe_Definition(s_ceCupBoard.edition.competition.stable_competition);
    DIYCompetitionEx *c=&_diyCompetitionStart;
    uint32_t i,n=0u;
    if(!s_ceCupBoard.active || !ed || !d)return;
    c->base.competitionType=DIY_CUP;
    c->base.hasGroups=0;
    snprintf((char *)c->base.competitionName,sizeof c->base.competitionName,"%.35s",d->name);
    for(i=0u;i<ed->nentrants && n<64u;++i) {
        const TeamFileEx *t=ExpandedUniversalCatalog_Roster(ed->entrants[i].club);
        if(!t)continue;
        BE_WRITE_U16((uint8_t *)&c->cupTeamIds[n],PACK_BE_U16(t->team.countryNumber,t->team.teamOrdinal));
        ++n;
    }
    if(!n)return;
    c->base.numTeams=(int16_t)n;
    SetupDIYCompetitionStatsMenu();
}
static void ce_cup_board_stats_select(MenuEntry *e){(void)e;ce_cup_board_stats();}
static int ce_cup_seq_cmp(const void *a,const void *b)
{
    const uint64_t x=*(const uint64_t *)a,y=*(const uint64_t *)b;
    return x<y?-1:x>y?1:0;
}
int ExpandedUniversal_OGCupBoardPresent(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    ExpandedWorldDate due={0},today_d={0};
    ExpandedClubKey manager={0};
    const ExpandedUniversalFixture *f=NULL,*g=NULL;
    const ExpCeDef *d;
    int32_t today=0;
    uint8_t kind=0u;
    uint32_t i,ties=0u,phase,legs;
    uint64_t pre=0u;
    char round[40];
    int result;
    if(s_ceCupBoard.active || s_ceNativeCup.active || !r || r->recovery_required ||
       _gameType!=4 || !ExpandedUniversalLive_Active() ||
       ExpandedUniversalLive_ManagerNext(&due)!=2 ||
       !ExpandedUniversalLive_Today(&today_d) ||
       !ExpandedWorld_DateToIndex(today_d,&today) ||
       !ExpandedUniversal_NativeTeamKey(&_careerTeam,&manager) ||
       !r->scheduler.count ||
       ExpandedUniversal_NextForClub(r,manager,
           r->scheduler.events[r->scheduler.count-1u].day_index,&f)!=EXP_UNIVERSAL_DUE ||
       !f || f->day_index!=today || !ExpCe_FixturePhase(f,&kind,NULL) ||
       kind!=EXP_CE_PHASE_KNOCKOUT ||
       !(d=ExpCe_Definition(f->key.edition.competition.stable_competition)))
        return 0;
    /* the next world event must be this round (earlier events: FORWARD) */
    if(ExpandedUniversal_Peek(r,today,&g)!=EXP_UNIVERSAL_DUE || !g ||
       !ExpandedEditionKey_Equal(g->key.edition,f->key.edition) ||
       g->key.stage!=f->key.stage || g->key.leg!=f->key.leg)
        return 0;
    memset(&s_ceCupBoard,0,sizeof s_ceCupBoard);
    memset(&s_ceNativeCup,0,sizeof s_ceNativeCup);
    s_ceCupBoard.edition=f->key.edition;s_ceCupBoard.stage=f->key.stage;
    s_ceCupBoard.leg=f->key.leg;s_ceCupBoard.manager=f->event_id;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *x=&r->fixtures[i];
        if(!ExpandedEditionKey_Equal(x->key.edition,f->key.edition) ||
           x->key.stage!=f->key.stage || x->key.leg!=f->key.leg)continue;
        ++ties;
        if(x->day_index!=today)continue;
        if(s_ceCupBoard.nseq>=64u)return 0;
        s_ceCupBoard.seq[s_ceCupBoard.nseq++]=x->event_id;
        if(x->event_id<f->event_id &&
           ExpandedUniversal_EntryFor(r,x)->state==EXP_FIXTURE_SCHEDULED &&
           x->event_id>pre)pre=x->event_id;
    }
    qsort(s_ceCupBoard.seq,s_ceCupBoard.nseq,sizeof s_ceCupBoard.seq[0],ce_cup_seq_cmp);
    /* OG opens on the manager tie: the ties of the day before it are already played */
    if(pre && ExpandedUniversalLive_ViewRoundCpuResult(pre)!=2) {
        fprintf(stderr,"[universal-og-cup] PRE_MANAGER_ROUND_BLOCKED stop=%llu\n",(unsigned long long)pre);
        return 0;
    }
    s_ceCupBoard.cur=f->event_id;
    ce_cup_board_names(f->event_id);
    phase=EXP_CE_STAGE_PHASE(f->key.stage);
    legs=phase==EXP_CE_PLAYOUT_PHASE?d->playout.match.legs:
         (phase<d->nphases?d->phases[phase].match.legs:1u);
    if(phase==EXP_CE_PLAYOUT_PHASE)snprintf(round,sizeof round,"PLAY-OUT");
    else if(ties==1u)snprintf(round,sizeof round,"FINAL");
    else if(ties==2u)snprintf(round,sizeof round,"SEMI-FINALS");
    else if(ties==4u)snprintf(round,sizeof round,"QUARTER-FINALS");
    else snprintf(round,sizeof round,"ROUND OF %u",(unsigned)(ties*2u));
    if(legs==2u)
        snprintf(round+strlen(round),sizeof round-strlen(round),"%s",f->key.leg?" 2ND LEG":" 1ST LEG");
    snprintf(s_ceNativeCup.title,sizeof s_ceNativeCup.title,"%.30s %s",
             d->name[0]?d->name:"LEAGUE",round);
    s_ceNativeCup.edition=f->key.edition;
    if(!ce_cup_board_rows()){memset(&s_ceCupBoard,0,sizeof s_ceCupBoard);return 0;}
    s_ceNativeCup.active=1;s_ceCupBoard.active=1;
    fprintf(stderr,"[universal-og-cup] OG_CUP_MENU_OPEN event=%llu ties=%u day-ties=%u "
        "source=CE_LEDGER CAR=UNCHANGED\n",(unsigned long long)f->event_id,
        (unsigned)ties,s_ceCupBoard.nseq);
    {   /* OG callbacks store the pressed ordinal in the career DIY slot: keep it intact */
        static DIYCompetitionEx saved;
        memcpy(&saved,&_diyCompetitionStart,sizeof saved);
        ShowMenu(_diyCupMenu);
        memcpy(&_diyCompetitionStart,&saved,sizeof saved);
    }
    result=s_ceCupBoard.committed?2:1;
    s_ceNativeCup.active=0;s_ceCupBoard.active=0;
    free(s_ceNativeCup.rows);memset(&s_ceNativeCup,0,sizeof s_ceNativeCup);
    memset(&s_ceCupBoard,0,sizeof s_ceCupBoard);
    fprintf(stderr,"[universal-og-cup] OG_CUP_MENU_EXIT result=%d no-legacy-writer=1\n",result);
    return result;
}
/* Same original cup renderer as the phase links, without a synthetic
 * role suffix. No sporting mutation, legacy slot or fake packed cup. */
int ExpCeViewer_OpenNativeCup(ExpandedEditionKey key)
{
    const ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    const ExpCeDef *d=ExpCe_Definition(key.competition.stable_competition);
    const ExpCeEdition *ed=ExpCe_Edition(key);
    uint8_t mask=0;
    uint32_t i;
    char title[48];
    ExpandedWorldDate expiry={0};
    if(!ExpCeViewer_Active() || !r || !d || !ed ||
       d->world!=key.competition.world ||
       (!d->viewer_button && !d->viewer_native_id_plus_one) ||
       d->type!=1u || !d->nphases || d->nphases>EXP_CE_MAX_PHASES ||
       ed->state>=EXP_CE_ED_ARCHIVED)return 0;
    for(i=0;i<d->nphases;++i) {
        if(d->phases[i].kind!=EXP_CE_PHASE_KNOCKOUT)return 0;
        mask|=(uint8_t)(1u<<i);
    }
    for(i=0;i<r->fixture_count;++i)
        if(ExpandedEditionKey_Equal(r->fixtures[i].key.edition,key))break;
    if(i==r->fixture_count)return 0;
    if(r->recovery_required || r->fixture_count!=r->ledger.count ||
       s_ceNativeCup.active) {
        ce_x_error("FIXTURE DATA UNAVAILABLE");
        return 1;
    }
    if(ed->expiry_day && ExpandedWorld_IndexToDate(ed->expiry_day,&expiry) &&
       expiry.year>ed->year)
        snprintf(title,sizeof title,"%.32s %u/%02u",d->name,
                 (unsigned)ed->year,(unsigned)(expiry.year%100u));
    else snprintf(title,sizeof title,"%.36s %u",d->name,(unsigned)ed->year);
    fprintf(stderr,"[ce-viewer] NATIVE_CUP def=%u year=%u readonly=1\n",
            (unsigned)d->id,(unsigned)ed->year);
    ce_native_cup_open(key,mask,title,NULL);
    return 1;
}
static void ce_x_pool_open(void)
{
    const ExpCeEdition *ed=ExpCe_Edition(s_ceExtra.edition);
    const ExpCeDef *d=ExpCe_Definition(s_ceExtra.edition.competition.stable_competition);
    uint32_t i,j,n=s_ceExtra.npool;
    char names[64][18],title[64];
    void *savedNames[64];uint8_t savedIndex[64];
    extern uintptr_t _chooseTeamMenuName;
    extern uint16_t _numTeamsCountryBuffer,_teamSelected;
    extern uint16_t _selTeamsColumn1,_selTeamsColumn2,_selTeamsColumn3;
    extern uint16_t _selTeamsListTopY,_selTeamsListBottomY;
    extern void *_dwArrayCountries[];
    extern uint8_t _indexCountries[],_selectTeamsForTacticsMenu[];
    const uintptr_t oldTitle=_chooseTeamMenuName;
    const uint16_t oldCount=_numTeamsCountryBuffer,oldSelected=_teamSelected;
    const uint16_t old1=_selTeamsColumn1,old2=_selTeamsColumn2,old3=_selTeamsColumn3;
    const uint16_t oldTop=_selTeamsListTopY,oldBottom=_selTeamsListBottomY;
    if(!ed || !d || !n || n>64u || ed->successor ||
       !ed->nentrants || ed->state>=EXP_CE_ED_COMPLETED)goto unavailable;
    /* No successor/transfer membership may be inferred from the initial list. */
    for(i=0;i<ed->nentrants;++i)
        if(ed->entrants[i].source!=EXP_CE_SRC_START_DIVISION)goto unavailable;
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *e=ExpCe_EditionAt(i);
        if(e && e->def_id==ed->def_id &&
           e->key.start_day<ed->key.start_day)goto unavailable;
    }
    for(i=0;i<n;++i) {
        ExpandedClubKey key={.world=ed->key.competition.world,.stable_club=s_ceExtra.pool[i]};
        const TeamFileEx *t=ExpandedUniversalCatalog_Roster(key);
        if(!t)goto unavailable;
        /* Check every live league edition, not only the displayed division. */
        for(j=0;j<ExpCe_EditionCount();++j) {
            const ExpCeEdition *other=ExpCe_EditionAt(j);
            const ExpCeDef *od=other?ExpCe_Definition(other->def_id):NULL;
            if(!other || !od || od->origin!=EXP_CE_ORIGIN_LEAGUE ||
               other->key.competition.world!=key.world || other->successor ||
               other->state>=EXP_CE_ED_ARCHIVED)continue;
            for(unsigned k=0;k<other->nentrants;++k)
                if(ExpandedClubKey_Equal(other->entrants[k].club,key))goto unavailable;
        }
        snprintf(names[i],sizeof names[i],"%.17s",(const char *)t->team.teamName);
    }
    /* OG team-list renderer, but no VerifyTeamLoaded / squad-edit callback.
     * All names are immutable catalogue copies and selecting one only exits. */
    memcpy(savedNames,_dwArrayCountries,n*sizeof *savedNames);
    memcpy(savedIndex,_indexCountries,n*sizeof *savedIndex);
    for(i=0;i<n;++i){_dwArrayCountries[i]=names[i];_indexCountries[i]=(uint8_t)i;}
    snprintf(title,sizeof title,"%.30s POOL %u",s_ceExtra.title,(unsigned)ed->year);
    _chooseTeamMenuName=(uintptr_t)title;_numTeamsCountryBuffer=(uint16_t)n;
    ShowMenu(_selectTeamsForTacticsMenu);
    memcpy(_dwArrayCountries,savedNames,n*sizeof *savedNames);
    memcpy(_indexCountries,savedIndex,n*sizeof *savedIndex);
    _chooseTeamMenuName=oldTitle;_numTeamsCountryBuffer=oldCount;_teamSelected=oldSelected;
    _selTeamsColumn1=old1;_selTeamsColumn2=old2;_selTeamsColumn3=old3;
    _selTeamsListTopY=oldTop;_selTeamsListBottomY=oldBottom;
    return;
unavailable:
    ce_x_error("POOL MEMBERSHIP DATA UNAVAILABLE");
}
static void ce_board_extra_select(MenuEntry *entry)
{
    unsigned role;
    if(!s_ceBoardActive || !s_ceBoardViewOnly)return;
    for(role=0;role<CE_X_COUNT;++role)
        if(entry->fg==(uintptr_t)ce_x_name[role] && s_ceExtra.visible[role])break;
    if(role==CE_X_COUNT)return;
    if(role==CE_X_POOL)ce_x_pool_open();
    else ce_native_cup_open(s_ceExtra.edition,s_ceExtra.phases[role],
                            s_ceExtra.title,ce_x_name[role]);
    _fireResetFlag=1;
    DrawMenu();
}
static void ce_board_extra_buttons(void)
{
    unsigned roles[CE_X_COUNT],n=0,total=0,i;
    const MenuEntry stats=_menuEntries[9];
    for(i=0;i<CE_X_COUNT;++i)if(s_ceExtra.visible[i]) {
        roles[n++]=i;total+=ce_x_width[i]+1u;
    }
    if(!n)return;
    /* Existing OG view-only action slots 1..8 are hidden. Clone STATS
     * style into the rightmost slots, preserving original entry ordinals.
     * No change to interactive manager actions or STATS/EXIT geometry. */
    if(stats.x<24u+total) {
        fprintf(stderr,"[ce-native-links] BUTTON_CAPACITY needed=%u available=%u\n",
                total,(unsigned)stats.x);
        return;
    }
    unsigned first=9u-n,x=(unsigned)stats.x-total;
    for(i=0;i<n;++i) {
        unsigned slot=first+i,role=roles[i];
        MenuEntry *e=&_menuEntries[slot];
        *e=stats;e->ordinal=(uint16_t)slot;e->alreadyDrawn=0;
        e->x=(uint16_t)x;e->dwWidth=(uint16_t)ce_x_width[role];
        e->fg=(uintptr_t)ce_x_name[role];e->onSelect=ce_board_extra_select;
        e->isInvisible=e->isDisabled=0;
        e->leftEntry=i?(int8_t)(slot-1u):-1;
        e->rightEntry=(int8_t)(slot+1u);
        e->skipLeftEntry=e->skipRightEntry=-1;
        x+=ce_x_width[role]+1u;
    }
    _menuEntries[9].leftEntry=8;
}
#endif
