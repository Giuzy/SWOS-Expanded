/* SWOS Expanded — Fase 2 universal Competition Engine lifecycle.
 * See include/expanded_ce_engine.h. Pure C11 (planner needs SWOS_SDL_BUILD
 * only because of its file guard; no SDL symbol is used here).
 */
#include "expanded_ce_engine.h"
#include "expanded_universal_date_planner.h"
#include "expanded_universal_pairings.h"
#include "expanded_world_time.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
    ExpCeDef *defs;
    uint32_t ndefs, cdefs;
    ExpCeEdition *eds;
    uint32_t neds, ceds;
    ExpCePostponement *posts;
    uint32_t nposts, cposts;
    uint16_t world;
    uint32_t last_def; /* lookup cache */
} s_ce;

static int ce_grow(void **arr,uint32_t *cap,uint32_t need,size_t size)
{
    uint32_t next;
    void *p;
    if(need<=*cap)return 1;
    next=*cap?*cap:16u;
    while(next<need){if(next>0x7fffffffu)return 0;next*=2u;}
    p=realloc(*arr,(size_t)next*size);
    if(!p)return 0;
    *arr=p;*cap=next;
    return 1;
}
void ExpCe_Reset(void)
{
    ExpCeTable_HistoryReset();
    free(s_ce.defs);free(s_ce.eds);free(s_ce.posts);
    memset(&s_ce,0,sizeof s_ce);
}
uint32_t ExpCe_DefinitionCount(void){return s_ce.ndefs;}
const ExpCeDef *ExpCe_DefinitionAt(uint32_t i){return i<s_ce.ndefs?&s_ce.defs[i]:NULL;}
static ExpCeDef *def_find(uint32_t id)
{
    uint32_t i;
    if(s_ce.last_def<s_ce.ndefs && s_ce.defs[s_ce.last_def].id==id)
        return &s_ce.defs[s_ce.last_def];
    for(i=0;i<s_ce.ndefs;++i)
        if(s_ce.defs[i].id==id){s_ce.last_def=i;return &s_ce.defs[i];}
    return NULL;
}
const ExpCeDef *ExpCe_Definition(uint32_t id){return def_find(id);}
uint32_t ExpCe_EditionCount(void){return s_ce.neds;}
const ExpCeEdition *ExpCe_EditionAt(uint32_t i){return i<s_ce.neds?&s_ce.eds[i]:NULL;}
static ExpCeEdition *ed_find(ExpandedEditionKey k)
{
    uint32_t i;
    for(i=0;i<s_ce.neds;++i)
        if(ExpandedEditionKey_Equal(s_ce.eds[i].key,k))return &s_ce.eds[i];
    return NULL;
}
const ExpCeEdition *ExpCe_Edition(ExpandedEditionKey k){return ed_find(k);}
uint32_t ExpCe_PostponementCount(void){return s_ce.nposts;}
const ExpCePostponement *ExpCe_PostponementAt(uint32_t i)
{return i<s_ce.nposts?&s_ce.posts[i]:NULL;}

static const char *const k_states[]={"?","CREATED","PARTICIPANTS_CONFIRMED",
    "SCHEDULED","ACTIVE","COMPLETED","ARCHIVED"};
static const char *const k_blocks[]={"NONE","DATA_MISSING","RULES_INVALID",
    "RULES_CHANGED","DATE_PLAN","COLLISION","DEPENDENCY","TIE_UNRESOLVED",
    "ROSTER","ENTRANTS","MATCH_REFUSED","CAPACITY"};
static const char *const k_sources[]={"?","LISTED","START_DIVISION","RETAINED",
    "PROMOTED","RELEGATED","LEAGUE_RANK","CUP_WINNER","CUP_RUNNER_UP",
    "PHASE_RANK","PHASE_WINNER","BYE","REALLOCATED","JOIN","KEPT_NO_MOVEMENT"};
const char *ExpCe_StateName(uint8_t s){return s<=6u?k_states[s]:"?";}
const char *ExpCe_BlockName(uint8_t b){return b<=11u?k_blocks[b]:"?";}
const char *ExpCe_SourceName(uint8_t s){return s<=14u?k_sources[s]:"?";}

uint32_t ExpCe_LeagueCompetitionId(uint8_t country,uint8_t comp_id,
                                   uint8_t division)
{
    /* SAME formula as expanded_universal_world_catalog.c::cat_pair_emit:
     * a league's competition id never changes across editions. */
    return ((uint32_t)(country+1u)<<24)|((uint32_t)comp_id<<16)|
           ((uint32_t)(division+1u)<<8)|1u;
}

/* ------------------------------------------------------------------ */
/* definitions                                                          */
/* ------------------------------------------------------------------ */
static uint64_t def_hash(const ExpCeDef *d)
{
    uint64_t h=UINT64_C(14695981039346656037);
    uint32_t p;
    for(p=0;p<d->nphases;++p) {
        const ExpCePhaseDef *ph=&d->phases[p];
        const uint64_t r=ExpCeRules_Hash(ph->kind==EXP_CE_PHASE_TABLE?
                                         &ph->table:NULL,&ph->match);
        uint8_t b[8];
        unsigned i;
        b[0]=ph->kind;b[1]=ph->groups;b[2]=ph->meetings;b[3]=ph->qualify;
        b[4]=ph->pairing;b[5]=ph->byes;b[6]=ph->higher_home;b[7]=(uint8_t)p;
        for(i=0;i<8u;++i){h^=b[i];h*=UINT64_C(1099511628211);}
        for(i=0;i<8u;++i){h^=(uint8_t)(r>>(8u*i));h*=UINT64_C(1099511628211);}
        for(i=0;i<4u;++i){h^=(uint8_t)(ph->host_nonmember_comp>>(8u*i));h*=UINT64_C(1099511628211);}
        if(ph->extras.rounds || ph->extras.nrivalries) {
            uint32_t k,j,z;
            h^=ph->extras.rounds;h*=UINT64_C(1099511628211);
            h^=ph->extras.nrivalries;h*=UINT64_C(1099511628211);
            for(k=0u;k<ph->extras.nrivalries&&k<EXP_PAIR_MAX_RIVALRIES;++k)
                for(j=0u;j<2u;++j)for(z=0u;z<4u;++z) {
                    h^=(uint8_t)(ph->extras.rivalries[k][j]>>(8u*z));
                    h*=UINT64_C(1099511628211);
                }
        }
    }
    return h;
}
static int def_check(ExpCeDef *d)
{
    uint32_t p;
    ExpCeRuleError err;
    char buf[96];
    if(d->missing[0])return 0;
    if(!d->id || d->nphases<1u || d->nphases>EXP_CE_MAX_PHASES || d->type>2u) {
        snprintf(d->missing,sizeof d->missing,"definition: id/type/phases invalid");
        return 0;
    }
    for(p=0;p<d->nphases;++p) {
        ExpCePhaseDef *ph=&d->phases[p];
        if(ph->missing[0]) {
            snprintf(buf,sizeof buf,"phase %u: %.79s",(unsigned)p,ph->missing);
            snprintf(d->missing,sizeof d->missing,"%s",buf);
            return 0;
        }
        if(ph->kind==EXP_CE_PHASE_TABLE) {
            if(ph->pairing==EXP_CE_PAIR_RESEED_TABLE) {
                snprintf(d->missing,sizeof d->missing,"phase %u: RESEED_TABLE is KO only",(unsigned)p);
                return 0;
            }
            if(!ExpCeRules_ValidateTable(&ph->table,&err)) {
                snprintf(d->missing,sizeof d->missing,"phase %u %.24s: %.48s",
                         (unsigned)p,err.field,err.reason);return 0;
            }
            if((ph->extras.rounds||ph->extras.nrivalries) &&
               (ph->groups!=1u || !ExpPair_ValidExtras(d->clubs,ph->meetings,&ph->extras))) {
                snprintf(d->missing,sizeof d->missing,"phase %u: extra round policy invalid",(unsigned)p);
                return 0;
            }
            if(!ph->groups || ph->groups>EXP_CE_MAX_GROUPS || !ph->meetings ||
               ph->meetings>4u || ph->match.needs_winner) {
                snprintf(d->missing,sizeof d->missing,
                         "phase %u: groups/meetings invalid",(unsigned)p);
                return 0;
            }
        } else if(ph->kind==EXP_CE_PHASE_KNOCKOUT) {
            if(ph->pairing==EXP_CE_PAIR_RESEED_TABLE &&
               (!p || d->phases[0].kind!=EXP_CE_PHASE_TABLE ||
                d->phases[0].groups!=1u)) {
                snprintf(d->missing,sizeof d->missing,
                         "phase %u: RESEED_TABLE requires opening single table",(unsigned)p);
                return 0;
            }
            if(!ph->match.needs_winner || !ph->pairing ||
               (ph->pairing!=EXP_CE_PAIR_LISTED && ph->higher_home!=1u &&
                ph->higher_home!=2u)) {
                snprintf(d->missing,sizeof d->missing,
                         "phase %u: knockout needs pairing+higher_home",(unsigned)p);
                return 0;
            }
        } else {
            snprintf(d->missing,sizeof d->missing,"phase %u: kind",(unsigned)p);
            return 0;
        }
        if(!ExpCeRules_ValidateMatch(&ph->match,&err)) {
            snprintf(d->missing,sizeof d->missing,"phase %u %.24s: %.48s",
                     (unsigned)p,err.field,err.reason);return 0;
        }
        if(!ph->has_window && !ph->ndays) {
            snprintf(d->missing,sizeof d->missing,
                     "phase %u: no date window/days",(unsigned)p);return 0;
        }
        if(p+1u<d->nphases && ph->kind==EXP_CE_PHASE_TABLE &&
           !ph->qualify) {
            snprintf(d->missing,sizeof d->missing,
                     "phase %u: qualifiers to next phase missing",(unsigned)p);
            return 0;
        }
    }
    {
        const ExpCePhaseDef *last=&d->phases[d->nphases-1u];
        if(last->kind==EXP_CE_PHASE_TABLE && last->groups!=1u) {
            snprintf(d->missing,sizeof d->missing,
                     "last phase has groups: no title decider");
            return 0;
        }
    }
    return 1;
}
int ExpCe_AddDefinition(const ExpCeDef *def)
{
    ExpCeDef *d;
    if(!def || !def->id || def_find(def->id))return 0;
    if(!ce_grow((void **)&s_ce.defs,&s_ce.cdefs,s_ce.ndefs+1u,sizeof *s_ce.defs))
        return 0;
    d=&s_ce.defs[s_ce.ndefs++];
    *d=*def;
    (void)def_check(d);
    d->rules_hash=def_hash(d);
    return 1;
}
int ExpCe_LeagueDefinition(uint16_t world,uint8_t country,uint8_t comp_id,
    uint8_t division,uint8_t divisions,uint16_t clubs,uint8_t meetings,
    uint8_t points_for_win,uint8_t subs,uint8_t bench,uint8_t promoted,
    uint8_t relegated,uint8_t promoted_po,uint8_t relegated_po,
    const ExpCeWindow *window,ExpCeDef *out)
{
    ExpCePhaseDef *ph;
    if(!out)return 0;
    memset(out,0,sizeof *out);
    out->id=ExpCe_LeagueCompetitionId(country,comp_id,division);
    out->world=world;out->country=country;out->type=0u;
    out->origin=EXP_CE_ORIGIN_LEAGUE;
    out->comp_id=comp_id;out->division=division;out->divisions=divisions;
    out->promoted=promoted;out->relegated=relegated;out->clubs=clubs;
    snprintf(out->name,sizeof out->name,"LEAGUE %u/%u/%u",
             (unsigned)country,(unsigned)comp_id,(unsigned)division+1u);
    snprintf(out->source_file,sizeof out->source_file,"leagues.json");
    out->nphases=1u;
    ph=&out->phases[0];
    ph->kind=EXP_CE_PHASE_TABLE;ph->groups=1u;ph->meetings=meetings;
    if(!ExpCeRules_FromOgLeague(points_for_win,subs,bench,&ph->table,&ph->match))
        snprintf(ph->missing,sizeof ph->missing,
                 "OG league rules invalid (points/subs/bench)");
    if(window){ph->has_window=1u;ph->window=*window;}
    else snprintf(ph->missing,sizeof ph->missing,"league_calendar.json row");
    /* Playoff/playout counts exist in the OG record but their format does
     * not: never invent a playoff bracket. */
    if(promoted_po || relegated_po)
        snprintf(out->missing,sizeof out->missing,
                 "promotion/relegation playoff format not defined");
    out->has_expiry=0u; /* expiry = end of the league window */
    return 1;
}

/* ------------------------------------------------------------------ */
/* dates                                                                */
/* ------------------------------------------------------------------ */
static int md_on_or_after(uint8_t m,uint8_t d,int32_t from,int32_t *out)
{
    ExpandedWorldDate f;
    uint32_t k;
    if(!ExpandedWorld_IndexToDate(from,&f))return 0;
    for(k=0;k<3u;++k) {
        ExpandedWorldDate x={(uint16_t)(f.year+k),m,d};
        int32_t idx=0;
        if(m==2u && d==29u && !ExpandedWorld_DateToIndex(x,&idx))x.day=28u;
        if(!ExpandedWorld_DateToIndex(x,&idx))return 0;
        if(idx>=from){*out=idx;return 1;}
    }
    return 0;
}
static int edition_start_day(const ExpCeDef *d,uint16_t year,int32_t *out)
{
    const ExpCeWindow *w=&d->phases[0].window;
    if(d->phases[0].ndays && year==d->first_year){*out=d->phases[0].days[0];return 1;}
    if(!d->phases[0].has_window)return 0;
    ExpandedWorldDate x={year,w->first_m,w->first_d};
    return ExpandedWorld_DateToIndex(x,out);
}
static int edition_expiry_day(const ExpCeDef *d,uint16_t year,int32_t start,
                              int32_t *out)
{
    const ExpCePhaseDef *lp=&d->phases[d->nphases-1u];
    const ExpCeWindow *w=&lp->window;
    if(d->has_expiry) {
        ExpandedWorldDate x={(uint16_t)(year+d->expiry_year_offset),
                             d->expiry_m,d->expiry_d};
        return ExpandedWorld_DateToIndex(x,out);
    }
    if(lp->ndays && year==d->first_year){*out=lp->days[lp->ndays-1u];return 1;}
    if(!lp->has_window)return 0;
    /* the last phase window end, first occurrence after the start */
    return md_on_or_after(w->last_m,w->last_d,start,out);
}
/* Plan `rounds` civil days for a phase. earliest = first allowed day. */
static int plan_window_days(const ExpCePhaseDef *ph,int32_t edition_start,
                            int32_t earliest,uint32_t rounds,int32_t *days);
/* explicit verified days (first edition) or the window planner */
static int plan_phase_days_ex(const ExpCeDef *d,const ExpCeEdition *e,
                              const ExpCePhaseDef *ph,int32_t earliest,
                              uint32_t rounds,int32_t *days)
{
    if(ph->ndays && e->year==d->first_year) {
        uint32_t i;
        if(ph->ndays!=rounds)return 0;
        for(i=0;i<rounds;++i) {
            if(ph->days[i]<earliest || (i && ph->days[i]<=ph->days[i-1u]))return 0;
            days[i]=ph->days[i];
        }
        return 1;
    }
    if(!ph->has_window)return 0;
    return plan_window_days(ph,e->key.start_day,earliest,rounds,days);
}
static int plan_window_days(const ExpCePhaseDef *ph,int32_t edition_start,
                            int32_t earliest,uint32_t rounds,int32_t *days)
{
    ExpandedDatePlanPolicy pol;
    const ExpCeWindow *w=&ph->window;
    int32_t first=0,last=0;
    uint32_t i;
    int reason=0;
    memset(&pol,0,sizeof pol);
    if(!md_on_or_after(w->first_m,w->first_d,edition_start,&first))return 0;
    if(!md_on_or_after(w->last_m,w->last_d,first,&last))return 0;
    if(w->last_year_offset) {
        /* explicit year offset (league windows crossing New Year) */
        ExpandedWorldDate fs,x;
        if(!ExpandedWorld_IndexToDate(first,&fs))return 0;
        x=(ExpandedWorldDate){(uint16_t)(fs.year+w->last_year_offset),
                              w->last_m,w->last_d};
        if(!ExpandedWorld_DateToIndex(x,&last))return 0;
    }
    if(first<earliest)first=earliest;
    if(last<first)return 0;
    pol.first_day=first;pol.last_day=last;
    pol.match_weekdays=w->weekdays;pol.extra_weekdays=w->extra_weekdays;
    pol.min_rest_days=w->rest;
    for(i=0;i<w->nbreaks && i<EXP_DATE_PLAN_MAX_BREAKS;++i) {
        ExpandedDatePlanBreak *b=&pol.breaks[pol.break_count];
        if(!md_on_or_after(w->breaks[i][0],w->breaks[i][1],first,&b->first_day) ||
           !md_on_or_after(w->breaks[i][2],w->breaks[i][3],b->first_day,
                           &b->last_day))return 0;
        ++pol.break_count;
    }
    return ExpandedDatePlanner_Plan(&pol,rounds,days,&reason);
}

