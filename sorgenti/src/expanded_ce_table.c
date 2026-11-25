/* SWOS Expanded — Fase 2 standings and knockout decisions (see header). */
#include "expanded_ce_table.h"
#include <stdlib.h>
#include <string.h>

typedef struct HistoryRule {
    ExpandedEditionKey edition;
    uint32_t played,count;
    ExpCeInitialDeduction *deductions;
} HistoryRule;
static HistoryRule *s_history;
static uint32_t s_history_n;
void ExpCeTable_HistoryReset(void)
{
    uint32_t i;
    for(i=0;i<s_history_n;++i)free(s_history[i].deductions);
    free(s_history);s_history=NULL;s_history_n=0;
}
int ExpCeTable_HistoryAdd(ExpandedEditionKey edition,uint32_t played,
    const ExpCeInitialDeduction *deductions,uint32_t count)
{
    HistoryRule *rules;
    ExpCeInitialDeduction *copy=NULL;
    uint32_t i,j;
    if(!played || (count&&!deductions))return 0;
    for(i=0;i<s_history_n;++i)
        if(ExpandedEditionKey_Equal(s_history[i].edition,edition))return 0;
    for(i=0;i<count;++i){
        if(!deductions[i].club||!deductions[i].points)return 0;
        for(j=0;j<i;++j)if(deductions[j].club==deductions[i].club)return 0;
    }
    if(count){
        copy=(ExpCeInitialDeduction *)malloc((size_t)count*sizeof *copy);
        if(!copy)return 0;
        memcpy(copy,deductions,(size_t)count*sizeof *copy);
    }
    rules=(HistoryRule *)realloc(s_history,(size_t)(s_history_n+1u)*sizeof *rules);
    if(!rules){free(copy);return 0;}
    s_history=rules;
    s_history[s_history_n++]=(HistoryRule){edition,played,count,copy};
    return 1;
}
typedef struct TbCtx {
    const ExpandedUniversalRegistry *r;
    ExpandedEditionKey edition;
    uint32_t stage;
    const ExpCeTableRules *rules;
    ExpCeNameCompare name_cmp;
    void *user;
} TbCtx;

