/* SWOS Expanded v00.13.305 — universal Competition Engine viewer.
 * See include/expanded_ce_viewer.h. Readonly: reads CE definitions,
 * editions and the CE fixture ledger; never writes anything.
 *
 * ONE OG-style page (data_json/menus/openswos/ceViewerMenu.json, 13 rows +
 * NEXT PAGE + BACK) shows every competition. Its content is built only from
 * the JSON definition (phases: table / groups / knockout, legs) and the CE
 * state (participants, dates, results, winners): no per-competition code. */
#include "expanded_ce_viewer.h"

#ifdef SWOS_SDL_BUILD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef EXP_CE_VIEWER_PURE
#include "menu_system.h"
#include "menu_load_json.h"
#include "menu_symbols.h"
#include "text_render.h"
#include "menus.h"
#include "expanded_universal_native_match.h"
#endif
#include "game_types.h"
#include "world.h"
#include "expanded_world_time.h"
#include "expanded_ce_engine.h"
#include "expanded_universal_car.h"
#include "expanded_universal_live.h"
#include "expanded_universal_world_catalog.h"

#ifndef EXP_CE_VIEWER_PURE
extern MenuEntry _menuEntries[];
extern uint8_t _currentMenu[];
extern void *_charTables[];
extern void *_countriesTable[];
extern void *_competitionsTable[];
extern TeamFileEx _careerTeam;
extern uint16_t _gameType;
#endif

#define CV_ROWS 13u
#define CV_MAX_LINES 3000u
#define CV_FIRST_ROW 1u
#define CV_PAGE_BTN 14u
#define CV_BACK_BTN 15u
#define CV_COLOR_NORMAL 2u
#define CV_COLOR_HEAD 15u

typedef struct CvLine {
    char left[60];
    char right[44];
    char col[7][8];
    uint8_t ncol,color,sel;
    uint32_t arg;
} CvLine;

static CvLine *s_lines;
static uint32_t s_n,s_page;
static char s_title[48];
#ifndef EXP_CE_VIEWER_PURE
static uint32_t s_pages;
static char s_pageText[24];
static int s_listMode;
#endif
static int32_t s_pick;
static const ExpandedUniversalRegistry *s_reg;

#ifndef EXP_CE_VIEWER_PURE
int ExpCeViewer_Active(void)
{
    return _gameType==4u && World_Current()==WORLD_2627 &&
           ExpandedUniversalLive_Active() &&
           ExpandedUniversal_CarRegistry()!=NULL;
}
#endif

/* ------------------------------------------------------------------ */
/* line buffer                                                          */
/* ------------------------------------------------------------------ */
static void cv_upper(char *s)
{
    for(;*s;++s)
        if(*s>='a' && *s<='z')*s=(char)(*s-'a'+'A');
}
/* Country catalogue visibility does not depend on sporting readiness. */
static uint32_t cv_world_buttons(uint8_t country,uint32_t *ids,
                                  char (*names)[40],uint32_t cap)
{
    uint32_t i,n=0u;
    if(!ids || !names || World_Current()!=WORLD_2627)return 0u;
    for(i=0u;i<ExpCe_DefinitionCount() && n<cap;++i) {
        const ExpCeDef *d=ExpCe_DefinitionAt(i);
        if(!d || !d->viewer_button || d->country!=country ||
           d->world!=(uint16_t)World_Current())continue;
        ids[n]=d->id;
        snprintf(names[n],40u,"%s",d->name);
        cv_upper(names[n]);
        ++n;
    }
    return n;
}
static CvLine *cv_add(const char *left,const char *right,unsigned color)
{
    CvLine *l;
    if(!s_lines || s_n>=CV_MAX_LINES)return NULL;
    l=&s_lines[s_n++];
    memset(l,0,sizeof *l);
    snprintf(l->left,sizeof l->left,"%s",left?left:"");
    snprintf(l->right,sizeof l->right,"%s",right?right:"");
    cv_upper(l->left);cv_upper(l->right);
    l->color=(uint8_t)color;
    return l;
}
static int cv_begin(const char *title)
{
    if(!s_lines)s_lines=(CvLine *)malloc(CV_MAX_LINES*sizeof *s_lines);
    if(!s_lines)return 0;
    s_n=0u;s_page=0u;s_pick=-1;
    snprintf(s_title,sizeof s_title,"%s",title?title:"");
    cv_upper(s_title);
    return 1;
}
static const char *cv_club(ExpandedClubKey k,char *b,size_t n)
{
    const TeamFileEx *t=ExpandedUniversalCatalog_Roster(k);
    if(t)snprintf(b,n,"%.17s",(const char *)t->team.teamName);
    else snprintf(b,n,"CLUB %u:%u",(unsigned)(k.stable_club>>16),
                  (unsigned)(k.stable_club&0xffffu));
    return b;
}
static const char *cv_date(int32_t day,char *b,size_t n)
{
    ExpandedWorldDate d={0};
    if(ExpandedWorld_IndexToDate(day,&d))
        snprintf(b,n,"%02u/%02u",(unsigned)d.day,(unsigned)d.month);
    else snprintf(b,n,"--/--");
    return b;
}
static void cv_def_name(const ExpCeDef *d,char *out,size_t n)
{
    snprintf(out,n,"%s",d->name);
#ifndef EXP_CE_VIEWER_PURE
    if(d->origin==EXP_CE_ORIGIN_LEAGUE && d->country<80u) {
        const void *const *tab=(const void *const *)_competitionsTable[d->country];
        const Competition *c=tab && tab[0] && (uintptr_t)tab[0]>4096u &&
            (uintptr_t)tab[0]!=(uintptr_t)-1 && (uintptr_t)tab[0]!=(uintptr_t)-2?
            (const Competition *)tab[0]:NULL;
        const char *nm=c && c->type==0 && c->compId==d->comp_id?
            comp_name_at((Competition *)c,(size_t)d->division*2u):NULL;
        if(nm && *nm)snprintf(out,n,"%.39s",nm);
        else snprintf(out,n,"LEAGUE DIVISION %u",(unsigned)d->division+1u);
    }
#endif
}
#ifndef EXP_CE_VIEWER_PURE
static const char *cv_country_name(unsigned c,char *b,size_t n)
{
    if(c<80u && _countriesTable[c])
        snprintf(b,n,"%.28s",(const char *)_countriesTable[c]+1);
    else snprintf(b,n,"COUNTRY %u",c);
    return b;
}
#endif