/* ------------------------------------------------------------------ */
/* fixtures                                                             */
/* ------------------------------------------------------------------ */
static uint64_t fixture_event_id(const ExpandedFixtureKey *k)
{
    uint64_t h=UINT64_C(14695981039346656037);
    uint32_t v[7];
    unsigned i;
    v[0]=k->edition.competition.stable_competition;
    v[1]=(uint32_t)k->edition.start_day;v[2]=k->edition.sequence;
    v[3]=k->stage;v[4]=k->tie;v[5]=k->leg;v[6]=k->replay;
    for(i=0;i<sizeof v;++i){h^=((const uint8_t *)v)[i];h*=UINT64_C(1099511628211);}
    h&=UINT64_C(0x0fffffffffffffff);
    if(!h)h=1u;
    return ((uint64_t)k->edition.competition.world<<60)|h;
}
typedef struct GenBuf {
    ExpandedUniversalFixture *f;
    uint32_t n,cap;
} GenBuf;
static int gen_push(GenBuf *g,const ExpandedUniversalFixture *f)
{
    if(!ce_grow((void **)&g->f,&g->cap,g->n+1u,sizeof *g->f))return 0;
    g->f[g->n++]=*f;
    return 1;
}
static void fixture_base(const ExpCeDef *d,const ExpCeEdition *e,
                         ExpandedUniversalFixture *f)
{
    memset(f,0,sizeof *f);
    f->key.edition=e->key;
    f->country=d->country;
    f->slot=d->origin==EXP_CE_ORIGIN_LEAGUE?d->division:(uint8_t)e->phase;
    f->competition_type=d->type;
}
typedef struct RrCtx {
    GenBuf *g;
    const ExpCeDef *d;
    const ExpCeEdition *e;
    const ExpandedClubKey *clubs;
    uint32_t n,rounds_per_cycle,group,phase;
    const int32_t *days;
    uint32_t serial_base;
} RrCtx;
static int rr_emit(void *user,uint32_t cycle,uint32_t round,uint32_t serial,
                   uint32_t home,uint32_t away)
{
    RrCtx *c=(RrCtx *)user;
    ExpandedUniversalFixture f;
    if(home>=c->n || away>=c->n)return 0;
    fixture_base(c->d,c->e,&f);
    f.home=c->clubs[home];f.away=c->clubs[away];
    f.day_index=c->days[cycle*c->rounds_per_cycle+round];
    f.key.stage=EXP_CE_STAGE(c->phase,c->group);
    f.key.tie=c->serial_base+serial+1u;
    f.key.leg=(uint16_t)cycle;
    f.event_id=fixture_event_id(&f.key);
    return gen_push(c->g,&f);
}
static int key_cmp(const void *a,const void *b)
{
    const ExpandedClubKey *x=(const ExpandedClubKey *)a,*y=(const ExpandedClubKey *)b;
    return x->stable_club<y->stable_club?-1:x->stable_club>y->stable_club?1:0;
}
/* deterministic CE game draw (documented): FNV seeded by edition/phase */
static uint64_t draw_next(uint64_t *s)
{
    *s^=*s<<13;*s^=*s>>7;*s^=*s<<17;
    return *s;
}
static uint64_t draw_seed(const ExpCeEdition *e,uint32_t phase)
{
    uint64_t h=UINT64_C(14695981039346656037);
    uint32_t v[4]={e->key.competition.stable_competition,
                   (uint32_t)e->key.start_day,e->key.sequence,phase};
    unsigned i;
    for(i=0;i<sizeof v;++i){h^=((const uint8_t *)v)[i];h*=UINT64_C(1099511628211);}
    return h?h:1u;
}
static void draw_shuffle(ExpCeEntrant *a,uint32_t n,uint64_t *seed)
{
    uint32_t i;
    for(i=n;i>1u;--i) {
        const uint32_t j=(uint32_t)(draw_next(seed)%i);
        ExpCeEntrant t=a[i-1u];a[i-1u]=a[j];a[j]=t;
    }
}

/* ------------------------------------------------------------------ */
/* phase entrants, pairings, generation                                 */
/* ------------------------------------------------------------------ */
static int32_t stage_last_day(const ExpandedUniversalRegistry *r,
                              ExpandedEditionKey ed,uint32_t phase)
{
    uint32_t i;
    int32_t last=INT32_MIN;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        if(ExpandedEditionKey_Equal(f->key.edition,ed) &&
           EXP_CE_STAGE_PHASE(f->key.stage)==phase && f->day_index>last)
            last=f->day_index;
    }
    return last;
}
static uint32_t ko_ties(const ExpandedUniversalRegistry *r,
                        ExpandedEditionKey ed,uint32_t phase)
{
    uint32_t i,ties=0u;
    const uint32_t stage=EXP_CE_STAGE(phase,EXP_CE_KO_GROUP);
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        if(ExpandedEditionKey_Equal(f->key.edition,ed) &&
           f->key.stage==stage && f->key.tie>ties)ties=f->key.tie;
    }
    return ties;
}
static int phase_generated(const ExpandedUniversalRegistry *r,
                           ExpandedEditionKey ed,uint32_t phase)
{
    return stage_last_day(r,ed,phase)!=INT32_MIN;
}
static int entrant_has(const ExpCeEntrant *a,uint32_t n,ExpandedClubKey k)
{
    uint32_t i;
    for(i=0;i<n;++i)if(ExpandedClubKey_Equal(a[i].club,k))return 1;
    return 0;
}
/* 1 complete+decided, 0 pending, -1 unresolved/invalid tie. */
static int phase_complete(const ExpandedUniversalRegistry *r,const ExpCeDef *d,
                          const ExpCeEdition *e,uint32_t phase)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    uint32_t g,t,n=0u;
    if(!phase_generated(r,e->key,phase))return 0;
    if(ph->kind==EXP_CE_PHASE_TABLE) {
        for(g=1u;g<=ph->groups;++g)
            if(!ExpCeTable_StageComplete(r,e->key,EXP_CE_STAGE(phase,g),&n))
                return 0;
        return 1;
    }
    n=ko_ties(r,e->key,phase);
    for(t=1u;t<=n;++t) {
        ExpandedClubKey w,l;
        const ExpCeTieResult res=ExpCeTie_Resolve(r,e->key,
            EXP_CE_STAGE(phase,EXP_CE_KO_GROUP),t,&ph->match,&w,&l);
        if(res==EXP_CE_TIE_PENDING)return 0;
        if(res!=EXP_CE_TIE_DECIDED)return -1;
    }
    return n>0u;
}
static int phase_entrants(const ExpandedUniversalRegistry *r,
                          const ExpCeDef *d,const ExpCeEdition *e,
                          const ExpCeHost *host,uint32_t phase,
                          ExpCeEntrant *out,uint32_t *n);
/* qualifiers of phase p (to p+1), byes of p included */
static int phase_qualifiers(const ExpandedUniversalRegistry *r,
                            const ExpCeDef *d,const ExpCeEdition *e,
                            const ExpCeHost *host,uint32_t phase,
                            ExpCeEntrant *out,uint32_t *n)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    uint32_t k=0u,g,t;
    *n=0u;
    if(ph->kind==EXP_CE_PHASE_TABLE) {
        ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
        uint32_t used=0u,rank;
        /* rank-major order: all group winners first, then runners-up */
        for(rank=1u;rank<=ph->qualify;++rank)
            for(g=1u;g<=ph->groups;++g) {
                if(!ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(phase,g),&ph->table,
                       host?host->name_cmp:NULL,host?host->user:NULL,
                       rows,EXP_CE_MAX_ENTRANTS,&used) || used<rank ||
                   k>=EXP_CE_MAX_ENTRANTS)return 0;
                out[k]=(ExpCeEntrant){rows[rank-1u].club,EXP_CE_SRC_PHASE_RANK,
                    (uint8_t)rank,(uint8_t)g,(uint8_t)(k+1u),0u,
                    e->key.competition.stable_competition,e->key.start_day};
                ++k;
            }
    } else {
        ExpCeEntrant prev[EXP_CE_MAX_ENTRANTS];
        uint32_t np=0u,ties=ko_ties(r,e->key,phase);
        for(t=1u;t<=ties;++t) {
            ExpandedClubKey w,l;
            if(ExpCeTie_Resolve(r,e->key,EXP_CE_STAGE(phase,EXP_CE_KO_GROUP),t,
                                &ph->match,&w,&l)!=EXP_CE_TIE_DECIDED ||
               k>=EXP_CE_MAX_ENTRANTS)return 0;
            out[k]=(ExpCeEntrant){w,EXP_CE_SRC_PHASE_WINNER,0u,0u,
                (uint8_t)(k+1u),0u,e->key.competition.stable_competition,
                e->key.start_day};
            ++k;
        }
        /* byes of this phase go on, before the winners (they are seeds) */
        if(ph->byes) {
            uint32_t b;
            if(!phase_entrants(r,d,e,host,phase,prev,&np) || np<ph->byes)
                return 0;
            if(k+ph->byes>EXP_CE_MAX_ENTRANTS)return 0;
            memmove(out+ph->byes,out,(size_t)k*sizeof *out);
            for(b=0;b<ph->byes;++b) {
                out[b]=prev[b];out[b].source=EXP_CE_SRC_BYE;
                out[b].seed=(uint8_t)(b+1u);out[b].join_phase=0u;
            }
            k+=ph->byes;
        }
    }
    *n=k;
    return 1;
}
/* Reconstruct original single-table seeding from the authoritative ledger.
 * No persisted random draw or extra writer; active editions retain all
 * earlier phase fixtures. Never seed from incomplete results. */