static int fixture_in(const TbCtx *c,const ExpandedUniversalFixture *f)
{
    return ExpandedEditionKey_Equal(f->key.edition,c->edition) &&
           f->key.stage==c->stage;
}
static int row_find(ExpCeTableRow *rows,uint32_t n,ExpandedClubKey k)
{
    uint32_t i;
    for(i=0;i<n;++i)if(ExpandedClubKey_Equal(rows[i].club,k))return (int)i;
    return -1;
}
/* Score lite view (goals incl. extra time). */
typedef struct ExpFixtureScoreLite { uint32_t home,away; } ExpFixtureScoreLite;
static void row_apply(ExpCeTableRow *h,ExpCeTableRow *a,
                      const ExpFixtureScoreLite *s,const ExpCeTableRules *t)
{
    ++h->played;++a->played;
    h->goals_for+=s->home;h->goals_against+=s->away;
    a->goals_for+=s->away;a->goals_against+=s->home;
    a->away_goals_for+=s->away;
    if(s->home>s->away){++h->wins;++a->losses;h->points+=t->win;a->points+=t->loss;}
    else if(s->home<s->away){++a->wins;++h->losses;a->points+=t->win;h->points+=t->loss;}
    else {++h->draws;++a->draws;h->points+=t->draw;a->points+=t->draw;}
}
/* Mini-table among rows[from..to): only fixtures between two of them. */
static int mini_values(const TbCtx *c,ExpCeTableRow *rows,uint32_t from,
                       uint32_t to,uint8_t tb,int64_t *values)
{
    const ExpandedUniversalRegistry *r=c->r;
    ExpCeTableRow *mini;
    uint32_t i,n=to-from;
    mini=(ExpCeTableRow *)calloc(n?n:1u,sizeof *mini);
    if(!mini)return 0;
    for(i=0;i<n;++i)mini[i].club=rows[from+i].club;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *l=&r->ledger.entries[i];
        int hi,ai;
        ExpFixtureScoreLite s;
        if(!fixture_in(c,f) || l->state!=EXP_FIXTURE_PLAYED)continue;
        hi=row_find(mini,n,f->home);ai=row_find(mini,n,f->away);
        if(hi<0 || ai<0)continue;
        s.home=l->score.home;s.away=l->score.away;
        row_apply(&mini[hi],&mini[ai],&s,c->rules);
    }
    for(i=0;i<n;++i) {
        const ExpCeTableRow *m=&mini[i];
        values[i]=tb==EXP_CE_TB_H2H_POINTS?(int64_t)m->points:
                  tb==EXP_CE_TB_H2H_GOAL_DIFF?(int64_t)m->goals_for-(int64_t)m->goals_against:
                  tb==EXP_CE_TB_H2H_GOALS_FOR?(int64_t)m->goals_for:
                  (int64_t)m->away_goals_for;
    }
    free(mini);
    return 1;
}
static uint64_t lot_key(const TbCtx *c,ExpandedClubKey k)
{
    uint64_t h=UINT64_C(14695981039346656037);
    uint32_t v[4];
    unsigned i;
    v[0]=c->edition.competition.stable_competition;
    v[1]=(uint32_t)c->edition.start_day;
    v[2]=c->stage;v[3]=k.stable_club;
    for(i=0;i<16u;++i){h^=((const uint8_t *)v)[i];h*=UINT64_C(1099511628211);}
    return h;
}
/* Sort rows[from..to) by criterion index ci, then recurse on equal runs. */
static int rank_segment(const TbCtx *c,ExpCeTableRow *rows,uint32_t from,
                        uint32_t to,uint32_t ci)
{
    const ExpCeTableRules *t=c->rules;
    uint32_t n=to-from,i,j,run;
    int64_t *values;
    uint8_t tb;
    if(n<2u || ci>=t->ntb)return 1;
    tb=t->tiebreak[ci];
    values=(int64_t *)malloc((size_t)n*sizeof *values);
    if(!values)return 0;
    if(tb>=EXP_CE_TB_H2H_POINTS && tb<=EXP_CE_TB_H2H_AWAY_GOALS) {
        if(!mini_values(c,rows,from,to,tb,values)){free(values);return 0;}
    } else if(tb==EXP_CE_TB_NAME || tb==EXP_CE_TB_LOT) {
        /* total orders: insertion sort with the comparator directly */
        for(i=from+1u;i<to;++i) {
            ExpCeTableRow x=rows[i];
            j=i;
            while(j>from) {
                int before;
                if(tb==EXP_CE_TB_NAME && c->name_cmp)
                    before=c->name_cmp(c->user,x.club,rows[j-1u].club)<0;
                else if(tb==EXP_CE_TB_NAME)
                    before=x.club.stable_club<rows[j-1u].club.stable_club;
                else before=lot_key(c,x.club)<lot_key(c,rows[j-1u].club);
                if(!before)break;
                rows[j]=rows[j-1u];--j;
            }
            rows[j]=x;
        }
        free(values);
        return 1;
    } else {
        for(i=0;i<n;++i) {
            const ExpCeTableRow *w=&rows[from+i];
            values[i]=tb==EXP_CE_TB_POINTS?(int64_t)w->points:
                      tb==EXP_CE_TB_GOAL_DIFF?(int64_t)w->goals_for-(int64_t)w->goals_against:
                      tb==EXP_CE_TB_GOALS_FOR?(int64_t)w->goals_for:
                      tb==EXP_CE_TB_WINS?(int64_t)w->wins:
                      (int64_t)w->away_goals_for;
        }
    }
    /* stable insertion sort by value desc (rows + values together) */
    for(i=1u;i<n;++i) {
        ExpCeTableRow x=rows[from+i];
        const int64_t v=values[i];
        j=i;
        while(j>0u && values[j-1u]<v) {
            rows[from+j]=rows[from+j-1u];values[j]=values[j-1u];--j;
        }
        rows[from+j]=x;values[j]=v;
    }
    for(i=0;i<n;i=run) {
        for(run=i+1u;run<n && values[run]==values[i];++run){}
        if(run-i>1u && !rank_segment(c,rows,from+i,from+run,ci+1u)) {
            free(values);return 0;
        }
    }
    free(values);
    return 1;
}
int ExpCeTable_Compute(const ExpandedUniversalRegistry *r,
                       ExpandedEditionKey edition,uint32_t stage,
                       const ExpCeTableRules *rules,
                       ExpCeNameCompare name_cmp,void *user,
                       ExpCeTableRow *rows,uint32_t capacity,uint32_t *used)
{
    TbCtx c;
    uint32_t i,n=0u,historical=0u;
    if(used)*used=0u;
    if(!r || !rules || !rows || !used || !capacity || !stage ||
       !r->fixtures || !r->ledger.entries ||
       r->fixture_count!=r->ledger.count ||
       !ExpCeRules_ValidateTable(rules,NULL))return 0;
    c.r=r;c.edition=edition;c.stage=stage;c.rules=rules;
    c.name_cmp=name_cmp;c.user=user;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *l=&r->ledger.entries[i];
        int hi,ai;
        ExpFixtureScoreLite s;
        if(!ExpandedFixtureKey_Equal(f->key,l->key))return 0;
        if(!fixture_in(&c,f))continue;
        if(f->key.replay!=0u)return 0;
        hi=row_find(rows,n,f->home);
        if(hi<0) {
            if(n==capacity)return 0;
            memset(&rows[n],0,sizeof rows[n]);rows[n].club=f->home;hi=(int)n++;
        }
        ai=row_find(rows,n,f->away);
        if(ai<0) {
            if(n==capacity)return 0;
            memset(&rows[n],0,sizeof rows[n]);rows[n].club=f->away;ai=(int)n++;
        }
        if(l->state==EXP_FIXTURE_CLAIMED)return 0; /* unresolved */
        if(l->state!=EXP_FIXTURE_PLAYED)continue;
        s.home=l->score.home;s.away=l->score.away;
        row_apply(&rows[hi],&rows[ai],&s,rules);
        if(l->score.flags&EXP_CE_SCORE_START_HISTORY)++historical;
    }
    if(historical){
        const HistoryRule *h=NULL;uint32_t j,k;
        for(j=0;j<s_history_n;++j)if(ExpandedEditionKey_Equal(s_history[j].edition,edition))h=&s_history[j];
        if(!h || stage!=1u || historical!=h->played)return 0;
        for(j=0;j<h->count;++j){
            for(k=0;k<n&&rows[k].club.stable_club!=h->deductions[j].club;++k){}
            if(k==n||rows[k].points<h->deductions[j].points)return 0;
            rows[k].points-=h->deductions[j].points;
        }
    }
    if(!rank_segment(&c,rows,0u,n,0u))return 0;
    for(i=0;i<n;++i)rows[i].rank=i+1u;
    *used=n;
    return 1;
}
int ExpCeTable_StageComplete(const ExpandedUniversalRegistry *r,
                             ExpandedEditionKey edition,uint32_t stage,
                             uint32_t *fixtures)
{
    uint32_t i,n=0u;
    if(fixtures)*fixtures=0u;
    if(!r || !r->fixtures || r->fixture_count!=r->ledger.count)return 0;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        if(!ExpandedEditionKey_Equal(f->key.edition,edition) ||
           f->key.stage!=stage)continue;
        ++n;
        if(r->ledger.entries[i].state!=EXP_FIXTURE_PLAYED)return 0;
    }
    if(fixtures)*fixtures=n;
    return n>0u;
}
ExpCeTieResult ExpCeTie_Resolve(const ExpandedUniversalRegistry *r,
                                ExpandedEditionKey edition,uint32_t stage,
                                uint32_t tie,const ExpCeMatchRules *rules,
                                ExpandedClubKey *winner,ExpandedClubKey *loser)
{
    const ExpandedUniversalFixture *leg[2]={0,0};
    const ExpandedFixtureLedgerEntry *led[2]={0,0};
    uint32_t i,legs;
    uint32_t a_goals=0,b_goals=0,a_away=0,b_away=0;
    ExpandedClubKey a,b;
    const ExpandedFixtureLedgerEntry *last;
    if(winner)*winner=(ExpandedClubKey){0};
    if(loser)*loser=(ExpandedClubKey){0};
    if(!r || !rules || !winner || !loser || !tie || !rules->needs_winner ||
       !r->fixtures || r->fixture_count!=r->ledger.count)
        return EXP_CE_TIE_INVALID;
    legs=rules->legs;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        if(!ExpandedEditionKey_Equal(f->key.edition,edition) ||
           f->key.stage!=stage || f->key.tie!=tie)continue;
        if(f->key.leg>=legs || f->key.replay || leg[f->key.leg])
            return EXP_CE_TIE_INVALID;
        leg[f->key.leg]=f;led[f->key.leg]=&r->ledger.entries[i];
    }
    for(i=0;i<legs;++i)
        if(!leg[i])return EXP_CE_TIE_INVALID;
    a=leg[0]->home;b=leg[0]->away;
    if(legs==2u && (!ExpandedClubKey_Equal(leg[1]->home,b) ||
                    !ExpandedClubKey_Equal(leg[1]->away,a)))
        return EXP_CE_TIE_INVALID;
    for(i=0;i<legs;++i) {
        if(led[i]->state==EXP_FIXTURE_CLAIMED)return EXP_CE_TIE_PENDING;
        if(led[i]->state!=EXP_FIXTURE_PLAYED)return EXP_CE_TIE_PENDING;
    }
    a_goals=led[0]->score.home;b_goals=led[0]->score.away;b_away=led[0]->score.away;
    if(legs==2u) {
        a_goals+=led[1]->score.away;b_goals+=led[1]->score.home;
        a_away=led[1]->score.away;
    }
    last=led[legs-1u];
    if(a_goals!=b_goals) {
        *winner=a_goals>b_goals?a:b;*loser=a_goals>b_goals?b:a;
        return EXP_CE_TIE_DECIDED;
    }
    if(rules->away_goals && legs==2u && a_away!=b_away) {
        *winner=a_away>b_away?a:b;*loser=a_away>b_away?b:a;
        return EXP_CE_TIE_DECIDED;
    }
    if((last->score.flags&EXP_CE_SCORE_PENALTIES) &&
       last->score.penalties_home!=last->score.penalties_away) {
        /* the deciding leg's home side is b on a 2-leg tie, a otherwise */
        const ExpandedClubKey home=legs==2u?b:a,away=legs==2u?a:b;
        const int home_wins=last->score.penalties_home>last->score.penalties_away;
        *winner=home_wins?home:away;*loser=home_wins?away:home;
        return EXP_CE_TIE_DECIDED;
    }
    return EXP_CE_TIE_UNRESOLVED;
}
int ExpCeTie_MatchContext(const ExpandedUniversalRegistry *r,
                          const ExpandedUniversalFixture *f,
                          const ExpCeMatchRules *rules,uint8_t *decides_tie,
                          uint16_t *home_first,uint16_t *away_first)
{
    uint32_t i;
    if(decides_tie)*decides_tie=0u;
    if(home_first)*home_first=0u;
    if(away_first)*away_first=0u;
    if(!r || !f || !rules || !decides_tie || !home_first || !away_first)return 0;
    if(!rules->needs_winner)return 1;
    if(rules->legs==1u){*decides_tie=1u;return 1;}
    if(f->key.leg==0u)return 1; /* first leg: a level score is final */
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *g=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *l=&r->ledger.entries[i];
        if(!ExpandedEditionKey_Equal(g->key.edition,f->key.edition) ||
           g->key.stage!=f->key.stage || g->key.tie!=f->key.tie ||
           g->key.leg!=0u)continue;
        if(l->state!=EXP_FIXTURE_PLAYED ||
           !ExpandedClubKey_Equal(g->home,f->away) ||
           !ExpandedClubKey_Equal(g->away,f->home))return 0;
        /* first leg seen from THIS match's home side */
        *home_first=l->score.away;*away_first=l->score.home;
        *decides_tie=1u;
        return 1;
    }
    return 0;
}