/* ------------------------------------------------------------------ */
/* edition selection                                                    */
/* ------------------------------------------------------------------ */
static const ExpCeEdition *cv_pick_edition(uint32_t def_id)
{
    const ExpCeEdition *best=NULL;
    uint32_t i;
    for(i=0u;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *e=ExpCe_EditionAt(i);
        if(!e || e->def_id!=def_id)continue;
        if(!best) {best=e;continue;}
        if(e->state<EXP_CE_ED_ARCHIVED && best->state==EXP_CE_ED_ARCHIVED)
            best=e;
        else if(e->state<EXP_CE_ED_ARCHIVED &&
                best->state<EXP_CE_ED_ARCHIVED && e->year<best->year)
            best=e;
        else if(e->state==EXP_CE_ED_ARCHIVED &&
                best->state==EXP_CE_ED_ARCHIVED && e->year>best->year)
            best=e;
    }
    return best;
}
static void cv_edition_label(const ExpCeEdition *e,char *b,size_t n)
{
    ExpandedWorldDate d={0};
    if(e->expiry_day && ExpandedWorld_IndexToDate(e->expiry_day,&d) &&
       d.year>e->year)
        snprintf(b,n,"%u/%02u",(unsigned)e->year,(unsigned)(d.year%100u));
    else snprintf(b,n,"%u",(unsigned)e->year);
}

/* ------------------------------------------------------------------ */
/* content of one competition                                           */
/* ------------------------------------------------------------------ */
static int cv_fixture_cmp(const void *a,const void *b)
{
    const ExpandedUniversalFixture *x=&s_reg->fixtures[*(const uint32_t *)a];
    const ExpandedUniversalFixture *y=&s_reg->fixtures[*(const uint32_t *)b];
    if(x->key.tie!=y->key.tie)return x->key.tie<y->key.tie?-1:1;
    if(x->key.leg!=y->key.leg)return x->key.leg<y->key.leg?-1:1;
    return x->event_id<y->event_id?-1:x->event_id>y->event_id;
}
static const char *cv_round_name(unsigned ties,char *b,size_t n)
{
    if(ties==1u)snprintf(b,n,"FINAL");
    else if(ties==2u)snprintf(b,n,"SEMI-FINALS");
    else if(ties==4u)snprintf(b,n,"QUARTER-FINALS");
    else snprintf(b,n,"ROUND OF %u",ties*2u);
    return b;
}
static void cv_result_text(const ExpandedFixtureLedgerEntry *e,
                           char *b,size_t n)
{
    if(!e || e->state!=EXP_FIXTURE_PLAYED){snprintf(b,n,"-");return;}
    snprintf(b,n,"%u-%u",(unsigned)e->score.home,(unsigned)e->score.away);
    if(e->score.flags&EXP_CE_SCORE_EXTRA_TIME)
        snprintf(b+strlen(b),n-strlen(b)," ET");
    if(e->score.flags&EXP_CE_SCORE_PENALTIES)
        snprintf(b+strlen(b),n-strlen(b)," P %u-%u",
                 (unsigned)e->score.penalties_home,
                 (unsigned)e->score.penalties_away);
}
static void cv_table(const ExpCeEdition *ed,uint32_t stage,
                     const char *head)
{
    ExpCeTableRules rules;
    ExpCeTableRow *rows;
    uint32_t used=0u,i;
    char nb[24];
    if(!ExpCe_TableRules(ed->key,stage,&rules))return;
    rows=(ExpCeTableRow *)calloc(EXP_CE_MAX_ENTRANTS,sizeof *rows);
    if(!rows)return;
    if(ExpCeTable_Compute(s_reg,ed->key,stage,&rules,
           ExpandedUniversalLive_ClubNameCompare,NULL,rows,
           EXP_CE_MAX_ENTRANTS,&used) && used) {
        CvLine *h=cv_add(head,"",CV_COLOR_HEAD);
        if(h){
            static const char *k[7]={"P","W","D","L","GF","GA","PT"};
            unsigned c;
            for(c=0;c<7u;++c)snprintf(h->col[c],sizeof h->col[c],"%s",k[c]);
            h->ncol=7u;
        }
        for(i=0u;i<used;++i) {
            char left[60];
            CvLine *l;
            snprintf(left,sizeof left,"%u %s",(unsigned)rows[i].rank,
                     cv_club(rows[i].club,nb,sizeof nb));
            l=cv_add(left,"",CV_COLOR_NORMAL);
            if(l){
                snprintf(l->col[0],8,"%u",(unsigned)rows[i].played);
                snprintf(l->col[1],8,"%u",(unsigned)rows[i].wins);
                snprintf(l->col[2],8,"%u",(unsigned)rows[i].draws);
                snprintf(l->col[3],8,"%u",(unsigned)rows[i].losses);
                snprintf(l->col[4],8,"%u",(unsigned)rows[i].goals_for);
                snprintf(l->col[5],8,"%u",(unsigned)rows[i].goals_against);
                snprintf(l->col[6],8,"%u",(unsigned)rows[i].points);
                l->ncol=7u;
            }
        }
    }
    free(rows);
}
static void cv_knockout(const ExpCeDef *d,const ExpCeEdition *ed,
                        uint32_t phase)
{
    uint32_t *idx,cnt=0u,i,ties=0u;
    const uint32_t stage=EXP_CE_STAGE(phase,EXP_CE_KO_GROUP);
    char hb[40];
    idx=(uint32_t *)malloc((size_t)(s_reg->fixture_count?s_reg->fixture_count:1u)*sizeof *idx);
    if(!idx)return;
    for(i=0u;i<s_reg->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&s_reg->fixtures[i];
        if(f->key.stage==stage &&
           ExpandedEditionKey_Equal(f->key.edition,ed->key)) {
            idx[cnt++]=i;
            if(f->key.leg==0u)++ties;
        }
    }
    if(!cnt){free(idx);return;}
    qsort(idx,cnt,sizeof *idx,cv_fixture_cmp);
    cv_add(cv_round_name(ties,hb,sizeof hb),"",CV_COLOR_HEAD);
    for(i=0u;i<cnt;++i) {
        const ExpandedUniversalFixture *f=&s_reg->fixtures[idx[i]];
        const ExpandedFixtureLedgerEntry *e=&s_reg->ledger.entries[idx[i]];
        char a[24],b[24],dt[12],left[60],rs[40];
        snprintf(left,sizeof left,"%s %s - %s",
                 cv_date(f->day_index,dt,sizeof dt),
                 cv_club(f->home,a,sizeof a),cv_club(f->away,b,sizeof b));
        cv_result_text(e,rs,sizeof rs);
        cv_add(left,rs,CV_COLOR_NORMAL);
        if(d->phases[phase].match.legs==2u && f->key.leg==1u) {
            ExpandedClubKey w={0},lo={0};
            if(ExpCeTie_Resolve(s_reg,ed->key,stage,f->key.tie,
                   &d->phases[phase].match,&w,&lo)==EXP_CE_TIE_DECIDED) {
                char nb[24],t[60];
                snprintf(t,sizeof t,"  ADVANCES: %s",
                         cv_club(w,nb,sizeof nb));
                cv_add(t,"",CV_COLOR_HEAD);
            }
        }
    }
    free(idx);
}
/* v00.13.342 (user 26/11/2026): an edition whose participants are not yet
 * confirmed lists every expected slot: the name of a club already certain
 * (LISTED with as many clubs as places) and, for the others, which club it
 * will be (league winner, 2nd place, cup winner ...). Read-only. */
