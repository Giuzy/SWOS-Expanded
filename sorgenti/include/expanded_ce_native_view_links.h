#ifndef EXPANDED_CE_NATIVE_VIEW_LINKS_H
#define EXPANDED_CE_NATIVE_VIEW_LINKS_H
/* Read-only presentation adapter included by diy_competition.c.
 * No CE rules, scheduling, result, roster or CAR writer. The UI links
 * file identifies display roles only; an empty phase array is pending.
 * A phase index is never a fixture id. The original cup menu supplies
 * its six-row renderer and scrolling; no fake packed cup is created. */
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
    _menuEntries[10].leftEntry=s_ceNativeCup.n>6u?17:-1;
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
    snprintf(s_ceNativeCup.title,sizeof s_ceNativeCup.title,"%.26s %.9s %u",
             label,role,(unsigned)ed->year);
    snprintf(s_ceNativeCup.status,sizeof s_ceNativeCup.status,"%s",
        !mask || ed->block || ed->state>=EXP_CE_ED_COMPLETED?
        "FIXTURE DATA UNAVAILABLE":"COMPETITION NOT STARTED - START DATE UNAVAILABLE");
    s_ceNativeCup.active=1;
    ShowMenu(_diyCupMenu);
    s_ceNativeCup.active=0;
    free(rows);memset(&s_ceNativeCup,0,sizeof s_ceNativeCup);
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