static int reseed_table(const ExpandedUniversalRegistry *r,const ExpCeDef *d,
                        const ExpCeEdition *e,const ExpCeHost *host,
                        ExpCeEntrant *out,uint32_t n)
{
    ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
    uint32_t nr=0u,i,j;
    if(d->phases[0].kind!=EXP_CE_PHASE_TABLE || d->phases[0].groups!=1u ||
       phase_complete(r,d,e,0u)!=1 ||
       !ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(0u,1u),&d->phases[0].table,
           host?host->name_cmp:NULL,host?host->user:NULL,
           rows,EXP_CE_MAX_ENTRANTS,&nr) || nr!=e->nentrants)return 0;
    for(i=0;i<n;++i) {
        for(j=0;j<nr;++j)
            if(ExpandedClubKey_Equal(out[i].club,rows[j].club))break;
        if(j==nr || j>=d->phases[0].qualify)return 0;
        out[i].seed=(uint8_t)(j+1u);
        for(j=0;j<i;++j)if(out[j].seed==out[i].seed)return 0;
    }
    for(i=1u;i<n;++i) {
        const ExpCeEntrant x=out[i];
        j=i;
        while(j && out[j-1u].seed>x.seed){out[j]=out[j-1u];--j;}
        out[j]=x;
    }
    return 1;
}
static int phase_entrants(const ExpandedUniversalRegistry *r,
                          const ExpCeDef *d,const ExpCeEdition *e,
                          const ExpCeHost *host,uint32_t phase,
                          ExpCeEntrant *out,uint32_t *n)
{
    uint32_t i,k=0u;
    *n=0u;
    if(phase>0u && !phase_qualifiers(r,d,e,host,phase-1u,out,&k))return 0;
    for(i=0;i<e->nentrants;++i)
        if(e->entrants[i].join_phase==phase) {
            if(k>=EXP_CE_MAX_ENTRANTS ||
               entrant_has(out,k,e->entrants[i].club))return 0;
            out[k]=e->entrants[i];
            if(phase>0u)out[k].source=out[k].source?out[k].source:EXP_CE_SRC_JOIN;
            ++k;
        }
    if(d->phases[phase].pairing==EXP_CE_PAIR_RESEED_TABLE &&
       !reseed_table(r,d,e,host,out,k))return 0;
    *n=k;
    return 1;
}
typedef struct KoPair { ExpCeEntrant hi,lo; } KoPair;
/* membership of a club in the CURRENT (latest) edition of a competition */
static int club_in_current_edition(uint32_t comp,ExpandedClubKey c)
{
    const ExpCeEdition *best=NULL;
    uint32_t i;
    for(i=0;i<s_ce.neds;++i) {
        const ExpCeEdition *o=&s_ce.eds[i];
        if(o->def_id!=comp || !o->nentrants)continue;
        if(!best || o->key.start_day>best->key.start_day)best=o;
    }
    return best && entrant_has(best->entrants,best->nentrants,c);
}
static void apply_host_rule(const ExpCeDef *d,uint32_t phase,KoPair *pairs,uint32_t np)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    uint32_t i;
    if(!ph->host_nonmember_comp || ph->match.legs!=1u || phase+1u>=d->nphases ||
       (ph->higher_home!=1u && ph->pairing!=EXP_CE_PAIR_LISTED))return;
    for(i=0;i<np;++i) {
        const int hi_in=club_in_current_edition(ph->host_nonmember_comp,pairs[i].hi.club);
        const int lo_in=club_in_current_edition(ph->host_nonmember_comp,pairs[i].lo.club);
        if(hi_in && !lo_in) { /* the non-member hosts the member */
            const ExpCeEntrant t=pairs[i].hi;
            pairs[i].hi=pairs[i].lo;pairs[i].lo=t;
        }
    }
}
static int ko_pairs_raw(const ExpCeDef *d,const ExpCeEdition *e,uint32_t phase,
                    ExpCeEntrant *ent,uint32_t n,KoPair *pairs,uint32_t *np,
                    char *why,size_t whylen)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    uint32_t i,m;
    ExpCeEntrant *pool=ent+ph->byes;
    if(n<ph->byes+2u || ((n-ph->byes)&1u)) {
        snprintf(why,whylen,"knockout phase %u: %lu entrants, %u byes: odd",
                 (unsigned)phase,(unsigned long)n,(unsigned)ph->byes);
        return 0;
    }
    m=n-ph->byes;
    *np=m/2u;
    switch(ph->pairing) {
    case EXP_CE_PAIR_LISTED:
    case EXP_CE_PAIR_BRACKET:
        for(i=0;i<*np;++i){pairs[i].hi=pool[2u*i];pairs[i].lo=pool[2u*i+1u];}
        return 1;
    case EXP_CE_PAIR_RESEED_TABLE:
    case EXP_CE_PAIR_SEEDED:
        for(i=0;i<*np;++i){pairs[i].hi=pool[i];pairs[i].lo=pool[m-1u-i];}
        return 1;
    case EXP_CE_PAIR_DRAW: {
        uint64_t seed=draw_seed(e,phase);
        draw_shuffle(pool,m,&seed);
        for(i=0;i<*np;++i){pairs[i].hi=pool[2u*i];pairs[i].lo=pool[2u*i+1u];}
        return 1;
    }
    case EXP_CE_PAIR_CROSS_GROUPS: {
        /* pool is rank-major: winners W[0..G), runners-up R[0..G) */
        const uint32_t groups=m/2u;
        if(groups<2u || (groups&1u)) {
            snprintf(why,whylen,"CROSS_GROUPS needs an even number of groups "
                     "and 2 qualifiers each");
            return 0;
        }
        for(i=0;i<groups;++i) {
            const uint32_t other=i^1u;
            if(pool[i].rank!=1u || pool[groups+other].rank!=2u) {
                snprintf(why,whylen,"CROSS_GROUPS: qualifiers not 1st/2nd");
                return 0;
            }
            pairs[i].hi=pool[i];pairs[i].lo=pool[groups+other];
        }
        return 1;
    }
    default:
        snprintf(why,whylen,"knockout pairing %u unsupported",(unsigned)ph->pairing);
        return 0;
    }
}
static int ko_pairs(const ExpCeDef *d,const ExpCeEdition *e,uint32_t phase,
                    ExpCeEntrant *ent,uint32_t n,KoPair *pairs,uint32_t *np,
                    char *why,size_t whylen)
{
    if(!ko_pairs_raw(d,e,phase,ent,n,pairs,np,why,whylen))return 0;
    apply_host_rule(d,phase,pairs,*np);
    return 1;
}
static int form_groups(const ExpCeDef *d,const ExpCeEdition *e,uint32_t phase,
                       ExpCeEntrant *ent,uint32_t n,
                       ExpandedClubKey groups[EXP_CE_MAX_GROUPS][EXP_CE_MAX_ENTRANTS],
                       uint32_t *size,char *why,size_t whylen)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    uint32_t i,g;
    memset(size,0,EXP_CE_MAX_GROUPS*sizeof *size);
    if(ph->groups==1u) {
        for(i=0;i<n;++i)groups[0][size[0]++]=ent[i].club;
    } else if(ph->pairing==EXP_CE_PAIR_LISTED) {
        for(i=0;i<n;++i) {
            g=ent[i].group;
            if(g<1u || g>ph->groups) {
                snprintf(why,whylen,"phase %u: entrant without listed group",
                         (unsigned)phase);
                return 0;
            }
            groups[g-1u][size[g-1u]++]=ent[i].club;
        }
    } else if(ph->pairing==EXP_CE_PAIR_POTS) {
        uint64_t seed=draw_seed(e,phase);
        if(n%ph->groups) {
            snprintf(why,whylen,"phase %u: %lu entrants not divisible by %u "
                     "groups",(unsigned)phase,(unsigned long)n,(unsigned)ph->groups);
            return 0;
        }
        for(i=0;i<n;i+=ph->groups) {
            draw_shuffle(ent+i,ph->groups,&seed);
            for(g=0;g<ph->groups;++g)groups[g][size[g]++]=ent[i+g].club;
        }
    } else {
        snprintf(why,whylen,"phase %u: group formation %u unsupported",
                 (unsigned)phase,(unsigned)ph->pairing);
        return 0;
    }
    for(g=0;g<ph->groups;++g) {
        if(size[g]<2u) {
            snprintf(why,whylen,"phase %u group %u: <2 clubs",(unsigned)phase,
                     (unsigned)g+1u);
            return 0;
        }
        qsort(groups[g],size[g],sizeof groups[g][0],key_cmp);
    }
    return 1;
}
/* Generate the fixtures of `phase` of edition e. 1 ok, 0 = blocked (why). */
static int generate_phase(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                          const ExpCeDef *d,ExpCeEdition *e,uint32_t phase,
                          int32_t today,uint8_t *block,char *why,size_t whylen)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    ExpCeEntrant ent[EXP_CE_MAX_ENTRANTS];
    uint32_t n=0u,i;
    GenBuf g={0};
    int32_t earliest=today+1;
    int ok=0;
    *block=EXP_CE_BLOCK_DATA_MISSING;
    if(phase>0u) {
        const int32_t prev=stage_last_day(r,e->key,phase-1u);
        if(prev!=INT32_MIN && prev+1+(int32_t)ph->window.rest>earliest)
            earliest=prev+1+(int32_t)ph->window.rest;
    }
    if(!phase_entrants(r,d,e,host,phase,ent,&n) || n<2u) {
        snprintf(why,whylen,"phase %u: entrants unavailable (%lu)",
                 (unsigned)phase,(unsigned long)n);
        *block=EXP_CE_BLOCK_ENTRANTS;
        return 0;
    }
    /* v299: one participant kind per edition (clubs OR national teams) */
    for(i=0;i<n;++i)
        if(ent[i].club.kind!=ent[0].club.kind ||
           (phase>0u && ent[i].club.kind!=e->participant_kind)) {
            snprintf(why,whylen,"phase %u: clubs and national teams mixed",
                     (unsigned)phase);
            *block=EXP_CE_BLOCK_ENTRANTS;
            return 0;
        }
    if(phase==0u)e->participant_kind=ent[0].club.kind;
    for(i=0;i<n;++i)
        if(host && host->club_ok && !host->club_ok(host->user,ent[i].club)) {
            snprintf(why,whylen,"phase %u: club %lu has no roster",
                     (unsigned)phase,(unsigned long)ent[i].club.stable_club);
            *block=EXP_CE_BLOCK_ROSTER;
            return 0;
        }
    if(ph->kind==EXP_CE_PHASE_TABLE) {
        static ExpandedClubKey groups[EXP_CE_MAX_GROUPS][EXP_CE_MAX_ENTRANTS];
        uint32_t size[EXP_CE_MAX_GROUPS],rounds=0u,gi;
        int32_t days[2048];
        if(!form_groups(d,e,phase,ent,n,groups,size,why,whylen))return 0;
        for(gi=0;gi<ph->groups;++gi) {
            const uint32_t rr=((size[gi]&1u)?size[gi]:size[gi]-1u)*ph->meetings+ph->extras.rounds;
            if(rr>rounds)rounds=rr;
        }
        if(!rounds || rounds>2048u ||
           !plan_phase_days_ex(d,e,ph,earliest,rounds,days)) {
            snprintf(why,whylen,"phase %u: %lu rounds do not fit the window",
                     (unsigned)phase,(unsigned long)rounds);
            *block=EXP_CE_BLOCK_DATE_PLAN;
            return 0;
        }
        for(gi=0;gi<ph->groups;++gi) {
            RrCtx c;
            memset(&c,0,sizeof c);
            c.g=&g;c.d=d;c.e=e;c.clubs=groups[gi];c.n=size[gi];
            c.rounds_per_cycle=(size[gi]&1u)?size[gi]:size[gi]-1u;
            c.group=gi+1u;c.phase=phase;c.days=days;
            uint32_t keys[EXP_PAIR_MAX_CLUBS],j;
            if(ph->extras.rounds) {
                if(size[gi]>EXP_PAIR_MAX_CLUBS)goto done;
                for(j=0u;j<size[gi];++j)keys[j]=groups[gi][j].stable_club;
            }
            if(!ExpandedUniversal_GenerateLeague(size[gi],ph->meetings,
                 ph->extras.rounds?keys:NULL,&ph->extras,
                 ExpPair_Seed(d->id,e->key.start_day),rr_emit,&c)) {
                snprintf(why,whylen,"phase %u: round robin failed",(unsigned)phase);
                *block=EXP_CE_BLOCK_CAPACITY;goto done;
            }
        }
    } else {
        KoPair pairs[EXP_CE_MAX_ENTRANTS/2u];
        uint32_t np=0u,t;
        int32_t days[2];
        if(!ko_pairs(d,e,phase,ent,n,pairs,&np,why,whylen))return 0;
        if(!plan_phase_days_ex(d,e,ph,earliest,ph->match.legs,days)) {
            snprintf(why,whylen,"phase %u: knockout legs do not fit the window",
                     (unsigned)phase);
            *block=EXP_CE_BLOCK_DATE_PLAN;
            return 0;
        }
        for(t=0;t<np;++t) {
            uint32_t leg;
            /* higher_home: 1 higher seed hosts leg 1/single match;
             * 2 higher seed hosts the deciding leg (lower hosts a single) */
            const ExpCeEntrant *first=
                (ph->pairing==EXP_CE_PAIR_LISTED || ph->higher_home==1u)?
                &pairs[t].hi:&pairs[t].lo;
            const ExpCeEntrant *second=first==&pairs[t].hi?&pairs[t].lo:&pairs[t].hi;
            for(leg=0;leg<ph->match.legs;++leg) {
                ExpandedUniversalFixture f;
                fixture_base(d,e,&f);
                f.home=leg?second->club:first->club;
                f.away=leg?first->club:second->club;
                f.day_index=days[leg];
                f.key.stage=EXP_CE_STAGE(phase,EXP_CE_KO_GROUP);
                f.key.tie=t+1u;f.key.leg=(uint16_t)leg;
                f.event_id=fixture_event_id(&f.key);
                if(!gen_push(&g,&f)){*block=EXP_CE_BLOCK_CAPACITY;goto done;}
            }
        }
    }
    for(i=0;i<g.n;++i)
        if(!ExpandedUniversal_FixtureValid(&g.f[i])) {
            snprintf(why,whylen,"phase %u: generated fixture invalid",(unsigned)phase);
            *block=EXP_CE_BLOCK_CAPACITY;goto done;
        }
    if(!host || !host->insert || !host->insert(host->user,g.f,g.n)) {
        snprintf(why,whylen,"phase %u: %lu fixtures rejected by the registry "
                 "(same club same day / id collision)",(unsigned)phase,
                 (unsigned long)g.n);
        *block=EXP_CE_BLOCK_COLLISION;goto done;
    }
    (void)r;
    ok=1;
    fprintf(stderr,"[ce-engine] PHASE_GENERATED world=%u competition=%lu "
        "edition=%ld year=%u phase=%u kind=%s fixtures=%lu entrants=%lu "
        "dates=%s\n",(unsigned)e->key.competition.world,
        (unsigned long)e->key.competition.stable_competition,
        (long)e->key.start_day,(unsigned)e->year,(unsigned)phase,
        ph->kind==EXP_CE_PHASE_TABLE?"TABLE":"KNOCKOUT",(unsigned long)g.n,
        (unsigned long)n,
        ph->match.provenance==EXP_CE_PROV_JSON_VERIFIED?"VERIFIED_WINDOW":
        "GAME_TEMPLATE");
done:
    free(g.f);
    return ok;
}

/* ------------------------------------------------------------------ */
/* lifecycle                                                            */
/* ------------------------------------------------------------------ */
static ExpCeEdition *ed_new(const ExpCeDef *d,uint16_t year,int32_t start)
{
    ExpCeEdition *e;
    if(!ce_grow((void **)&s_ce.eds,&s_ce.ceds,s_ce.neds+1u,sizeof *s_ce.eds))
        return NULL;
    e=&s_ce.eds[s_ce.neds++];
    memset(e,0,sizeof *e);
    e->key.competition=(ExpandedCompetitionKey){d->world,d->id};
    e->key.start_day=start;e->key.sequence=0u;
    e->def_id=d->id;e->year=year;e->state=EXP_CE_ED_CREATED;
    e->rules_hash=d->rules_hash;
    return e;
}
static void ed_log(const char *tag,const ExpCeEdition *e,const char *why)
{
    ExpandedWorldDate s={0};
    (void)ExpandedWorld_IndexToDate(e->key.start_day,&s);
    fprintf(stderr,"[ce-engine] %s world=%u competition=%lu edition=%04u-%02u-%02u "
        "year=%u state=%s phase=%u block=%s why=%s\n",tag,
        (unsigned)e->key.competition.world,
        (unsigned long)e->key.competition.stable_competition,
        (unsigned)s.year,(unsigned)s.month,(unsigned)s.day,(unsigned)e->year,
        ExpCe_StateName(e->state),(unsigned)e->phase,ExpCe_BlockName(e->block),
        why?why:"-");
}
static void ed_block(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                     ExpCeEdition *e,uint8_t block,const char *why)
{
    uint32_t i;
    (void)r;
    if(e->block)return;
    e->block=block;
    snprintf(e->why,sizeof e->why,"%s",why?why:"");
    if(host && host->hold)(void)host->hold(host->user,e->key,1);
    ed_log("EDITION_BLOCKED",e,e->why);
    /* dependents: not-yet-confirmed editions that draw entrants from this
     * competition wait as DEPENDENCY (never consume an unfinished result) */
    for(i=0;i<s_ce.neds;++i) {
        ExpCeEdition *o=&s_ce.eds[i];
        const ExpCeDef *od=def_find(o->def_id);
        uint32_t k;
        if(o==e || o->block || o->state!=EXP_CE_ED_CREATED || !od ||
           o->nentrants)continue; /* listed entrants need no source */
        for(k=0;k<od->nsources;++k)
            if(od->sources[k].competition==e->key.competition.stable_competition) {
                char w[96];
                snprintf(w,sizeof w,"source competition %lu blocked",
                         (unsigned long)e->key.competition.stable_competition);
                ed_block(r,host,o,EXP_CE_BLOCK_DEPENDENCY,w);
                break;
            }
    }
}
int ExpCe_BlockEdition(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                       ExpandedEditionKey edition,uint8_t block,const char *why)
{
    ExpCeEdition *e=ed_find(edition);
    if(!e || !block)return 0;
    ed_block(r,host,e,block,why);
    return 1;
}
int ExpCe_EditionBlocked(ExpandedEditionKey edition)
{
    const ExpCeEdition *e=ed_find(edition);
    return e && e->block;
}
/* final order: champion first. TABLE last phase: table ranks.
 * KO last phase: winner, runner-up, then losers of earlier KO phases by
 * elimination (latest first), then group non-qualifiers by group rank. */
static int final_order(const ExpandedUniversalRegistry *r,const ExpCeDef *d,
                       ExpCeEdition *e,const ExpCeHost *host)
{
    int32_t p;
    uint32_t k=0u;
    e->nfinal=0u;
    for(p=(int32_t)d->nphases-1;p>=0;--p) {
        const ExpCePhaseDef *ph=&d->phases[p];
        if(ph->kind==EXP_CE_PHASE_TABLE) {
            ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
            uint32_t used=0u,rank,g;
            const uint32_t from=(uint32_t)p+1u==d->nphases?1u:(uint32_t)ph->qualify+1u;
            for(rank=from;rank<=EXP_CE_MAX_ENTRANTS;++rank) {
                int any=0;
                for(g=1u;g<=ph->groups;++g) {
                    if(!ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(p,g),&ph->table,
                           host?host->name_cmp:NULL,host?host->user:NULL,
                           rows,EXP_CE_MAX_ENTRANTS,&used))return 0;
                    if(used>=rank) {
                        if(k>=EXP_CE_MAX_ENTRANTS)return 0;
                        e->final_order[k++]=rows[rank-1u].club.stable_club;any=1;
                    }
                }
                if(!any)break;
            }
        } else {
            const uint32_t ties=ko_ties(r,e->key,(uint32_t)p);
            uint32_t t;
            if((uint32_t)p+1u==d->nphases) {
                ExpandedClubKey w,l;
                if(ties!=1u || ExpCeTie_Resolve(r,e->key,
                       EXP_CE_STAGE(p,EXP_CE_KO_GROUP),1u,&ph->match,&w,&l)!=
                   EXP_CE_TIE_DECIDED)return 0;
                e->final_order[k++]=w.stable_club;
                e->final_order[k++]=l.stable_club;
            } else
                for(t=1u;t<=ties;++t) {
                    ExpandedClubKey w,l;
                    if(ExpCeTie_Resolve(r,e->key,EXP_CE_STAGE(p,EXP_CE_KO_GROUP),
                           t,&ph->match,&w,&l)!=EXP_CE_TIE_DECIDED ||
                       k>=EXP_CE_MAX_ENTRANTS)return 0;
                    e->final_order[k++]=l.stable_club;
                }
        }
    }
    e->nfinal=k;
    if(d->origin==EXP_CE_ORIGIN_LEAGUE && d->nphases>1u) {
        ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
        uint32_t nr=0u,i;
        if(e->partial || k!=e->nentrants || d->phases[0].groups!=1u ||
           !ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(0u,1u),&d->phases[0].table,
                host?host->name_cmp:NULL,host?host->user:NULL,
                rows,EXP_CE_MAX_ENTRANTS,&nr) || nr!=e->nentrants)return 0;
        for(i=0;i<nr;++i)e->regular_order[i]=rows[i].club.stable_club;
        e->nregular=nr;
    }
    return k>=2u;
}
static uint32_t league_rank_count(const ExpCeEdition *e)
{return e->nregular?e->nregular:e->nfinal;}
static uint32_t league_rank_at(const ExpCeEdition *e,uint32_t i)
{return e->nregular?e->regular_order[i]:e->final_order[i];}
/* latest COMPLETED/ARCHIVED edition of a competition (source of entrants) */
static const ExpCeEdition *latest_closed(uint32_t competition,int *blocked)
{
    const ExpCeEdition *best=NULL;
    uint32_t i;
    *blocked=0;
    for(i=0;i<s_ce.neds;++i) {
        const ExpCeEdition *o=&s_ce.eds[i];
        if(o->key.competition.stable_competition!=competition)continue;
        if(o->block && o->state<EXP_CE_ED_COMPLETED)*blocked=1;
        if(o->state<EXP_CE_ED_COMPLETED || !o->nfinal)continue;
        if(o->partial){*blocked=1;continue;} /* not a sporting result */
        if(!best || o->key.start_day>best->key.start_day)best=o;
    }
    return best;
}
/* JSON successor: entrants from the source specs, with provenance.
 * 1 confirmed, 0 wait (source not complete yet), -1 blocked (why). */