static const char *cv_ordinal(uint32_t r,char *b,size_t n)
{
    if(r==1u)snprintf(b,n,"WINNER");
    else snprintf(b,n,"%u%s PLACE",(unsigned)r,
                  (r%10u==2u && r%100u!=12u)?"ND":(r%10u==3u && r%100u!=13u)?"RD":"TH");
    return b;
}
static uint32_t cv_expected_slots(const ExpCeDef *d,int list,uint32_t *known)
{
    uint32_t s,r,total=0u,nk=0u;
    char nm[48],o[24],t[72];
    for(s=0u;s<d->nsources;++s) {
        const ExpCeSourceSpec *x=&d->sources[s];
        const ExpCeDef *src=x->competition?ExpCe_Definition(x->competition):NULL;
        if(src)cv_def_name(src,nm,sizeof nm);
        else if((x->competition&0xffu)==1u)   /* league id: (country+1)<<24 | comp<<16 | (division+1)<<8 | 1 */
            snprintf(nm,sizeof nm,"LEAGUE DIVISION %u",(unsigned)((x->competition>>8)&0xffu));
        else snprintf(nm,sizeof nm,"COMPETITION");
        switch(x->source) {
        case EXP_CE_SRC_LISTED:
            for(r=0u;r<x->count;++r) {
                ++total;
                if(x->nclubs==x->count)++nk;
                if(!list)continue;
                if(x->nclubs==x->count) {
                    ExpandedClubKey k={d->world,x->clubs[r],EXP_PARTICIPANT_CLUB};
                    cv_add(cv_club(k,t,sizeof t),"",CV_COLOR_NORMAL);
                } else cv_add("QUALIFIER","",CV_COLOR_NORMAL);
            }
            break;
        case EXP_CE_SRC_LEAGUE_RANK:
            for(r=x->rank_from;r>=1u && r<=x->rank_to;++r) {
                ++total;
                if(!list)continue;
                snprintf(t,sizeof t,"%s %.30s",cv_ordinal(r,o,sizeof o),nm);
                cv_add(t,"",CV_COLOR_NORMAL);
            }
            break;
        case EXP_CE_SRC_CUP_WINNER:
        case EXP_CE_SRC_CUP_RUNNER_UP:
            ++total;
            if(!list)break;
            snprintf(t,sizeof t,"%s %.30s",x->source==EXP_CE_SRC_CUP_WINNER?"WINNER":"RUNNER-UP",nm);
            cv_add(t,"",CV_COLOR_NORMAL);
            break;
        default:
            break;
        }
    }
    if(known)*known=nk;
    return total;
}
static void cv_build_view(const ExpCeDef *d,const ExpCeEdition *ed)
{
    char nm[48],lab[16],t[64],nb[24];
    uint32_t i,p;
    int32_t first=0,last=0;
    uint32_t fixtures=0u;
    cv_def_name(d,nm,sizeof nm);
    if(ed){
        cv_edition_label(ed,lab,sizeof lab);
        snprintf(t,sizeof t,"%s %s",nm,lab);
    } else snprintf(t,sizeof t,"%s",nm);
    cv_begin(t);
    if(!ed){
        cv_add("NO EDITION IN THE COMPETITION ENGINE YET","",CV_COLOR_NORMAL);
        return;
    }
    for(i=0u;i<s_reg->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&s_reg->fixtures[i];
        if(!ExpandedEditionKey_Equal(f->key.edition,ed->key))continue;
        if(!fixtures || f->day_index<first)first=f->day_index;
        if(!fixtures || f->day_index>last)last=f->day_index;
        ++fixtures;
    }
    {
        char s[32];
        snprintf(s,sizeof s,"%s",ed->block?"BLOCKED":ExpCe_StateName(ed->state));
        cv_add("STATUS",s,CV_COLOR_NORMAL);
        if(!ed->nentrants && d->nsources)
        {   uint32_t nk=0u;
            const uint32_t tot=cv_expected_slots(d,0,&nk);
            snprintf(s,sizeof s,"%u OF %u",(unsigned)nk,(unsigned)tot);
        }
        else snprintf(s,sizeof s,"%u",(unsigned)ed->nentrants);
        cv_add("PARTICIPANTS",s,CV_COLOR_NORMAL);
        if(fixtures){
            char a[12],b[12];
            snprintf(s,sizeof s,"%s - %s",cv_date(first,a,sizeof a),
                     cv_date(last,b,sizeof b));
            cv_add("MATCH DATES",s,CV_COLOR_NORMAL);
        }
    }
    if(ed->nfinal>=1u) {
        ExpandedClubKey k=ed->entrants[0].club;
        k.stable_club=ed->final_order[0];
        cv_add("WINNER",cv_club(k,nb,sizeof nb),CV_COLOR_HEAD);
        if(ed->nfinal>=2u){
            k.stable_club=ed->final_order[1];
            cv_add("RUNNER-UP",cv_club(k,nb,sizeof nb),CV_COLOR_NORMAL);
        }
    }
    for(p=0u;p<d->nphases;++p) {
        const ExpCePhaseDef *ph=&d->phases[p];
        uint32_t started=0u;
        for(i=0u;i<s_reg->fixture_count;++i)
            if(EXP_CE_STAGE_PHASE(s_reg->fixtures[i].key.stage)==p &&
               ExpandedEditionKey_Equal(s_reg->fixtures[i].key.edition,ed->key))
                {++started;break;}
        if(ph->kind==EXP_CE_PHASE_TABLE) {
            uint32_t g;
            if(!started)continue;
            for(g=1u;g<=ph->groups;++g) {
                char head[40];
                if(ph->groups>1u)snprintf(head,sizeof head,"GROUP %u",(unsigned)g);
                else snprintf(head,sizeof head,"%s",d->nphases>1u?"TABLE":"STANDINGS");
                cv_table(ed,EXP_CE_STAGE(p,g),head);
            }
        } else if(ph->kind==EXP_CE_PHASE_KNOCKOUT) {
            if(started)cv_knockout(d,ed,p);
            else if(p==0u || ed->phase>=p)
                {} /* a later phase is drawn by the engine when ready */
        }
    }
    if(!fixtures && ed->nentrants) {
        cv_add("PARTICIPANTS","",CV_COLOR_HEAD);
        for(i=0u;i<ed->nentrants;++i)
            cv_add(cv_club(ed->entrants[i].club,nb,sizeof nb),"",CV_COLOR_NORMAL);
    } else if(!fixtures && d->nsources && cv_expected_slots(d,0,NULL)) {
        cv_add("EXPECTED PARTICIPANTS","",CV_COLOR_HEAD);
        (void)cv_expected_slots(d,1,NULL);
    } else if(!fixtures)
        cv_add("PARTICIPANTS NOT YET DETERMINED","",CV_COLOR_NORMAL);
    /* roll of honour: archived editions of this competition */
    {
        uint32_t ny=0u,j;
        const ExpCeEdition *arch[64];
        for(i=0u;i<ExpCe_EditionCount() && ny<64u;++i) {
            const ExpCeEdition *e=ExpCe_EditionAt(i);
            if(e && e->def_id==d->id && e->state==EXP_CE_ED_ARCHIVED &&
               e->nfinal>=1u){arch[ny]=e;++ny;}
        }
        if(ny){
            cv_add("ROLL OF HONOUR","",CV_COLOR_HEAD);
            for(j=ny;j>0u;--j){
                const ExpCeEdition *e=arch[j-1u];
                char l2[60],rn[24];
                ExpandedClubKey k=e->entrants[0].club,k2=k;
                char lb[16];
                k.stable_club=e->final_order[0];
                k2.stable_club=e->nfinal>1u?e->final_order[1]:0u;
                cv_edition_label(e,lb,sizeof lb);
                snprintf(l2,sizeof l2,"%s %s",lb,cv_club(k,nb,sizeof nb));
                cv_add(l2,e->nfinal>1u?cv_club(k2,rn,sizeof rn):"",CV_COLOR_NORMAL);
            }
        }
    }
}

