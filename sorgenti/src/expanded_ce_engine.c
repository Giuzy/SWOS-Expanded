/* SWOS Expanded — Fase 2 universal Competition Engine lifecycle.
 * See include/expanded_ce_engine.h. Pure C11 (planner needs SWOS_SDL_BUILD
 * only because of its file guard; no SDL symbol is used here).
 */
#include "expanded_ce_engine.h"
#include "expanded_ce_pool_transition.h"
#include "expanded_universal_date_planner.h"
#include "expanded_universal_pairings.h"
#include "expanded_world_time.h"
#include "mjson.h"
#include "expanded_fifa_ranking.h"
#include "expanded_ce_calendar_validator.h"
#include "expanded_international_windows.h"
#include "expanded_ce_discipline.h"
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
static ExpCeEdition *ed_find(ExpandedEditionKey k);
/* v00.13.355 data of the generic table criteria ACCESS_LIST / DISCIPLINE */
static uint32_t ce_access_rank(void *u,ExpandedEditionKey k,ExpandedClubKey c)
{
    const ExpCeEdition *e=ed_find(k);
    uint32_t i;
    (void)u;
    if(!e)return 0u;
    for(i=0;i<e->nentrants;++i)
        if(ExpandedClubKey_Equal(e->entrants[i].club,c))return e->entrants[i].seed;
    return 0u;
}
static int ce_discipline_of(void *u,uint64_t id,uint32_t *h,uint32_t *a)
{(void)u;return ExpCeDisc_Get(id,h,a);}
void ExpCe_Reset(void)
{
    ExpCeTable_SetHooks(ce_access_rank,ce_discipline_of,NULL);
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
    if(d->cycle_years>1u){h^=0xCCu;h*=UINT64_C(1099511628211);h^=d->cycle_years;h*=UINT64_C(1099511628211);}
    for(p=0;p<d->nphases;++p) {
        const ExpCePhaseDef *ph=&d->phases[p];
        const uint64_t r=ExpCeRules_Hash(ph->kind==EXP_CE_PHASE_TABLE?
                                         &ph->table:NULL,&ph->match);
        uint8_t b[8];
        unsigned i;
        b[0]=ph->kind;b[1]=ph->groups;b[2]=ph->meetings;b[3]=ph->qualify;
        b[4]=ph->pairing;b[5]=ph->byes;b[6]=ph->higher_home;b[7]=(uint8_t)p;
        if(ph->qualify_from>1u){h^=0xC3u;h*=UINT64_C(1099511628211);h^=ph->qualify_from;h*=UINT64_C(1099511628211);}
        for(i=0;i<8u;++i){h^=b[i];h*=UINT64_C(1099511628211);}
        for(i=0;i<8u;++i){h^=(uint8_t)(r>>(8u*i));h*=UINT64_C(1099511628211);}
        for(i=0;i<4u;++i){h^=(uint8_t)(ph->host_nonmember_comp>>(8u*i));h*=UINT64_C(1099511628211);}
        if(ph->nties){unsigned z;h^=0xE7u;h*=UINT64_C(1099511628211);
            for(z=0u;z<ph->nties*6u;++z){h^=ph->tsrc[z/6u][(z/3u)%2u][z%3u];h*=UINT64_C(1099511628211);}}
        if(ph->ngp){unsigned z;h^=0xE1u;h*=UINT64_C(1099511628211);
            for(z=0u;z<ph->ngp*4u;++z){h^=ph->gp[z/4u][z%4u];h*=UINT64_C(1099511628211);}}
        h=ExpCal_Hash(h,&ph->cal);
        if(ph->parallel || ph->source_phase_p1 || ph->nhi || ph->qualify_groups) {
            unsigned z;
            h^=0x5Au;h*=UINT64_C(1099511628211);
            h^=ph->parallel;h*=UINT64_C(1099511628211);
            h^=ph->source_phase_p1;h*=UINT64_C(1099511628211);
            h^=ph->qualify_groups;h*=UINT64_C(1099511628211);
            for(z=0;z<ph->nhi;++z){h^=ph->hi[z];h*=UINT64_C(1099511628211);h^=ph->lo[z];h*=UINT64_C(1099511628211);}
        }
        if(ph->fifa_importance || ph->fifa_protect || ph->fifa_seeding) {
            h^=0xF1u;h*=UINT64_C(1099511628211);
            h^=ph->fifa_importance;h*=UINT64_C(1099511628211);
            h^=ph->fifa_protect;h*=UINT64_C(1099511628211);
            h^=ph->fifa_seeding;h*=UINT64_C(1099511628211);
        }
        if(ph->band_size || ph->ntpl_rounds) {   /* v00.13.355, only when used */
            unsigned z;
            h^=0xB7u;h*=UINT64_C(1099511628211);
            h^=ph->band_size;h*=UINT64_C(1099511628211);h^=ph->band_groups;h*=UINT64_C(1099511628211);
            h^=ph->band_per_group;h*=UINT64_C(1099511628211);
            h^=ph->ntpl_rounds;h*=UINT64_C(1099511628211);h^=ph->ntpl_matches;h*=UINT64_C(1099511628211);
            for(z=0;z<(unsigned)ph->ntpl_rounds*ph->ntpl_matches*2u;++z)
                {h^=ph->tpl[z/(ph->ntpl_matches*2u)][(z/2u)%ph->ntpl_matches][z%2u];h*=UINT64_C(1099511628211);}
        }
        if(ph->nrel) { /* only when used: existing definitions keep their hash */
            h^=0xA5u;h*=UINT64_C(1099511628211);
            h^=ph->rel_anchor;h*=UINT64_C(1099511628211);
            for(i=0;i<ph->nrel;++i){h^=(uint8_t)ph->rel[i];h*=UINT64_C(1099511628211);
                h^=(uint8_t)(ph->rel[i]>>8);h*=UINT64_C(1099511628211);}
        }
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
    if(d->promotion_playoff){h^=0x77u;h*=UINT64_C(1099511628211);}
    if(d->fin_gap_min||d->playout_gap_min){h^=0x91u;h*=UINT64_C(1099511628211);
        h^=d->fin_gap_hi;h*=UINT64_C(1099511628211);h^=d->fin_gap_lo;h*=UINT64_C(1099511628211);
        h^=d->fin_gap_min;h*=UINT64_C(1099511628211);h^=d->playout_gap_min;h*=UINT64_C(1099511628211);}
    if(d->first_from_sources){h^=0x2Fu;h*=UINT64_C(1099511628211);}
    if(d->xch_on){unsigned i;h^=0x7Bu;h*=UINT64_C(1099511628211);
        h^=d->xch_rank;h*=UINT64_C(1099511628211);h^=d->xch_champ;h*=UINT64_C(1099511628211);
        for(i=0u;i<d->xch_nroute && i<16u;++i){h^=d->xch_route_club[i];h*=UINT64_C(1099511628211);
            h^=d->xch_route_div[i];h*=UINT64_C(1099511628211);}}
    if(d->pool_direct||d->pool_playoff){unsigned i;h^=0x6Du;h*=UINT64_C(1099511628211);
        h^=d->pool_direct;h*=UINT64_C(1099511628211);h^=d->pool_playoff;h*=UINT64_C(1099511628211);
        for(i=0u;i<d->pool_n && i<8u;++i){h^=(uint8_t)d->pool_clubs[i];h*=UINT64_C(1099511628211);
            h^=(uint8_t)(d->pool_clubs[i]>>8);h*=UINT64_C(1099511628211);h^=(uint8_t)(d->pool_clubs[i]>>16);h*=UINT64_C(1099511628211);}}
    if(d->par_n) {
        unsigned i;
        h^=0x5Bu;h*=UINT64_C(1099511628211);
        h^=d->par_n;h*=UINT64_C(1099511628211);
        h^=d->par_per_group;h*=UINT64_C(1099511628211);
        h^=d->par_extra;h*=UINT64_C(1099511628211);
        for(i=0u;i<d->par_n && i<8u;++i){h^=d->par_groups[i];h*=UINT64_C(1099511628211);}
    }
    if(d->has_playout) { /* only when used: existing definitions keep their hash */
        const ExpCePhaseDef *po=&d->playout;
        const uint64_t r=ExpCeRules_Hash(NULL,&po->match);
        unsigned i;
        h^=0x3Du;h*=UINT64_C(1099511628211);
        h^=d->playout_hi;h*=UINT64_C(1099511628211);
        h^=d->playout_lo;h*=UINT64_C(1099511628211);
        h^=d->playout_relegated_total;h*=UINT64_C(1099511628211);
        h^=po->higher_home;h*=UINT64_C(1099511628211);
        for(i=0u;i<8u;++i){h^=(uint8_t)(r>>(8u*i));h*=UINT64_C(1099511628211);}
        for(i=0u;i<po->nrel;++i){h^=(uint8_t)po->rel[i];h*=UINT64_C(1099511628211);}
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
        if(ph->kind==EXP_CE_PHASE_TABLE && ph->table.draw_penalties &&
           (!ph->match.penalties || ph->match.extra_time || ph->match.legs!=1u) &&
           !ph->missing[0])
            snprintf(ph->missing,sizeof ph->missing,
                     "table.draw_penalties needs single-leg match penalties and no extra time");
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
               ph->meetings>4u ||
               (ph->match.needs_winner && !ph->table.draw_penalties)) {
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
        {   /* v311: a group phase whose draws are decided on penalties validates its match
             * rules as a tie that needs a winner (penalties allowed, no extra time) */
            ExpCeMatchRules mchk=ph->match;
            if(ph->kind==EXP_CE_PHASE_TABLE && ph->table.draw_penalties)mchk.needs_winner=1u;
            if(!ExpCeRules_ValidateMatch(&mchk,&err)) {                snprintf(d->missing,sizeof d->missing,"phase %u %.24s: %.48s",
                     (unsigned)p,err.field,err.reason);return 0;
            }
        }
        if(!ph->has_window && !ph->ndays && !ph->nrel) {
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
/* v00.13.342: days on which a club of the phase being generated already has
 * a fixture of another competition (sorted). The planner never uses them. */
#define EXP_CE_BUSY_MAX 32768u
static int32_t s_busy[EXP_CE_BUSY_MAX];
static uint32_t s_busy_n;
static int busy_cmp(const void *a,const void *b)
{
    const int32_t x=*(const int32_t *)a,y=*(const int32_t *)b;
    return x<y?-1:x>y?1:0;
}
static int busy_blocked(void *ctx,int32_t day)
{
    (void)ctx;
    return s_busy_n && bsearch(&day,s_busy,s_busy_n,sizeof s_busy[0],busy_cmp)!=NULL;
}
static void busy_build(const ExpandedUniversalRegistry *r,const ExpCeEntrant *ent,uint32_t n)
{
    uint32_t i,j,w=0u;
    s_busy_n=0u;
    if(!r)return;
    for(i=0u;i<r->fixture_count && w<EXP_CE_BUSY_MAX;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        for(j=0u;j<n;++j)
            if(ent[j].club.stable_club==f->home.stable_club ||
               ent[j].club.stable_club==f->away.stable_club) {
                s_busy[w++]=f->day_index;break;
            }
    }
    qsort(s_busy,w,sizeof s_busy[0],busy_cmp);
    s_busy_n=w;
}
/* breaks of the last planned window (read by the day profile) */
static ExpandedDatePlanBreak s_plan_breaks[EXP_DATE_PLAN_MAX_BREAKS];
static uint32_t s_plan_nbreaks;
static int s_plan_national;   /* edition being planned has national teams */
static uint8_t s_plan_intl;   /* effective international window types of the phase */
/* Plan `rounds` civil days for a phase. earliest = first allowed day. */
static int plan_window_days(const ExpCePhaseDef *ph,int32_t edition_start,
                            int32_t earliest,uint32_t rounds,int32_t *days);
/* explicit verified days (first edition) or the window planner */
static int plan_phase_days_ex(const ExpCeDef *d,const ExpCeEdition *e,
                              const ExpCePhaseDef *ph,int32_t earliest,
                              uint32_t rounds,int32_t *days)
{
    s_plan_nbreaks=0u;
    s_plan_national=e && e->participant_kind==EXP_PARTICIPANT_NATIONAL;
    s_plan_intl=ph->window.intl;
    if(s_plan_intl==EXP_INTL_INHERIT)   /* league finals/play-outs: same as the league season */
        s_plan_intl=(d && d->origin==EXP_CE_ORIGIN_LEAGUE && d->phases[0].window.intl!=EXP_INTL_INHERIT)?
                    d->phases[0].window.intl:0u;
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
    pol.blocked=busy_blocked;
    for(i=0;i<w->nbreaks && i<EXP_DATE_PLAN_MAX_BREAKS;++i) {
        ExpandedDatePlanBreak *b=&pol.breaks[pol.break_count];
        if(w->break_season[i]) {   /* break of one edition only */
            ExpandedWorldDate es;
            if(!ExpandedWorld_IndexToDate(edition_start,&es) || es.year!=w->break_season[i])continue;
        }
        if(!md_on_or_after(w->breaks[i][0],w->breaks[i][1],first,&b->first_day) ||
           !md_on_or_after(w->breaks[i][2],w->breaks[i][3],b->first_day,
                           &b->last_day))return 0;
        ++pol.break_count;
    }
    {   /* v00.13.354: international windows (global table) of the declared
         * types; a national-team edition plays IN them. Games stopped by a
         * window move after it (window extended, logged). */
        int32_t ext=0;
        const int ok=ExpIntl_PlanWithWindows(&pol,(uint8_t)(s_plan_national?0u:s_plan_intl),
                                             rounds,days,&ext);
        /* the day profile later moves single games: same breaks */
        s_plan_nbreaks=pol.break_count;
        memcpy(s_plan_breaks,pol.breaks,sizeof s_plan_breaks);
        if(ok && ext)fprintf(stderr,"[ce-intl] WINDOW_EXTENDED_AFTER_INTERNATIONAL_BREAK +%ld days "
                             "(window end %ld -> %ld)\n",(long)ext,(long)last,(long)pol.last_day);
        return ok;
    }
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
/* v00.13.342 generic priority + make-up (recupero) rule. A phase is planned
 * avoiding every day on which one of its clubs already has a fixture; when
 * its own window cannot fit (e.g. a short knockout window) the phase has
 * priority: it is planned ignoring those days and the already registered,
 * still SCHEDULED, future fixtures of OTHER editions that collide are moved
 * to the first following day free for both clubs. Nothing is invented. */
static int plan_with_priority(const ExpCeDef *d,const ExpCeEdition *e,
                              const ExpCePhaseDef *ph,int32_t earliest,
                              uint32_t rounds,int32_t *days)
{
    const uint32_t keep=s_busy_n;
    if(plan_phase_days_ex(d,e,ph,earliest,rounds,days))return 1;
    if(!keep)return 0;
    s_busy_n=0u;                    /* priority: ignore the other competitions' days */
    if(plan_phase_days_ex(d,e,ph,earliest,rounds,days))return 1;
    s_busy_n=keep;
    return 0;
}
#define EXP_CE_MAKEUP_MAX 64u
#define EXP_CE_MAKEUP_SPAN 35
static int club_free_on(const ExpandedUniversalRegistry *r,const GenBuf *g,uint64_t skip_id,
                        uint32_t c1,uint32_t c2,int32_t day)
{
    uint32_t i;
    for(i=0u;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *x=&r->fixtures[i];
        if(x->day_index!=day || x->event_id==skip_id)continue;
        if(x->home.stable_club==c1 || x->away.stable_club==c1 ||
           x->home.stable_club==c2 || x->away.stable_club==c2)return 0;
    }
    for(i=0u;i<g->n;++i) {
        const ExpandedUniversalFixture *x=&g->f[i];
        if(x->day_index!=day)continue;
        if(x->home.stable_club==c1 || x->away.stable_club==c1 ||
           x->home.stable_club==c2 || x->away.stable_club==c2)return 0;
    }
    return 1;
}
/* 1 = no collision remains (after moving make-ups), 0 = cannot be resolved */
static int resolve_collisions(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                              const GenBuf *g,int32_t today)
{
    uint64_t ids[EXP_CE_MAKEUP_MAX];
    uint32_t n=0u,i,j,k;
    if(!r)return 1;
    for(i=0u;i<g->n;++i)
        for(j=0u;j<r->fixture_count;++j) {
            const ExpandedUniversalFixture *x=&r->fixtures[j],*y=&g->f[i];
            int dup=0;
            if(x->day_index!=y->day_index)continue;
            if(x->home.stable_club!=y->home.stable_club && x->home.stable_club!=y->away.stable_club &&
               x->away.stable_club!=y->home.stable_club && x->away.stable_club!=y->away.stable_club)continue;
            if(x->event_id==y->event_id)return 0;
            if(x->day_index<=today ||
               ExpandedUniversal_EntryFor(r,x)->state!=EXP_FIXTURE_SCHEDULED)return 0;
            for(k=0u;k<n;++k)if(ids[k]==x->event_id)dup=1;
            if(dup)continue;
            if(n>=EXP_CE_MAKEUP_MAX || !host || !host->reschedule)return 0;
            ids[n++]=x->event_id;
        }
    for(k=0u;k<n;++k) {
        const ExpandedUniversalFixture *x=ExpandedUniversal_Find(r,ids[k]);
        int32_t day,to=0,from;
        if(!x)return 0;
        from=x->day_index;
        for(day=x->day_index+1;day<=x->day_index+EXP_CE_MAKEUP_SPAN;++day)
            if(club_free_on(r,g,ids[k],x->home.stable_club,x->away.stable_club,day)){to=day;break;}
        if(!to || !host->reschedule(host->user,ids[k],to))return 0;
        fprintf(stderr,"[ce-engine] MAKEUP event=%llu moved +%d days (priority phase)\n",
                (unsigned long long)ids[k],(int)(to-from));
    }
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
    const uint8_t *return_map;   /* ASYMMETRIC: odd cycle round -> position */
    const int8_t *group_off;     /* first edition verified day offsets of this group */
    uint32_t cycles;
} RrCtx;
static int rr_emit(void *user,uint32_t cycle,uint32_t round,uint32_t serial,
                   uint32_t home,uint32_t away)
{
    RrCtx *c=(RrCtx *)user;
    ExpandedUniversalFixture f;
    if(home>=c->n || away>=c->n)return 0;
    fixture_base(c->d,c->e,&f);
    f.home=c->clubs[home];f.away=c->clubs[away];
    {
        const uint32_t pos=(c->return_map && (cycle&1u) && cycle<c->cycles &&
                            round<c->rounds_per_cycle)?c->return_map[round]:round;
        f.day_index=c->days[cycle*c->rounds_per_cycle+pos];
        if(c->group_off && cycle*c->rounds_per_cycle+pos<16u)
            f.day_index+=c->group_off[cycle*c->rounds_per_cycle+pos];
    }
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

/* ---- v00.13.354 calendar rules: asymmetric return, draw, day profile ---- */
/* registry + this phase: is `day` free for both clubs of x? (profile) */
typedef struct ProfileCtx {const ExpandedUniversalRegistry *r;} ProfileCtx;
static int profile_free_day(void *ctx,const ExpandedUniversalFixture *x,
                            const ExpandedUniversalFixture *all,uint32_t n,int32_t day)
{
    const ProfileCtx *p=(const ProfileCtx *)ctx;
    GenBuf g;
    uint32_t b;
    for(b=0u;b<s_plan_nbreaks;++b)
        if(day>=s_plan_breaks[b].first_day && day<=s_plan_breaks[b].last_day)return 0;
    g.f=(ExpandedUniversalFixture *)all;g.n=n;g.cap=n;
    return !p->r || club_free_on(p->r,&g,x->event_id,x->home.stable_club,x->away.stable_club,day);
}
/* Independent validation of a generated TABLE phase before publication.
 * Returns the HARD failures (0 = publish). */
static uint32_t validate_table_phase(const GenBuf *g,const ExpCeDef *d,const ExpCeEdition *e,
    uint32_t phase,ExpandedClubKey groups[EXP_CE_MAX_GROUPS][EXP_CE_MAX_ENTRANTS],
    const uint32_t *size)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    ExpCevReport rep;
    uint32_t gi,hard=0u;
    char tag[80];
    for(gi=0u;gi<ph->groups;++gi) {
        uint32_t clubs[EXP_CE_MAX_ENTRANTS],j;
        ExpCevRoundRobin x;
        for(j=0u;j<size[gi];++j)clubs[j]=groups[gi][j].stable_club;
        memset(&x,0,sizeof x);
        if(ph->ntpl_rounds) {   /* fixed template: slots checked at load, global checks only */
            ExpCev_Init(&rep);
            ExpCev_Global(g->f,g->n,0u,&rep);
            snprintf(tag,sizeof tag,"competition=%lu edition=%ld phase=%u group=%u template=1",
                     (unsigned long)d->id,(long)e->key.start_day,(unsigned)phase,(unsigned)gi+1u);
            hard+=ExpCev_Log(&rep,tag);
            continue;
        }
        x.clubs=clubs;x.nclubs=size[gi];x.cycles=ph->meetings;
        x.extra_rounds=ph->extras.rounds;x.min_return_gap=ph->cal.min_return_gap;
        x.max_run=ph->cal.max_run;
        x.mirror=(uint8_t)(ph->meetings<2u?0u:ph->cal.return_order==1u?2u:ph->extras.rounds?0u:1u);
        ExpCev_Init(&rep);
        ExpCev_RoundRobin(g->f,g->n,e->key,EXP_CE_STAGE(phase,gi+1u),&x,&rep);
        ExpCev_Global(g->f,g->n,ph->cal.nprofile?(ph->window.rest>1u?ph->window.rest:1u):0u,&rep);
        snprintf(tag,sizeof tag,"competition=%lu edition=%ld phase=%u group=%u",
                 (unsigned long)d->id,(long)e->key.start_day,(unsigned)phase,(unsigned)gi+1u);
        hard+=ExpCev_Log(&rep,tag);
    }
    return hard;
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
        for(rank=ph->qualify_from?ph->qualify_from:1u;rank<=ph->qualify;++rank)
            for(g=1u;g<=(ph->qualify_groups?ph->qualify_groups:ph->groups);++g) {
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
        if(j==nr || j>=d->phases[0].qualify ||
           j+1u<(d->phases[0].qualify_from?d->phases[0].qualify_from:1u))return 0;
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
/* ---- v00.13.355 interim ranking across groups (art. 19, generic) ---- */
enum { EXP_CE_XC_GROUP_POSITION=1, EXP_CE_XC_POINTS, EXP_CE_XC_GOAL_DIFF, EXP_CE_XC_GOALS_FOR,
       EXP_CE_XC_AWAY_GOALS_FOR, EXP_CE_XC_WINS, EXP_CE_XC_AWAY_WINS, EXP_CE_XC_DISCIPLINE,
       EXP_CE_XC_ACCESS_LIST };
typedef struct XRow { ExpandedClubKey club; uint32_t pos, gsize, group;
    int64_t pts,gd,gf,agf,w,aw,disc; int disc_known; uint32_t access; } XRow;
static uint32_t ce_access_rank(void *u,ExpandedEditionKey k,ExpandedClubKey c);
/* stats of club over the group stage, optionally without games v `skip` */
static void xrow_stats(const ExpandedUniversalRegistry *r,const ExpCeTableRules *t,
                       ExpandedEditionKey ed,uint32_t stage,XRow *x,const ExpandedClubKey *skip)
{
    uint32_t i;
    x->pts=x->gd=x->gf=x->agf=x->w=x->aw=x->disc=0;x->disc_known=1;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *l=&r->ledger.entries[i];
        int home;
        uint32_t gs,gc,dh=0,da=0;
        if(!ExpandedEditionKey_Equal(f->key.edition,ed) || f->key.stage!=stage ||
           l->state!=EXP_FIXTURE_PLAYED)continue;
        if(ExpandedClubKey_Equal(f->home,x->club))home=1;
        else if(ExpandedClubKey_Equal(f->away,x->club))home=0;
        else continue;
        if(skip && (ExpandedClubKey_Equal(home?f->away:f->home,*skip)))continue;
        gs=home?l->score.home:l->score.away;gc=home?l->score.away:l->score.home;
        x->gf+=gs;x->gd+=(int64_t)gs-(int64_t)gc;
        if(!home)x->agf+=gs;
        if(gs>gc){x->pts+=t->win;++x->w;if(!home)++x->aw;}
        else if(gs==gc)x->pts+=t->draw;else x->pts+=t->loss;
        if(ExpCeDisc_Get(f->event_id,&dh,&da))x->disc+=home?dh:da;else x->disc_known=0;
    }
}
static const uint8_t *s_xc;static uint8_t s_nxc;
static int xrow_cmp(const void *pa,const void *pb)
{
    const XRow *a=(const XRow *)pa,*b=(const XRow *)pb;
    uint8_t i;
    for(i=0;i<s_nxc;++i) {
        int64_t x=0,y=0;
        switch(s_xc[i]) {
        case EXP_CE_XC_GROUP_POSITION:x=-(int64_t)a->pos;y=-(int64_t)b->pos;break;
        case EXP_CE_XC_POINTS:x=a->pts;y=b->pts;break;
        case EXP_CE_XC_GOAL_DIFF:x=a->gd;y=b->gd;break;
        case EXP_CE_XC_GOALS_FOR:x=a->gf;y=b->gf;break;
        case EXP_CE_XC_AWAY_GOALS_FOR:x=a->agf;y=b->agf;break;
        case EXP_CE_XC_WINS:x=a->w;y=b->w;break;
        case EXP_CE_XC_AWAY_WINS:x=a->aw;y=b->aw;break;
        case EXP_CE_XC_DISCIPLINE:   /* only when both are known: never guessed */
            if(a->disc_known && b->disc_known){x=-a->disc;y=-b->disc;}
            break;
        case EXP_CE_XC_ACCESS_LIST:
            x=a->access?-(int64_t)a->access:INT64_MIN/2;y=b->access?-(int64_t)b->access:INT64_MIN/2;break;
        default:break;
        }
        if(x!=y)return x>y?-1:1;
    }
    return 0;
}
/* Interim ranking of edition e (all tiers, in order). 1 ok (out[0..*n)). */
static int interim_ranking(const ExpandedUniversalRegistry *r,const ExpCeDef *d,
                           const ExpCeEdition *e,const ExpCeHost *host,
                           ExpandedClubKey *out,uint32_t *n)
{
    const uint32_t p=d->interim_phase;
    const ExpCePhaseDef *ph;
    uint32_t tier,g0=1u,k=0u;
    *n=0u;
    if(!d->ninterim_tiers || p>=d->nphases)return 0;
    ph=&d->phases[p];
    if(ph->kind!=EXP_CE_PHASE_TABLE || phase_complete(r,d,e,p)!=1)return 0;
    for(tier=0;tier<d->ninterim_tiers;++tier) {
        XRow rows[EXP_CE_MAX_ENTRANTS];
        uint32_t nr=0u,g,maxsize=0u,minsize=255u,i;
        for(g=g0;g<g0+d->interim_tiers[tier];++g) {
            ExpCeTableRow tr[EXP_CE_MAX_ENTRANTS];
            uint32_t used=0u;
            if(g>ph->groups || !ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(p,g),&ph->table,
                   host?host->name_cmp:NULL,host?host->user:NULL,tr,EXP_CE_MAX_ENTRANTS,&used))return 0;
            if(used>maxsize)maxsize=used;
            if(used<minsize)minsize=used;
            for(i=0;i<used && nr<EXP_CE_MAX_ENTRANTS;++i) {
                memset(&rows[nr],0,sizeof rows[nr]);
                rows[nr].club=tr[i].club;rows[nr].pos=i+1u;rows[nr].gsize=used;rows[nr].group=g;
                rows[nr].access=ce_access_rank(NULL,e->key,tr[i].club);
                ++nr;
            }
        }
        for(i=0;i<nr;++i) {
            /* unequal sizes: positions above the last of a larger group compare
             * without the games against that group's last-placed team */
            const ExpandedClubKey *skip=NULL;
            ExpandedClubKey last;
            if(d->interim_exclude_last && maxsize!=minsize && rows[i].gsize==maxsize && rows[i].pos<maxsize) {
                uint32_t j;
                for(j=0;j<nr;++j)
                    if(rows[j].group==rows[i].group && rows[j].pos==maxsize){last=rows[j].club;skip=&last;}
            }
            xrow_stats(r,&ph->table,e->key,EXP_CE_STAGE(p,rows[i].group),&rows[i],skip);
        }
        s_xc=d->interim_cross;s_nxc=d->ninterim_cross;
        qsort(rows,nr,sizeof rows[0],xrow_cmp);
        for(i=0;i<nr;++i){if(k>=EXP_CE_MAX_ENTRANTS)return 0;out[k++]=rows[i].club;}
        g0+=d->interim_tiers[tier];
    }
    *n=k;
    return 1;
}
int ExpCe_InterimRanking(const ExpandedUniversalRegistry *r,const ExpCeHost *host,
                         ExpandedEditionKey edition,
                         ExpandedClubKey *out,uint32_t cap,uint32_t *n)
{
    const ExpCeEdition *e=ed_find(edition);
    const ExpCeDef *d=e?def_find(e->def_id):NULL;
    ExpandedClubKey tmp[EXP_CE_MAX_ENTRANTS];
    uint32_t m=0u;
    if(n)*n=0u;
    if(!r || !d || !out || !n || !interim_ranking(r,d,e,host,tmp,&m) || m>cap)return 0;
    memcpy(out,tmp,(size_t)m*sizeof *tmp);*n=m;
    return 1;
}
static int phase_entrants(const ExpandedUniversalRegistry *r,
                          const ExpCeDef *d,const ExpCeEdition *e,
                          const ExpCeHost *host,uint32_t phase,
                          ExpCeEntrant *out,uint32_t *n)
{
    uint32_t i,k=0u;
    *n=0u;
    if(d->phases[phase].nhi) {
        /* v00.13.355: two pots from the interim ranking (hi[] then lo[]) */
        ExpandedClubKey ir[EXP_CE_MAX_ENTRANTS];
        uint32_t ni=0u,j;
        const ExpCePhaseDef *pp=&d->phases[phase];
        if(!interim_ranking(r,d,e,host,ir,&ni))return 0;
        for(j=0;j<(uint32_t)pp->nhi+pp->nlo;++j) {
            const uint32_t rk=j<pp->nhi?pp->hi[j]:pp->lo[j-pp->nhi];
            if(!rk || rk>ni || k>=EXP_CE_MAX_ENTRANTS)return 0;
            out[k]=(ExpCeEntrant){ir[rk-1u],EXP_CE_SRC_PHASE_RANK,(uint8_t)rk,0u,(uint8_t)(k+1u),0u,
                e->key.competition.stable_competition,e->key.start_day};
            ++k;
        }
        *n=k;
        return 1;
    }
    if(phase>0u && !phase_qualifiers(r,d,e,host,
          d->phases[phase].source_phase_p1?(uint32_t)d->phases[phase].source_phase_p1-1u:phase-1u,
          out,&k))return 0;
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
/* FIFA seeding (phase "fifa.seeding":"FIFA_RANKING", national editions):
 * order the entrants best first by the ranking snapshot frozen at the FIRST
 * draw of this edition phase; later ranking changes never alter it. Byes,
 * SEEDED pairs and POTS then use the generic order. */
static int fifa_seed_entrants(const ExpCeDef *d,const ExpCeEdition *e,uint32_t phase,
                              ExpCeEntrant *ent,uint32_t n,char *why,size_t whylen)
{
    uint32_t clubs[EXP_FIFA_SNAPSHOT_CLUBS],i,k;
    ExpCeEntrant tmp[EXP_CE_MAX_ENTRANTS];
    if(!d->phases[phase].fifa_seeding || n<2u)return 1;
    if(e->participant_kind!=EXP_PARTICIPANT_NATIONAL || n>EXP_FIFA_SNAPSHOT_CLUBS ||
       n>EXP_CE_MAX_ENTRANTS || !ExpFifa_Active()) {
        snprintf(why,whylen,"phase %u: FIFA seeding unavailable",(unsigned)phase);
        return 0;
    }
    for(i=0;i<n;++i)clubs[i]=ent[i].club.stable_club;
    if(!ExpFifa_SeedOrder(e->key.competition.stable_competition,e->key.start_day,
                          e->key.sequence,phase,e->key.start_day,clubs,n)) {
        snprintf(why,whylen,"phase %u: entrant without FIFA ranking",(unsigned)phase);
        return 0;
    }
    memcpy(tmp,ent,(size_t)n*sizeof *ent);
    for(i=0;i<n;++i)
        for(k=0;k<n;++k)
            if(tmp[k].club.stable_club==clubs[i]){ent[i]=tmp[k];ent[i].seed=(uint8_t)(i+1u);break;}
    return 1;
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
    case EXP_CE_PAIR_SOURCES:
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
    case EXP_CE_PAIR_GROUP_TABLE: {
        uint32_t p;
        if(ph->ngp==0u || *np!=ph->ngp || ph->byes) {
            snprintf(why,whylen,"GROUP_TABLE: %lu entrants need %u explicit pairs",
                     (unsigned long)n,(unsigned)ph->ngp);
            return 0;
        }
        for(p=0u;p<ph->ngp;++p) {
            const ExpCeEntrant *hi=NULL,*lo=NULL;
            for(i=0u;i<n;++i) {
                if(pool[i].group==ph->gp[p][0] && pool[i].rank==ph->gp[p][1])hi=&pool[i];
                if(pool[i].group==ph->gp[p][2] && pool[i].rank==ph->gp[p][3])lo=&pool[i];
            }
            if(!hi || !lo || hi==lo) {
                snprintf(why,whylen,"GROUP_TABLE pair %u: qualifier not found",(unsigned)p+1u);
                return 0;
            }
            pairs[p].hi=*hi;pairs[p].lo=*lo;
        }
        return 1;
    }
    case EXP_CE_PAIR_POT_PAIRS: {
        /* v00.13.355 (UEFA play-off draw): LO pot drawn first into slots
         * 1..np, HI pot drawn second; hi = HI pot. Deterministic CE draw. */
        uint32_t lo_ord[64],hi_ord[64];
        uint64_t seed=draw_seed(e,phase);
        if(m>128u || (m&1u)){snprintf(why,whylen,"POT_PAIRS: odd pots");return 0;}
        for(i=0;i<*np;++i){lo_ord[i]=*np+i;hi_ord[i]=i;}
        for(i=*np;i>1u;--i){const uint32_t j=(uint32_t)(draw_next(&seed)%i);const uint32_t t=lo_ord[i-1u];lo_ord[i-1u]=lo_ord[j];lo_ord[j]=t;}
        for(i=*np;i>1u;--i){const uint32_t j=(uint32_t)(draw_next(&seed)%i);const uint32_t t=hi_ord[i-1u];hi_ord[i-1u]=hi_ord[j];hi_ord[j]=t;}
        for(i=0;i<*np;++i){pairs[i].lo=pool[lo_ord[i]];pairs[i].hi=pool[hi_ord[i]];}
        return 1;
    }
    case EXP_CE_PAIR_WINNERS_RUNNERS_DRAW: {
        /* v00.13.355 (UEFA Nations League QF draw, block7KnockoutDraw):
         * runners-up drawn first, each placed against the first winner (in
         * numeric slot order) of a different group that keeps the rest of
         * the draw feasible. Deterministic CE draw per edition/phase. */
        const uint32_t groups=m/2u;
        uint32_t order[8],used=0u,k;
        uint64_t seed=draw_seed(e,phase);
        if(groups<2u || groups>8u || m!=2u*groups) {
            snprintf(why,whylen,"WINNERS_RUNNERS_DRAW needs 2..8 groups x 2 qualifiers");
            return 0;
        }
        for(i=0;i<groups;++i)
            if(pool[i].rank!=1u || pool[groups+i].rank!=2u) {
                snprintf(why,whylen,"WINNERS_RUNNERS_DRAW: qualifiers not 1st/2nd");
                return 0;
            }
        for(i=0;i<groups;++i)order[i]=i;
        for(i=groups;i>1u;--i){const uint32_t j=(uint32_t)(draw_next(&seed)%i);const uint32_t t=order[i-1u];order[i-1u]=order[j];order[j]=t;}
        for(k=0;k<groups;++k) {
            const ExpCeEntrant *ru=&pool[groups+order[k]];
            uint32_t w;
            int placed=0;
            for(w=0;w<groups && !placed;++w) {
                uint32_t rest_ok=1u,a,b2;
                if((used>>w)&1u || pool[w].group==ru->group)continue;
                /* lookahead: the remaining runners must still be placeable
                 * (Hall: here simply every remaining runner keeps a free
                 * winner of another group; groups <= 8) */
                for(a=k+1u;a<groups && rest_ok;++a) {
                    const ExpCeEntrant *r2=&pool[groups+order[a]];
                    uint32_t found=0u;
                    for(b2=0;b2<groups;++b2)
                        if(b2!=w && !((used>>b2)&1u) && pool[b2].group!=r2->group)found=1u;
                    rest_ok=found;
                }
                if(k+2u==groups) {   /* last two: exact check */
                    const ExpCeEntrant *r2=&pool[groups+order[k+1u]];
                    uint32_t b3;
                    for(b3=0;b3<groups;++b3)
                        if(b3!=w && !((used>>b3)&1u))rest_ok=pool[b3].group!=r2->group;
                }
                if(!rest_ok)continue;
                used|=1u<<w;
                pairs[w].hi=pool[w];pairs[w].lo=*ru;placed=1;
            }
            if(!placed) {
                snprintf(why,whylen,"WINNERS_RUNNERS_DRAW: no feasible draw");
                return 0;
            }
        }
        *np=groups;
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
    if(!fifa_seed_entrants(d,e,phase,ent,n,why,whylen) ||
       !ko_pairs_raw(d,e,phase,ent,n,pairs,np,why,whylen))return 0;
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
    if(!fifa_seed_entrants(d,e,phase,ent,n,why,whylen))return 0;
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
    } else if(ph->pairing==EXP_CE_PAIR_BANDED_POTS) {
        /* v00.13.355: bands of band_size entrants by seed (access list), band b
         * -> groups b*band_groups.. ; pots of band_groups*per_group, each drawn
         * per_group to every group of the band (CE deterministic draw) */
        const uint32_t bs=ph->band_size,bg=ph->band_groups,pg=ph->band_per_group?ph->band_per_group:1u;
        uint64_t seed=draw_seed(e,phase);
        uint32_t b,pot;
        if(!bs || !bg || bs!=bg*pg*(bs/(bg*pg)) || n%bs || (n/bs)*bg!=ph->groups) {
            snprintf(why,whylen,"phase %u: %lu entrants do not fit bands %u/%u/%u",
                     (unsigned)phase,(unsigned long)n,(unsigned)bs,(unsigned)bg,(unsigned)pg);
            return 0;
        }
        for(i=1u;i<n;++i) {      /* by seed (access-list rank) */
            const ExpCeEntrant x=ent[i];
            uint32_t j=i;
            while(j && ent[j-1u].seed>x.seed){ent[j]=ent[j-1u];--j;}
            ent[j]=x;
        }
        for(b=0;b<n/bs;++b)
            for(pot=0;pot<bs/(bg*pg);++pot) {
                ExpCeEntrant *pe=ent+b*bs+pot*bg*pg;
                draw_shuffle(pe,bg*pg,&seed);
                for(i=0;i<bg*pg;++i)groups[b*bg+i/pg][size[b*bg+i/pg]++]=pe[i].club;
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
        if(ph->pairing!=EXP_CE_PAIR_BANDED_POTS)   /* BANDED_POTS: slot order = pot order */
            qsort(groups[g],size[g],sizeof groups[g][0],key_cmp);
        if(ph->ntpl_rounds) {
            uint32_t r2,m2;
            for(r2=0;r2<ph->ntpl_rounds;++r2)for(m2=0;m2<ph->ntpl_matches;++m2)
                if(ph->tpl[r2][m2][0]>=size[g] || ph->tpl[r2][m2][1]>=size[g]) {
                    snprintf(why,whylen,"phase %u group %u: round template needs more clubs",
                             (unsigned)phase,(unsigned)g+1u);
                    return 0;
                }
        }
    }
    return 1;
}
/* v311: sides of a SOURCES phase (rank of the opening table, winner/loser of an earlier
 * tie), in tie order: out[2t]=home, out[2t+1]=away. 1 ok, 0 not resolvable (yet). */
static int src_phase_entrants(const ExpandedUniversalRegistry *r,const ExpCeDef *d,
                              const ExpCeEdition *e,const ExpCeHost *host,uint32_t phase,
                              ExpCeEntrant *out,uint32_t *n)
{
    const ExpCePhaseDef *ph=&d->phases[phase];
    ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
    uint32_t nr=0u,t,s,k=0u;
    int have_table=0;
    *n=0u;
    if(!ph->nties || ph->nties>8u)return 0;
    for(t=0u;t<ph->nties;++t)
        for(s=0u;s<2u;++s) {
            const uint8_t *x=ph->tsrc[t][s];
            ExpandedClubKey c={0};
            uint32_t seed=0u;
            if(x[0]==1u) {
                if(!have_table) {
                    if(d->phases[0].kind!=EXP_CE_PHASE_TABLE || d->phases[0].groups!=1u ||
                       phase_complete(r,d,e,0u)!=1 ||
                       !ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(0u,1u),&d->phases[0].table,
                           host?host->name_cmp:NULL,host?host->user:NULL,rows,EXP_CE_MAX_ENTRANTS,&nr))
                        return 0;
                    have_table=1;
                }
                if(x[1]>nr)return 0;
                c=rows[x[1]-1u].club;seed=x[1];
            } else {
                ExpandedClubKey w,l;
                const uint32_t pp=x[1];
                if(pp>=phase || pp<1u || d->phases[pp].kind!=EXP_CE_PHASE_KNOCKOUT ||
                   ExpCeTie_Resolve(r,e->key,EXP_CE_STAGE(pp,EXP_CE_KO_GROUP),x[2],
                                    &d->phases[pp].match,&w,&l)!=EXP_CE_TIE_DECIDED)return 0;
                c=x[0]==2u?w:l;
            }
            if(k>=EXP_CE_MAX_ENTRANTS)return 0;
            out[k]=(ExpCeEntrant){c,x[0]==1u?EXP_CE_SRC_PHASE_RANK:EXP_CE_SRC_PHASE_WINNER,
                (uint8_t)seed,0u,(uint8_t)(k+1u),0u,e->key.competition.stable_competition,e->key.start_day};
            ++k;
        }
    *n=k;
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
    if(phase==0u && d->next_missing[0] && e->year!=d->first_year) {
        snprintf(why,whylen,"%.95s",d->next_missing);
        return 0;
    }
    if(phase>0u) {
        /* v00.13.355: a parallel phase starts after the phase BEFORE its block */
        uint32_t pp=phase-1u;
        if(ph->parallel){while(pp>0u && d->phases[pp].parallel)--pp; if(pp>0u)--pp;}
        const int32_t prev=stage_last_day(r,e->key,pp);
        if(prev!=INT32_MIN && prev+1+(int32_t)ph->window.rest>earliest)
            earliest=prev+1+(int32_t)ph->window.rest;
    }
    if(!((ph->kind==EXP_CE_PHASE_KNOCKOUT && ph->pairing==EXP_CE_PAIR_SOURCES)?
         src_phase_entrants(r,d,e,host,phase,ent,&n):
         phase_entrants(r,d,e,host,phase,ent,&n)) || n<2u) {
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
    busy_build(r,ent,n);
    if(ph->kind==EXP_CE_PHASE_TABLE) {
        static ExpandedClubKey groups[EXP_CE_MAX_GROUPS][EXP_CE_MAX_ENTRANTS];
        uint32_t size[EXP_CE_MAX_GROUPS],rounds=0u,gi;
        int32_t days[2048];
        if(!form_groups(d,e,phase,ent,n,groups,size,why,whylen))return 0;
        if(ph->cal.cal_draw)    /* crit. 38: calendar numbers drawn per edition */
            for(gi=0;gi<ph->groups;++gi)
                ExpCal_DrawOrder(groups[gi],size[gi],draw_seed(e,phase)^UINT64_C(0xC0FFEE)^gi);
        for(gi=0;gi<ph->groups;++gi) {
            const uint32_t rr=ph->ntpl_rounds?ph->ntpl_rounds:
                ((size[gi]&1u)?size[gi]:size[gi]-1u)*ph->meetings+ph->extras.rounds;
            if(rr>rounds)rounds=rr;
        }
        if(!rounds || rounds>2048u ||
           !plan_with_priority(d,e,ph,earliest,rounds,days)) {
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
            c.cycles=ph->meetings;
            if(ph->has_gdays && ph->ndays && e->year==d->first_year && gi<EXP_CE_MAX_GROUPS)
                c.group_off=ph->gday_off[gi];
            if(ph->ntpl_rounds) {   /* v00.13.355 fixed schedule of group slots */
                uint32_t r2,m2;
                c.rounds_per_cycle=ph->ntpl_rounds;c.cycles=1u;
                for(r2=0;r2<ph->ntpl_rounds;++r2)for(m2=0;m2<ph->ntpl_matches;++m2)
                    if(!rr_emit(&c,0u,r2,r2*ph->ntpl_matches+m2,ph->tpl[r2][m2][0],ph->tpl[r2][m2][1])) {
                        snprintf(why,whylen,"phase %u: round template failed",(unsigned)phase);
                        *block=EXP_CE_BLOCK_CAPACITY;goto done;
                    }
                continue;
            }
            uint32_t keys[EXP_PAIR_MAX_CLUBS],j;
            uint8_t rmap[EXP_CAL_RR_MAX];
            if(ph->cal.return_order==1u && ph->meetings>=2u) {
                int run=0;
                if(!ExpCal_ReturnMap(size[gi],c.rounds_per_cycle,ph->cal.min_return_gap,
                                  draw_seed(e,phase)^(gi+1u),rmap,&run)) {
                    snprintf(why,whylen,"phase %u: return gap %u impossible with %lu rounds",
                             (unsigned)phase,(unsigned)ph->cal.min_return_gap,
                             (unsigned long)c.rounds_per_cycle);
                    *block=EXP_CE_BLOCK_DATA_MISSING;goto done;
                }
                c.return_map=rmap;
                fprintf(stderr,"[ce-engine] RETURN_ORDER competition=%lu edition=%ld group=%u "
                    "order=ASYMMETRIC min_gap=%u longest_home_away_run=%d\n",(unsigned long)d->id,
                    (long)e->key.start_day,(unsigned)gi+1u,(unsigned)ph->cal.min_return_gap,run);
            }
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
        if(ph->cal.nprofile) {
            uint32_t moved=0u,kept=0u,skipped=0u;
            ProfileCtx pc={r};
            const int32_t lo=earliest>e->key.start_day?earliest:e->key.start_day;
            ExpCal_ApplyDayProfile(g.f,g.n,&ph->cal,ph->window.weekdays,ph->window.rest,
                days,rounds,lo,draw_seed(e,phase),profile_free_day,&pc,&moved,&kept,&skipped);
            fprintf(stderr,"[ce-engine] DAY_PROFILE competition=%lu edition=%ld phase=%u "
                "moved=%lu kept_on_round_day=%lu rounds_not_profiled=%lu\n",(unsigned long)d->id,
                (long)e->key.start_day,(unsigned)phase,(unsigned long)moved,
                (unsigned long)kept,(unsigned long)skipped);
        }
        if(validate_table_phase(&g,d,e,phase,groups,size)) {
            snprintf(why,whylen,"phase %u: calendar validation failed (CALENDAR_HARD_CONFLICT)",
                     (unsigned)phase);
            *block=EXP_CE_BLOCK_CAPACITY;goto done;
        }
    } else {
        KoPair pairs[EXP_CE_MAX_ENTRANTS/2u];
        uint32_t np=0u,t;
        int32_t days[2];
        if(!ko_pairs(d,e,phase,ent,n,pairs,&np,why,whylen))return 0;
        if(ph->nrel) {
            /* v309: days relative to the ACTUAL end of an earlier phase */
            const int32_t anchor=stage_last_day(r,e->key,ph->rel_anchor);
            uint32_t k;
            int okRel=anchor!=INT32_MIN && ph->rel_anchor<phase &&
                      ph->nrel==ph->match.legs;
            for(k=0u;okRel && k<ph->nrel;++k) {
                days[k]=anchor+(int32_t)ph->rel[k];
                if(days[k]<earliest)okRel=0;
            }
            if(!okRel) {
                snprintf(why,whylen,"phase %u: relative days do not fit after phase %u",
                         (unsigned)phase,(unsigned)ph->rel_anchor);
                *block=EXP_CE_BLOCK_DATE_PLAN;
                return 0;
            }
        } else if(!plan_with_priority(d,e,ph,earliest,ph->match.legs,days)) {
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
    if(!resolve_collisions(r,host,&g,today) ||
       !host || !host->insert || !host->insert(host->user,g.f,g.n)) {
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
    s_busy_n=0u;
    free(g.f);
    return ok;
}

/* ------------------------------------------------------------------ */
/* league PLAY-OUT (v310): a relegation tie between two table ranks     */
/* ------------------------------------------------------------------ */
/* points of regular ranks hi and lo (1-based) of the completed regular table */
static int regular_gap(const ExpandedUniversalRegistry *r,const ExpCeDef *d,const ExpCeEdition *e,
                       uint32_t hi,uint32_t lo,uint32_t *gap)
{
    ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
    uint32_t nr=0u;
    if(!r || !d || !e || hi<1u || lo<=hi || d->phases[0].kind!=EXP_CE_PHASE_TABLE ||
       d->phases[0].groups!=1u || e->partial || phase_complete(r,d,e,0u)!=1 ||
       !ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(0u,1u),&d->phases[0].table,NULL,NULL,
                           rows,EXP_CE_MAX_ENTRANTS,&nr) || nr<lo)return 0;
    *gap=rows[hi-1u].points>=rows[lo-1u].points?rows[hi-1u].points-rows[lo-1u].points:0u;
    return 1;
}
int ExpCe_FinalsSkipped(const ExpandedUniversalRegistry *r,const ExpCeDef *d,const ExpCeEdition *e)
{
    uint32_t gap=0u;
    return d && d->fin_gap_min && d->nphases>1u && regular_gap(r,d,e,d->fin_gap_hi,d->fin_gap_lo,&gap) &&
           gap>d->fin_gap_min;
}
int ExpCe_PlayoutSkipped(const ExpandedUniversalRegistry *r,const ExpCeDef *d,const ExpCeEdition *e)
{
    uint32_t gap=0u;
    return d && d->playout_gap_min && ExpCe_PlayoutActive(d) &&
           regular_gap(r,d,e,d->playout_hi,d->playout_lo,&gap) && gap>d->playout_gap_min;
}
int ExpCe_PlayoutActive(const ExpCeDef *d)
{
    if(d && d->origin==EXP_CE_ORIGIN_LEAGUE && d->has_playout && d->pool_playoff)  /* pool challenger */
        return !d->missing[0] && d->pool_direct && d->pool_n>d->pool_direct;
    return d && d->origin==EXP_CE_ORIGIN_LEAGUE && d->has_playout &&
           !d->missing[0] && d->playout_hi>=1u && d->playout_hi<d->playout_lo &&
           d->playout_lo+(uint32_t)(d->relegated-1u)<=d->clubs &&
           d->relegated==d->playout_relegated_total && d->relegated>=1u;
}
/* ---- v311 bottom pool: universe, current pool, deterministic role order ---- */
static int up_has(const uint32_t *a,uint32_t n,uint32_t v);
static int cmp_u32_local(const void *a,const void *b)
{const uint32_t x=*(const uint32_t *)a,y=*(const uint32_t *)b;return x<y?-1:x>y?1:0;}
/* all clubs this competition ever had (any edition) + the configured pool clubs */
static uint32_t pool_universe(const ExpCeDef *d,uint32_t *out,uint32_t cap)
{
    uint32_t n=0u,i,j;
    for(i=0u;i<d->pool_n && n<cap;++i)
        if(!up_has(out,n,d->pool_clubs[i]))out[n++]=d->pool_clubs[i];
    for(i=0u;i<s_ce.neds;++i) {
        const ExpCeEdition *x=&s_ce.eds[i];
        if(x->def_id!=d->id)continue;
        for(j=0u;j<x->nentrants && n<cap;++j)
            if(!up_has(out,n,x->entrants[j].club.stable_club))out[n++]=x->entrants[j].club.stable_club;
    }
    return n;
}
/* ---- v340 league exchange helpers ---- */
static uint32_t league_rank_count(const ExpCeEdition *e);
static uint32_t league_rank_at(const ExpCeEdition *e,uint32_t i);
/* the exchange config (an `xch_on` league of the same country/competition) that routes `d` */
static const ExpCeDef *xch_link(const ExpCeDef *d)
{
    uint32_t i,k;
    if(!d || d->origin!=EXP_CE_ORIGIN_LEAGUE || d->xch_on)return NULL;
    for(i=0u;i<s_ce.ndefs;++i) {
        const ExpCeDef *a=&s_ce.defs[i];
        if(!a->xch_on || a->country!=d->country || a->comp_id!=d->comp_id)continue;
        for(k=0u;k<a->xch_nroute;++k)if(a->xch_route_div[k]==d->division)return a;
    }
    return NULL;
}
static const ExpCeEdition *latest_edition_of(uint32_t def_id)
{
    const ExpCeEdition *best=NULL;
    uint32_t i;
    for(i=0u;i<s_ce.neds;++i)
        if(s_ce.eds[i].def_id==def_id && (!best || s_ce.eds[i].key.start_day>best->key.start_day))
            best=&s_ce.eds[i];
    return best;
}
static int ed_has_club(const ExpCeEdition *e,uint32_t club)
{
    uint32_t i;
    for(i=0u;i<e->nentrants;++i)if(e->entrants[i].club.stable_club==club)return 1;
    return 0;
}
static int xch_up_has(const uint32_t *a,uint32_t n,uint32_t v)
{uint32_t i;for(i=0u;i<n;++i)if(a[i]==v)return 1;return 0;}
/* the league division (region) a club belongs to: static routing first, else the region it has
 * been a member of in any edition. -1 = unknown (never invented). */
static int xch_region_of(const ExpCeDef *a,uint32_t club)
{
    uint32_t k,i;
    for(k=0u;k<a->xch_nroute;++k)if(a->xch_route_club[k]==club)return (int)a->xch_route_div[k];
    for(k=0u;k<a->xch_nroute;++k) {
        const ExpCeDef *rd=def_find(ExpCe_LeagueCompetitionId(a->country,a->comp_id,a->xch_route_div[k]));
        if(!rd)continue;
        for(i=0u;i<s_ce.neds;++i)
            if(s_ce.eds[i].def_id==rd->id && ed_has_club(&s_ce.eds[i],club))return (int)a->xch_route_div[k];
    }
    return -1;
}
/* A-League side: 1 exchange (W replaces X), 0 none, -1 wait (the champion edition is not complete) */
static int xch_decide(const ExpCeDef *a,const ExpCeEdition *e,uint32_t *w,uint32_t *x)
{
    const ExpCeEdition *src=NULL;
    uint32_t i;
    if(!a->xch_on || e->partial || league_rank_count(e)!=e->nentrants || e->nentrants<a->xch_rank)return 0;
    if(!def_find(a->xch_champ))return 0;
    for(i=0u;i<s_ce.neds;++i)
        if(s_ce.eds[i].def_id==a->xch_champ && s_ce.eds[i].year==e->year)src=&s_ce.eds[i];
    if(!src)return 0;
    if(src->block && src->state<EXP_CE_ED_COMPLETED)return 0;
    if(src->state<EXP_CE_ED_COMPLETED)return -1;
    if(src->partial || !src->nfinal)return 0;
    *w=src->final_order[0];
    if(ed_has_club(e,*w))return 0;                 /* already in the league: no promotion, no relegation */
    *x=league_rank_at(e,a->xch_rank-1u);
    if(xch_region_of(a,*x)<0)return 0;             /* destination unknown: no movement */
    return 1;
}
/* region side: the A-League editions around this region edition (P running at its start, S its
 * successor). 1 ok, 0 nothing to do, -1 wait for the A-League rollover. */
static int xch_region_state(const ExpCeDef *a,const ExpCeEdition *e,const ExpCeEdition **p,const ExpCeEdition **s)
{
    const ExpCeEdition *best=NULL;
    uint32_t i;
    for(i=0u;i<s_ce.neds;++i) {
        const ExpCeEdition *o=&s_ce.eds[i];
        if(o->def_id!=a->id || o->key.start_day>e->key.start_day)continue;
        if(!best || o->key.start_day>best->key.start_day)best=o;
    }
    if(!best)return 0;
    if(!best->successor)return best->state>=EXP_CE_ED_COMPLETED?-1:0;
    for(i=0u;i<s_ce.neds;++i)
        if(s_ce.eds[i].def_id==a->id && s_ce.eds[i].year==(uint16_t)(best->year+1u)){*p=best;*s=&s_ce.eds[i];return 1;}
    return -1;
}
/* region side: does the A-League movement of P -> S concern this region at all? */
static int xch_region_affected(const ExpCeDef *d,const ExpCeDef *a,const ExpCeEdition *e,
                               const ExpCeEdition *p,const ExpCeEdition *sx)
{
    uint32_t i;
    for(i=0u;i<p->nentrants;++i) {
        const uint32_t c=p->entrants[i].club.stable_club;
        if(!ed_has_club(sx,c) && xch_region_of(a,c)==(int)d->division)return 1;
    }
    for(i=0u;i<sx->nentrants;++i) {
        const uint32_t c=sx->entrants[i].club.stable_club;
        if(!ed_has_club(p,c) && ed_has_club(e,c))return 1;
    }
    return 0;
}
/* region side: apply the exchange to the next membership nx (count *nn, size N). `ranked` = the
 * current members best first. -1: no pool club available / inconsistent (never invented). */
static int xch_region_adjust(const ExpCeDef *d,const ExpCeDef *a,const ExpCeEdition *e,
    const ExpCeEdition *p,const ExpCeEdition *sx,uint32_t *nx,uint32_t *nn,uint32_t N,
    const uint32_t *ranked,uint32_t nr,const uint32_t *pool,uint32_t np,uint32_t *xin,uint32_t *nxin)
{
    uint32_t i,k,n=*nn;
    *nxin=0u;
    for(i=0u;i<p->nentrants;++i) {          /* relegated into this region */
        const uint32_t c=p->entrants[i].club.stable_club;
        if(!ed_has_club(sx,c) && xch_region_of(a,c)==(int)d->division && !xch_up_has(nx,n,c)){
            if(*nxin>=4u || n>=EXP_CE_MAX_ENTRANTS)return -1;
            xin[(*nxin)++]=c;nx[n++]=c;
        }
    }
    for(i=0u;i<sx->nentrants;++i) {         /* promoted out of this region */
        const uint32_t c=sx->entrants[i].club.stable_club;
        if(!ed_has_club(p,c) && ed_has_club(e,c)) {
            for(k=0u;k<n;++k)if(nx[k]==c){nx[k]=nx[--n];break;}
        }
    }
    while(n>N) {                             /* one too many: the worst kept club goes to the pool */
        int done=0;
        for(i=nr;i>0u && !done;--i) {
            const uint32_t c=ranked[i-1u];
            if(xch_up_has(xin,*nxin,c))continue;
            for(k=0u;k<n;++k)if(nx[k]==c){nx[k]=nx[--n];done=1;break;}
        }
        if(!done)return -1;
    }
    for(i=0u;i<np && n<N;++i) {              /* one short: the next pool club by the CE draw order */
        const uint32_t c=pool[i];
        if(xch_up_has(nx,n,c) || ed_has_club(sx,c))continue;
        nx[n++]=c;
    }
    if(n!=N)return -1;
    *nn=n;
    return 1;
}

/* pool = universe minus members, then the role order of THIS edition: a deterministic CE
 * draw seeded by (competition, year), never catalogue order. pool[0..direct) are the direct
 * entrants, pool[direct] the play-out challenger. Returns the pool size, UINT32_MAX on error. */
static uint32_t pool_roles(const ExpCeDef *d,const ExpCeEdition *e,uint32_t *pool,uint32_t cap)
{
    uint32_t uni[64],nu=pool_universe(d,uni,64u),np=0u,i;
    uint64_t seed;
    if(nu>=64u)return UINT32_MAX;
    for(i=0u;i<nu;++i) {
        uint32_t j;int in=0;
        for(j=0u;j<e->nentrants;++j)if(e->entrants[j].club.stable_club==uni[i])in=1;
        {   /* v340: a club that now plays in the linked top league is not a free club */
            const ExpCeDef *xa=xch_link(d);
            const ExpCeEdition *top=xa?latest_edition_of(xa->id):NULL;
            if(top && ed_has_club(top,uni[i]))in=1;
        }
        if(!in){if(np>=cap)return UINT32_MAX;pool[np++]=uni[i];}
    }
    qsort(pool,np,sizeof *pool,cmp_u32_local);
    seed=UINT64_C(0x9E3779B97F4A7C15)^((uint64_t)d->id<<20)^e->year;
    for(i=np;i>1u;--i) {                       /* Fisher-Yates with a fixed LCG */
        uint32_t j,t;
        seed=seed*UINT64_C(6364136223846793005)+UINT64_C(1442695040888963407);
        j=(uint32_t)((seed>>33)%i);
        t=pool[i-1u];pool[i-1u]=pool[j];pool[j]=t;
    }
    return np;
}
/* 1 generated and fully played with a decided tie (loser set), 0 otherwise */
int ExpCe_PlayoutLoser(const ExpandedUniversalRegistry *r,const ExpCeDef *d,
                       const ExpCeEdition *e,uint32_t *loser)
{
    ExpandedClubKey w,l;
    if(!r || !d || !e || !loser || !ExpCe_PlayoutActive(d))return 0;
    if(ExpCe_PlayoutSkipped(r,d,e)) {   /* the gap rule: the lower rank goes down directly */
        ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
        uint32_t nr=0u;
        if(!ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(0u,1u),&d->phases[0].table,NULL,NULL,
                               rows,EXP_CE_MAX_ENTRANTS,&nr) || nr<d->playout_lo)return 0;
        *loser=rows[d->playout_lo-1u].club.stable_club;
        return 1;
    }
    if(ExpCeTie_Resolve(r,e->key,EXP_CE_STAGE(EXP_CE_PLAYOUT_PHASE,EXP_CE_KO_GROUP),1u,
                        &d->playout.match,&w,&l)!=EXP_CE_TIE_DECIDED)return 0;
    *loser=l.stable_club;
    return 1;
}
/* 1 generated, 0 not yet / not applicable, -1 blocked (why). */
static int generate_playout(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                            const ExpCeDef *d,ExpCeEdition *e,int32_t today,
                            uint8_t *block,char *why,size_t wl)
{
    ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
    uint32_t nr=0u,leg;
    const ExpCePhaseDef *ph=&d->playout;
    ExpandedUniversalFixture fx[2];
    ExpandedClubKey hi,lo,first,second;
    int32_t days[2],anchor;
    *block=EXP_CE_BLOCK_DATA_MISSING;
    if(!ExpCe_PlayoutActive(d) || e->partial)return 0;
    if(phase_complete(r,d,e,0u)!=1)return 0;
    if(ExpCe_PlayoutSkipped(r,d,e))return 0; /* gap rule: no play-out is played */
    if(!ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(0u,1u),&d->phases[0].table,
           host?host->name_cmp:NULL,host?host->user:NULL,rows,EXP_CE_MAX_ENTRANTS,&nr) ||
       nr!=e->nentrants || (!d->pool_playoff && nr<d->playout_lo)) {
        snprintf(why,wl,"play-out: regular table unavailable");return -1;
    }
    if(d->pool_playoff) {   /* v311: upper = just above the direct drop, lower = pool challenger */
        uint32_t pool[16],np=pool_roles(d,e,pool,16u);
        if(np==UINT32_MAX || np<=d->pool_direct || nr<=d->pool_direct+1u) {
            snprintf(why,wl,"play-out: bottom pool has no challenger");return -1;
        }
        hi=rows[nr-d->pool_direct-1u].club;
        lo=(ExpandedClubKey){e->key.competition.world,pool[d->pool_direct],EXP_PARTICIPANT_CLUB};
    } else {
        hi=rows[d->playout_hi-1u].club;lo=rows[d->playout_lo-1u].club;
    }
    for(leg=0u;leg<ph->match.legs;++leg)days[leg]=0;
    anchor=stage_last_day(r,e->key,0u);
    if(anchor==INT32_MIN || ph->nrel!=ph->match.legs) {
        snprintf(why,wl,"play-out: no anchor / relative days mismatch");
        *block=EXP_CE_BLOCK_DATE_PLAN;return -1;
    }
    for(leg=0u;leg<ph->nrel;++leg) {
        days[leg]=anchor+(int32_t)ph->rel[leg];
        if(days[leg]<=today){
            snprintf(why,wl,"play-out: relative day already passed");
            *block=EXP_CE_BLOCK_DATE_PLAN;return -1;
        }
    }
    if(host && host->club_ok &&
       (!host->club_ok(host->user,hi) || !host->club_ok(host->user,lo))) {
        snprintf(why,wl,"play-out: club has no roster");*block=EXP_CE_BLOCK_ROSTER;return -1;
    }
    /* higher_home 1: the better rank hosts leg 0; 2: it hosts the deciding leg */
    first=ph->higher_home==1u?hi:lo;second=first.stable_club==hi.stable_club?lo:hi;
    for(leg=0u;leg<ph->match.legs;++leg) {
        fixture_base(d,e,&fx[leg]);
        fx[leg].home=leg?second:first;
        fx[leg].away=leg?first:second;
        fx[leg].day_index=days[leg];
        fx[leg].key.stage=EXP_CE_STAGE(EXP_CE_PLAYOUT_PHASE,EXP_CE_KO_GROUP);
        fx[leg].key.tie=1u;fx[leg].key.leg=(uint16_t)leg;
        fx[leg].event_id=fixture_event_id(&fx[leg].key);
        if(!ExpandedUniversal_FixtureValid(&fx[leg])) {
            snprintf(why,wl,"play-out: generated fixture invalid");
            *block=EXP_CE_BLOCK_CAPACITY;return -1;
        }
    }
    if(!host || !host->insert || !host->insert(host->user,fx,ph->match.legs)) {
        snprintf(why,wl,"play-out: fixtures rejected by the registry");
        *block=EXP_CE_BLOCK_COLLISION;return -1;
    }
    fprintf(stderr,"[ce-engine] PLAYOUT_GENERATED world=%u competition=%lu year=%u "
        "ranks=%u/%u legs=%u source=CE_TABLE no-invented-club=1\n",
        (unsigned)e->key.competition.world,
        (unsigned long)e->key.competition.stable_competition,(unsigned)e->year,
        (unsigned)d->playout_hi,(unsigned)d->playout_lo,(unsigned)ph->match.legs);
    return 1;
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
static void fo_push(ExpCeEdition *e,uint32_t *k,uint32_t club)
{
    uint32_t i;
    for(i=0u;i<*k;++i)if(e->final_order[i]==club)return; /* already placed (bracket graphs) */
    e->final_order[(*k)++]=club;
}
/* v00.13.355 final ranking by data buckets. Selector kinds: 1 winner of
 * (phase a, tie b), 2 loser of (a,b), 3 winners of phase a, 4 losers of
 * phase a, 5 interim ranks a..b. Phases are 0-based here. */
static int ko_side(const ExpandedUniversalRegistry *r,const ExpCeDef *d,const ExpCeEdition *e,
                   uint32_t phase,uint32_t tie,int winner,ExpandedClubKey *out)
{
    ExpandedClubKey w,l;
    if(phase>=d->nphases || ExpCeTie_Resolve(r,e->key,EXP_CE_STAGE(phase,EXP_CE_KO_GROUP),tie,
           &d->phases[phase].match,&w,&l)!=EXP_CE_TIE_DECIDED)return 0;
    *out=winner?w:l;
    return 1;
}
static int bucket_order(const ExpandedUniversalRegistry *r,const ExpCeDef *d,ExpCeEdition *e,
                        const ExpCeHost *host)
{
    ExpandedClubKey ir[EXP_CE_MAX_ENTRANTS];
    uint32_t ni=0u,b,k=0u;
    if(!interim_ranking(r,d,e,host,ir,&ni))return 0;
    for(b=0;b<d->nbuckets;++b) {
        ExpandedClubKey set[EXP_CE_MAX_ENTRANTS];
        uint32_t ns=0u,s2,i,j;
        for(s2=0;s2<d->buckets[b].nsel;++s2) {
            const uint8_t kind=d->buckets[b].sel[s2].kind,a=d->buckets[b].sel[s2].a,bb=d->buckets[b].sel[s2].b;
            if(kind==1u||kind==2u) {
                if(ns>=EXP_CE_MAX_ENTRANTS || !ko_side(r,d,e,a,bb,kind==1u,&set[ns]))return 0;
                ++ns;
            } else if(kind==3u||kind==4u) {
                const uint32_t ties=ko_ties(r,e->key,a);
                uint32_t t;
                if(!ties)return 0;
                for(t=1;t<=ties;++t){if(ns>=EXP_CE_MAX_ENTRANTS || !ko_side(r,d,e,a,t,kind==3u,&set[ns]))return 0;++ns;}
            } else if(kind==5u) {
                uint32_t q;
                if(!a || bb<a || bb>ni)return 0;
                for(q=a;q<=bb;++q){if(ns>=EXP_CE_MAX_ENTRANTS)return 0;set[ns++]=ir[q-1u];}
            } else return 0;
        }
        if(d->buckets[b].by_interim) {   /* stable order by interim rank */
            for(i=1;i<ns;++i){
                ExpandedClubKey x=set[i];uint32_t rx=0,ry;
                for(rx=0;rx<ni && !ExpandedClubKey_Equal(ir[rx],x);++rx){}
                j=i;
                while(j>0){
                    for(ry=0;ry<ni && !ExpandedClubKey_Equal(ir[ry],set[j-1u]);++ry){}
                    if(ry<=rx)break;
                    set[j]=set[j-1u];--j;
                }
                set[j]=x;
            }
        }
        for(i=0;i<ns;++i) {
            for(j=0;j<k;++j)if(e->final_order[j]==set[i].stable_club)return 0;   /* placed twice */
            if(k>=EXP_CE_MAX_ENTRANTS)return 0;
            e->final_order[k++]=set[i].stable_club;
        }
    }
    if(k!=e->nentrants)return 0;   /* every entrant exactly once */
    e->nfinal=k;
    return 1;
}
static int final_order(const ExpandedUniversalRegistry *r,const ExpCeDef *d,
                       ExpCeEdition *e,const ExpCeHost *host)
{
    int32_t p;
    uint32_t k=0u;
    e->nfinal=0u;
    if(d->nbuckets)return bucket_order(r,d,e,host);
    const int skipped=d->origin==EXP_CE_ORIGIN_LEAGUE && ExpCe_FinalsSkipped(r,d,e);
    for(p=skipped?0:(int32_t)d->nphases-1;p>=0;--p) {
        const ExpCePhaseDef *ph=&d->phases[p];
        if(ph->kind==EXP_CE_PHASE_TABLE) {
            ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
            uint32_t used=0u,rank,g;
            const uint32_t from=((uint32_t)p+1u==d->nphases || skipped)?1u:(uint32_t)ph->qualify+1u;
            /* v309: ranks ABOVE the first qualifying rank (e.g. the two directly
             * promoted Serie B clubs) did not play the finals; they follow the
             * finals result in the title order (rollover uses regular_order) */
            if(!skipped && (uint32_t)p+1u!=d->nphases && ph->qualify_from>1u)
                for(rank=1u;rank<ph->qualify_from;++rank)
                    for(g=1u;g<=ph->groups;++g) {
                        if(!ExpCeTable_Compute(r,e->key,EXP_CE_STAGE(p,g),&ph->table,
                               host?host->name_cmp:NULL,host?host->user:NULL,
                               rows,EXP_CE_MAX_ENTRANTS,&used))return 0;
                        if(used>=rank) {
                            if(k>=EXP_CE_MAX_ENTRANTS)return 0;
                            e->final_order[k++]=rows[rank-1u].club.stable_club;
                        }
                    }
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
                fo_push(e,&k,w.stable_club);
                fo_push(e,&k,l.stable_club);
            } else
                for(t=1u;t<=ties;++t) {
                    ExpandedClubKey w,l;
                    if(ExpCeTie_Resolve(r,e->key,EXP_CE_STAGE(p,EXP_CE_KO_GROUP),
                           t,&ph->match,&w,&l)!=EXP_CE_TIE_DECIDED ||
                       k>=EXP_CE_MAX_ENTRANTS)return 0;
                    fo_push(e,&k,l.stable_club);
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
/* v341: the source edition a dependent edition (starting at `dep_start`) must use. Among the
 * editions of `competition` that END (their own expiry, from their definition) no later than
 * `dep_start`, the one that started last: complete -> use it; not complete -> the dependent edition
 * WAITS (pending) or is blocked (blocked). This ties an annual dependency to the right season (the
 * Championship of 2027 needs the 2027 league seasons, not the closed 2026 ones) while a cup played
 * in December still uses the last season that finished (Supercoppa 2027 <- league 2026/27).
 * NULL with neither flag: no edition ends before dep_start, the caller falls back to latest_closed. */
static const ExpCeEdition *source_for(uint32_t competition,int32_t dep_start,int *blocked,int *pending)
{
    const ExpCeEdition *best=NULL;
    const ExpCeDef *d=def_find(competition);
    uint32_t i;
    *blocked=0;*pending=0;
    if(!d)return NULL;
    for(i=0;i<s_ce.neds;++i) {
        const ExpCeEdition *o=&s_ce.eds[i];
        int32_t end=0;
        if(o->key.competition.stable_competition!=competition || o->partial)continue;
        {   int ok=edition_expiry_day(d,o->year,o->key.start_day,&end);
            int p;
            /* a league whose last phase is a finals series with relative days has no absolute end:
             * its regular-season window is the earliest the competition can end */
            for(p=(int)d->nphases-1;p>=0 && !ok;--p)
                if(d->phases[p].has_window)
                    ok=md_on_or_after(d->phases[p].window.last_m,d->phases[p].window.last_d,o->key.start_day,&end);
            if(!ok || end>dep_start || o->key.start_day>=dep_start)continue;
        }
        if(!best || o->key.start_day>best->key.start_day)best=o;
    }
    if(!best)return NULL;
    if(best->state>=EXP_CE_ED_COMPLETED && best->nfinal)return best;
    if(best->block)*blocked=1; else *pending=1;
    return NULL;
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
        {   int pend=0;
            src=source_for(sp->competition,e->key.start_day,&blocked,&pend);
            if(!src && pend)return 0;           /* the season this edition needs is still being played */
            if(!src && !blocked)src=latest_closed(sp->competition,&blocked);
            if(!src && !blocked && sp->alt_competition) {   /* v00.13.355 predecessor definition */
                pend=0;
                src=source_for(sp->alt_competition,e->key.start_day,&blocked,&pend);
                if(!src && pend)return 0;
                if(!src && !blocked)src=latest_closed(sp->alt_competition,&blocked);
            }
        }
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
    uint32_t xw=0u,xx=0u;
    int xs=0;                                   /* v340: A-League side of the exchange */
    const ExpCeDef *xa=NULL;                    /* v340: region side */
    const ExpCeEdition *xp=NULL,*xsx=NULL;
    int xr=0;
    if(e->successor || e->block || e->year==65535u ||
       e->nentrants!=d->clubs || e->nentrants>EXP_CE_MAX_ENTRANTS)return 0;
    for(i=0u;i<s_ce.neds;++i)
        if(s_ce.eds[i].def_id==d->id && s_ce.eds[i].year==year)return 0;
    if(d->xch_on) {
        xs=xch_decide(d,e,&xw,&xx);
        if(xs<0)return 0;                       /* the champion edition of this year is not complete */
    } else if((xa=xch_link(d))!=NULL) {
        xr=xch_region_state(xa,e,&xp,&xsx);
        if(xr<0)return 0;                       /* the top league has not rolled over yet */
    }
    if(!edition_start_day(d,year,&start) || !(n=ed_new(d,year,start)))return 0;
    e=&s_ce.eds[idx]; /* ed_new may reallocate */
    e->successor=1u;
    if(xa)xr=xch_region_state(xa,e,&xp,&xsx);   /* ed_new may have moved the edition array */
    n->nentrants=e->nentrants;
    for(i=0u;i<e->nentrants;++i)
        n->entrants[i]=(ExpCeEntrant){e->entrants[i].club,
            EXP_CE_SRC_KEPT_NO_MOVEMENT,0u,0u,(uint8_t)(i+1u),0u,
            d->id,e->key.start_day};
    n->state=EXP_CE_ED_PARTICIPANTS;
    if(xs==1) {                                  /* the champion replaces the club ranked last */
        for(i=0u;i<n->nentrants;++i)
            if(n->entrants[i].club.stable_club==xx) {
                n->entrants[i]=(ExpCeEntrant){(ExpandedClubKey){d->world,xw,EXP_PARTICIPANT_CLUB},
                    EXP_CE_SRC_CUP_WINNER,0u,0u,(uint8_t)(i+1u),0u,d->xch_champ,e->key.start_day};
                break;
            }
        ed_log("EDITION_PARTICIPANTS",n,"league exchange: cup winner promoted, last ranked relegated to its region");
        return 1;
    }
    if(d->pool_direct && !e->partial && league_rank_count(e)==e->nentrants && e->nentrants<=64u) {
        /* v311 bottom pool exchange: complete regular order, deterministic roles, the OG
         * play-out winner when a challenger plays; one atomic successor */
        uint32_t members[64],ranked[64],pool[16],np,nxm[64],nxp[16],win=0u,k;
        ExpCePoolSwap work[8];
        int rc;
        for(k=0u;k<e->nentrants;++k){members[k]=e->entrants[k].club.stable_club;ranked[k]=league_rank_at(e,k);}
        np=pool_roles(d,e,pool,16u);
        if(np==UINT32_MAX || np<d->pool_direct+d->pool_playoff)goto pool_fail;
        if(d->pool_playoff) {
            ExpandedClubKey w,l;
            if(ExpCeTie_Resolve(r,e->key,EXP_CE_STAGE(EXP_CE_PLAYOUT_PHASE,EXP_CE_KO_GROUP),1u,
                                &d->playout.match,&w,&l)!=EXP_CE_TIE_DECIDED)goto pool_fail;
            win=w.stable_club;
        }
        rc=ExpCePool_Transition(1,1,members,ranked,e->nentrants,pool,np,d->pool_direct,d->pool_playoff,
                                win,work,8u,nxm,nxp);
        if(rc!=EXP_CE_POOL_READY)goto pool_fail;
        if(xr==1) {                              /* v340: exchange with the top league through the pool */
            uint32_t nn=e->nentrants,xin[4],nxin=0u;
            if(xch_region_adjust(d,xa,e,xp,xsx,nxm,&nn,e->nentrants,ranked,e->nentrants,pool,np,xin,&nxin)<0) {
                ed_block(r,host,n,EXP_CE_BLOCK_DATA_MISSING,"league exchange: no free club available for the region");
                return 1;
            }
        }
        for(k=0u;k<e->nentrants;++k) {
            uint32_t q,from=0u;
            for(q=0u;q<e->nentrants;++q)if(e->entrants[q].club.stable_club==nxm[k])from=1u;
            n->entrants[k]=(ExpCeEntrant){(ExpandedClubKey){d->world,nxm[k],EXP_PARTICIPANT_CLUB},
                from?EXP_CE_SRC_KEPT_NO_MOVEMENT:EXP_CE_SRC_PROMOTED,0u,0u,(uint8_t)(k+1u),0u,d->id,e->key.start_day};
            if(!from)n->entrants[k].source=EXP_CE_SRC_PROMOTED; /* entered from the pool */
            if(!from && xr==1 && ed_has_club(xp,nxm[k]))n->entrants[k].source=EXP_CE_SRC_RELEGATED; /* from the top league */
        }
        ed_log("EDITION_PARTICIPANTS",n,"bottom pool exchange (direct drops, play-out, roles by CE draw)");
        (void)r;(void)host;
        return 1;
pool_fail:
        if(xr==1 && xch_region_affected(d,xa,e,xp,xsx)) {   /* v340: never lose the club that moved */
            ed_block(r,host,n,EXP_CE_BLOCK_DATA_MISSING,"league exchange: the region pool cannot be applied");
            return 1;
        }
        ed_log("EDITION_PARTICIPANTS",n,"bottom pool exchange not possible yet: same clubs");
    } else {
    if(xr==1 && xch_region_affected(d,xa,e,xp,xsx)) {      /* v340: a region without a pool cannot absorb it */
        ed_block(r,host,n,EXP_CE_BLOCK_DATA_MISSING,"league exchange: the region has no pool");
        return 1;
    }
    ed_log("EDITION_PARTICIPANTS",n,"independent calendar: same clubs, no movement");
    }
    (void)r;(void)host;
    return 1;
}
/* League family rollover: all divisions of (country, comp_id) completed.
 * 1 created successors, 0 wait, -1 blocked. */
/* Prepare every successor in private memory before publishing any of them.
 * One malformed ranking or allocation failure must not split a family.
 * The existing prefix/partial policies are preserved, not expanded here. */
/* v309: the clubs a division promotes at rollover (stable ids, best first).
 * Normal: the first `promoted` regular ranks. promotion_playoff: the first
 * promoted-1 regular ranks plus the WINNER of the finals (a club outside them
 * by construction). UINT32_MAX = the finals result is missing/inconsistent. */
static uint32_t league_up_set(const ExpandedUniversalRegistry *r,const ExpCeDef *dd,
                             const ExpCeEdition *o,uint32_t *out)
{
    uint32_t n=0u,j;
    if(!dd->promoted)return 0u;
    if(dd->promoted>15u)return UINT32_MAX;
    if(!dd->promotion_playoff || ExpCe_FinalsSkipped(r,dd,o)) {
        if(league_rank_count(o)<dd->promoted)return UINT32_MAX;
        for(j=0;j<dd->promoted;++j)out[n++]=league_rank_at(o,j);
        return n;
    }
    if(o->nfinal<1u || league_rank_count(o)<dd->promoted)return UINT32_MAX;
    for(j=0;j+1u<dd->promoted;++j)out[n++]=league_rank_at(o,j);
    for(j=0;j<n;++j)if(out[j]==o->final_order[0])return UINT32_MAX;
    out[n++]=o->final_order[0];
    return n;
}
static int up_has(const uint32_t *a,uint32_t n,uint32_t v)
{uint32_t i;for(i=0;i<n;++i)if(a[i]==v)return 1;return 0;}
/* v310: the clubs a division relegates (`want` movements). Normal: the bottom
 * `want` regular ranks. With an active play-out: the bottom want-1 ranks plus the
 * LOSER of the play-out tie (UINT32_MAX when it is not decided/inconsistent). */
static uint32_t league_down_set(const ExpandedUniversalRegistry *r,const ExpCeDef *dd,
                                const ExpCeEdition *o,uint32_t want,uint32_t *out)
{
    const uint32_t cnt=league_rank_count(o);
    uint32_t n=0u,j,lo=0u;
    if(!want)return 0u;
    if(want>15u || cnt<want)return UINT32_MAX;
    if(ExpCe_PlayoutActive(dd) && want==dd->relegated) {
        if(!ExpCe_PlayoutLoser(r,dd,o,&lo))return UINT32_MAX;
        for(j=cnt-(want-1u);j<cnt;++j)out[n++]=league_rank_at(o,j);
        if(up_has(out,n,lo))return UINT32_MAX;
        out[n++]=lo;
        return n;
    }
    for(j=cnt-want;j<cnt;++j)out[n++]=league_rank_at(o,j);
    return n;
}
/* v311: promoted clubs of a parallel level and where the relegated ones go.
 * Fills P (promoted, best first) with their group index, and R (relegated, in order
 * of the upper division regular table). 1 ok, 0 not ready/inconsistent. */
static int parallel_level_plan(const ExpandedUniversalRegistry *r,const ExpCeDef *up,
                               const ExpCeEdition *upc,const ExpCeDef *const *div,
                               const ExpCeEdition *const *cur,
                               uint32_t *P,uint32_t *Pg,uint32_t *nP,uint32_t *R,uint32_t *nR)
{
    uint32_t gi,e,used[8]={0},n=0u,j;
    *nP=0u;*nR=0u;
    for(gi=0u;gi<up->par_n;++gi) {
        uint32_t ups[16],nup=league_up_set(r,div[up->par_groups[gi]],cur[up->par_groups[gi]],ups);
        if(nup==UINT32_MAX || nup!=up->par_per_group)return 0;
        for(j=0u;j<nup;++j){P[n]=ups[j];Pg[n]=gi;++n;}
        used[gi]=nup;
    }
    for(e=0u;e<up->par_extra;++e) {
        int best=-1;uint32_t bp=0,bf=0,bpl=1u,bclub=0u;
        int32_t bg=0;
        for(gi=0u;gi<up->par_n;++gi) {
            const uint32_t g=up->par_groups[gi];
            ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
            uint32_t nr=0u,x,club;
            if(used[gi]>=league_rank_count(cur[g]))continue;
            club=league_rank_at(cur[g],used[gi]);
            if(!ExpCeTable_Compute(r,cur[g]->key,EXP_CE_STAGE(0u,1u),&div[g]->phases[0].table,
                                   NULL,NULL,rows,EXP_CE_MAX_ENTRANTS,&nr))return 0;
            for(x=0u;x<nr && rows[x].club.stable_club!=club;++x){}
            if(x==nr || !rows[x].played)return 0;
            {   /* points per game: pts/played compared by cross multiplication */
                const uint64_t a=(uint64_t)rows[x].points*bpl,b=(uint64_t)bp*rows[x].played;
                const int32_t gd=(int32_t)rows[x].goals_for-(int32_t)rows[x].goals_against;
                int better=best<0;
                if(!better) {
                    if(a!=b)better=a>b;
                    else if(gd!=bg)better=gd>bg;
                    else if(rows[x].goals_for!=bf)better=rows[x].goals_for>bf;
                }
                if(better){best=(int)gi;bp=rows[x].points;bpl=rows[x].played;
                           bg=gd;bf=rows[x].goals_for;bclub=club;}
            }
        }
        if(best<0)return 0;
        P[n]=bclub;Pg[n]=(uint32_t)best;++n;++used[best];
    }
    *nP=n;
    {   uint32_t nrel=league_down_set(r,up,upc,up->relegated,R);
        if(nrel==UINT32_MAX || nrel!=n)return 0;
        *nR=nrel;
    }
    return n>0u && n<=15u;
}
static int league_family_rollover(ExpandedUniversalRegistry *r,
                                  const ExpCeHost *host,const ExpCeDef *d0)
{
    const ExpCeDef *div[16];
    const ExpCeEdition *cur[16];
    uint32_t old[16],nd=d0->divisions,i,j,k,seg=0u;
    ExpCeEdition *next;
    int chain=1,invalid=0,plActive=0;
    uint32_t plP[16],plPg[16],nplP=0u,plR[16],nplR=0u;
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
    {
        const ExpCeDef *upd=(chain==2 && seg>=1u)?div[seg-1u]:NULL;
        if(upd && upd->par_n) {
            uint32_t gi;
            plActive=1;
            for(gi=0u;gi<upd->par_n;++gi)
                if(upd->par_groups[gi]<seg || upd->par_groups[gi]>=nd)plActive=0;
            if(plActive && !parallel_level_plan(r,upd,cur[seg-1u],div,cur,plP,plPg,&nplP,plR,&nplR))
                return -1;
        }
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
        down=(chain==2 && i+1u==seg && !plActive)?0u:div[i]->relegated;
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
                /* v311: a parallel group exchanges clubs with the division above */
                uint32_t gi=UINT32_MAX;
                if(plActive) {
                    const ExpCeDef *upd=div[seg-1u];
                    uint32_t q;
                    for(q=0u;q<upd->par_n;++q)if(upd->par_groups[q]==i)gi=q;
                }
                for(j=0;j<cur[i]->nentrants;++j) {
                    uint32_t q,gone=0u;
                    if(gi!=UINT32_MAX)
                        for(q=0u;q<nplP;++q)
                            if(plPg[q]==gi && plP[q]==cur[i]->entrants[j].club.stable_club)gone=1u;
                    if(gone)continue;
                    n->entrants[k]=(ExpCeEntrant){cur[i]->entrants[j].club,
                        EXP_CE_SRC_KEPT_NO_MOVEMENT,0u,0u,(uint8_t)(k+1u),0u,
                        div[i]->id,cur[i]->key.start_day};
                    ++k;
                }
                if(gi!=UINT32_MAX) {
                    uint32_t q;
                    for(q=0u;q<nplR;++q) {
                        if(plPg[q]!=gi)continue;
                        if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                        n->entrants[k]=(ExpCeEntrant){
                            (ExpandedClubKey){d0->world,plR[q],EXP_PARTICIPANT_CLUB},
                            EXP_CE_SRC_RELEGATED,(uint8_t)(q+1u),0u,(uint8_t)(k+1u),0u,
                            div[seg-1u]->id,cur[seg-1u]->key.start_day};
                        ++k;
                    }
                }
            } else {
                const uint32_t down=(chain==2 && i+1u==seg && !plActive)?0u:div[i]->relegated;
                uint32_t ups[16],nup=league_up_set(r,div[i],cur[i],ups);
                uint32_t dn[16],ndn=league_down_set(r,div[i],cur[i],down,dn);
                if(nup==UINT32_MAX || ndn==UINT32_MAX)goto fail;
                for(j=0u;j<league_rank_count(cur[i]);++j) {
                    if(up_has(ups,nup,league_rank_at(cur[i],j)) ||
                       up_has(dn,ndn,league_rank_at(cur[i],j)))continue;
                    if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                    n->entrants[k]=(ExpCeEntrant){
                        (ExpandedClubKey){d0->world,league_rank_at(cur[i],j),EXP_PARTICIPANT_CLUB},
                        EXP_CE_SRC_RETAINED,(uint8_t)(j+1u),0u,(uint8_t)(k+1u),0u,
                        div[i]->id,cur[i]->key.start_day};
                    ++k;
                }
                if(i>0u) {
                    uint32_t dn2[16],ndn2=league_down_set(r,div[i-1u],cur[i-1u],
                                                          div[i-1u]->relegated,dn2);
                    if(ndn2==UINT32_MAX)goto fail;
                    for(j=0u;j<ndn2;++j) {
                        if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                        n->entrants[k]=(ExpCeEntrant){
                            (ExpandedClubKey){d0->world,dn2[j],EXP_PARTICIPANT_CLUB},
                            EXP_CE_SRC_RELEGATED,(uint8_t)(j+1u),0u,(uint8_t)(k+1u),0u,
                            div[i-1u]->id,cur[i-1u]->key.start_day};
                        ++k;
                    }
                }
                if(plActive && i+1u==seg) {
                    uint32_t q;
                    for(q=0u;q<nplP;++q) {
                        const uint32_t gdiv=div[seg-1u]->par_groups[plPg[q]];
                        if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                        n->entrants[k]=(ExpCeEntrant){
                            (ExpandedClubKey){d0->world,plP[q],EXP_PARTICIPANT_CLUB},
                            EXP_CE_SRC_PROMOTED,(uint8_t)(q+1u),0u,(uint8_t)(k+1u),0u,
                            div[gdiv]->id,cur[gdiv]->key.start_day};
                        ++k;
                    }
                } else if(i+1u<(chain==2?seg:nd)) {
                    uint32_t ups2[16],nup2=league_up_set(r,div[i+1u],cur[i+1u],ups2);
                    if(nup2==UINT32_MAX)goto fail;
                    for(j=0;j<nup2;++j) {
                        if(k>=EXP_CE_MAX_ENTRANTS)goto fail;
                        n->entrants[k]=(ExpCeEntrant){
                            (ExpandedClubKey){d0->world,ups2[j],EXP_PARTICIPANT_CLUB},
                            EXP_CE_SRC_PROMOTED,(uint8_t)(j+1u),0u,(uint8_t)(k+1u),0u,
                            div[i+1u]->id,cur[i+1u]->key.start_day};
                        ++k;
                    }
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
                else if(res>0){
                    /* v00.13.355: the edition kind follows its entrants as soon as
                     * they are confirmed (a successor blocked before phase 0 must
                     * persist in UL01 with entrants of the same kind) */
                    uint32_t q;int same=e->nentrants>0u;
                    for(q=1u;q<e->nentrants;++q)
                        if(e->entrants[q].club.kind!=e->entrants[0].club.kind)same=0;
                    if(same)e->participant_kind=e->entrants[0].club.kind;
                    e->state=EXP_CE_ED_PARTICIPANTS;
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
                int pc=phase_complete(r,d,e,e->phase);
                {   /* v00.13.355: a parallel block is complete when all its phases are */
                    uint32_t q=e->phase;
                    while(pc==1 && q>0u && d->phases[q].parallel){--q;pc=phase_complete(r,d,e,q);}
                }
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
                /* v310: league play-out, generated beside the linear phases once the
                 * regular season is complete; the edition cannot close before it */
                if(d->origin==EXP_CE_ORIGIN_LEAGUE && ExpCe_PlayoutActive(d) &&
                   !e->partial && !phase_generated(r,e->key,EXP_CE_PLAYOUT_PHASE) &&
                   phase_complete(r,d,e,0u)==1 && !ExpCe_PlayoutSkipped(r,d,e)) {
                    uint8_t pblock=0u;
                    const int gi=generate_playout(r,host,d,e,today,&pblock,why,sizeof why);
                    if(gi<0){ed_block(r,host,&s_ce.eds[ed_index(e)],pblock,why);++moved;break;}
                    if(gi>0)++moved;
                    e=&s_ce.eds[ed_index(e)];
                }
                if(pc<0) {
                    snprintf(why,sizeof why,"phase %u: tie level although a "
                             "winner is required",(unsigned)e->phase);
                    ed_block(r,host,e,EXP_CE_BLOCK_TIE_UNRESOLVED,why);++moved;
                    break;
                }
                if(!pc)break;
                if((uint32_t)e->phase+1u<d->nphases &&
                   !(e->phase==0u && d->origin==EXP_CE_ORIGIN_LEAGUE && ExpCe_FinalsSkipped(r,d,e))) {
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
                        /* v00.13.355: phases flagged parallel start together */
                        while((uint32_t)e->phase+1u<d->nphases && d->phases[e->phase+1u].parallel) {
                            if(!generate_phase(r,host,d,e,(uint32_t)e->phase+1u,today,&block,why,sizeof why)) {
                                e=&s_ce.eds[idx];ed_block(r,host,e,block,why);break;
                            }
                            e=&s_ce.eds[idx];++e->phase;
                            ed_log("PHASE_ADVANCED",e,"parallel");
                        }
                    }
                    ++moved;break;
                }
                if(d->origin==EXP_CE_ORIGIN_LEAGUE && ExpCe_PlayoutActive(d) && !e->partial) {
                    uint32_t lo=0u;
                    if((!phase_generated(r,e->key,EXP_CE_PLAYOUT_PHASE) &&
                        !ExpCe_PlayoutSkipped(r,d,e)) ||
                       !ExpCe_PlayoutLoser(r,d,e,&lo))break; /* play-out still to be played */
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
                        const uint16_t ny=(uint16_t)(e->year+(d->cycle_years>1u?d->cycle_years:1u));
                        if(d->last_year && ny>d->last_year) {   /* v00.13.355 continues as another definition */
                            s_ce.eds[idx].successor=1u;
                            ed_log("LAST_EDITION",e,"no successor of this definition");
                        } else if(edition_start_day(d,ny,&start) &&
                           (n=ed_new(d,ny,start))!=NULL) {
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
                    /* v309: a league whose finals only decide a promotion keeps the
                     * regular-season leader as champion */
                    ar=d->promotion_playoff && e->nregular>=2u?
                       host->archive(host->user,e->key,e->regular_order[0],
                                     e->regular_order[1],today):
                       host->archive(host->user,e->key,e->final_order[0],
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
                if(n->def_id==e->def_id && n->year>e->year &&
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
            if(!e->nentrants && !d->first_from_sources)
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
    if(phase==EXP_CE_PLAYOUT_PHASE) { /* v310 league play-out */
        if(!ExpCe_PlayoutActive(d) || f->competition_type!=d->type)return 0;
        if(match)*match=d->playout.match;
        if(table)memset(table,0,sizeof *table);
        return 1;
    }
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
    s_busy_n=0u;
    return plan_phase_days_ex(d,&e,&d->phases[phase],earliest,rounds,days);
}
int ExpCe_FixturePhase(const ExpandedUniversalFixture *f,uint8_t *kind,
                       uint8_t *meetings)
{
    const ExpCeDef *d;
    uint32_t phase;
    if(!f || !(d=def_find(f->key.edition.competition.stable_competition)))return 0;
    phase=EXP_CE_STAGE_PHASE(f->key.stage);
    if(phase==EXP_CE_PLAYOUT_PHASE && ExpCe_PlayoutActive(d)) {
        if(kind)*kind=EXP_CE_PHASE_KNOCKOUT;
        if(meetings)*meetings=d->playout.match.legs;
        return 1;
    }
    if(phase>=d->nphases)return 0;
    if(kind)*kind=d->phases[phase].kind;
    if(meetings)*meetings=d->phases[phase].meetings;
    return 1;
}
int ExpCe_FixtureFifa(const ExpandedUniversalFixture *f,int *importance,int *protect)
{
    const ExpCeDef *d;
    const ExpCeEdition *e;
    const ExpCePhaseDef *ph;
    uint32_t phase;
    if(!f || f->key.edition.competition.world!=WORLD_2627 ||
       f->home.kind!=EXP_PARTICIPANT_NATIONAL || f->away.kind!=EXP_PARTICIPANT_NATIONAL ||
       !(d=def_find(f->key.edition.competition.stable_competition)))return 0;
    phase=EXP_CE_STAGE_PHASE(f->key.stage);
    if(phase>=d->nphases)return 0;
    e=ExpCe_Edition(f->key.edition);
    if(e && e->participant_kind!=EXP_PARTICIPANT_NATIONAL)return 0;
    ph=&d->phases[phase];
    if(!ph->fifa_importance)return 0;
    if(importance)*importance=ph->fifa_importance;
    if(protect)*protect=ph->fifa_protect==1u ||
        (ph->fifa_protect==0u && ph->kind==EXP_CE_PHASE_KNOCKOUT && ph->fifa_importance>=35u);
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
    {   /* v00.13.354: default FIFA windows (user rule 27/11/2026: every club
         * competition stops when the national teams play); "" = none */
        char it[48];
        const int il=mjson_get_string(j,l,"$.international_breaks",it,(int)sizeof it);
        w->intl=EXP_INTL_INHERIT;   /* absent: league finals inherit the league row, others none */
        if(il==0)w->intl=0u;
        else if(il>0 && !ExpIntl_ParseMask(it,&w->intl)) {
            snprintf(why,wl,"window.international_breaks");return 0;
        }
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
    {"RESEED_TABLE",EXP_CE_PAIR_RESEED_TABLE},{"GROUP_TABLE",EXP_CE_PAIR_GROUP_TABLE},{"SOURCES",EXP_CE_PAIR_SOURCES},
    {"WINNERS_RUNNERS_DRAW",EXP_CE_PAIR_WINNERS_RUNNERS_DRAW},{"POT_PAIRS",EXP_CE_PAIR_POT_PAIRS},
    {"BANDED_POTS",EXP_CE_PAIR_BANDED_POTS}};
static int parse_calendar_rules(const char *o,int ol,ExpCePhaseDef *ph,char *why,size_t wl)
{
    if(ph->kind!=EXP_CE_PHASE_TABLE){snprintf(why,wl,"calendar: TABLE phase only");return 0;}
    return ExpCal_Parse(o,ol,ph->has_window?ph->window.weekdays:0u,&ph->cal,why,wl);
}
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
    NUM("$.qualify_groups",qualify_groups,0,EXP_CE_MAX_GROUPS);
    NUM("$.source_phase",source_phase_p1,0,EXP_CE_MAX_PHASES);   /* 1-based phase */
    {   int pb=0;
        if(mjson_get_bool(j,l,"$.parallel",&pb)>0)ph->parallel=(uint8_t)(pb!=0);
    }
    {   /* v00.13.355 bands and round template */
        const char *bo=NULL,*ta=NULL;int bl=0,tl=0;
        if(mjson_find(j,l,"$.bands",&bo,&bl)==MJSON_TOK_OBJECT) {
            double x=0;
            if(mjson_get_number(bo,bl,"$.size",&x)>0 && x>=2 && x<=64)ph->band_size=(uint8_t)x;
            if(mjson_get_number(bo,bl,"$.groups",&x)>0 && x>=1 && x<=EXP_CE_MAX_GROUPS)ph->band_groups=(uint8_t)x;
            if(mjson_get_number(bo,bl,"$.per_group",&x)>0 && x>=1 && x<=8)ph->band_per_group=(uint8_t)x;
            if(!ph->band_size || !ph->band_groups) {
                snprintf(ph->missing,sizeof ph->missing,"bands size/groups");return 0;
            }
        }
        if(mjson_find(j,l,"$.round_template",&ta,&tl)==MJSON_TOK_ARRAY) {
            int ro=0,rk,rkl,rv,rvl,rt;
            while((ro=mjson_next(ta,tl,ro,&rk,&rkl,&rv,&rvl,&rt))>0) {
                int mo=0,mk,mkl,mv,mvl,mt;uint8_t nm=0,used[64]={0};
                if(rt!=MJSON_TOK_ARRAY || ph->ntpl_rounds>=16u){snprintf(ph->missing,sizeof ph->missing,"round_template rounds");return 0;}
                while((mo=mjson_next(ta+rv,rvl,mo,&mk,&mkl,&mv,&mvl,&mt))>0) {
                    double h=0,a=0;
                    if(mt!=MJSON_TOK_ARRAY || nm>=8u ||
                       mjson_get_number(ta+rv+mv,mvl,"$[0]",&h)<=0 || mjson_get_number(ta+rv+mv,mvl,"$[1]",&a)<=0 ||
                       h<0 || a<0 || h>63 || a>63 || h==a || used[(int)h] || used[(int)a]) {
                        snprintf(ph->missing,sizeof ph->missing,"round_template match");return 0;
                    }
                    used[(int)h]=used[(int)a]=1u;
                    ph->tpl[ph->ntpl_rounds][nm][0]=(uint8_t)h;ph->tpl[ph->ntpl_rounds][nm][1]=(uint8_t)a;++nm;
                }
                if(!nm || (ph->ntpl_rounds && nm!=ph->ntpl_matches)){snprintf(ph->missing,sizeof ph->missing,"round_template size");return 0;}
                ph->ntpl_matches=nm;++ph->ntpl_rounds;
            }
        }
    }
    {   /* v00.13.355 display names (viewer only) */
        const char *ga=NULL;int gl=0;
        (void)json_str(j,l,"$.name",ph->name,sizeof ph->name);
        if(mjson_find(j,l,"$.group_labels",&ga,&gl)==MJSON_TOK_ARRAY) {
            int go=0,gk,gkl,gv,gvl,gt;unsigned gi=0u;
            while((go=mjson_next(ga,gl,go,&gk,&gkl,&gv,&gvl,&gt))>0 && gi<EXP_CE_MAX_GROUPS) {
                if(gt==MJSON_TOK_STRING && gvl>=3 && gvl-2<(int)sizeof ph->group_label[0]) {
                    memcpy(ph->group_label[gi],ga+gv+1,(size_t)gvl-2u);
                    ph->group_label[gi][gvl-2]=0;
                }
                ++gi;
            }
        }
    }
    {   /* "interim_pots":{"hi":[11,12,13,14],"lo":[21,22,23,24]} */
        const char *ip=NULL;int il=0;
        if(mjson_find(j,l,"$.interim_pots",&ip,&il)==MJSON_TOK_OBJECT) {
            uint32_t side;
            for(side=0;side<2u;++side) {
                const char *a=NULL;int al=0,off=0,ko,kl,vo,vl,vt;
                if(mjson_find(ip,il,side?"$.lo":"$.hi",&a,&al)!=MJSON_TOK_ARRAY){snprintf(ph->missing,sizeof ph->missing,"interim_pots");return 0;}
                while((off=mjson_next(a,al,off,&ko,&kl,&vo,&vl,&vt))>0) {
                    char nb[8];double dv;
                    if(vt!=MJSON_TOK_NUMBER || vl>=(int)sizeof nb || (side?ph->nlo:ph->nhi)>=8u){snprintf(ph->missing,sizeof ph->missing,"interim_pots rank");return 0;}
                    memcpy(nb,a+vo,(size_t)vl);nb[vl]=0;dv=atof(nb);
                    if(dv<1||dv>128){snprintf(ph->missing,sizeof ph->missing,"interim_pots rank");return 0;}
                    if(side)ph->lo[ph->nlo++]=(uint8_t)dv;else ph->hi[ph->nhi++]=(uint8_t)dv;
                }
            }
            if(ph->nhi!=ph->nlo || !ph->nhi){snprintf(ph->missing,sizeof ph->missing,"interim_pots sizes");return 0;}
        }
    }
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
    {   /* v311: a group phase whose drawn matches are decided on penalties plays them like a
         * single-leg tie that needs a winner (the table keeps the regulation draw) */
        const char *tp=NULL;int tl=0;
        if(ph->kind==EXP_CE_PHASE_TABLE &&
           ((tbase && tbase->draw_penalties) ||
            mjson_find(j,l,"$.table.draw_penalties",&tp,&tl)==MJSON_TOK_ARRAY))
            ph->match.needs_winner=1u;
    }
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
    if(mjson_find(j,l,"$.fifa",&o,&ol)==MJSON_TOK_OBJECT) {
        double fi=0;
        int fb=0;
        char fs[32];
        if(mjson_get_number(o,ol,"$.importance",&fi)>0) {
            if(!ExpFifa_ImportanceValid((int)fi) || fi!=(double)(int)fi) {
                snprintf(ph->missing,sizeof ph->missing,"fifa.importance");return 0;
            }
            ph->fifa_importance=(uint8_t)fi;
        }
        if(mjson_get_bool(o,ol,"$.protect",&fb)>0)ph->fifa_protect=fb?1u:2u;
        if(json_str(o,ol,"$.seeding",fs,sizeof fs)) {
            if(strcmp(fs,"FIFA_RANKING")){snprintf(ph->missing,sizeof ph->missing,"fifa.seeding");return 0;}
            ph->fifa_seeding=1u;
        }
    }
    if(mjson_find(j,l,"$.window",&o,&ol)==MJSON_TOK_OBJECT) {
        if(!parse_window(o,ol,&ph->window,ph->missing,sizeof ph->missing))return 0;
        ph->has_window=1u;
    }
    if(mjson_find(j,l,"$.calendar",&o,&ol)==MJSON_TOK_OBJECT &&
       !parse_calendar_rules(o,ol,ph,ph->missing,sizeof ph->missing))return 0;
    if(mjson_find(j,l,"$.ties",&o,&ol)==MJSON_TOK_ARRAY) {
        /* "ties": [{"home":{"rank":1},"away":{"rank":2}}, {"home":{"loser":[1,1]},"away":{"winner":[1,2]}}] */
        int to=0,tk,tkl,tv,tvl,tt;
        while((to=mjson_next(o,ol,to,&tk,&tkl,&tv,&tvl,&tt))>0) {
            unsigned side;
            if(tt!=MJSON_TOK_OBJECT || ph->nties>=8u){snprintf(ph->missing,sizeof ph->missing,"ties");return 0;}
            for(side=0u;side<2u;++side) {
                const char *sp=NULL;int sl=0;
                double a=0,b=0;
                uint8_t *x=ph->tsrc[ph->nties][side];
                if(mjson_find(o+tv,tvl,side?"$.away":"$.home",&sp,&sl)!=MJSON_TOK_OBJECT){
                    snprintf(ph->missing,sizeof ph->missing,"ties side");return 0;}
                if(mjson_get_number(sp,sl,"$.rank",&a)>0 && a>=1 && a<=64){x[0]=1u;x[1]=(uint8_t)a;}
                else if(mjson_get_number(sp,sl,"$.winner[0]",&a)>0 && mjson_get_number(sp,sl,"$.winner[1]",&b)>0 &&
                        a>=1 && a<=7 && b>=1 && b<=8){x[0]=2u;x[1]=(uint8_t)a;x[2]=(uint8_t)b;}
                else if(mjson_get_number(sp,sl,"$.loser[0]",&a)>0 && mjson_get_number(sp,sl,"$.loser[1]",&b)>0 &&
                        a>=1 && a<=7 && b>=1 && b<=8){x[0]=3u;x[1]=(uint8_t)a;x[2]=(uint8_t)b;}
                else {snprintf(ph->missing,sizeof ph->missing,"ties source");return 0;}
            }
            ++ph->nties;
        }
    }
    if(mjson_find(j,l,"$.pairs",&o,&ol)==MJSON_TOK_ARRAY) {
        /* "pairs": [[homeGroup,homeRank,awayGroup,awayRank], ...] (GROUP_TABLE) */
        int po2=0,pk2,pkl2,pv2,pvl2,pt2;
        while((po2=mjson_next(o,ol,po2,&pk2,&pkl2,&pv2,&pvl2,&pt2))>0) {
            double q[4];
            unsigned z;
            if(pt2!=MJSON_TOK_ARRAY || ph->ngp>=8u){snprintf(ph->missing,sizeof ph->missing,"pairs");return 0;}
            for(z=0u;z<4u;++z) {
                char path[8];
                snprintf(path,sizeof path,"$[%u]",z);
                if(mjson_get_number(o+pv2,pvl2,path,&q[z])<=0 || q[z]<1 || q[z]>16){
                    snprintf(ph->missing,sizeof ph->missing,"pairs values");return 0;}
                ph->gp[ph->ngp][z]=(uint8_t)q[z];
            }
            ++ph->ngp;
        }
    }
    if(mjson_find(j,l,"$.after_phase_end",&o,&ol)==MJSON_TOK_OBJECT) {
        /* "after_phase_end": {"phase": 0, "days": [8, 11]} */
        double av=0;const char *da=NULL;int dl=0,doff=0,dko,dkl,dvo,dvl,dvt;
        if(mjson_get_number(o,ol,"$.phase",&av)<=0 || av<0 || av>6 ||
           mjson_find(o,ol,"$.days",&da,&dl)!=MJSON_TOK_ARRAY) {
            snprintf(ph->missing,sizeof ph->missing,"after_phase_end");return 0;
        }
        ph->rel_anchor=(uint8_t)av;
        while((doff=mjson_next(da,dl,doff,&dko,&dkl,&dvo,&dvl,&dvt))>0) {
            double dv=0;char nb[16];
            if(dvt!=MJSON_TOK_NUMBER || dvl>=(int)sizeof nb || ph->nrel>=4u){
                snprintf(ph->missing,sizeof ph->missing,"after_phase_end.days");return 0;}
            memcpy(nb,da+dvo,(size_t)dvl);nb[dvl]=0;dv=atof(nb);
            if(dv<1 || dv>200 || (ph->nrel && (uint16_t)dv<=ph->rel[ph->nrel-1u])){
                snprintf(ph->missing,sizeof ph->missing,"after_phase_end.days 1..200 ascending");return 0;}
            ph->rel[ph->nrel++]=(uint16_t)dv;
        }
        if(!ph->nrel){snprintf(ph->missing,sizeof ph->missing,"after_phase_end.days empty");return 0;}
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
    if(mjson_find(j,l,"$.first_edition_group_days",&o,&ol)==MJSON_TOK_ARRAY) {
        /* one array of round dates per group (group order 1..groups) */
        int goff=0,gk,gkl,gvo,gvl,gvt;
        uint32_t g=0u;
        if(ph->kind!=EXP_CE_PHASE_TABLE || !ph->ndays) {
            snprintf(ph->missing,sizeof ph->missing,"first_edition_group_days needs first_edition_days");return 0;
        }
        while((goff=mjson_next(o,ol,goff,&gk,&gkl,&gvo,&gvl,&gvt))>0) {
            int roff=0,rk,rkl,rvo,rvl,rvt;
            uint32_t rr=0u;
            if(gvt!=MJSON_TOK_ARRAY || g>=EXP_CE_MAX_GROUPS){snprintf(ph->missing,sizeof ph->missing,"first_edition_group_days");return 0;}
            while((roff=mjson_next(o+gvo,gvl,roff,&rk,&rkl,&rvo,&rvl,&rvt))>0) {
                unsigned y,m,dd;
                char t2[16];
                int32_t idx=0,delta;
                if(rvt!=MJSON_TOK_STRING || rvl!=12 || rr>=ph->ndays){snprintf(ph->missing,sizeof ph->missing,"first_edition_group_days round");return 0;}
                memcpy(t2,o+gvo+rvo+1,10u);t2[10]=0;
                if(sscanf(t2,"%4u-%2u-%2u",&y,&m,&dd)!=3 ||
                   !ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)dd},&idx)){
                    snprintf(ph->missing,sizeof ph->missing,"first_edition_group_days date");return 0;}
                delta=idx-ph->days[rr];
                if(delta<-7 || delta>7){snprintf(ph->missing,sizeof ph->missing,"first_edition_group_days offset");return 0;}
                ph->gday_off[g][rr]=(int8_t)delta;
                ++rr;
            }
            if(rr!=ph->ndays){snprintf(ph->missing,sizeof ph->missing,"first_edition_group_days rounds");return 0;}
            ++g;
        }
        if(g!=ph->groups){snprintf(ph->missing,sizeof ph->missing,"first_edition_group_days groups");return 0;}
        ph->has_gdays=1u;
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
        double country=0,comp=0,div=0,qual=0,from=1;
        int pp=0;
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
        if(!d) {
            /* v309: a finals row of a league this world does not define (e.g. a
             * reduced test/mod league set) is not an error of the others */
            fprintf(stderr,"[ce-engine] LEAGUE_FINALS_SKIPPED country=%u comp=%u "
                "division=%u reason=league-not-defined-in-this-world\n",
                (unsigned)country,(unsigned)comp,(unsigned)div);
            continue;
        }
        if(d->world!=world || d->origin!=EXP_CE_ORIGIN_LEAGUE ||
           d->nphases!=1u || d->missing[0] || qual>d->clubs)goto done;
        i=(uint32_t)(d-s_ce.defs);
        if(staged[i])goto done;
        copy=(ExpCeDef *)malloc(sizeof *copy);
        if(!copy)goto done;
        staged[i]=copy;*copy=*d;
        if(mjson_get_number(o,ol,"$.qualify_from",&from)>0 &&
           (from<1 || from>qual-1 || from!=(double)(uint8_t)from))goto done;
        if(mjson_get_bool(o,ol,"$.promotion_playoff",&pp)>0)copy->promotion_playoff=(uint8_t)(pp!=0);
        {   /* v311: "skip_if_gap": {"hi_rank":3,"lo_rank":4,"points_greater_than":14} */
            const char *go=NULL;int gl=0;
            double ghi=0,glo=0,gmin=0;
            if(mjson_find(o,ol,"$.skip_if_gap",&go,&gl)==MJSON_TOK_OBJECT) {
                if(mjson_get_number(go,gl,"$.hi_rank",&ghi)<=0 || mjson_get_number(go,gl,"$.lo_rank",&glo)<=0 ||
                   mjson_get_number(go,gl,"$.points_greater_than",&gmin)<=0 ||
                   ghi<1 || glo<=ghi || glo>64 || gmin<1 || gmin>99)goto done;
                copy->fin_gap_hi=(uint8_t)ghi;copy->fin_gap_lo=(uint8_t)glo;copy->fin_gap_min=(uint8_t)gmin;
            }
        }
        copy->phases[0].qualify=(uint8_t)qual;
        copy->phases[0].qualify_from=(uint8_t)from;
        while((po=mjson_next(phases,pl,po,&pk,&pkl,&pv,&pvl,&pt))>0) {
            ExpCePhaseDef *ph;
            if(pt!=MJSON_TOK_OBJECT || copy->nphases>=EXP_CE_MAX_PHASES)goto done;
            ph=&copy->phases[copy->nphases++];
            if(!parse_phase(phases+pv,pvl,&copy->phases[0].table,
                &copy->phases[0].match,ph) || ph->kind!=EXP_CE_PHASE_KNOCKOUT)goto done;
        }
        if(po<0 || copy->nphases<2u || !def_check(copy))goto done;
        /* Check qualifier count and byes now, not halfway through the season. */
        {uint32_t p,n=(uint32_t)(qual-from+1.0);
         for(p=1u;p<copy->nphases;++p){
             const ExpCePhaseDef *ph=&copy->phases[p];
             if(ph->pairing==EXP_CE_PAIR_SOURCES) {   /* v311: bracket graph, validate references */
                 uint32_t tt,ss;
                 if(!ph->nties)goto done;
                 for(tt=0u;tt<ph->nties;++tt)
                     for(ss=0u;ss<2u;++ss) {
                         const uint8_t *x=ph->tsrc[tt][ss];
                         if(x[0]==1u){ if(x[1]<(uint8_t)(from) || x[1]>(uint8_t)qual)goto done; }
                         else if(x[0]==2u || x[0]==3u) {
                             if(x[1]<1u || x[1]>=p || copy->phases[x[1]].kind!=EXP_CE_PHASE_KNOCKOUT ||
                                x[2]<1u || x[2]>(copy->phases[x[1]].pairing==EXP_CE_PAIR_SOURCES?
                                                 copy->phases[x[1]].nties:255u))goto done;
                         } else goto done;
                     }
                 n=ph->nties; /* ties of the last SOURCES phase decide the closing arithmetic */
                 continue;
             }
             if((ph->pairing!=EXP_CE_PAIR_RESEED_TABLE &&
                 (p<2u || (ph->pairing!=EXP_CE_PAIR_SEEDED && ph->pairing!=EXP_CE_PAIR_BRACKET))) ||
                n<ph->byes+2u ||
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
/* v311: the shared pool clubs of one league division (stable keys); replaces the list. The
 * definition must be an independent regional pyramid. 1 ok. */
int ExpCe_SetPoolClubs(uint8_t country,uint8_t comp_id,uint8_t division,const uint32_t *keys,uint32_t n)
{
    ExpCeDef *d=(ExpCeDef *)def_find(ExpCe_LeagueCompetitionId(country,comp_id,division));
    uint32_t i,j;
    if(!d || d->origin!=EXP_CE_ORIGIN_LEAGUE || n>8u || (n && !keys))return 0;
    for(i=0u;i<n;++i) {
        if(!keys[i])return 0;
        for(j=0u;j<i;++j)if(keys[j]==keys[i])return 0;
    }
    memset(d->pool_clubs,0,sizeof d->pool_clubs);
    for(i=0u;i<n;++i)d->pool_clubs[i]=keys[i];
    d->pool_n=(uint8_t)n;
    d->rules_hash=def_hash(d);
    return 1;
}
/* "pool": {"direct":N,"playoff":0|1} on a league_calendar.json row; the pool clubs come from
 * ExpCe_SetPoolClubs. A play-out needs the row's "playout" (rules/days). */
/* v340: "exchange": {"champion_competition": id, "relegate_rank": N, "routing":
 * [{"division": d, "clubs": ["44:5", ...]}, ...]} on a league_calendar.json row. */
int ExpCe_LoadLeagueExchanges(const char *json,int len,uint16_t world)
{
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt,applied=0;
    double schema=0;
    if(!json || mjson_get_number(json,len,"$.schema",&schema)<=0 || schema!=1 ||
       mjson_find(json,len,"$.leagues",&arr,&alen)!=MJSON_TOK_ARRAY)return -1;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *row=arr+vo,*o=NULL,*rt=NULL;
        int ol=0,rl=0,roff=0,rko,rkl,rvo,rvl,rvt;
        double country=0,comp=0,div=0,champ=0,rank=0;
        ExpCeDef *d;
        uint32_t n=0u;
        if(vt!=MJSON_TOK_OBJECT || mjson_find(row,vl,"$.exchange",&o,&ol)!=MJSON_TOK_OBJECT)continue;
        if(mjson_get_number(row,vl,"$.team_number",&country)<=0 ||
           mjson_get_number(row,vl,"$.comp_id",&comp)<=0 ||
           mjson_get_number(row,vl,"$.division",&div)<=0 ||
           mjson_get_number(o,ol,"$.champion_competition",&champ)<=0 || champ<1 ||
           mjson_get_number(o,ol,"$.relegate_rank",&rank)<=0 || rank<2 || rank>15 ||
           mjson_find(o,ol,"$.routing",&rt,&rl)!=MJSON_TOK_ARRAY)return -1;
        d=(ExpCeDef *)def_find(ExpCe_LeagueCompetitionId((uint8_t)country,(uint8_t)comp,(uint8_t)div));
        if(!d)continue;
        if(d->world!=world || d->origin!=EXP_CE_ORIGIN_LEAGUE || d->missing[0] || d->nphases<1u)return -1;
        while((roff=mjson_next(rt,rl,roff,&rko,&rkl,&rvo,&rvl,&rvt))>0) {
            const char *g=rt+rvo,*cl=NULL;
            int gl=rvl,cll=0,coff=0,cko,ckl,cvo,cvl,cvt;
            double dv=0;
            if(rvt!=MJSON_TOK_OBJECT || mjson_get_number(g,gl,"$.division",&dv)<=0 || dv<0 || dv>15 ||
               mjson_find(g,gl,"$.clubs",&cl,&cll)!=MJSON_TOK_ARRAY)return -1;
            while((coff=mjson_next(cl,cll,coff,&cko,&ckl,&cvo,&cvl,&cvt))>0) {
                char t[24];
                ExpandedClubKey k;
                if(cvt!=MJSON_TOK_STRING || cvl<3 || cvl>=(int)sizeof t || n>=16u)return -1;
                memcpy(t,cl+cvo+1,(size_t)cvl-2u);t[cvl-2]=0;
                if(!parse_club(t,world,&k))return -1;
                d->xch_route_club[n]=k.stable_club;d->xch_route_div[n]=(uint8_t)dv;++n;
            }
            if(coff<0)return -1;
        }
        if(roff<0 || !n)return -1;
        d->xch_on=1u;d->xch_rank=(uint8_t)rank;d->xch_nroute=(uint8_t)n;d->xch_champ=(uint32_t)champ;
        d->rules_hash=def_hash(d);
        ++applied;
    }
    if(off<0)return -1;
    return applied;
}
/* v331: "match_override": {bench, subs, windows, halftime_free, et_extra_subs, et_extra_windows,
 * provenance} on a league_calendar.json row: the REAL substitution regulation of that league, applied
 * on top of every phase's OG-native match rules (bench/subs from leagues.json). Only the substitution
 * fields are read; a malformed or invalid override fails the whole load (no silent default). */
int ExpCe_LoadLeagueMatchOverrides(const char *json,int len,uint16_t world)
{
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt,applied=0;
    double schema=0;
    if(!json || mjson_get_number(json,len,"$.schema",&schema)<=0 || schema!=1 ||
       mjson_find(json,len,"$.leagues",&arr,&alen)!=MJSON_TOK_ARRAY)return -1;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *row=arr+vo,*o=NULL;
        int ol=0;
        double country=0,comp=0,div=0;
        ExpCeDef *d;
        unsigned i;
        if(vt!=MJSON_TOK_OBJECT || mjson_find(row,vl,"$.match_override",&o,&ol)!=MJSON_TOK_OBJECT)continue;
        if(mjson_get_number(row,vl,"$.team_number",&country)<=0 ||
           mjson_get_number(row,vl,"$.comp_id",&comp)<=0 ||
           mjson_get_number(row,vl,"$.division",&div)<=0)return -1;
        d=(ExpCeDef *)def_find(ExpCe_LeagueCompetitionId((uint8_t)country,(uint8_t)comp,(uint8_t)div));
        if(!d)continue;
        if(d->world!=world || d->origin!=EXP_CE_ORIGIN_LEAGUE || d->missing[0] || d->nphases<1u)return -1;
        for(i=0u;i<d->nphases;++i) {
            ExpCeMatchRules m;
            ExpCeRuleError err;
            if(!ExpCeRules_ParseMatch(o,ol,&d->phases[i].match,&m,&err))return -1;
            d->phases[i].match=m;
        }
        d->rules_hash=def_hash(d);
        ++applied;
    }
    if(off<0)return -1;
    return applied;
}
int ExpCe_LoadLeaguePools(const char *json,int len,uint16_t world)
{
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt,added=0;
    double schema=0;
    if(!json || mjson_get_number(json,len,"$.schema",&schema)<=0 || schema!=1 ||
       mjson_find(json,len,"$.leagues",&arr,&alen)!=MJSON_TOK_ARRAY)return -1;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        const char *row=arr+vo,*o=NULL;
        int ol=0;
        double country=0,comp=0,div=0,dir=0,po=0;
        ExpCeDef *d;
        if(mjson_find(row,vl,"$.pool",&o,&ol)==MJSON_TOK_INVALID)continue;
        if(vt!=MJSON_TOK_OBJECT || mjson_find(row,vl,"$.pool",&o,&ol)!=MJSON_TOK_OBJECT ||
           mjson_get_number(row,vl,"$.team_number",&country)<=0 ||
           mjson_get_number(row,vl,"$.comp_id",&comp)<=0 ||
           mjson_get_number(row,vl,"$.division",&div)<=0 ||
           mjson_get_number(o,ol,"$.direct",&dir)<=0 || dir<1 || dir>3)return -1;
        (void)mjson_get_number(o,ol,"$.playoff",&po);
        if(po!=0 && po!=1)return -1;
        d=(ExpCeDef *)def_find(ExpCe_LeagueCompetitionId((uint8_t)country,(uint8_t)comp,(uint8_t)div));
        if(!d)continue;
        if(d->world!=world || d->origin!=EXP_CE_ORIGIN_LEAGUE || d->missing[0] || d->nphases<1u ||
           (po && !d->has_playout))return -1;
        d->pool_direct=(uint8_t)dir;d->pool_playoff=(uint8_t)po;
        d->rules_hash=def_hash(d);
        ++added;
    }
    if(off<0)return -1;
    return added;
}
/* v311: optional parallel level below a league division (league_calendar.json row):
 *   "parallel_level": {"groups":[2,3,4],"promoted_per_group":1,"extra_promoted":1}
 * The division's relegations become groups*promoted_per_group+extra (the movement
 * the data declare); every group must exist in the same family and promote
 * `promoted_per_group`. All rows validated first. -1 malformed. */
int ExpCe_LoadParallelLevels(const char *json,int len,uint16_t world)
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
        const char *row=arr+vo,*o=NULL,*ga=NULL;
        int ol=0,gl=0,goff=0,gko,gkl,gvo,gvl,gvt;
        double country=0,comp=0,div=0,per=0,extra=0;
        ExpCeDef *d,*copy;
        if(mjson_find(row,vl,"$.parallel_level",&o,&ol)==MJSON_TOK_INVALID)continue;
        if(vt!=MJSON_TOK_OBJECT || mjson_find(row,vl,"$.parallel_level",&o,&ol)!=MJSON_TOK_OBJECT ||
           mjson_get_number(row,vl,"$.team_number",&country)<=0 ||
           mjson_get_number(row,vl,"$.comp_id",&comp)<=0 ||
           mjson_get_number(row,vl,"$.division",&div)<=0 ||
           mjson_get_number(o,ol,"$.promoted_per_group",&per)<=0 || per<1 || per>4 ||
           mjson_find(o,ol,"$.groups",&ga,&gl)!=MJSON_TOK_ARRAY)goto done;
        (void)mjson_get_number(o,ol,"$.extra_promoted",&extra);
        if(extra<0 || extra>4)goto done;
        d=def_find(ExpCe_LeagueCompetitionId((uint8_t)country,(uint8_t)comp,(uint8_t)div));
        if(!d)continue;
        if(d->world!=world || d->origin!=EXP_CE_ORIGIN_LEAGUE || d->missing[0])goto done;
        i=(uint32_t)(d-s_ce.defs);
        if(staged[i])goto done;
        copy=(ExpCeDef *)malloc(sizeof *copy);
        if(!copy)goto done;
        staged[i]=copy;*copy=*d;
        copy->par_n=0u;
        while((goff=mjson_next(ga,gl,goff,&gko,&gkl,&gvo,&gvl,&gvt))>0) {
            double g=0;
            char nb[16];
            const ExpCeDef *gd;
            if(gvt!=MJSON_TOK_NUMBER || gvl>=(int)sizeof nb || copy->par_n>=8u)goto done;
            memcpy(nb,ga+gvo,(size_t)gvl);nb[gvl]=0;g=atof(nb);
            if(g<=div || g>15)goto done;
            gd=def_find(ExpCe_LeagueCompetitionId((uint8_t)country,(uint8_t)comp,(uint8_t)g));
            if(!gd || gd->missing[0] || gd->origin!=EXP_CE_ORIGIN_LEAGUE ||
               gd->promoted!=(uint8_t)per || gd->clubs<(uint16_t)(per+extra+1))goto done;
            copy->par_groups[copy->par_n++]=(uint8_t)g;
        }
        if(goff<0 || copy->par_n<1u)goto done;
        copy->par_per_group=(uint8_t)per;copy->par_extra=(uint8_t)extra;
        copy->relegated=(uint8_t)((double)copy->par_n*per+extra);
        if(copy->relegated>15u || copy->relegated>=copy->clubs)goto done;
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
/* v310: optional league play-out on league_calendar.json rows:
 *   "playout": {"rank_hi":16,"rank_lo":17,"relegated_total":4,"kind":"KNOCKOUT",
 *     "pairing":"LISTED","higher_home":2,"match":{...},"after_phase_end":{...}}
 * Validated for ALL rows before any definition changes. A row whose league is not
 * defined in this world is skipped. -1 malformed, else the number applied. */
int ExpCe_LoadLeaguePlayouts(const char *json,int len,uint16_t world)
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
        const char *row=arr+vo,*o=NULL;
        int ol=0;
        double country=0,comp=0,div=0,hi=0,lo=0,tot=0;
        ExpCeDef *d,*copy;
        if(mjson_find(row,vl,"$.playout",&o,&ol)==MJSON_TOK_INVALID)continue;
        if(vt!=MJSON_TOK_OBJECT || mjson_find(row,vl,"$.playout",&o,&ol)!=MJSON_TOK_OBJECT ||
           mjson_get_number(row,vl,"$.team_number",&country)<=0 ||
           mjson_get_number(row,vl,"$.comp_id",&comp)<=0 ||
           mjson_get_number(row,vl,"$.division",&div)<=0 ||
           country<0 || country>254 || comp<0 || comp>255 || div<0 || div>15 ||
           mjson_get_number(o,ol,"$.rank_hi",&hi)<=0 || mjson_get_number(o,ol,"$.rank_lo",&lo)<=0 ||
           mjson_get_number(o,ol,"$.relegated_total",&tot)<=0 ||
           hi<1 || lo<=hi || lo>64 || tot<1 || tot>15)goto done;
        d=def_find(ExpCe_LeagueCompetitionId((uint8_t)country,(uint8_t)comp,(uint8_t)div));
        if(!d)continue; /* league not defined in this world */
        if(d->world!=world || d->origin!=EXP_CE_ORIGIN_LEAGUE || d->nphases<1u ||
           d->missing[0] || lo>d->clubs)goto done;
        i=(uint32_t)(d-s_ce.defs);
        if(staged[i])goto done;
        copy=(ExpCeDef *)malloc(sizeof *copy);
        if(!copy)goto done;
        staged[i]=copy;*copy=*d;
        memset(&copy->playout,0,sizeof copy->playout);
        if(!parse_phase(o,ol,&copy->phases[0].table,&copy->phases[0].match,&copy->playout) ||
           copy->playout.kind!=EXP_CE_PHASE_KNOCKOUT ||
           copy->playout.nrel!=copy->playout.match.legs || copy->playout.rel_anchor!=0u ||
           copy->playout.higher_home<1u || copy->playout.higher_home>2u)goto done;
        {   ExpCeRuleError err;
            if(!ExpCeRules_ValidateMatch(&copy->playout.match,&err))goto done; }
        {   double pg=0;
            if(mjson_get_number(o,ol,"$.skip_if_gap_points_greater_than",&pg)>0) {
                if(pg<1 || pg>99)goto done;
                copy->playout_gap_min=(uint8_t)pg;
            }
        }
        copy->has_playout=1u;copy->playout_hi=(uint8_t)hi;copy->playout_lo=(uint8_t)lo;
        copy->playout_relegated_total=(uint8_t)tot;
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
            {   double vr=0;
                if(mjson_get_number(c,vl,"$.viewer_region",&vr)>0 && vr>=80 && vr<=85)
                    d->viewer_region=(uint8_t)vr;
            }
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
        {   /* v00.13.355 "interim":{"phase":0,"tiers":[4,4,4,2],"cross":[...],"exclude_last_unequal":true} */
            const char *io=NULL;int iol=0;
            if(mjson_find(c,vl,"$.interim",&io,&iol)==MJSON_TOK_OBJECT) {
                static const char *xc[]={"","GROUP_POSITION","POINTS","GOAL_DIFF","GOALS_FOR","AWAY_GOALS_FOR",
                                         "WINS","AWAY_WINS","DISCIPLINE","ACCESS_LIST"};
                const char *a=NULL;int al=0,off=0,ko,kl,vo,vl2,vt;
                double pv=0;int eb=0;
                if(mjson_get_number(io,iol,"$.phase",&pv)>0)d->interim_phase=(uint8_t)pv;
                if(mjson_get_bool(io,iol,"$.exclude_last_unequal",&eb)>0)d->interim_exclude_last=(uint8_t)(eb!=0);
                if(mjson_find(io,iol,"$.tiers",&a,&al)==MJSON_TOK_ARRAY)
                    while((off=mjson_next(a,al,off,&ko,&kl,&vo,&vl2,&vt))>0 && d->ninterim_tiers<8u) {
                        char nb[8];if(vl2>=(int)sizeof nb)break;memcpy(nb,a+vo,(size_t)vl2);nb[vl2]=0;
                        d->interim_tiers[d->ninterim_tiers++]=(uint8_t)atoi(nb);
                    }
                off=0;
                if(mjson_find(io,iol,"$.cross",&a,&al)==MJSON_TOK_ARRAY)
                    while((off=mjson_next(a,al,off,&ko,&kl,&vo,&vl2,&vt))>0 && d->ninterim_cross<12u) {
                        char nb[24];unsigned q;int f=0;
                        if(vt!=MJSON_TOK_STRING || vl2-2>=(int)sizeof nb)break;
                        memcpy(nb,a+vo+1,(size_t)(vl2-2));nb[vl2-2]=0;
                        for(q=1;q<sizeof xc/sizeof xc[0];++q)if(!strcmp(nb,xc[q])){d->interim_cross[d->ninterim_cross++]=(uint8_t)q;f=1;}
                        if(!f && !d->missing[0])snprintf(d->missing,sizeof d->missing,"interim.cross %.40s",nb);
                    }
                if(!d->ninterim_tiers || !d->ninterim_cross)snprintf(d->missing,sizeof d->missing,"interim");
            }
        }
        {   /* v00.13.355 "final_ranking":[{"order":"FIXED"|"INTERIM","select":[{"winner":[p,t]}|
             * {"loser":[p,t]}|{"winners":p}|{"losers":p}|{"interim":[a,b]}]}] (phases 1-based) */
            const char *fa=NULL;int fl=0,off=0,ko,kl,vo,vl2,vt;
            if(mjson_find(c,vl,"$.final_ranking",&fa,&fl)==MJSON_TOK_ARRAY)
                while((off=mjson_next(fa,fl,off,&ko,&kl,&vo,&vl2,&vt))>0) {
                    const char *sa=NULL;int sl=0,so=0,sk,skl,sv,svl,st;
                    char ord[16];
                    if(vt!=MJSON_TOK_OBJECT || d->nbuckets>=16u){snprintf(d->missing,sizeof d->missing,"final_ranking");break;}
                    d->buckets[d->nbuckets].by_interim=(uint8_t)(json_str(fa+vo,vl2,"$.order",ord,sizeof ord) && !strcmp(ord,"INTERIM"));
                    if(mjson_find(fa+vo,vl2,"$.select",&sa,&sl)!=MJSON_TOK_ARRAY){snprintf(d->missing,sizeof d->missing,"final_ranking.select");break;}
                    while((so=mjson_next(sa,sl,so,&sk,&skl,&sv,&svl,&st))>0) {
                        double x=0,y=0;
                        uint8_t kind=0;
                        const char *o2=sa+sv;
                        if(d->buckets[d->nbuckets].nsel>=4u)break;
                        if(mjson_get_number(o2,svl,"$.winner[0]",&x)>0 && mjson_get_number(o2,svl,"$.winner[1]",&y)>0)kind=1;
                        else if(mjson_get_number(o2,svl,"$.loser[0]",&x)>0 && mjson_get_number(o2,svl,"$.loser[1]",&y)>0)kind=2;
                        else if(mjson_get_number(o2,svl,"$.winners",&x)>0)kind=3;
                        else if(mjson_get_number(o2,svl,"$.losers",&x)>0)kind=4;
                        else if(mjson_get_number(o2,svl,"$.interim[0]",&x)>0 && mjson_get_number(o2,svl,"$.interim[1]",&y)>0)kind=5;
                        if(!kind){snprintf(d->missing,sizeof d->missing,"final_ranking selector");break;}
                        if(kind!=5 && x>=1)x-=1;   /* phases are 1-based in JSON */
                        d->buckets[d->nbuckets].sel[d->buckets[d->nbuckets].nsel].kind=kind;
                        d->buckets[d->nbuckets].sel[d->buckets[d->nbuckets].nsel].a=(uint8_t)x;
                        d->buckets[d->nbuckets].sel[d->buckets[d->nbuckets].nsel].b=(uint8_t)y;
                        ++d->buckets[d->nbuckets].nsel;
                    }
                    ++d->nbuckets;
                }
        }
        (void)json_str(c,vl,"$.next_missing",d->next_missing,sizeof d->next_missing);
        {   double cyv=0;   /* v00.13.355 biennial competitions */
            if(mjson_get_number(c,vl,"$.cycle_years",&cyv)>0 && cyv>=1 && cyv<=8)d->cycle_years=(uint8_t)cyv;
        }
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
            {   int fs=0;
                if(mjson_get_bool(o,ol,"$.from_sources",&fs)>0)d->first_from_sources=(uint8_t)(fs!=0);
            }
            if(mjson_find(o,ol,"$.entrants",&ea,&el)==MJSON_TOK_ARRAY)
                while((eo=mjson_next(ea,el,eo,&ek,&ekl,&ev,&evl,&et))>0) {
                    ExpCeEntrant *x;
                    char ck[24];
                    if(d->nfirst>=EXP_CE_MAX_ENTRANTS)break;
                    x=&d->first[d->nfirst];
                    memset(x,0,sizeof *x);
                    if(et==MJSON_TOK_OBJECT &&
                       json_str(ea+ev,evl,"$.nation",ck,sizeof ck)) {
                        /* v00.13.355 national team: "file:ordinal" of the OG
                         * region files 80..85 (NATIONAL key, never a club) */
                        unsigned nf=0,no=0;
                        if(sscanf(ck,"%u:%u",&nf,&no)!=2 || nf<80u || nf>85u || no>255u) {
                            if(!d->missing[0])snprintf(d->missing,sizeof d->missing,
                                "first_edition.entrants[%lu].nation",(unsigned long)d->nfirst);
                            continue;
                        }
                        x->club=ExpandedNationalKey(world,(uint16_t)((nf<<8)|no));
                    } else if(et!=MJSON_TOK_OBJECT ||
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
            if(!d->nfirst && !d->first_from_sources && !d->missing[0])
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
                if(mjson_get_number(sj,svl,"$.alt_competition",&v)>0)sp->alt_competition=(uint32_t)v;
                if(sp->source==EXP_CE_SRC_LISTED ? (!sp->count || !sp->nclubs) : !sp->competition)continue;
                ++d->nsources;
            }
        }
        if(mjson_get_number(c,vl,"$.last_year",&v)>0 && v>=1990 && v<=2199)d->last_year=(uint16_t)v;
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
        /* v342 (27/11/2026): an edition with NO fixture played or claimed yet adopts
         * the current rules (e.g. a bench size changed in the data): nothing has
         * been decided under the old ones. Its already generated calendar is kept.
         * A RULES_CHANGED block of such an edition is lifted and its events released. */
        if(e->state>=EXP_CE_ED_SCHEDULED && e->rules_hash!=d->rules_hash) {
            uint32_t k,decided=0u;
            for(k=0;k<r->fixture_count;++k)
                if(ExpandedEditionKey_Equal(r->fixtures[k].key.edition,e->key) &&
                   r->ledger.entries[k].state!=EXP_FIXTURE_SCHEDULED)++decided;
            if(!decided && (!e->block || e->block==EXP_CE_BLOCK_RULES_CHANGED)) {
                if(e->block==EXP_CE_BLOCK_RULES_CHANGED) {
                    /* fixtures held while the clock moved on: the whole unplayed calendar is
                     * shifted by whole weeks (same weekdays, same order) so that it starts
                     * after today, latest fixture first so no club is ever doubled. */
                    const int32_t today=r->scheduler.clock.day_index;
                    int32_t first=INT32_MAX,delta=0;
                    for(k=0;k<r->fixture_count;++k)
                        if(ExpandedEditionKey_Equal(r->fixtures[k].key.edition,e->key) &&
                           r->fixtures[k].day_index<first)first=r->fixtures[k].day_index;
                    if(first!=INT32_MAX && first<=today)delta=((today+1-first+6)/7)*7;
                    if(delta) {
                        int ok=host && host->reschedule;
                        uint32_t n=0u,q,w;
                        uint64_t *ids=(uint64_t *)malloc((size_t)r->fixture_count*sizeof *ids);
                        int32_t *days=(int32_t *)malloc((size_t)r->fixture_count*sizeof *days);
                        if(!ids || !days)ok=0;
                        for(k=0;ok && k<r->fixture_count;++k)
                            if(ExpandedEditionKey_Equal(r->fixtures[k].key.edition,e->key)) {
                                ids[n]=r->fixtures[k].event_id;days[n]=r->fixtures[k].day_index;++n;
                            }
                        for(q=0;ok && q<n;++q) {            /* latest first */
                            uint32_t m=q;
                            for(w=q+1u;w<n;++w)if(days[w]>days[m])m=w;
                            { uint64_t ti=ids[q];int32_t td=days[q];ids[q]=ids[m];days[q]=days[m];ids[m]=ti;days[m]=td; }
                            if(!host->reschedule(host->user,ids[q],days[q]+delta))ok=0;
                        }
                        free(ids);free(days);
                        fprintf(stderr,"[ce-engine] EDITION_CALENDAR_SHIFTED competition=%lu year=%u "
                                "weeks=%d fixtures=%lu ok=%d\n",(unsigned long)e->def_id,(unsigned)e->year,
                                (int)(delta/7),(unsigned long)n,ok);
                    }
                    e->block=0u;e->why[0]=0;
                    if(host && host->hold && !host->hold(host->user,e->key,0)) {
                        ed_block(r,host,e,EXP_CE_BLOCK_RULES_CHANGED,
                                 "rules changed; held fixtures could not be moved after today");
                        continue;
                    }
                }
                e->rules_hash=d->rules_hash;
                ed_log("EDITION_RULES_ADOPTED",e,"no fixture played yet");
                ++changed;
                continue;
            }
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