static int resolve_sources(const ExpCeDef *d,ExpCeEdition *e,char *why,size_t wl)
{
    uint32_t s,k=0u,deficit=0u;
    if(!d->nsources) {
        snprintf(why,wl,"no participant rule for editions after the first");
        return -1;
    }
    /* Sources are applied in order. A cup slot whose club already
     * qualified passes on (deficit) ONLY if the spec says so; the next
     * LEAGUE_RANK spec with reallocate=1 takes that many extra clubs from
     * its table, skipping duplicates (e.g. "next best league club"). */
    for(s=0;s<d->nsources;++s) {
        const ExpCeSourceSpec *sp=&d->sources[s];
        int blocked=0;
        const ExpCeEdition *src;
        uint32_t want,rank,taken=0u;
        const int league=sp->source==EXP_CE_SRC_LEAGUE_RANK;
        if(sp->source==EXP_CE_SRC_LISTED) {
            /* ordered pool (placeholder clubs for paths not in the game):
             * the first `count` not already qualified; never random, never a
             * duplicate; an exhausted pool blocks the edition. */
            uint32_t q;
            want=sp->count;
            if(sp->reallocate){want+=deficit;deficit=0u;}
            for(q=0;q<sp->nclubs && taken<want;++q) {
                const ExpandedClubKey c={e->key.competition.world,sp->clubs[q],
                                         EXP_PARTICIPANT_CLUB};
                if(entrant_has(e->entrants,k,c))continue;
                if(k>=EXP_CE_MAX_ENTRANTS)return -1;
                e->entrants[k]=(ExpCeEntrant){c,EXP_CE_SRC_LISTED,0u,0u,
                    (uint8_t)(k+1u),0u,0u,0};
                ++k;++taken;
            }
            if(taken<want) {
                snprintf(why,wl,"listed pool exhausted: %lu of %lu clubs",
                         (unsigned long)taken,(unsigned long)want);
                return -1;
            }
            continue;
        }
        src=latest_closed(sp->competition,&blocked);
        if(!src) {
            if(blocked) {
                snprintf(why,wl,"source %lu blocked",(unsigned long)sp->competition);
                return -2; /* dependency */
            }
            if(!def_find(sp->competition)) {
                snprintf(why,wl,"source %lu not defined",(unsigned long)sp->competition);
                return -1;
            }
            return 0; /* wait for it */
        }
        if(src->key.start_day>=e->key.start_day) {
            snprintf(why,wl,"source %lu newer than the edition",
                     (unsigned long)sp->competition);
            return -1;
        }
        if(league) {
            if(!sp->rank_from || sp->rank_to<sp->rank_from) {
                snprintf(why,wl,"source %lu rank range invalid",
                         (unsigned long)sp->competition);
                return -1;
            }
            rank=sp->rank_from;
            want=(uint32_t)sp->rank_to-sp->rank_from+1u;
            if(sp->reallocate){want+=deficit;deficit=0u;}
        } else {
            rank=sp->source==EXP_CE_SRC_CUP_WINNER?1u:2u;
            want=1u;
        }
        while(taken<want) {
            ExpandedClubKey c;
            uint32_t xi;
            int skip=0;
            if(rank<1u || rank>(league?league_rank_count(src):src->nfinal)) {
                if(league && sp->reallocate) { /* table too short: the slots go
                                                  to the next receiving rule */
                    deficit+=want-taken;
                    break;
                }
                snprintf(why,wl,"source %lu has no rank %lu",
                         (unsigned long)sp->competition,(unsigned long)rank);
                return -1;
            }
            c=(ExpandedClubKey){e->key.competition.world,
              league?league_rank_at(src,rank-1u):src->final_order[rank-1u],
              src->participant_kind};
            for(xi=0;xi<sp->nexclude;++xi)
                if(sp->exclude[xi]==c.stable_club)skip=1;
            if(skip){++rank;continue;} /* excluded: does not use a slot */
            if(entrant_has(e->entrants,k,c)) {
                if(!sp->reallocate) {
                    snprintf(why,wl,"club %lu qualified twice, no reallocation rule",
                             (unsigned long)c.stable_club);
                    return -1;
                }
                if(!league){++deficit;break;} /* slot passes on */
                ++rank;continue;
            }
            if(k>=EXP_CE_MAX_ENTRANTS)return -1;
            e->entrants[k]=(ExpCeEntrant){c,
                (league && rank!=sp->rank_from+taken)?EXP_CE_SRC_REALLOCATED:
                    sp->source,
                (uint8_t)rank,0u,(uint8_t)(k+1u),0u,sp->competition,
                src->key.start_day};
            ++k;++taken;++rank;
        }
    }
    if(deficit) {
        snprintf(why,wl,"%lu reallocated slot(s) without a receiving rule",
                 (unsigned long)deficit);
        return -1;
    }
    e->nentrants=k;
    return 1;
}
/* 1 = the family's promotion/relegation counts form a simple chain
 * (relegated(d) == promoted(d+1), none above the top / below the bottom)
 * and every division is defined; 0 = rollover needs a distribution rule. */
int ExpCe_LeagueFamilyChainOk(uint8_t country,uint8_t comp_id)
{
    const ExpCeDef *d0=def_find(ExpCe_LeagueCompetitionId(country,comp_id,0u));
    uint32_t i;
    if(!d0 || !d0->divisions || d0->divisions>16u || d0->promoted)return 0;
    for(i=0;i<d0->divisions;++i) {
        const ExpCeDef *a=def_find(ExpCe_LeagueCompetitionId(country,comp_id,(uint8_t)i));
        const ExpCeDef *b=i+1u<d0->divisions?
            def_find(ExpCe_LeagueCompetitionId(country,comp_id,(uint8_t)(i+1u))):NULL;
        if(!a)return 0;
        if(i+1u==d0->divisions){if(a->relegated)return 0;}
        else if(!b || a->relegated!=b->promoted)return 0;
    }
    return 1;
}
/* Length of the LEADING simple chain of a family whose full chain is broken
 * (e.g. parallel groups below): divisions 0..len-1 with relegated(d) ==
 * promoted(d+1) > 0 for every inner pair. Movement is performed inside this
 * prefix only; the division at its bottom executes no relegation toward the
 * non-chain divisions and the divisions below keep their clubs. A prefix of
 * one division is no chain (the family stays blocked). */
static uint32_t league_chain_prefix(const ExpCeDef *d0)
{
    uint32_t i,len=1u;
    if(!d0 || !d0->divisions || d0->divisions>16u || d0->promoted)return 0u;
    for(i=0;i+1u<d0->divisions;++i) {
        const ExpCeDef *a=def_find(ExpCe_LeagueCompetitionId(d0->country,d0->comp_id,(uint8_t)i));
        const ExpCeDef *b=def_find(ExpCe_LeagueCompetitionId(d0->country,d0->comp_id,(uint8_t)(i+1u)));
        if(!a || !b || !a->relegated || a->relegated!=b->promoted)break;
        len=i+2u;
    }
    return len>=2u?len:0u;
}
/* Distinct calendar windows and no sporting links: these are independent
 * divisions sharing a native catalogue record, not one promotion chain.
 * Preserve the existing family path whenever a movement/link is declared. */
static int league_family_independent(const ExpCeDef *d)
{
    const ExpCeDef *first=NULL;
    uint32_t i;
    int different=0;
    if(!d || d->origin!=EXP_CE_ORIGIN_LEAGUE || d->divisions<2u ||
       d->divisions>16u)return 0;
    for(i=0u;i<d->divisions;++i) {
        const ExpCeDef *x=def_find(ExpCe_LeagueCompetitionId(d->country,d->comp_id,(uint8_t)i));
        const ExpCeWindow *w,*f;
        if(!x || x->divisions!=d->divisions || x->promoted || x->relegated ||
           x->missing[0] || !x->phases[0].has_window)return 0;
        if(!first){first=x;continue;}
        w=&x->phases[0].window;f=&first->phases[0].window;
        if(w->first_m!=f->first_m || w->first_d!=f->first_d ||
           w->last_m!=f->last_m || w->last_d!=f->last_d ||
           w->last_year_offset!=f->last_year_offset)different=1;
    }
    return different;
}
static int league_independent_rollover(ExpandedUniversalRegistry *r,
    const ExpCeHost *host,const ExpCeDef *d,ExpCeEdition *e)
{
    const uint32_t idx=(uint32_t)(e-s_ce.eds);
    const uint16_t year=(uint16_t)(e->year+1u);
    ExpCeEdition *n;
    int32_t start=0;
    uint32_t i;
    if(e->successor || e->block || e->year==65535u ||
       e->nentrants!=d->clubs || e->nentrants>EXP_CE_MAX_ENTRANTS)return 0;
    for(i=0u;i<s_ce.neds;++i)
        if(s_ce.eds[i].def_id==d->id && s_ce.eds[i].year==year)return 0;
    if(!edition_start_day(d,year,&start) || !(n=ed_new(d,year,start)))return 0;
    e=&s_ce.eds[idx]; /* ed_new may reallocate */
    e->successor=1u;
    n->nentrants=e->nentrants;
    for(i=0u;i<e->nentrants;++i)
        n->entrants[i]=(ExpCeEntrant){e->entrants[i].club,
            EXP_CE_SRC_KEPT_NO_MOVEMENT,0u,0u,(uint8_t)(i+1u),0u,
            d->id,e->key.start_day};
    n->state=EXP_CE_ED_PARTICIPANTS;
    ed_log("EDITION_PARTICIPANTS",n,"independent calendar: same clubs, no movement");
    (void)r;(void)host;
    return 1;
}
/* League family rollover: all divisions of (country, comp_id) completed.
 * 1 created successors, 0 wait, -1 blocked. */
/* Prepare every successor in private memory before publishing any of them.
 * One malformed ranking or allocation failure must not split a family.
 * The existing prefix/partial policies are preserved, not expanded here. */
static int league_family_rollover(ExpandedUniversalRegistry *r,
                                  const ExpCeHost *host,const ExpCeDef *d0)
{
    const ExpCeDef *div[16];
    const ExpCeEdition *cur[16];
    uint32_t old[16],nd=d0->divisions,i,j,k,seg=0u;
    ExpCeEdition *next;
    int chain=1,invalid=0;
    if(!nd || nd>16u)return -1;
    for(i=0;i<nd;++i) {
        uint32_t best=UINT32_MAX;
        div[i]=def_find(ExpCe_LeagueCompetitionId(d0->country,d0->comp_id,(uint8_t)i));
        if(!div[i])return -1;
        for(j=0;j<s_ce.neds;++j) {
            const ExpCeEdition *o=&s_ce.eds[j];
            if(o->def_id!=div[i]->id)continue;
            if(best==UINT32_MAX || o->key.start_day>s_ce.eds[best].key.start_day)best=j;
        }
        if(best==UINT32_MAX)return -1;
        old[i]=best;cur[i]=&s_ce.eds[best];
        if(cur[i]->successor || cur[i]->block)return cur[i]->block?-1:0;
        if(cur[i]->state<EXP_CE_ED_COMPLETED)return 0;
        if(cur[i]->year==UINT16_MAX || (i && cur[i]->year!=cur[0]->year))return -1;
        if(cur[i]->partial)chain=-1;
    }
    if(chain>0)chain=ExpCe_LeagueFamilyChainOk(d0->country,d0->comp_id);
    if(chain==0) {
        seg=league_chain_prefix(d0);
        if(seg>=2u && seg<nd)chain=2;
    }
    next=calloc(nd,sizeof *next);
    if(!next)return -1;
    for(i=0;i<nd;++i) {
        ExpCeEdition *n=&next[i];
        const ExpCeEdition *o=cur[i];
        uint32_t up=0u,down=0u;
        n->year=(uint16_t)(o->year+1u);
        if(!edition_start_day(div[i],n->year,&n->key.start_day))goto fail;
        n->key.competition=(ExpandedCompetitionKey){div[i]->world,div[i]->id};
        n->def_id=div[i]->id;n->rules_hash=div[i]->rules_hash;
        n->state=EXP_CE_ED_CREATED;
        if(ed_find(n->key))goto fail;
        if(div[i]->missing[0] || chain==0) {
            invalid=1;
            continue;
        }
        if(o->nentrants!=div[i]->clubs || o->nentrants>EXP_CE_MAX_ENTRANTS)goto fail;
        for(j=0;j<o->nentrants;++j) {
            const ExpandedClubKey c=o->entrants[j].club;
            if(c.world!=d0->world || c.kind!=EXP_PARTICIPANT_CLUB || !c.stable_club ||
               entrant_has(o->entrants,j,c))goto fail;
        }
        if(chain<0 || (chain==2 && i>=seg))continue;
        up=div[i]->promoted;
        down=(chain==2 && i+1u==seg)?0u:div[i]->relegated;
        if(league_rank_count(o)!=o->nentrants || up+down>league_rank_count(o))goto fail;
        for(j=0;j<league_rank_count(o);++j) {
            const ExpandedClubKey c={d0->world,league_rank_at(o,j),EXP_PARTICIPANT_CLUB};
            if(!entrant_has(o->entrants,o->nentrants,c))goto fail;
            for(k=0;k<j;++k)if(league_rank_at(o,k)==league_rank_at(o,j))goto fail;
        }
    }
    if(!invalid) {
        for(i=0;i<nd;++i) {
            ExpCeEdition *n=&next[i];
            k=0u;
            if(chain<0 || (chain==2 && i>=seg)) {
                for(j=0;j<cur[i]->nentrants;++j) {
                    n->entrants[k]=(ExpCeEntrant){cur[i]->entrants[j].club,
                        EXP_CE_SRC_KEPT_NO_MOVEMENT,0u,0u,(uint8_t)(k+1u),0u,
                        div[i]->id,cur[i]->key.start_day};
                    ++k;
                }
            } else {
                const uint32_t down=(chain==2 && i+1u==seg)?0u:div[i]->relegated;
                for(j=div[i]->promoted;j+down<league_rank_count(cur[i]);++j) {
                    if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                    n->entrants[k]=(ExpCeEntrant){
                        (ExpandedClubKey){d0->world,league_rank_at(cur[i],j),EXP_PARTICIPANT_CLUB},
                        EXP_CE_SRC_RETAINED,(uint8_t)(j+1u),0u,(uint8_t)(k+1u),0u,
                        div[i]->id,cur[i]->key.start_day};
                    ++k;
                }
                if(i>0u)
                    for(j=league_rank_count(cur[i-1u])-div[i-1u]->relegated;j<league_rank_count(cur[i-1u]);++j) {
                        if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                        n->entrants[k]=(ExpCeEntrant){
                            (ExpandedClubKey){d0->world,league_rank_at(cur[i-1u],j),EXP_PARTICIPANT_CLUB},
                            EXP_CE_SRC_RELEGATED,(uint8_t)(j+1u),0u,(uint8_t)(k+1u),0u,
                            div[i-1u]->id,cur[i-1u]->key.start_day};
                        ++k;
                    }
                if(i+1u<(chain==2?seg:nd))
                    for(j=0;j<div[i+1u]->promoted;++j) {
                        if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                        n->entrants[k]=(ExpCeEntrant){
                            (ExpandedClubKey){d0->world,league_rank_at(cur[i+1u],j),EXP_PARTICIPANT_CLUB},
                            EXP_CE_SRC_PROMOTED,(uint8_t)(j+1u),0u,(uint8_t)(k+1u),0u,
                            div[i+1u]->id,cur[i+1u]->key.start_day};
                        ++k;
                    }
            }
            n->nentrants=k;
            if(k!=div[i]->clubs)goto fail;
            for(j=0;j<k;++j) {
                uint32_t q;
                const ExpandedClubKey c=n->entrants[j].club;
                if(entrant_has(n->entrants,j,c))goto fail;
                for(q=0;q<i;++q)if(entrant_has(next[q].entrants,next[q].nentrants,c))goto fail;
            }
            n->state=EXP_CE_ED_PARTICIPANTS;
        }
    }
    /* Reserve once; no fallible operations between publication and flags. */
    if(s_ce.neds>UINT32_MAX-nd ||
       !ce_grow((void **)&s_ce.eds,&s_ce.ceds,s_ce.neds+nd,sizeof *s_ce.eds))goto fail;
    k=s_ce.neds;
    memcpy(s_ce.eds+k,next,(size_t)nd*sizeof *next);
    s_ce.neds+=nd;
    for(i=0;i<nd;++i)s_ce.eds[old[i]].successor=1u;
    free(next);
    for(i=0;i<nd;++i) {
        ExpCeEdition *n=&s_ce.eds[k+i];
        if(invalid)
            ed_block(r,host,n,EXP_CE_BLOCK_DATA_MISSING,
                     div[i]->missing[0]?div[i]->missing:"promotion/relegation targets not a simple chain (parallel groups): distribution rule missing");
        else
            ed_log("EDITION_PARTICIPANTS",n,"validated atomic league family rollover");
    }
    return 1;
fail:
    free(next);
    return -1;
}
static int ed_index(const ExpCeEdition *e){return (int)(e-s_ce.eds);}
/* Defer every division of a league family, all still PARTICIPANTS with no
 * fixtures, from `year` to `year+1` when `year`'s window has already opened.
 * Fail-closed (0) when any division already has a schedule, when the
 * successor year already exists, or when the window has not opened yet
 * (then the plan is impossible for a data reason and must stay blocked). */