#ifndef EXP_CE_VIEWER_PURE
/* ------------------------------------------------------------------ */
/* menu                                                                 */
/* ------------------------------------------------------------------ */
static void cv_set_entry_nav(unsigned e,int l,int r,int u,int d)
{
    _menuEntries[e].leftEntry=(int8_t)l;_menuEntries[e].rightEntry=(int8_t)r;
    _menuEntries[e].upEntry=(int8_t)u;_menuEntries[e].downEntry=(int8_t)d;
}
static unsigned cv_entry_index(const MenuEntry *e)
{
    return (unsigned)(((const uint8_t *)e-(const uint8_t *)_menuEntries)/
                      MENUENTRY_STRIDE);
}
static void cv_apply_page(void)
{
    unsigned r,sel[CV_ROWS],nsel=0u;
    const uint32_t base=s_page*CV_ROWS;
    int pageVisible;
    s_pages=s_n?(s_n+CV_ROWS-1u)/CV_ROWS:1u;
    if(s_page>=s_pages)s_page=s_pages-1u;
    pageVisible=s_pages>1u;
    for(r=0u;r<CV_ROWS;++r) {
        MenuEntry *e=&_menuEntries[CV_FIRST_ROW+r];
        const uint32_t li=s_page*CV_ROWS+r;
        const int has=li<s_n;
        e->isInvisible=has?0:1;
        e->isDisabled=(has && s_listMode && s_lines[li].sel)?0:1;
        if(!e->isDisabled)sel[nsel++]=CV_FIRST_ROW+r;
        (void)base;
    }
    snprintf(s_pageText,sizeof s_pageText,"NEXT PAGE %u/%u",
             (unsigned)s_page+1u,(unsigned)s_pages);
    _menuEntries[0].fg=(uintptr_t)s_title;
    _menuEntries[CV_PAGE_BTN].fg=(uintptr_t)s_pageText;
    _menuEntries[CV_PAGE_BTN].isInvisible=pageVisible?0:1;
    _menuEntries[CV_PAGE_BTN].isDisabled=pageVisible?0:1;
    {
        static char backText[]="BACK";
        _menuEntries[CV_BACK_BTN].fg=(uintptr_t)backText;
    }
    {
        const int pg=pageVisible?(int)CV_PAGE_BTN:(int)CV_BACK_BTN;
        const int first=nsel?(int)sel[0]:pg;
        const int lastRow=nsel?(int)sel[nsel-1u]:(int)CV_BACK_BTN;
        unsigned k;
        for(k=0u;k<nsel;++k)
            cv_set_entry_nav(sel[k],(int)sel[k],(int)sel[k],
                k?(int)sel[k-1u]:(int)CV_BACK_BTN,
                k+1u<nsel?(int)sel[k+1u]:pg);
        cv_set_entry_nav(CV_PAGE_BTN,CV_BACK_BTN,CV_BACK_BTN,lastRow,first==pg?CV_BACK_BTN:first);
        cv_set_entry_nav(CV_BACK_BTN,pageVisible?(int)CV_PAGE_BTN:(int)CV_BACK_BTN,
                         pageVisible?(int)CV_PAGE_BTN:(int)CV_BACK_BTN,
                         pageVisible?(int)CV_PAGE_BTN:lastRow,first);
        SetCurrentEntry((uint16_t)(nsel?sel[0]:CV_BACK_BTN));
    }
}
static void ceViewer_OnInit(void)
{
    cv_apply_page();
}
static void ceViewer_row_AfterDraw(MenuEntry *e)
{
    const unsigned idx=cv_entry_index(e);
    const uint32_t li=s_page*CV_ROWS+(idx-CV_FIRST_ROW);
    const CvLine *l;
    uint8_t *ct=(uint8_t *)_charTables[0];
    int x,right;
    if(idx<CV_FIRST_ROW || idx>=CV_FIRST_ROW+CV_ROWS || li>=s_n)return;
    l=&s_lines[li];
    x=(int)e->x+4;right=(int)e->x+(int)e->dwWidth-4;
    if(l->ncol) {
        unsigned c;
        int cx=right;
        for(c=l->ncol;c>0u;--c){
            DrawMenuTextRightAligned((uint16_t)cx,(uint16_t)(e->y+2),l->color,
                                     (uint8_t *)l->col[c-1u],ct);
            cx-=24;
        }
        right=cx;
    }
    if(l->right[0]) {
        DrawMenuTextRightAligned((uint16_t)right,(uint16_t)(e->y+2),l->color,
                                 (uint8_t *)l->right,ct);
        right-=(int)GetTextWidth((uint8_t *)l->right,ct)+8;
    }
    if(l->left[0]) {
        char fit[60];
        size_t n;
        snprintf(fit,sizeof fit,"%s",l->left);
        n=strlen(fit);
        while(n>1u && x+(int)GetTextWidth((uint8_t *)fit,ct)>right) {
            fit[--n]=0;
        }
        DrawMenuText((uint16_t)x,(uint16_t)(e->y+2),l->color,(uint8_t *)fit,ct);
    }
}
static void ceViewer_row_OnSelect(void)
{
    const Menu *m=(const Menu *)_currentMenu;
    const unsigned idx=cv_entry_index((const MenuEntry *)m->selectedEntry);
    const uint32_t li=s_page*CV_ROWS+(idx-CV_FIRST_ROW);
    if(idx<CV_FIRST_ROW || idx>=CV_FIRST_ROW+CV_ROWS || li>=s_n ||
       !s_listMode || !s_lines[li].sel)return;
    s_pick=(int32_t)s_lines[li].arg;
    SetExitMenuFlag();
}
static void ceViewer_page_OnSelect(void)
{
    s_page=(s_page+1u>=s_pages)?0u:s_page+1u;
    cv_apply_page();
    DrawMenu();
}
static void ceViewer_back_OnSelect(void)
{
    s_pick=-1;
    SetExitMenuFlag();
}
#define CV_HANDLER(fn) { #fn, (void *)fn, SYM_FUNC }
static const MenuSymbol k_cvHandlers[]={
    CV_HANDLER(ceViewer_OnInit),CV_HANDLER(ceViewer_row_AfterDraw),
    CV_HANDLER(ceViewer_row_OnSelect),CV_HANDLER(ceViewer_page_OnSelect),
    CV_HANDLER(ceViewer_back_OnSelect),
};
static int cv_show(void)
{
    static int registered;
    uint8_t *menu;
    if(!registered){
        registered=1;
        MenuJson_RegisterSymbols(k_cvHandlers,
            (int)(sizeof k_cvHandlers/sizeof k_cvHandlers[0]));
    }
    menu=MenuJson_Load("openswos/ceViewerMenu");
    if(!menu){
        static uint8_t msg[]="CE VIEWER MENU FILE MISSING";
        fprintf(stderr,"[ce-viewer] data_json/menus/openswos/ceViewerMenu.json "
                       "cannot be loaded\n");
        ShowErrorMenu(msg);
        return 0;
    }
    ShowMenu(menu);
    return 1;
}

