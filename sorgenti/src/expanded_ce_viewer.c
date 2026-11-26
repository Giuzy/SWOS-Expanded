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
        snprintf(s,sizeof s,"%u",(unsigned)ed->nentrants);
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
    cv_build_view(d,e);
    s_listMode=0;
    fprintf(stderr,"[ce-viewer] OPEN def=%u name=%s edition=%s lines=%u "
        "source=CE readonly=1\n",(unsigned)def_id,d->name,
        e?ExpCe_StateName(e->state):"NONE",(unsigned)s_n);
    return cv_show();
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
    uint32_t i,n=0u;
    if(!ExpCeViewer_Active())return 0u;
    for(i=0u;i<ExpCe_DefinitionCount() && n<cap;++i) {
        const ExpCeDef *d=ExpCe_DefinitionAt(i);
        if(!d || !d->viewer_button || d->country!=country ||
           d->world!=(uint16_t)World_Current() || !cv_pick_edition(d->id))continue;
        ids[n]=d->id;
        snprintf(names[n],40u,"%s",d->name);
        cv_upper(names[n]);
        ++n;
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
        const ExpCeEdition *e;
        uint32_t k;
        int in=0;
        if(!d || !d->viewer_button || d->world!=(uint16_t)World_Current())continue;
        e=cv_pick_edition(d->id);
        if(!e || e->state==EXP_CE_ED_ARCHIVED)continue;
        for(k=0u;k<e->nentrants;++k)
            if(ExpandedClubKey_Equal(e->entrants[k].club,club)){in=1;break;}
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
/* isolated tests: the same content builder, no menu */
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