static int league_family_defer(const ExpCeDef *d,uint16_t year,int32_t today)
{
    uint32_t i,n=0u;
    int32_t start=0;
    ExpCeEdition *fam[256];
    if(!d || d->origin!=EXP_CE_ORIGIN_LEAGUE || year>=0xFFFFu)return 0;
    for(i=0;i<s_ce.neds;++i) {
        ExpCeEdition *o=&s_ce.eds[i];
        const ExpCeDef *od=def_find(o->def_id);
        if(!od || od->origin!=EXP_CE_ORIGIN_LEAGUE || od->country!=d->country ||
           od->comp_id!=d->comp_id ||
           (league_family_independent(d) && od->id!=d->id))continue;
        if(o->year==(uint16_t)(year+1u))return 0;      /* successor exists */
        if(o->year!=year)continue;
        if(o->state!=EXP_CE_ED_PARTICIPANTS || o->block || n>=256u)return 0;
        if(today<o->key.start_day)return 0;            /* not late: real gap */
        fam[n++]=o;
    }
    if(!n)return 0;
    for(i=0;i<n;++i) {
        const ExpCeDef *od=def_find(fam[i]->def_id);
        if(!od || !edition_start_day(od,(uint16_t)(year+1u),&start) ||
           start<=today)return 0;
    }
    for(i=0;i<n;++i) {
        const ExpCeDef *od=def_find(fam[i]->def_id);
        char why[96];
        ExpandedWorldDate was={0};
        (void)ExpandedWorld_IndexToDate(fam[i]->key.start_day,&was);
        (void)edition_start_day(od,(uint16_t)(year+1u),&start);
        fam[i]->year=(uint16_t)(year+1u);
        fam[i]->key.start_day=start;
        snprintf(why,sizeof why,"window %04u-%02u-%02u already open when "
                 "confirmed: same clubs, next legal window",(unsigned)was.year,
                 (unsigned)was.month,(unsigned)was.day);
        ed_log("EDITION_DEFERRED_TO_NEXT_WINDOW",fam[i],why);
    }
    return 1;
}

int ExpCe_Tick(ExpandedUniversalRegistry *r,const ExpCeHost *host,int32_t today)
{
    int changes=0,pass;
    uint32_t i;
    if(!r)return -1;
    for(pass=0;pass<8;++pass) {
        int moved=0;
        for(i=0;i<s_ce.neds;++i) {
            ExpCeEdition *e=&s_ce.eds[i];
            const ExpCeDef *d=def_find(e->def_id);
            char why[96];
            uint8_t block=0;
            if(!d || e->block || e->state==EXP_CE_ED_ARCHIVED)continue;
            switch(e->state) {
            case EXP_CE_ED_CREATED: {
                int res;
                if(d->missing[0]) {
                    ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,d->missing);
                    ++moved;break;
                }
                if(d->origin==EXP_CE_ORIGIN_LEAGUE)break; /* family rollover */
                if(e->nentrants)res=1; /* first edition: listed entrants */
                else res=resolve_sources(d,e,why,sizeof why);
                if(res<0){ed_block(r,host,e,res==-2?EXP_CE_BLOCK_DEPENDENCY:
                                   EXP_CE_BLOCK_ENTRANTS,why);++moved;}
                else if(res>0){e->state=EXP_CE_ED_PARTICIPANTS;
                    ed_log("EDITION_PARTICIPANTS",e,NULL);++moved;}
                break;
            }
            case EXP_CE_ED_PARTICIPANTS:
                if(!generate_phase(r,host,d,e,0u,today,&block,why,sizeof why)) {
                    e=&s_ce.eds[i];
                    /* v298: a league edition confirmed only AFTER its window
                     * opened (e.g. a pre-v297 CAR repaired at CONTINUE) cannot
                     * fit its rounds any more: never shorten it, never squeeze
                     * the rest days, never invent catch-up matches -> move the
                     * whole family to its next legal window, same clubs. */
                    if(block==EXP_CE_BLOCK_DATE_PLAN &&
                       league_family_defer(d,e->year,today)) {
                        ++moved;break;
                    }
                    ed_block(r,host,e,block,why);
                } else {
                    e=&s_ce.eds[i];
                    e->state=EXP_CE_ED_SCHEDULED;e->phase=0u;e->phase_ready=1u;
                    if(!edition_expiry_day(d,e->year,e->key.start_day,&e->expiry_day))
                        e->expiry_day=e->key.start_day;
                    ed_log("EDITION_SCHEDULED",e,NULL);
                }
                ++moved;
                break;
            case EXP_CE_ED_SCHEDULED:
            case EXP_CE_ED_ACTIVE: {
                const int pc=phase_complete(r,d,e,e->phase);
                if(e->state==EXP_CE_ED_SCHEDULED) {
                    /* any played fixture -> ACTIVE */
                    uint32_t k;
                    for(k=0;k<r->fixture_count;++k)
                        if(ExpandedEditionKey_Equal(r->fixtures[k].key.edition,e->key) &&
                           r->ledger.entries[k].state==EXP_FIXTURE_PLAYED) {
                            e->state=EXP_CE_ED_ACTIVE;++moved;
                            ed_log("EDITION_ACTIVE",e,NULL);break;
                        }
                }
                if(pc<0) {
                    snprintf(why,sizeof why,"phase %u: tie level although a "
                             "winner is required",(unsigned)e->phase);
                    ed_block(r,host,e,EXP_CE_BLOCK_TIE_UNRESOLVED,why);++moved;
                    break;
                }
                if(!pc)break;
                if((uint32_t)e->phase+1u<d->nphases) {
                    if(d->origin==EXP_CE_ORIGIN_LEAGUE && e->partial) {
                        ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,
                                 "finals require complete regular table");++moved;break;
                    }
                    const int idx=ed_index(e);
                    if(!generate_phase(r,host,d,e,(uint32_t)e->phase+1u,today,
                                       &block,why,sizeof why)) {
                        e=&s_ce.eds[idx];ed_block(r,host,e,block,why);
                    } else {
                        e=&s_ce.eds[idx];++e->phase;
                        ed_log("PHASE_ADVANCED",e,NULL);
                    }
                    ++moved;break;
                }
                if(!final_order(r,d,e,host)) {
                    ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,
                             "final ranking undefined");++moved;break;
                }
                e->state=EXP_CE_ED_COMPLETED;
                if(e->expiry_day<e->key.start_day)e->expiry_day=today;
                ed_log("EDITION_COMPLETED",e,NULL);
                ++moved;
                break;
            }
            case EXP_CE_ED_COMPLETED: {
                const int idx=ed_index(e);
                if(d->origin==EXP_CE_ORIGIN_LEAGUE && d->nphases>1u &&
                   today<e->expiry_day)break;
                if(!e->successor) {
                    if(d->origin==EXP_CE_ORIGIN_LEAGUE) {
                        const ExpCeDef *d0=def_find(ExpCe_LeagueCompetitionId(
                            d->country,d->comp_id,0u));
                        if(league_family_independent(d)) {
                            if(league_independent_rollover(r,host,d,e))++moved;
                        } else if(d0 && league_family_rollover(r,host,d0)>0)++moved;
                    } else {
                        int32_t start=0;
                        ExpCeEdition *n;
                        if(edition_start_day(d,(uint16_t)(e->year+1u),&start) &&
                           (n=ed_new(d,(uint16_t)(e->year+1u),start))!=NULL) {
                            s_ce.eds[idx].successor=1u;
                            ed_log("EDITION_CREATED",n,"successor");
                            ++moved;
                        }
                    }
                }
                e=&s_ce.eds[idx];
                if(today>=e->expiry_day) {
                    int ar;
                    if(!host || !host->archive)break;
                    ar=host->archive(host->user,e->key,e->final_order[0],
                                     e->final_order[1],today);
                    if(ar!=0 && ar!=1)
                        break; /* never archive without the registry proof */
                    e=&s_ce.eds[idx];
                    e->state=EXP_CE_ED_ARCHIVED;
                    ed_log("EDITION_ARCHIVED",e,NULL);
                    ++moved;
                }
                break;
            }
            default:
                break;
            }
        }
        /* prune the fixtures of archived editions whose successor runs */
        for(i=0;i<s_ce.neds;++i) {
            ExpCeEdition *e=&s_ce.eds[i];
            uint32_t j;
            if(e->state!=EXP_CE_ED_ARCHIVED || !e->successor || e->phase_ready==2u)
                continue;
            for(j=0;j<s_ce.neds;++j) {
                const ExpCeEdition *n=&s_ce.eds[j];
                if(n->def_id==e->def_id && n->year==e->year+1u &&
                   n->state>=EXP_CE_ED_SCHEDULED && host && host->prune &&
                   host->prune(host->user,e->key)) {
                    e->phase_ready=2u; /* fixtures pruned */
                    ed_log("EDITION_PRUNED",e,NULL);
                    ++moved;break;
                }
            }
        }
        changes+=moved;
        if(!moved)break;
    }
    return changes;
}
/* A league with no first-edition fixture in the registry:
 *  - its whole first-year window ended before career START (e.g. Jan-Jun
 *    seasons): the career's first edition of it is the NEXT season, with the
 *    clubs of the division at START (source START_DIVISION, the data's own
 *    state on 1 July) -> CREATED with entrants, generated by Tick;
 *  - otherwise (clubs/calendar mismatch): blocked, data missing. */
static int begin_league_without_fixtures(ExpandedUniversalRegistry *r,
    const ExpCeHost *host,const ExpCeDef *d,uint16_t first_year)
{
    ExpCeEdition *e;
    int32_t start=0,last=0;
    const ExpCeWindow *w=&d->phases[0].window;
    const int32_t today=r->scheduler.clock.day_index;
    if(!d->missing[0] && d->phases[0].has_window &&
       edition_start_day(d,first_year,&start) &&
       md_on_or_after(w->last_m,w->last_d,start,&last) &&
       (w->last_year_offset?0:1) && last<today && host && host->division_clubs) {
        ExpandedClubKey clubs[EXP_CE_MAX_ENTRANTS];
        const uint32_t n=host->division_clubs(host->user,d->country,d->division,
                                              clubs,EXP_CE_MAX_ENTRANTS);
        const uint16_t year=(uint16_t)(first_year+1u);
        uint32_t k;
        if(n==d->clubs && edition_start_day(d,year,&start) &&
           (e=ed_new(d,year,start))!=NULL) {
            for(k=0;k<n;++k)
                e->entrants[k]=(ExpCeEntrant){clubs[k],EXP_CE_SRC_START_DIVISION,
                    0u,0u,(uint8_t)(k+1u),0u,0u,0};
            e->nentrants=n;
            e->state=EXP_CE_ED_PARTICIPANTS;
            ed_log("EDITION_PARTICIPANTS",e,
                   "first season ended before START: next season, START clubs");
            return 1;
        }
    }
    if(!edition_start_day(d,first_year,&start))start=0;
    if(!(e=ed_new(d,first_year,start)))return 0;
    ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,
             d->missing[0]?d->missing:
             "no first-edition fixtures (clubs/calendar mismatch)");
    return 1;
}
int ExpCe_BeginWorld(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                     uint16_t world,uint16_t first_year)
{
    uint32_t i,k;
    if(!r)return 0;
    s_ce.world=world;
    for(i=0;i<s_ce.ndefs;++i) {
        const ExpCeDef *d=&s_ce.defs[i];
        ExpCeEdition *e;
        int32_t start=0;
        if(d->world!=world)continue;
        if(d->origin==EXP_CE_ORIGIN_LEAGUE) {
            /* catalog fixtures of the first edition: find their key */
            const ExpandedUniversalFixture *f=NULL;
            for(k=0;k<r->fixture_count;++k)
                if(r->fixtures[k].key.edition.competition.stable_competition==d->id) {
                    f=&r->fixtures[k];break;
                }
            if(!f) {
                if(!begin_league_without_fixtures(r,host,d,first_year))return 0;
                continue;
            }
            if(ed_find(f->key.edition))continue;
            if(!(e=ed_new(d,first_year,f->key.edition.start_day)))return 0;
            e->key=f->key.edition;
            for(k=0;k<r->fixture_count;++k) {
                const ExpandedUniversalFixture *g=&r->fixtures[k];
                if(!ExpandedEditionKey_Equal(g->key.edition,e->key))continue;
                if(!entrant_has(e->entrants,e->nentrants,g->home) &&
                   e->nentrants<EXP_CE_MAX_ENTRANTS)
                    e->entrants[e->nentrants++]=(ExpCeEntrant){g->home,
                        EXP_CE_SRC_START_DIVISION,0u,0u,0u,0u,0u,0};
                if(!entrant_has(e->entrants,e->nentrants,g->away) &&
                   e->nentrants<EXP_CE_MAX_ENTRANTS)
                    e->entrants[e->nentrants++]=(ExpCeEntrant){g->away,
                        EXP_CE_SRC_START_DIVISION,0u,0u,0u,0u,0u,0};
            }
            e->state=EXP_CE_ED_SCHEDULED;e->phase=0u;e->phase_ready=1u;
            if(!edition_expiry_day(d,first_year,e->key.start_day,&e->expiry_day))
                e->expiry_day=e->key.start_day;
            {
                /* fixtures before START are not in the registry (catalog
                 * past_at_start): a round robin with fewer matches than
                 * n(n-1)/2 x meetings is a PARTIAL first edition */
                uint32_t have=0u;
                const uint32_t n=d->clubs,expect=n*(n-1u)/2u*d->phases[0].meetings+n*d->phases[0].extras.rounds/2u;
                for(k=0;k<r->fixture_count;++k)
                    if(ExpandedEditionKey_Equal(r->fixtures[k].key.edition,e->key))++have;
                if(have<expect) {
                    e->partial=1u;
                    fprintf(stderr,"[ce-engine] FIRST_EDITION_PARTIAL world=%u "
                        "competition=%lu fixtures=%lu expected=%lu reason=started-"
                        "before-career-START rollover=BLOCKED_UNTIL_REAL_RESULTS\n",
                        (unsigned)world,(unsigned long)d->id,(unsigned long)have,
                        (unsigned long)expect);
                }
            }
            if(d->missing[0])
                fprintf(stderr,"[ce-engine] LEAGUE_FIRST_EDITION_RUNS world=%u "
                    "competition=%lu note=%s (blocks only the rollover)\n",
                    (unsigned)world,(unsigned long)d->id,d->missing);
        } else {
            if(!d->first_year || d->first_year<first_year) {
                /* an edition before START: not part of this career */
                if(d->missing[0] || !d->first_year) {
                    if(!edition_start_day(d,first_year,&start))start=0;
                    if(!(e=ed_new(d,first_year,start)))return 0;
                    ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,
                             d->missing[0]?d->missing:"first edition undefined");
                }
                continue;
            }
            if(!edition_start_day(d,d->first_year,&start)) {
                if(!(e=ed_new(d,d->first_year,0)))return 0;
                ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,"start date");
                continue;
            }
            if(!(e=ed_new(d,d->first_year,start)))return 0;
            if(d->missing[0]){ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,d->missing);continue;}
            memcpy(e->entrants,d->first,(size_t)d->nfirst*sizeof *d->first);
            e->nentrants=d->nfirst;
            if(!e->nentrants)
                ed_block(r,host,e,EXP_CE_BLOCK_ENTRANTS,"first edition entrants");
        }
    }
    return 1;
}
int ExpCe_RulesForFixture(const ExpandedUniversalFixture *f,
                          ExpCeMatchRules *match,ExpCeTableRules *table)
{
    const ExpCeDef *d;
    uint32_t phase;
    if(!f || !(d=def_find(f->key.edition.competition.stable_competition)))return 0;
    phase=EXP_CE_STAGE_PHASE(f->key.stage);
    /* integrity: the fixture's declared type must be its definition's */
    if(phase>=d->nphases || d->phases[phase].missing[0] ||
       f->competition_type!=d->type)return 0;
    if(match)*match=d->phases[phase].match;
    if(table)*table=d->phases[phase].table;
    return 1;
}
/* Validator/diagnostics: plan the round days of (definition, year, phase)
 * exactly as generate_phase does (no registry change). */