/* ------------------------------------------------------------------ */
/* entry points (the OG menus only add ONE button, nothing else changes) */
/* ------------------------------------------------------------------ */
/* Extended RULES is a read-only projection of the selected CE edition.
 * Provisional text page: no OG rule conversion, no fixture or CAR writer.
 * Call only from a native modal; the generic viewer shares this line buffer. */
static void cv_rule_u(const char *label,unsigned value)
{
    char t[16];
    snprintf(t,sizeof t,"%u",value);
    cv_add(label,t,CV_COLOR_NORMAL);
}
static void cv_rule_bool(const char *label,unsigned value)
{
    cv_add(label,value?"YES":"NO",CV_COLOR_NORMAL);
}
static const char *cv_rule_provenance(unsigned p)
{
    switch(p) {
        case EXP_CE_PROV_OG_NATIVE:return "OG NATIVE";
        case EXP_CE_PROV_JSON_VERIFIED:return "JSON VERIFIED";
        case EXP_CE_PROV_JSON_TEMPLATE:return "GAME TEMPLATE";
        default:return "DATA UNAVAILABLE";
    }
}
static void cv_rule_date(const char *label,int32_t index)
{
    ExpandedWorldDate date={0};
    char t[24];
    if(!ExpandedWorld_IndexToDate(index,&date)) {
        cv_add(label,"DATE UNAVAILABLE",CV_COLOR_NORMAL);return;
    }
    snprintf(t,sizeof t,"%02u/%02u/%04u",(unsigned)date.day,
             (unsigned)date.month,(unsigned)date.year);
    cv_add(label,t,CV_COLOR_NORMAL);
}
static void cv_rule_weekdays(const char *label,unsigned mask)
{
    static const char *const names[]={"MON","TUE","WED","THU","FRI","SAT","SUN"};
    char t[32];size_t pos=0u;
    t[0]=0;
    for(unsigned i=0u;i<7u;++i)if(mask&(1u<<i)) {
        if(pos)t[pos++]=' ';
        memcpy(t+pos,names[i],3u);pos+=3u;t[pos]=0;
    }
    cv_add(label,pos?t:"NONE",CV_COLOR_NORMAL);
}
static void cv_rule_phase(const ExpCePhaseDef *p,const char *heading)
{
    static const char *const ties[]={
        "END","POINTS","GOAL DIFFERENCE","GOALS FOR","WINS","AWAY GOALS FOR",
        "HEAD TO HEAD POINTS","HEAD TO HEAD GOAL DIFF","HEAD TO HEAD GOALS",
        "HEAD TO HEAD AWAY GOALS","NAME","DRAW"
    };
    char t[48],l[40];
    cv_add(heading,p->kind==EXP_CE_PHASE_TABLE?"TABLE":
           p->kind==EXP_CE_PHASE_KNOCKOUT?"KNOCKOUT":"UNKNOWN",CV_COLOR_HEAD);
    if(p->missing[0] || !p->match.provenance ||
       (p->kind==EXP_CE_PHASE_TABLE && !p->table.provenance)) {
        cv_add("PHASE RULES","NO DATA",CV_COLOR_NORMAL);return;
    }
    if(p->kind==EXP_CE_PHASE_TABLE) {
        snprintf(t,sizeof t,"%u / %u",(unsigned)p->groups,(unsigned)p->meetings);
        cv_add("GROUPS / MEETINGS",t,CV_COLOR_NORMAL);
        snprintf(t,sizeof t,"%u / %u / %u",(unsigned)p->table.win,
                 (unsigned)p->table.draw,(unsigned)p->table.loss);
        cv_add("POINTS W / D / L",t,CV_COLOR_NORMAL);
        if(p->table.draw_penalties) {
            snprintf(t,sizeof t,"%u / %u",(unsigned)p->table.draw_pen_win,
                     (unsigned)p->table.draw_pen_loss);
            cv_add("SHOOTOUT POINTS W / L",t,CV_COLOR_NORMAL);
        }
        if(p->qualify) {
            snprintf(t,sizeof t,"%u - %u",(unsigned)(p->qualify_from?p->qualify_from:1u),
                     (unsigned)p->qualify);
            cv_add("QUALIFYING RANKS PER GROUP",t,CV_COLOR_NORMAL);
        }
        cv_add("TABLE RULES SOURCE",cv_rule_provenance(p->table.provenance),CV_COLOR_NORMAL);
        for(unsigned i=0u;i<p->table.ntb && i<EXP_CE_MAX_TIEBREAK;++i) {
            const unsigned k=p->table.tiebreak[i];
            snprintf(l,sizeof l,"TIE BREAK %u",i+1u);
            cv_add(l,k<sizeof ties/sizeof ties[0]?ties[k]:"UNKNOWN",CV_COLOR_NORMAL);
        }
    } else if(p->kind==EXP_CE_PHASE_KNOCKOUT) {
        snprintf(t,sizeof t,"%u / %u / %s",(unsigned)p->match.legs,
                 (unsigned)p->byes,p->match.needs_winner?"YES":"NO");
        cv_add("LEGS / BYES / WINNER REQUIRED",t,CV_COLOR_NORMAL);
        snprintf(t,sizeof t,"%s / %s / %s",p->match.extra_time?"YES":"NO",
                 p->match.penalties?"YES":"NO",p->match.away_goals?"YES":"NO");
        cv_add("EXTRA TIME / PENS / AWAY GOALS",t,CV_COLOR_NORMAL);
        if(p->match.rank_tiebreak)
            cv_add("REGULAR RANK TIE BREAK",p->match.rank_tiebreak==2u?
                   "UNLESS LEVEL ON POINTS":"YES",CV_COLOR_NORMAL);
    }
    cv_add("MATCH RULES SOURCE",cv_rule_provenance(p->match.provenance),CV_COLOR_NORMAL);
    snprintf(t,sizeof t,"%u FROM %u",(unsigned)p->match.subs,
             (unsigned)p->match.bench);
    cv_add("SUBSTITUTIONS",t,CV_COLOR_NORMAL);
    if(p->match.windows)
        snprintf(t,sizeof t,"%u / %s",(unsigned)p->match.windows,
                 p->match.halftime_free?"YES":"NO");
    else snprintf(t,sizeof t,"UNLIMITED / %s",p->match.halftime_free?"YES":"NO");
    cv_add("WINDOWS / HALF TIME FREE",t,CV_COLOR_NORMAL);
    if(p->match.extra_time || p->match.et_extra_subs || p->match.et_extra_windows) {
        snprintf(t,sizeof t,"%u / %u",(unsigned)p->match.et_extra_subs,
                 (unsigned)p->match.et_extra_windows);
        cv_add("ET EXTRA SUBS / WINDOWS",t,CV_COLOR_NORMAL);
    }
    if(p->has_window) {
        const ExpCeWindow *w=&p->window;
        snprintf(t,sizeof t,"%02u/%02u - %02u/%02u (+%uY)",
                 (unsigned)w->first_d,(unsigned)w->first_m,
                 (unsigned)w->last_d,(unsigned)w->last_m,
                 (unsigned)w->last_year_offset);
        cv_add("CALENDAR TEMPLATE",t,CV_COLOR_NORMAL);
        cv_rule_weekdays("GAME DAYS",w->weekdays);
        cv_rule_weekdays("EXTRA GAME DAYS",w->extra_weekdays);
        cv_rule_u("REST DAYS",w->rest);
        for(unsigned i=0u;i<w->nbreaks && i<EXP_CE_MAX_BREAKS;++i) {
            snprintf(t,sizeof t,"%02u/%02u - %02u/%02u",
                     (unsigned)w->breaks[i][1],(unsigned)w->breaks[i][0],
                     (unsigned)w->breaks[i][3],(unsigned)w->breaks[i][2]);
            cv_add("CALENDAR BREAK",t,CV_COLOR_NORMAL);
        }
    }
    if(p->nrel) {
        size_t used=0u;
        t[0]=0;
        for(unsigned i=0u;i<p->nrel && i<sizeof p->rel/sizeof p->rel[0];++i) {
            int n=snprintf(t+used,sizeof t-used,"%s%u",i?"/":"",(unsigned)p->rel[i]);
            if(n<0 || (size_t)n>=sizeof t-used)break;
            used+=(size_t)n;
        }
        snprintf(l,sizeof l,"DAY OFFSETS FROM PHASE INDEX %u",(unsigned)p->rel_anchor);
        cv_add(l,t,CV_COLOR_NORMAL);
    }
}
int ExpCeViewer_OpenRules(ExpandedEditionKey key)
{
    const ExpCeEdition *e;
    const ExpCeDef *d;
    char title[48],label[40],value[40];
    uint32_t count=0u;
    int32_t first=0,last=0;
    if(!ExpCeViewer_Active())return 0;
    s_reg=ExpandedUniversal_CarRegistry();
    if(!s_reg || s_reg->recovery_required || key.competition.world!=s_reg->world)return 0;
    e=ExpCe_Edition(key);
    d=ExpCe_Definition(key.competition.stable_competition);
    if(!d || d->world!=key.competition.world)return 0;
    snprintf(title,sizeof title,"RULES %.39s",d->name);
    if(!cv_begin(title))return 0;
    s_listMode=0;
    if(!e || !ExpandedEditionKey_Equal(e->key,key) || e->def_id!=d->id) {
        cv_add("EDITION RULES","NO DATA",CV_COLOR_HEAD);
        return cv_show();
    }
    cv_rule_u("EDITION YEAR",e->year);
    cv_rule_date("EDITION START",key.start_day);
    cv_add("EDITION STATE",ExpCe_StateName(e->state),CV_COLOR_NORMAL);
    if(e->block)cv_add("EDITION BLOCK",ExpCe_BlockName(e->block),CV_COLOR_NORMAL);
    snprintf(value,sizeof value,"%u / %u",(unsigned)d->clubs,(unsigned)e->nentrants);
    cv_add("TEAMS CONFIGURED / CONFIRMED",value,CV_COLOR_NORMAL);
    if(e->rules_hash!=d->rules_hash) {
        cv_add("EDITION RULES UNAVAILABLE","RULE HASH MISMATCH",CV_COLOR_HEAD);
        return cv_show();
    }
    if(d->missing[0]) {
        cv_add("DEFINITION RULES","NO DATA",CV_COLOR_NORMAL);return cv_show();
    }
    cv_add("DEFINITION SOURCE",d->source_file,CV_COLOR_NORMAL);
    if(e->state>=EXP_CE_ED_SCHEDULED && e->expiry_day>=key.start_day)
        cv_rule_date("EDITION EXPIRY TARGET",e->expiry_day);
    else cv_add("EDITION EXPIRY TARGET","NOT YET AVAILABLE",CV_COLOR_NORMAL);
    cv_add("ROLLOVER","AFTER EXPIRY AND COMPLETION",CV_COLOR_NORMAL);
    if(s_reg->fixtures && s_reg->fixture_count==s_reg->ledger.count) {
        for(uint32_t i=0u;i<s_reg->fixture_count;++i) {
            const ExpandedUniversalFixture *f=&s_reg->fixtures[i];
            if(!ExpandedEditionKey_Equal(f->key.edition,key))continue;
            if(!count || f->day_index<first)first=f->day_index;
            if(!count || f->day_index>last)last=f->day_index;
            ++count;
        }
        cv_rule_u("MATERIALISED FIXTURES",count);
        if(count) {
            cv_rule_date("FIRST MATERIALISED FIXTURE",first);
            cv_rule_date("LAST MATERIALISED FIXTURE",last);
        }
    }
    if(d->origin==EXP_CE_ORIGIN_LEAGUE) {
        cv_add("MOVEMENT RULES","",CV_COLOR_HEAD);
        snprintf(value,sizeof value,"%u / %u",(unsigned)d->promoted,(unsigned)d->relegated);
        cv_add("REGULAR UP / DOWN",value,CV_COLOR_NORMAL);
        if(d->xch_on) {
            const ExpCeDef *cup=ExpCe_Definition(d->xch_champ);
            cv_add("EXCHANGE CHAMPION FROM",cup && cup->world==d->world?
                   cup->name:"NO DATA",CV_COLOR_NORMAL);
            cv_rule_u("REPLACES REGULAR LEAGUE RANK",d->xch_rank);
            cv_add("CHAMPION ALREADY MEMBER","NO EXCHANGE",CV_COLOR_NORMAL);
            cv_add("RELEGATED CLUB","RETURNS TO ITS REGION",CV_COLOR_NORMAL);
            cv_add("REGIONAL BALANCE","VIA BOTTOM POOL",CV_COLOR_NORMAL);
        }
        if(d->par_n) {
            cv_rule_u("LOWER DIVISION GROUPS",d->par_n);
            cv_rule_u("PROMOTED PER GROUP",d->par_per_group);
            cv_rule_u("EXTRA BEST NEXT-PLACED",d->par_extra);
        }
        if(e->partial)cv_add("PARTIAL EDITION","MOVEMENTS NOT CERTIFIED",CV_COLOR_NORMAL);
        if(d->promoted) {
            const unsigned direct=(unsigned)d->promoted-(d->promotion_playoff?1u:0u);
            if(direct) {
                snprintf(value,sizeof value,"TOP %u",direct);
                cv_add("DIRECT PROMOTION",value,CV_COLOR_NORMAL);
            }
            if(d->promotion_playoff && d->nphases>1u) {
                const ExpCePhaseDef *p=&d->phases[0];
                snprintf(value,sizeof value,"RANKS %u-%u",
                         (unsigned)(p->qualify_from?p->qualify_from:1u),(unsigned)p->qualify);
                cv_add("PLAYOFF FOR LAST PROMOTION",value,CV_COLOR_NORMAL);
            }
        }
        if(d->fin_gap_min) {
            snprintf(label,sizeof label,"RANK %u OVER %u BY MORE THAN",
                     (unsigned)d->fin_gap_hi,(unsigned)d->fin_gap_lo);
            snprintf(value,sizeof value,"%u PTS",(unsigned)d->fin_gap_min);
            cv_add(label,value,CV_COLOR_NORMAL);
            cv_add("THEN SKIP PLAYOFF",d->promotion_playoff?"NEXT RANK PROMOTED":"NO FINALS",
                   CV_COLOR_NORMAL);
        }
        if(d->relegated && !d->pool_playoff && d->clubs>=d->relegated) {
            const unsigned direct=(unsigned)d->relegated-
                (ExpCe_PlayoutActive(d)?1u:0u);
            if(direct) {
                snprintf(value,sizeof value,"BOTTOM %u",direct);
                cv_add("DIRECT RELEGATION",value,CV_COLOR_NORMAL);
            }
        }
        if(d->has_playout) {
            const int active=ExpCe_PlayoutActive(d);
            cv_add("PLAYOUT RULE",active?"ENABLED":"NOT ACTIVE",CV_COLOR_NORMAL);
            if(d->pool_playoff) {
                cv_add("PLAYOUT","CLUB VS POOL CHALLENGER",CV_COLOR_NORMAL);
                if(active)cv_add("LOSER","TO POOL",CV_COLOR_NORMAL);
            } else {
                snprintf(value,sizeof value,"RANK %u VS %u",(unsigned)d->playout_hi,
                         (unsigned)d->playout_lo);
                cv_add("PLAYOUT FOR SURVIVAL",value,CV_COLOR_NORMAL);
                if(active)cv_add("LOSER","RELEGATED",CV_COLOR_NORMAL);
                if(active && d->playout_gap_min) {
                    snprintf(label,sizeof label,"RANK %u OVER %u BY MORE THAN",
                             (unsigned)d->playout_hi,(unsigned)d->playout_lo);
                    snprintf(value,sizeof value,"%u PTS",(unsigned)d->playout_gap_min);
                    cv_add(label,value,CV_COLOR_NORMAL);
                    cv_add("THEN SKIP PLAYOUT","LOWER RANK RELEGATED",CV_COLOR_NORMAL);
                }
            }
            cv_rule_phase(&d->playout,"PLAYOUT");
        }
        if(d->pool_n) {
            snprintf(value,sizeof value,"%u / %u / %u",(unsigned)d->pool_n,
                     (unsigned)d->pool_direct,(unsigned)d->pool_playoff);
            cv_add("POOL / DIRECT / PLAYOFF",value,CV_COLOR_NORMAL);
            if(d->pool_direct) {
                snprintf(value,sizeof value,"BOTTOM %u",(unsigned)d->pool_direct);
                cv_add("DIRECT TO POOL",value,CV_COLOR_NORMAL);
                snprintf(value,sizeof value,"%u CLUBS",(unsigned)d->pool_direct);
                cv_add("DIRECT FROM POOL",value,CV_COLOR_NORMAL);
            }
        }
    }
    for(unsigned i=0u;i<d->nphases && i<EXP_CE_MAX_PHASES;++i) {
        snprintf(label,sizeof label,"PHASE %u",i+1u);
        cv_rule_phase(&d->phases[i],label);
    }
    if(d->nsources) {
        cv_add("NEXT EDITION QUALIFICATION","",CV_COLOR_HEAD);
        for(unsigned i=0u;i<d->nsources && i<EXP_CE_MAX_SOURCES;++i) {
            const ExpCeSourceSpec *sp=&d->sources[i];
            const ExpCeDef *from=ExpCe_Definition(sp->competition);
            cv_add("SOURCE",ExpCe_SourceName(sp->source),CV_COLOR_NORMAL);
            if(sp->competition) {
                if(from && from->world==d->world)cv_add("FROM",from->name,CV_COLOR_NORMAL);
                else cv_rule_u("SOURCE COMPETITION ID",sp->competition);
            }
            if(sp->rank_from || sp->rank_to) {
                snprintf(value,sizeof value,"%u - %u",(unsigned)sp->rank_from,
                         (unsigned)sp->rank_to);
                cv_add("RANKS",value,CV_COLOR_NORMAL);
            }
            if(sp->count)cv_rule_u("SLOTS",sp->count);
            if(sp->nexclude)cv_rule_u("EXCLUDED CLUBS",sp->nexclude);
            cv_rule_bool("REALLOCATE DUPLICATES",sp->reallocate);
        }
    }
    return cv_show();
}