int ExpCe_PlanPhaseDays(const ExpCeDef *d,uint16_t year,uint32_t phase,
                        int32_t earliest,uint32_t rounds,int32_t *days)
{
    ExpCeEdition e;
    int32_t start=0;
    if(!d || phase>=d->nphases || !days || !edition_start_day(d,year,&start))
        return 0;
    memset(&e,0,sizeof e);
    e.key.start_day=start;e.year=year;
    return plan_phase_days_ex(d,&e,&d->phases[phase],earliest,rounds,days);
}
int ExpCe_FixturePhase(const ExpandedUniversalFixture *f,uint8_t *kind,
                       uint8_t *meetings)
{
    const ExpCeDef *d;
    uint32_t phase;
    if(!f || !(d=def_find(f->key.edition.competition.stable_competition)))return 0;
    phase=EXP_CE_STAGE_PHASE(f->key.stage);
    if(phase>=d->nphases)return 0;
    if(kind)*kind=d->phases[phase].kind;
    if(meetings)*meetings=d->phases[phase].meetings;
    return 1;
}
/* CE-derived competition name of a fixture (readers: calendar, viewers).
 * 1 ok, 0 unknown competition (the caller keeps a neutral label). */
int ExpCe_FixtureCompetitionName(const ExpandedUniversalFixture *f,
                                 char *out,size_t n)
{
    const ExpCeDef *d;
    if(!f || !out || !n || !(d=def_find(f->key.edition.competition.stable_competition)) ||
       !d->name[0])return 0;
    snprintf(out,n,"%s",d->name);
    return 1;
}
int ExpCe_TableRules(ExpandedEditionKey edition,uint32_t stage,
                     ExpCeTableRules *table)
{
    const ExpCeDef *d=def_find(edition.competition.stable_competition);
    const uint32_t phase=EXP_CE_STAGE_PHASE(stage);
    if(!d || !table || phase>=d->nphases ||
       d->phases[phase].kind!=EXP_CE_PHASE_TABLE)return 0;
    *table=d->phases[phase].table;
    return ExpCeRules_ValidateTable(table,NULL);
}

/* ------------------------------------------------------------------ */
/* JSON definitions (ce_competitions.json)                              */
/* ------------------------------------------------------------------ */
static int json_str(const char *j,int l,const char *path,char *out,size_t n)
{
    return mjson_get_string(j,l,path,out,(int)n)>0;
}
static int parse_md(const char *t,uint8_t *m,uint8_t *d)
{
    unsigned a,b;
    if(!t || strlen(t)!=5u || t[2]!='-' ||
       sscanf(t,"%2u-%2u",&a,&b)!=2 || a<1u || a>12u || b<1u || b>31u)return 0;
    *m=(uint8_t)a;*d=(uint8_t)b;
    return 1;
}
static int parse_weekdays(const char *t,uint8_t *mask)
{
    static const char names[7][4]={"MON","TUE","WED","THU","FRI","SAT","SUN"};
    uint8_t out=0u;
    if(!t || !mask)return 0;
    if(!*t){*mask=0u;return 1;}
    for(;;) {
        int i,f=-1;
        for(i=0;i<7;++i)if(strncmp(t,names[i],3)==0)f=i;
        if(f<0 || (out&(1u<<f)))return 0;
        out=(uint8_t)(out|(1u<<f));t+=3;
        if(!*t)break;
        if(*t!=',')return 0;
        ++t;
    }
    *mask=out;
    return 1;
}
static int parse_window(const char *j,int l,ExpCeWindow *w,char *why,size_t wl)
{
    char t[40];
    double v=0;
    memset(w,0,sizeof *w);
    if(!json_str(j,l,"$.first",t,sizeof t) || !parse_md(t,&w->first_m,&w->first_d) ||
       !json_str(j,l,"$.last",t,sizeof t) || !parse_md(t,&w->last_m,&w->last_d)) {
        snprintf(why,wl,"window.first/last MM-DD");return 0;
    }
    if(!json_str(j,l,"$.weekdays",t,sizeof t) || !parse_weekdays(t,&w->weekdays) ||
       !w->weekdays) {snprintf(why,wl,"window.weekdays");return 0;}
    if(json_str(j,l,"$.extra_weekdays",t,sizeof t) &&
       !parse_weekdays(t,&w->extra_weekdays)) {
        snprintf(why,wl,"window.extra_weekdays");return 0;
    }
    if(mjson_get_number(j,l,"$.rest",&v)>0) {
        if(v<0 || v>14){snprintf(why,wl,"window.rest 0..14");return 0;}
        w->rest=(uint8_t)v;
    }
    if(mjson_get_number(j,l,"$.last_year_offset",&v)>0) {
        if(v<0 || v>2){snprintf(why,wl,"window.last_year_offset 0..2");return 0;}
        w->last_year_offset=(uint8_t)v;
    }
    return 1;
}
static int parse_club(const char *t,uint16_t world,ExpandedClubKey *k)
{
    unsigned c,o;
    /* "country:slot" stable world-pack key (same as CE club keys) */
    if(!t || sscanf(t,"%u:%u",&c,&o)!=2 || c>255u || !o || o>65535u)return 0;
    k->world=world;k->stable_club=((uint32_t)c<<16)|o;
    return 1;
}
static const struct {const char *n;uint8_t v;} k_pairings[]={
    {"LISTED",EXP_CE_PAIR_LISTED},{"SEEDED",EXP_CE_PAIR_SEEDED},
    {"CROSS_GROUPS",EXP_CE_PAIR_CROSS_GROUPS},{"BRACKET",EXP_CE_PAIR_BRACKET},
    {"DRAW",EXP_CE_PAIR_DRAW},{"POTS",EXP_CE_PAIR_POTS},
    {"RESEED_TABLE",EXP_CE_PAIR_RESEED_TABLE}};
static int parse_phase(const char *j,int l,const ExpCeTableRules *tbase,
                       const ExpCeMatchRules *mbase,ExpCePhaseDef *ph)
{
    char t[96];
    const char *o=NULL;
    int ol=0;
    double v=0;
    ExpCeRuleError err;
    memset(ph,0,sizeof *ph);
    if(!json_str(j,l,"$.kind",t,sizeof t)) {
        snprintf(ph->missing,sizeof ph->missing,"kind");return 0;
    }
    if(strcmp(t,"TABLE")==0)ph->kind=EXP_CE_PHASE_TABLE;
    else if(strcmp(t,"KNOCKOUT")==0)ph->kind=EXP_CE_PHASE_KNOCKOUT;
    else {snprintf(ph->missing,sizeof ph->missing,"kind TABLE|KNOCKOUT");return 0;}
#define NUM(path,field,lo,hi) do{ if(mjson_get_number(j,l,path,&v)>0){ \
        if(v<lo||v>hi){snprintf(ph->missing,sizeof ph->missing,"%s range",path);return 0;} \
        ph->field=(uint8_t)v;} }while(0)
    ph->groups=1u;ph->meetings=1u;
    NUM("$.groups",groups,1,EXP_CE_MAX_GROUPS);
    NUM("$.meetings",meetings,1,4);
    NUM("$.qualify",qualify,0,64);
    NUM("$.byes",byes,0,64);
    NUM("$.higher_home",higher_home,0,2);
#undef NUM
    if(json_str(j,l,"$.pairing",t,sizeof t)) {
        size_t i;
        for(i=0;i<sizeof k_pairings/sizeof k_pairings[0];++i)
            if(strcmp(t,k_pairings[i].n)==0)ph->pairing=k_pairings[i].v;
        if(!ph->pairing){snprintf(ph->missing,sizeof ph->missing,"pairing");return 0;}
    }
    /* explicit data gap declared by the data author */
    if(json_str(j,l,"$.missing",t,sizeof t)) {
        snprintf(ph->missing,sizeof ph->missing,"%.79s",t);
    }
    ph->match=*mbase;
    if(ph->kind==EXP_CE_PHASE_KNOCKOUT)ph->match.needs_winner=1u;
    if(mjson_find(j,l,"$.match",&o,&ol)==MJSON_TOK_OBJECT &&
       !ExpCeRules_ParseMatch(o,ol,&ph->match,&ph->match,&err)) {
        snprintf(ph->missing,sizeof ph->missing,"%.24s: %.50s",err.field,err.reason);
        return 0;
    }
    if(ph->kind==EXP_CE_PHASE_TABLE) {
        ph->table=*tbase;
        if(mjson_find(j,l,"$.table",&o,&ol)==MJSON_TOK_OBJECT &&
           !ExpCeRules_ParseTable(o,ol,&ph->table,&ph->table,&err)) {
            snprintf(ph->missing,sizeof ph->missing,"%.24s: %.50s",err.field,err.reason);
            return 0;
        }
    }
    if(mjson_find(j,l,"$.host_rule",&o,&ol)==MJSON_TOK_OBJECT) {
        double cc=0,cid=0,dv=0;
        char ht[32];
        if(!json_str(o,ol,"$.type",ht,sizeof ht) || strcmp(ht,"NON_MEMBER_HOSTS")!=0 ||
           mjson_get_number(o,ol,"$.league.country",&cc)<=0 ||
           mjson_get_number(o,ol,"$.league.comp_id",&cid)<=0 ||
           mjson_get_number(o,ol,"$.league.division",&dv)<=0) {
            snprintf(ph->missing,sizeof ph->missing,"host_rule");return 0;
        }
        ph->host_nonmember_comp=ExpCe_LeagueCompetitionId((uint8_t)cc,(uint8_t)cid,(uint8_t)dv);
    }
    if(mjson_find(j,l,"$.window",&o,&ol)==MJSON_TOK_OBJECT) {
        if(!parse_window(o,ol,&ph->window,ph->missing,sizeof ph->missing))return 0;
        ph->has_window=1u;
    }
    if(mjson_find(j,l,"$.first_edition_days",&o,&ol)==MJSON_TOK_ARRAY) {
        int off=0,ko,kl,vo,vl,vt;
        while((off=mjson_next(o,ol,off,&ko,&kl,&vo,&vl,&vt))>0) {
            unsigned y,m,dd;
            char t2[16];
            int32_t idx=0;
            if(vt!=MJSON_TOK_STRING || vl!=12 || ph->ndays>=16u) {
                snprintf(ph->missing,sizeof ph->missing,"first_edition_days");return 0;
            }
            memcpy(t2,o+vo+1,10u);t2[10]=0;
            if(sscanf(t2,"%4u-%2u-%2u",&y,&m,&dd)!=3 ||
               !ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,
                   (uint8_t)m,(uint8_t)dd},&idx)) {
                snprintf(ph->missing,sizeof ph->missing,"first_edition_days date");
                return 0;
            }
            ph->days[ph->ndays++]=idx;
        }
    }
    return 1;
}
/* Optional finals on league_calendar.json rows. No extra competition ID,
 * no independent schedule; validate all opt-ins before changing definitions. */
int ExpCe_LoadLeagueFinals(const char *json,int len,uint16_t world)
{
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt,added=0,rc=-1;
    double schema=0;
    ExpCeDef **staged=NULL;
    uint32_t i;
    if(!json || mjson_get_number(json,len,"$.schema",&schema)<=0 || schema!=1 ||
       mjson_find(json,len,"$.leagues",&arr,&alen)!=MJSON_TOK_ARRAY)return -1;
    staged=(ExpCeDef **)calloc(s_ce.ndefs?s_ce.ndefs:1u,sizeof *staged);
    if(!staged)return -1;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *row=arr+vo,*o=NULL,*phases=NULL;
        int ol=0,pl=0,po=0,pk,pkl,pv,pvl,pt;
        double country=0,comp=0,div=0,qual=0;
        ExpCeDef *d,*copy;
        if(mjson_find(row,vl,"$.finals",&o,&ol)==MJSON_TOK_INVALID)continue;
        if(vt!=MJSON_TOK_OBJECT || mjson_find(row,vl,"$.finals",&o,&ol)!=MJSON_TOK_OBJECT ||
           mjson_get_number(row,vl,"$.team_number",&country)<=0 ||
           mjson_get_number(row,vl,"$.comp_id",&comp)<=0 ||
           mjson_get_number(row,vl,"$.division",&div)<=0 ||
           country<0 || country>254 || comp<0 || comp>255 || div<0 || div>15 ||
           country!=(double)(uint8_t)country || comp!=(double)(uint8_t)comp ||
           div!=(double)(uint8_t)div ||
           mjson_get_number(o,ol,"$.qualify",&qual)<=0 || qual<2 || qual>64 ||
           qual!=(double)(uint8_t)qual ||
           mjson_find(o,ol,"$.phases",&phases,&pl)!=MJSON_TOK_ARRAY)goto done;
        d=def_find(ExpCe_LeagueCompetitionId((uint8_t)country,(uint8_t)comp,(uint8_t)div));
        if(!d || d->world!=world || d->origin!=EXP_CE_ORIGIN_LEAGUE ||
           d->nphases!=1u || d->missing[0] || qual>d->clubs)goto done;
        i=(uint32_t)(d-s_ce.defs);
        if(staged[i])goto done;
        copy=(ExpCeDef *)malloc(sizeof *copy);
        if(!copy)goto done;
        staged[i]=copy;*copy=*d;
        copy->phases[0].qualify=(uint8_t)qual;
        while((po=mjson_next(phases,pl,po,&pk,&pkl,&pv,&pvl,&pt))>0) {
            ExpCePhaseDef *ph;
            if(pt!=MJSON_TOK_OBJECT || copy->nphases>=EXP_CE_MAX_PHASES)goto done;
            ph=&copy->phases[copy->nphases++];
            if(!parse_phase(phases+pv,pvl,&copy->phases[0].table,
                &copy->phases[0].match,ph) || ph->kind!=EXP_CE_PHASE_KNOCKOUT)goto done;
        }
        if(po<0 || copy->nphases<2u || !def_check(copy))goto done;
        /* Check qualifier count and byes now, not halfway through the season. */
        {uint32_t p,n=(uint32_t)qual;
         for(p=1u;p<copy->nphases;++p){
             const ExpCePhaseDef *ph=&copy->phases[p];
             if(ph->pairing!=EXP_CE_PAIR_RESEED_TABLE || n<ph->byes+2u ||
                ((n-ph->byes)&1u))goto done;
             n=ph->byes+(n-ph->byes)/2u;
         }
         if(n!=1u)goto done;}
        copy->rules_hash=def_hash(copy);
        ++added;
    }
    if(off<0)goto done;
    for(i=0;i<s_ce.ndefs;++i)if(staged[i])s_ce.defs[i]=*staged[i];
    rc=added;
done:
    for(i=0;i<s_ce.ndefs;++i)free(staged[i]);
    free(staged);return rc;
}
int ExpCe_LoadJsonDefinitions(const char *json,int len,uint16_t world,
                              const char *source_file)
{
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt,added=0;
    double schema=0;
    if(!json || mjson_get_number(json,len,"$.schema",&schema)<=0 || schema!=1 ||
       mjson_find(json,len,"$.competitions",&arr,&alen)!=MJSON_TOK_ARRAY)
        return -1;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *c=arr+vo,*o=NULL;
        int ol=0,poff=0,pk,pl,pv,plen,pt;
        ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
        ExpCeTableRules tbase;
        ExpCeMatchRules mbase;
        ExpCeRuleError err;
        double v=0;
        char t[256];
        if(!d)return -1;
        memset(&tbase,0,sizeof tbase);memset(&mbase,0,sizeof mbase);
        d->world=world;d->origin=EXP_CE_ORIGIN_JSON;
        snprintf(d->source_file,sizeof d->source_file,"%s",
                 source_file?source_file:"?");
        if(vt!=MJSON_TOK_OBJECT || mjson_get_number(c,vl,"$.id",&v)<=0 ||
           v<1 || v>4294967295.0) {free(d);return -1;}
        d->id=(uint32_t)v;
        if((d->id&0xffu)!=2u)
            snprintf(d->missing,sizeof d->missing,
                     "id: low byte must be 2 (league ids end in 1)");
        if(json_str(c,vl,"$.name",t,sizeof t))
            snprintf(d->name,sizeof d->name,"%.39s",t);
        if(mjson_get_number(c,vl,"$.country",&v)>0 && v>=0 && v<=255)
            d->country=(uint8_t)v;
        {   /* v305: data flag - the competition has its own button in the
             * WORLD competition list of its country and in VIEW COMPETITIONS
             * of its participants (competitions with no OG slot) */
            int vb=0;
            if(mjson_get_bool(c,vl,"$.viewer_button",&vb)>0)d->viewer_button=(uint8_t)(vb!=0);
            if(mjson_find(c,vl,"$.viewer_native_comp_id",&o,&ol)!=MJSON_TOK_INVALID) {
                double native_id=0;
                if(mjson_get_number(c,vl,"$.viewer_native_comp_id",&native_id)>0 &&
                   native_id>=0.0 && native_id<=255.0 &&
                   native_id==(double)(unsigned)native_id)
                    d->viewer_native_id_plus_one=(uint16_t)((unsigned)native_id+1u);
                else {
                    d->viewer_button=0u;
                    fprintf(stderr,"[ce-viewer] invalid native menu alias def=%u\n",(unsigned)d->id);
                }
            }
        }
        if(mjson_get_number(c,vl,"$.type",&v)>0 && v>=0 && v<=2)d->type=(uint8_t)v;
        else if(!d->missing[0])snprintf(d->missing,sizeof d->missing,"type 0..2");
        if(json_str(c,vl,"$.missing",t,sizeof t) && !d->missing[0])
            snprintf(d->missing,sizeof d->missing,"%.95s",t);
        if(mjson_find(c,vl,"$.table",&o,&ol)==MJSON_TOK_OBJECT &&
           !ExpCeRules_ParseTable(o,ol,NULL,&tbase,&err) && !d->missing[0])
            snprintf(d->missing,sizeof d->missing,"table %.24s: %.56s",err.field,err.reason);
        if(mjson_find(c,vl,"$.match",&o,&ol)==MJSON_TOK_OBJECT) {
            /* base match rules validated per phase (knockout flag differs) */
            ExpCeMatchRules tmp;
            memset(&tmp,0,sizeof tmp);
            if(!ExpCeRules_ParseMatch(o,ol,NULL,&tmp,&err)) {
                /* the base alone may fail (e.g. penalties without knockout):
                 * re-parse leniently: copy fields, validate per phase */
                /* base rules: neutral (one leg, no knockout flags) unless
                 * the JSON says otherwise; phases override and validate */
                ExpCeMatchRules lenient;
                memset(&lenient,0,sizeof lenient);
                lenient.legs=1u;lenient.provenance=EXP_CE_PROV_JSON_TEMPLATE;
                if(ExpCeRules_ParseMatch(o,ol,&lenient,&tmp,&err))mbase=tmp;
                else if(!d->missing[0])
                    snprintf(d->missing,sizeof d->missing,"match %.24s: %.56s",
                             err.field,err.reason);
            } else mbase=tmp;
        }
        if(mjson_find(c,vl,"$.phases",&o,&ol)==MJSON_TOK_ARRAY)
            while((poff=mjson_next(o,ol,poff,&pk,&pl,&pv,&plen,&pt))>0 &&
                  d->nphases<EXP_CE_MAX_PHASES) {
                ExpCePhaseDef *ph=&d->phases[d->nphases++];
                (void)parse_phase(o+pv,plen,&tbase,&mbase,ph);
            }
        if(!d->nphases && !d->missing[0])
            snprintf(d->missing,sizeof d->missing,"phases");
        if(mjson_find(c,vl,"$.expiry",&o,&ol)==MJSON_TOK_OBJECT) {
            if(json_str(o,ol,"$.date",t,sizeof t) &&
               parse_md(t,&d->expiry_m,&d->expiry_d)) {
                d->has_expiry=1u;
                if(mjson_get_number(o,ol,"$.year_offset",&v)>0 && v>=0 && v<=2)
                    d->expiry_year_offset=(uint8_t)v;
            } else if(!d->missing[0])
                snprintf(d->missing,sizeof d->missing,"expiry.date MM-DD");
        }
        /* first edition */
        if(mjson_find(c,vl,"$.first_edition",&o,&ol)==MJSON_TOK_OBJECT) {
            const char *ea=NULL;
            int el=0,eo=0,ek,ekl,ev,evl,et;
            if(mjson_get_number(o,ol,"$.year",&v)>0 && v>=1990 && v<=2199)
                d->first_year=(uint16_t)v;
            if(json_str(o,ol,"$.missing",t,sizeof t) && !d->missing[0])
                snprintf(d->missing,sizeof d->missing,"first edition: %.80s",t);
            if(mjson_find(o,ol,"$.entrants",&ea,&el)==MJSON_TOK_ARRAY)
                while((eo=mjson_next(ea,el,eo,&ek,&ekl,&ev,&evl,&et))>0) {
                    ExpCeEntrant *x;
                    char ck[24];
                    if(d->nfirst>=EXP_CE_MAX_ENTRANTS)break;
                    x=&d->first[d->nfirst];
                    memset(x,0,sizeof *x);
                    if(et!=MJSON_TOK_OBJECT ||
                       !json_str(ea+ev,evl,"$.club",ck,sizeof ck) ||
                       !parse_club(ck,world,&x->club)) {
                        if(!d->missing[0])snprintf(d->missing,sizeof d->missing,
                            "first_edition.entrants[%lu].club",(unsigned long)d->nfirst);
                        continue;
                    }
                    if(!json_str(ea+ev,evl,"$.source",t,sizeof t) ||
                       strcmp(t,"VERIFIED")!=0) {
                        if(!d->missing[0])snprintf(d->missing,sizeof d->missing,
                            "first_edition.entrants[%lu]: not VERIFIED",
                            (unsigned long)d->nfirst);
                    }
                    x->source=EXP_CE_SRC_LISTED;
                    x->seed=(uint8_t)(d->nfirst+1u);
                    if(mjson_get_number(ea+ev,evl,"$.group",&v)>0 && v>=1 && v<=16)
                        x->group=(uint8_t)v;
                    if(mjson_get_number(ea+ev,evl,"$.join_phase",&v)>0 && v>=0 && v<8)
                        x->join_phase=(uint8_t)v;
                    ++d->nfirst;
                }
            if(!d->nfirst && !d->missing[0])
                snprintf(d->missing,sizeof d->missing,"first_edition.entrants");
        }
        /* participants of the following editions */
        if(mjson_find(c,vl,"$.next_entrants",&o,&ol)==MJSON_TOK_ARRAY) {
            int so=0,sk,skl,sv,svl,st;
            while((so=mjson_next(o,ol,so,&sk,&skl,&sv,&svl,&st))>0 &&
                  d->nsources<EXP_CE_MAX_SOURCES) {
                ExpCeSourceSpec *sp=&d->sources[d->nsources];
                const char *sj=o+sv;
                memset(sp,0,sizeof *sp);
                if(st!=MJSON_TOK_OBJECT || !json_str(sj,svl,"$.source",t,sizeof t))
                    continue;
                if(strcmp(t,"LEAGUE_RANK")==0)sp->source=EXP_CE_SRC_LEAGUE_RANK;
                else if(strcmp(t,"CUP_WINNER")==0)sp->source=EXP_CE_SRC_CUP_WINNER;
                else if(strcmp(t,"CUP_RUNNER_UP")==0)sp->source=EXP_CE_SRC_CUP_RUNNER_UP;
                else if(strcmp(t,"LISTED")==0)sp->source=EXP_CE_SRC_LISTED;
                else continue;
                {   /* generic pool / exclusion lists ("country:slot" keys) */
                    const char *la=NULL;
                    int ll=0,lo=0,lk,lkl,lv,lvl,lt;
                    if(mjson_find(sj,svl,"$.clubs",&la,&ll)==MJSON_TOK_ARRAY)
                        while((lo=mjson_next(la,ll,lo,&lk,&lkl,&lv,&lvl,&lt))>0 &&
                              sp->nclubs<EXP_CE_MAX_POOL) {
                            char ck[24];
                            ExpandedClubKey kk;
                            if(lt!=MJSON_TOK_STRING || lvl<5 || lvl>=26)continue;
                            memcpy(ck,la+lv+1,(size_t)lvl-2u);ck[lvl-2]=0;
                            if(parse_club(ck,world,&kk))
                                sp->clubs[sp->nclubs++]=kk.stable_club;
                        }
                    lo=0;
                    if(mjson_find(sj,svl,"$.exclude",&la,&ll)==MJSON_TOK_ARRAY)
                        while((lo=mjson_next(la,ll,lo,&lk,&lkl,&lv,&lvl,&lt))>0 &&
                              sp->nexclude<EXP_CE_MAX_EXCLUDE) {
                            char ck[24];
                            ExpandedClubKey kk;
                            if(lt!=MJSON_TOK_STRING || lvl<5 || lvl>=26)continue;
                            memcpy(ck,la+lv+1,(size_t)lvl-2u);ck[lvl-2]=0;
                            if(parse_club(ck,world,&kk))
                                sp->exclude[sp->nexclude++]=kk.stable_club;
                        }
                    if(mjson_get_number(sj,svl,"$.count",&v)>0 && v>=1 && v<=64)
                        sp->count=(uint8_t)v;
                }
                if(mjson_get_number(sj,svl,"$.competition",&v)>0)sp->competition=(uint32_t)v;
                else {
                    double cc=0,cid=0,dv=0;
                    if(mjson_get_number(sj,svl,"$.league.country",&cc)>0 &&
                       mjson_get_number(sj,svl,"$.league.comp_id",&cid)>0 &&
                       mjson_get_number(sj,svl,"$.league.division",&dv)>0)
                        sp->competition=ExpCe_LeagueCompetitionId(
                            (uint8_t)cc,(uint8_t)cid,(uint8_t)dv);
                }
                if(mjson_get_number(sj,svl,"$.from",&v)>0 && v>=1 && v<=255)
                    sp->rank_from=(uint8_t)v;
                if(mjson_get_number(sj,svl,"$.to",&v)>0 && v>=1 && v<=255)
                    sp->rank_to=(uint8_t)v;
                {
                    int b=0;
                    if(mjson_get_bool(sj,svl,"$.reallocate",&b)>0)sp->reallocate=(uint8_t)(b!=0);
                }
                if(sp->source==EXP_CE_SRC_LISTED ? (!sp->count || !sp->nclubs) : !sp->competition)continue;
                ++d->nsources;
            }
        }
        if(ExpCe_AddDefinition(d))++added;
        free(d);
    }
    return off<0?-1:added;
}

/* ------------------------------------------------------------------ */
/* UL01 persistence                                                     */
/* ------------------------------------------------------------------ */
typedef struct Wr { uint8_t *b; size_t n,cap; int ok; } Wr;
static void wr_put(Wr *w,const void *p,size_t n)
{
    if(!w->ok)return;
    if(w->n+n>w->cap) {
        size_t c=w->cap?w->cap*2u:4096u;
        uint8_t *q;
        while(c<w->n+n)c*=2u;
        q=(uint8_t *)realloc(w->b,c);
        if(!q){w->ok=0;return;}
        w->b=q;w->cap=c;
    }
    memcpy(w->b+w->n,p,n);w->n+=n;
}
#define WR(w,v) do{ switch(sizeof(v)){ \
    case 1:{uint8_t x_=(uint8_t)(v);wr_put((w),&x_,1u);break;} \
    case 2:{uint16_t x_=(uint16_t)(v);wr_put((w),&x_,2u);break;} \
    case 4:{uint32_t x_=(uint32_t)(v);wr_put((w),&x_,4u);break;} \
    default:{uint64_t x_=(uint64_t)(v);wr_put((w),&x_,8u);break;}} }while(0)