int ExpCeViewer_OpenDefinition(uint32_t def_id)
{
    const ExpCeDef *d=ExpCe_Definition(def_id);
    const ExpCeEdition *e;
    if(!d || !ExpCeViewer_Active())return 0;
    s_reg=ExpandedUniversal_CarRegistry();
    if(!s_reg)return 0;
    e=cv_pick_edition(def_id);
    /* CE cup buttons and native aliases with real fixtures use the original cup menu.
     * Mixed tournaments and non-materialised/archive views retain the
     * existing CE reader; never fabricate a native bracket. */
    if(e && ExpCeViewer_OpenNativeCup(e->key))return 1;
    /* The MORE RULES child shares the row buffer: close this view before
     * opening the same OG basic page, then rebuild on return. */
    for(;;) {
        ExpandedEditionKey key={{d->world,d->id},0,0};
        CvLine *line;
        if(e)key=e->key;
        cv_build_view(d,e);
        if(s_lines && s_n<CV_MAX_LINES) {
            memmove(s_lines+1,s_lines,s_n*sizeof *s_lines);
            ++s_n;line=&s_lines[0];memset(line,0,sizeof *line);
            snprintf(line->left,sizeof line->left,"RULES");
            line->color=CV_COLOR_HEAD;line->sel=1u;line->arg=1u;
        }
        s_listMode=1;s_pick=-1;
        if(!cv_show())return 0;
        if(s_pick!=1)return 1;
        (void)ExpCeViewer_OpenBasicRules(key);
    }
}
/* The OG competition list stores a pointer per row. A button row points at
 * this small read-only record (type byte 0xCE, never an OG competition) that
 * carries the CE definition id; the OG callers hand it back here. */
static uint8_t s_btnRec[8][32];
const void *ExpCeViewer_ButtonRecord(uint32_t def_id)
{
    unsigned k;
    for(k=0u;k<8u;++k)
        if(s_btnRec[k][1]==0xCEu &&
           memcmp(&s_btnRec[k][4],&def_id,sizeof def_id)==0)return s_btnRec[k];
    for(k=0u;k<8u;++k)
        if(s_btnRec[k][1]!=0xCEu) {
            s_btnRec[k][1]=0xCEu;
            memcpy(&s_btnRec[k][4],&def_id,sizeof def_id);
            return s_btnRec[k];
        }
    return NULL;
}
int ExpCeViewer_OpenRecord(const void *rec)
{
    unsigned k;
    for(k=0u;k<8u;++k)
        if(rec==(const void *)s_btnRec[k] && s_btnRec[k][1]==0xCEu) {
            uint32_t id;
            memcpy(&id,&s_btnRec[k][4],sizeof id);
            (void)ExpCeViewer_OpenDefinition(id);
            return 1;
        }
    return 0;
}
int ExpCeViewer_MatchesNative(uint32_t def_id,uint8_t country,const void *rec)
{
    const ExpCeDef *d=ExpCe_Definition(def_id);
    const Competition *c=(const Competition *)rec;
    if(!ExpCeViewer_Active() || !d || !d->viewer_button ||
       d->world!=(uint16_t)World_Current() || d->country!=country ||
       !d->viewer_native_id_plus_one || !rec || (uintptr_t)rec<256u ||
       rec==(const void *)(uintptr_t)-1 || rec==(const void *)(uintptr_t)-2)return 0;
    return (c->type==1 || c->type==2) &&
        (uint16_t)((uint8_t)c->compId+1u)==d->viewer_native_id_plus_one;
}
/* Competitions of a country with a CE button; replace an original row if
 * the definition declares its explicit native menu alias, else append. */