typedef struct Rd { const uint8_t *b; size_t n,pos; int ok; } Rd;
static void rd_get(Rd *r,void *p,size_t n)
{
    if(!r->ok || r->pos+n>r->n){r->ok=0;memset(p,0,n);return;}
    memcpy(p,r->b+r->pos,n);r->pos+=n;
}
struct ExpCeSnapshot {
    ExpCeEdition *eds; uint32_t neds;
    ExpCePostponement *posts; uint32_t nposts;
};
static char *encode_hex(const ExpCeEdition *eds,uint32_t neds,
                        const ExpCePostponement *posts,uint32_t nposts,
                        uint16_t world,size_t *len)
{
    Wr w={0};
    uint32_t i,k;
    int v3=0;
    uint64_t h=UINT64_C(14695981039346656037);
    char *hex;
    static const char dig[]="0123456789abcdef";
    w.ok=1;
    for(i=0;i<neds;++i)if(eds[i].nregular){v3=1;break;}
    wr_put(&w,v3?"UL03":"UL02",4u); /* UL03 adds regular ranking, old formats readable */
    WR(&w,(uint16_t)world);WR(&w,(uint32_t)neds);
    for(i=0;i<neds;++i) {
        const ExpCeEdition *e=&eds[i];
        uint8_t wl=0u;
        while(wl<sizeof e->why-1u && e->why[wl])++wl;
        WR(&w,e->key.competition.stable_competition);WR(&w,e->key.start_day);
        WR(&w,e->key.sequence);WR(&w,e->def_id);WR(&w,e->year);
        WR(&w,e->state);WR(&w,e->phase);WR(&w,e->block);WR(&w,e->phase_ready);
        WR(&w,e->successor);WR(&w,e->partial);WR(&w,e->participant_kind);
        WR(&w,e->rules_hash);
        WR(&w,e->expiry_day);
        WR(&w,(uint16_t)e->nentrants);
        for(k=0;k<e->nentrants;++k) {
            const ExpCeEntrant *x=&e->entrants[k];
            WR(&w,x->club.stable_club);WR(&w,x->club.kind);
            WR(&w,x->source);WR(&w,x->rank);
            WR(&w,x->group);WR(&w,x->seed);WR(&w,x->join_phase);
            WR(&w,x->from_competition);WR(&w,x->from_start_day);
        }
        WR(&w,(uint16_t)e->nfinal);
        for(k=0;k<e->nfinal;++k)WR(&w,e->final_order[k]);
        if(v3){
            WR(&w,(uint16_t)e->nregular);
            for(k=0;k<e->nregular;++k)WR(&w,e->regular_order[k]);
        }
        WR(&w,wl);wr_put(&w,e->why,wl);
    }
    WR(&w,(uint32_t)nposts);
    for(i=0;i<nposts;++i) {
        WR(&w,posts[i].event_id);WR(&w,posts[i].original_day);
        WR(&w,posts[i].new_day);WR(&w,posts[i].reason);
    }
    /* UL02: count of national call-up lists. Always 0 since the OG
     * national selector owns call-ups (v00.13.300); kept for format. */
    WR(&w,(uint32_t)0u);
    for(i=0;i<(uint32_t)w.n;++i){h^=w.b[i];h*=UINT64_C(1099511628211);}
    WR(&w,h);
    if(!w.ok){free(w.b);return NULL;}
    hex=(char *)malloc(w.n*2u+1u);
    if(!hex){free(w.b);return NULL;}
    for(i=0;i<(uint32_t)w.n;++i){hex[2u*i]=dig[w.b[i]>>4];hex[2u*i+1u]=dig[w.b[i]&15u];}
    hex[w.n*2u]=0;
    if(len)*len=w.n*2u;
    free(w.b);
    return hex;
}
char *ExpCe_EncodeHex(uint16_t world,size_t *len)
{
    return encode_hex(s_ce.eds,s_ce.neds,s_ce.posts,s_ce.nposts,world,len);
}
ExpCeSnapshot *ExpCe_SnapshotCapture(void)
{
    ExpCeSnapshot *s=(ExpCeSnapshot *)calloc(1u,sizeof *s);
    if(!s)return NULL;
    if(s_ce.neds) {
        s->eds=(ExpCeEdition *)malloc((size_t)s_ce.neds*sizeof *s->eds);
        if(!s->eds){free(s);return NULL;}
        memcpy(s->eds,s_ce.eds,(size_t)s_ce.neds*sizeof *s->eds);
    }
    if(s_ce.nposts) {
        s->posts=(ExpCePostponement *)malloc((size_t)s_ce.nposts*sizeof *s->posts);
        if(!s->posts){free(s->eds);free(s);return NULL;}
        memcpy(s->posts,s_ce.posts,(size_t)s_ce.nposts*sizeof *s->posts);
    }
    s->neds=s_ce.neds;s->nposts=s_ce.nposts;
    return s;
}
char *ExpCe_SnapshotEncodeHex(const ExpCeSnapshot *s,uint16_t world,size_t *len)
{
    return s?encode_hex(s->eds,s->neds,s->posts,s->nposts,world,len):NULL;
}
void ExpCe_SnapshotRelease(ExpCeSnapshot *s)
{
    if(!s)return;
    free(s->eds);free(s->posts);free(s);
}
static int hexval(char c)
{
    return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;
}
static int decode_hex(const char *hex,size_t len,uint16_t world,int commit);
int ExpCe_DecodeHex(const char *hex,size_t len,uint16_t world)
{return decode_hex(hex,len,world,1);}
int ExpCe_ValidateHex(const char *hex,size_t len,uint16_t world)
{return decode_hex(hex,len,world,0);}
static int decode_hex(const char *hex,size_t len,uint16_t world,int commit)
{
    uint8_t *b;
    size_t n,i;
    Rd r;
    uint16_t w16=0;
    uint32_t neds=0,nposts=0,k;
    uint64_t h=UINT64_C(14695981039346656037),stored=0;
    ExpCeEdition *eds=NULL;
    ExpCePostponement *posts=NULL;
    uint32_t ncalls=0;
    char magic[4];
    int v2=0,v3=0;
    if(!hex || (len&1u) || len<8u)return 0;
    n=len/2u;
    b=(uint8_t *)malloc(n);
    if(!b)return 0;
    for(i=0;i<n;++i) {
        const int hi=hexval(hex[2u*i]),lo=hexval(hex[2u*i+1u]);
        if(hi<0||lo<0){free(b);return 0;}
        b[i]=(uint8_t)((hi<<4)|lo);
    }
    if(n<12u){free(b);return 0;}
    for(i=0;i<n-8u;++i){h^=b[i];h*=UINT64_C(1099511628211);}
    memcpy(&stored,b+n-8u,8u);
    if(stored!=h){free(b);return 0;}
    r.b=b;r.n=n-8u;r.pos=0;r.ok=1;
    rd_get(&r,magic,4u);rd_get(&r,&w16,2u);rd_get(&r,&neds,4u);
    if(!r.ok || (memcmp(magic,"UL01",4u) && memcmp(magic,"UL02",4u) && memcmp(magic,"UL03",4u)) ||
       w16!=world || neds>1000000u){free(b);return 0;}
    v3=memcmp(magic,"UL03",4u)==0;
    v2=v3 || memcmp(magic,"UL02",4u)==0;
    if(neds && !(eds=(ExpCeEdition *)calloc(neds,sizeof *eds))){free(b);return 0;}
    for(i=0;i<neds && r.ok;++i) {
        ExpCeEdition *e=&eds[i];
        uint16_t ne=0,nf=0;
        uint8_t wl=0;
        e->key.competition.world=world;
        rd_get(&r,&e->key.competition.stable_competition,4u);
        rd_get(&r,&e->key.start_day,4u);rd_get(&r,&e->key.sequence,2u);
        rd_get(&r,&e->def_id,4u);rd_get(&r,&e->year,2u);
        rd_get(&r,&e->state,1u);rd_get(&r,&e->phase,1u);rd_get(&r,&e->block,1u);
        rd_get(&r,&e->phase_ready,1u);rd_get(&r,&e->successor,1u);
        rd_get(&r,&e->partial,1u);
        if(v2)rd_get(&r,&e->participant_kind,1u);
        if(e->participant_kind>EXP_PARTICIPANT_NATIONAL)r.ok=0;
        rd_get(&r,&e->rules_hash,8u);rd_get(&r,&e->expiry_day,4u);
        rd_get(&r,&ne,2u);
        if(ne>EXP_CE_MAX_ENTRANTS){r.ok=0;break;}
        e->nentrants=ne;
        for(k=0;k<ne;++k) {
            ExpCeEntrant *x=&e->entrants[k];
            x->club.world=world;
            rd_get(&r,&x->club.stable_club,4u);
            if(v2)rd_get(&r,&x->club.kind,1u);
            if(x->club.kind!=e->participant_kind)r.ok=0; /* never mixed */
            rd_get(&r,&x->source,1u);
            rd_get(&r,&x->rank,1u);rd_get(&r,&x->group,1u);rd_get(&r,&x->seed,1u);
            rd_get(&r,&x->join_phase,1u);rd_get(&r,&x->from_competition,4u);
            rd_get(&r,&x->from_start_day,4u);
        }
        rd_get(&r,&nf,2u);
        if(nf>EXP_CE_MAX_ENTRANTS){r.ok=0;break;}
        e->nfinal=nf;
        for(k=0;k<nf;++k)rd_get(&r,&e->final_order[k],4u);
        if(v3) {
            uint16_t nr=0;
            rd_get(&r,&nr,2u);
            if(nr>EXP_CE_MAX_ENTRANTS || (nr && (nr!=ne || nf!=ne ||
               e->state<EXP_CE_ED_COMPLETED || e->partial))) {r.ok=0;break;}
            e->nregular=nr;
            for(k=0;k<nr;++k) {
                uint32_t q;
                ExpandedClubKey c={world,0u,e->participant_kind};
                rd_get(&r,&e->regular_order[k],4u);c.stable_club=e->regular_order[k];
                if(!c.stable_club || !entrant_has(e->entrants,ne,c))r.ok=0;
                for(q=0;q<k;++q)if(e->regular_order[q]==c.stable_club)r.ok=0;
            }
        }
        rd_get(&r,&wl,1u);
        if(wl>=sizeof e->why){r.ok=0;break;}
        rd_get(&r,e->why,wl);e->why[wl]=0;
        if(e->state<EXP_CE_ED_CREATED || e->state>EXP_CE_ED_ARCHIVED ||
           e->block>EXP_CE_BLOCK_CAPACITY)r.ok=0;
    }
    rd_get(&r,&nposts,4u);
    if(r.ok && nposts>1000000u)r.ok=0;
    if(r.ok && nposts && !(posts=(ExpCePostponement *)calloc(nposts,sizeof *posts)))r.ok=0;
    for(i=0;r.ok && i<nposts;++i) {
        rd_get(&r,&posts[i].event_id,8u);rd_get(&r,&posts[i].original_day,4u);
        rd_get(&r,&posts[i].new_day,4u);rd_get(&r,&posts[i].reason,1u);
    }
    if(v2) {
        /* v00.13.299 wrote CE call-up lists here; the OG national selector
         * owns call-ups since v00.13.300: validate and discard them. */
        rd_get(&r,&ncalls,4u);
        if(r.ok && ncalls>65536u)r.ok=0;
        for(i=0;r.ok && i<ncalls;++i) {
            uint8_t lim=0,np=0,tmp8=0;
            uint32_t tmp32=0;
            rd_get(&r,&tmp32,4u);rd_get(&r,&tmp8,1u);rd_get(&r,&lim,1u);
            rd_get(&r,&tmp8,1u);rd_get(&r,&np,1u);rd_get(&r,&tmp32,4u);
            if(np>64u || np>lim){r.ok=0;break;}
            for(k=0;k<np;++k) {
                rd_get(&r,&tmp32,4u);rd_get(&r,&tmp8,1u);rd_get(&r,&tmp8,1u);
                rd_get(&r,&tmp8,1u);rd_get(&r,&tmp32,4u);
            }
        }
        if(ncalls)fprintf(stderr,"[ce-engine] UL02_V299_CALLUPS_DISCARDED lists=%lu "
            "owner=OG_NATIONAL_SELECTOR\n",(unsigned long)ncalls);
    }
    if(!r.ok || r.pos!=r.n){free(b);free(eds);free(posts);return 0;}
    free(b);
    if(!commit){free(eds);free(posts);return 1;}
    free(s_ce.eds);free(s_ce.posts);
    s_ce.eds=eds;s_ce.neds=s_ce.ceds=neds;
    s_ce.posts=posts;s_ce.nposts=s_ce.cposts=nposts;
    s_ce.world=world;
    return 1;
}
static void ed_remove(uint32_t i)
{
    if(i>=s_ce.neds)return;
    memmove(&s_ce.eds[i],&s_ce.eds[i+1u],(size_t)(s_ce.neds-i-1u)*sizeof *s_ce.eds);
    --s_ce.neds;
}
int ExpCe_ValidateAfterRestore(ExpandedUniversalRegistry *r,const ExpCeHost *host)
{
    uint32_t i;
    int changed=0;
    /* Re-evaluate blocked editions that never generated a fixture (CREATED,
     * data/entrant blocks): drop them and let the CURRENT rules recreate
     * them (predecessor's successor flag cleared, leagues without any
     * live edition restarted). No result, fixture or archive is touched. */
    for(i=0;i<s_ce.neds;) {
        ExpCeEdition *e=&s_ce.eds[i];
        const ExpCeDef *d=def_find(e->def_id);
        uint32_t j;
        if(!d || e->state!=EXP_CE_ED_CREATED ||
           (e->block!=EXP_CE_BLOCK_DATA_MISSING &&
            e->block!=EXP_CE_BLOCK_ENTRANTS)){++i;continue;}
        for(j=0;j<s_ce.neds;++j)
            if(s_ce.eds[j].def_id==e->def_id && s_ce.eds[j].year+1u==e->year)
                s_ce.eds[j].successor=0u;
        if(host && host->hold)(void)host->hold(host->user,e->key,0);
        fprintf(stderr,"[ce-engine] EDITION_REEVALUATED competition=%lu year=%u "
            "old-block=%s\n",(unsigned long)e->def_id,(unsigned)e->year,
            ExpCe_BlockName(e->block));
        ed_remove(i);
        ++changed;
    }
    /* v298: a league edition saved blocked DATE_PLAN before generating any
     * fixture (diagnostic 00.13.297) is released so the next tick can move
     * its family to the next legal window; nothing was played or scheduled */
    for(i=0;i<s_ce.neds;++i) {
        ExpCeEdition *e=&s_ce.eds[i];
        const ExpCeDef *d=def_find(e->def_id);
        uint32_t k,nf=0u;
        if(!d || d->origin!=EXP_CE_ORIGIN_LEAGUE || e->state!=EXP_CE_ED_PARTICIPANTS ||
           e->block!=EXP_CE_BLOCK_DATE_PLAN)continue;
        for(k=0;k<r->fixture_count;++k)
            if(ExpandedEditionKey_Equal(r->fixtures[k].key.edition,e->key))++nf;
        if(nf)continue;
        if(host && host->hold)(void)host->hold(host->user,e->key,0);
        fprintf(stderr,"[ce-engine] EDITION_REEVALUATED competition=%lu year=%u "
            "old-block=%s\n",(unsigned long)e->def_id,(unsigned)e->year,
            ExpCe_BlockName(e->block));
        e->block=0u;e->why[0]=0;
        ++changed;
    }
    for(i=0;i<s_ce.ndefs;++i) {
        const ExpCeDef *d=&s_ce.defs[i];
        uint32_t j;
        int any=0;
        if(d->origin!=EXP_CE_ORIGIN_LEAGUE || d->world!=s_ce.world)continue;
        for(j=0;j<s_ce.neds && !any;++j)any=s_ce.eds[j].def_id==d->id;
        if(!any) {
            ExpandedWorldDate today={0};
            (void)ExpandedWorld_IndexToDate(r->scheduler.clock.day_index,&today);
            (void)begin_league_without_fixtures(r,host,d,
                (uint16_t)(today.month>=7u?today.year:today.year-1u));
            ++changed;
        }
    }
    for(i=0;i<s_ce.neds;++i) {
        ExpCeEdition *e=&s_ce.eds[i];
        const ExpCeDef *d=def_find(e->def_id);
        if(e->state==EXP_CE_ED_ARCHIVED)continue;
        if(!d) {
            ed_block(r,host,e,EXP_CE_BLOCK_DATA_MISSING,"definition no longer present");
            ++changed;continue;
        }
        /* an edition keeps the rules it started with: never re-rule it
         * silently when the data changed between sessions */
        if(e->state>=EXP_CE_ED_SCHEDULED && e->rules_hash!=d->rules_hash &&
           e->block!=EXP_CE_BLOCK_RULES_CHANGED) {
            ed_block(r,host,e,EXP_CE_BLOCK_RULES_CHANGED,
                     "competition rules changed since the edition started");
            ++changed;
        }
    }
    return changed;
}