uint32_t ExpCeViewer_WorldButtons(uint8_t country,uint32_t *ids,
                                  char (*names)[40],uint32_t cap)
{
    uint32_t n;
    if(!ExpCeViewer_Active()) {
        fprintf(stderr,"[ce-viewer] WORLD_BUTTONS_INACTIVE country=%u world=%u game-type=%u\n",
                (unsigned)country,(unsigned)World_Current(),(unsigned)_gameType);
        return 0u;
    }
    n=cv_world_buttons(country,ids,names,cap);
    fprintf(stderr,"[ce-viewer] WORLD_BUTTONS country=%u count=%u readonly=1\n",
            (unsigned)country,(unsigned)n);
    for(uint32_t i=0u;i<n;++i) {
        const ExpCeEdition *e=cv_pick_edition(ids[i]);
        const ExpCeDef *d=ExpCe_Definition(ids[i]);
        fprintf(stderr,"[ce-viewer] WORLD_BUTTON def=%u name=%s edition=%s missing=%s\n",
            (unsigned)ids[i],names[i],e?ExpCe_StateName(e->state):"NONE",
            d && d->missing[0]?d->missing:"-");
    }
    return n;
}
/* Same, only for competitions the manager's club takes part in. */
uint32_t ExpCeViewer_ClubButtons(uint32_t *ids,char (*names)[40],uint32_t cap)
{
    ExpandedClubKey club={0};
    uint32_t i,n=0u;
    if(!ExpCeViewer_Active() ||
       !ExpandedUniversal_NativeTeamKey(&_careerTeam,&club))return 0u;
    for(i=0u;i<ExpCe_DefinitionCount() && n<cap;++i) {
        const ExpCeDef *d=ExpCe_DefinitionAt(i);
        uint32_t k;
        int in=0;
        if(!d || !d->viewer_button || d->world!=(uint16_t)World_Current())continue;
        /* v342 (user 27/11/2026, every competition): listed when the club is an entrant of ANY
         * live (not archived) edition, or when the next edition's participants are not yet
         * determined but the data already name the club (a LISTED source with a fixed list,
         * e.g. a founding club of the Australian Championship). */
        for(k=0u;k<ExpCe_EditionCount() && !in;++k) {
            const ExpCeEdition *x=ExpCe_EditionAt(k);
            uint32_t q;
            if(!x || x->def_id!=d->id || x->state==EXP_CE_ED_ARCHIVED)continue;
            for(q=0u;q<x->nentrants;++q)
                if(ExpandedClubKey_Equal(x->entrants[q].club,club)){in=1;break;}
            if(!in && !x->nentrants) {
                uint32_t sidx,c;
                for(sidx=0u;sidx<d->nsources && !in;++sidx) {
                    const ExpCeSourceSpec *sp=&d->sources[sidx];
                    if(sp->source!=EXP_CE_SRC_LISTED || sp->nclubs!=sp->count)continue;
                    for(c=0u;c<sp->nclubs;++c)if(sp->clubs[c]==club.stable_club){in=1;break;}
                }
            }
        }
        if(!in)continue;
        ids[n]=d->id;
        snprintf(names[n],40u,"%s",d->name);
        cv_upper(names[n]);
        ++n;
    }
    return n;
}
#endif /* !EXP_CE_VIEWER_PURE */
#ifdef EXP_CE_VIEWER_PURE
/* isolated tests: the same content builder and country enumeration, no menu */
uint32_t ExpCeViewer_TestWorldButtons(uint8_t country,uint32_t *ids,
                                     char (*names)[40],uint32_t cap)
{return cv_world_buttons(country,ids,names,cap);}
uint32_t ExpCeViewer_TestBuild(uint32_t def_id,char *title,size_t n)
{
    const ExpCeDef *d=ExpCe_Definition(def_id);
    s_reg=ExpandedUniversal_CarRegistry();
    if(!d || !s_reg)return 0u;
    cv_build_view(d,cv_pick_edition(def_id));
    snprintf(title,n,"%s",s_title);
    return s_n;
}
const char *ExpCeViewer_TestLine(uint32_t i,char *b,size_t n)
{
    unsigned c;
    if(i>=s_n)return NULL;
    snprintf(b,n,"%s|%s|",s_lines[i].left,s_lines[i].right);
    for(c=0u;c<s_lines[i].ncol;++c)
        snprintf(b+strlen(b),n-strlen(b),"%s ",s_lines[i].col[c]);
    return b;
}
#endif
#endif
