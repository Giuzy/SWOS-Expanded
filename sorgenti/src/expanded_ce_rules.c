/* SWOS Expanded — Fase 2 universal competition rules (see header). */
#include "expanded_ce_rules.h"
#include "mjson.h"
#include <stdio.h>
#include <string.h>

static int rule_fail(ExpCeRuleError *err,const char *field,const char *reason)
{
    if(err) {
        snprintf(err->field,sizeof err->field,"%s",field?field:"?");
        snprintf(err->reason,sizeof err->reason,"%s",reason?reason:"?");
    }
    return 0;
}

static const char *const k_tb_names[EXP_CE_TB_COUNT]={
    "END","POINTS","GOAL_DIFF","GOALS_FOR","WINS","AWAY_GOALS_FOR",
    "H2H_POINTS","H2H_GOAL_DIFF","H2H_GOALS_FOR","H2H_AWAY_GOALS",
    "NAME","LOT"
};
const char *ExpCeRules_TieBreakName(uint8_t tb)
{
    return tb<EXP_CE_TB_COUNT?k_tb_names[tb]:"?";
}
int ExpCeRules_TieBreakFromName(const char *name,uint8_t *out)
{
    uint8_t i;
    if(!name || !out)return 0;
    for(i=1u;i<EXP_CE_TB_COUNT;++i)
        if(strcmp(name,k_tb_names[i])==0) { *out=i; return 1; }
    return 0;
}

int ExpCeRules_FromOgLeague(uint8_t points_for_win,uint8_t subs,
                            uint8_t bench,ExpCeTableRules *table,
                            ExpCeMatchRules *match)
{
    if(!table || !match)return 0;
    memset(table,0,sizeof *table);
    memset(match,0,sizeof *match);
    /* OG native league: pointsForWin from the record, draw 1, loss 0
     * (DIY league table writer), order = DIY_SortLeagueTableByStandings:
     * points, goal difference, goals for, club name. */
    table->win=points_for_win;table->draw=1u;table->loss=0u;
    table->tiebreak[0]=EXP_CE_TB_POINTS;
    table->tiebreak[1]=EXP_CE_TB_GOAL_DIFF;
    table->tiebreak[2]=EXP_CE_TB_GOALS_FOR;
    table->tiebreak[3]=EXP_CE_TB_NAME;
    table->ntb=4u;
    table->provenance=EXP_CE_PROV_OG_NATIVE;
    match->legs=1u;          /* one fixture = one match; meetings = rounds */
    match->needs_winner=0u;
    match->bench=bench;
    match->subs=subs;
    match->windows=0u;       /* OG has no window rule */
    match->discipline=1u;    /* domestic: InitDIYLeague 0x3C = 1 */
    match->provenance=EXP_CE_PROV_OG_NATIVE;
    return ExpCeRules_ValidateTable(table,NULL) &&
           ExpCeRules_ValidateMatch(match,NULL);
}
int ExpCeRules_FromOgCupRound(uint8_t subs,uint8_t bench,uint8_t legs,
                              uint8_t extra_time,uint8_t penalties,
                              uint8_t away_goals_mode,uint8_t discipline,
                              ExpCeMatchRules *match,ExpCeRuleError *err)
{
    if(!match)return rule_fail(err,"match","null");
    memset(match,0,sizeof *match);
    if(legs>2u)return rule_fail(err,"round_legs","OG replay legs unsupported");
    if(extra_time>1u || penalties>1u)
        return rule_fail(err,"round_extra_time/penalties",
                         "OG value >1 means 'only on replay': replays unsupported");
    match->legs=legs?legs:1u;
    match->needs_winner=1u;
    match->extra_time=extra_time;
    match->penalties=penalties;
    match->away_goals=away_goals_mode==1u && match->legs==2u;
    match->bench=bench;match->subs=subs;
    match->discipline=discipline;
    match->provenance=EXP_CE_PROV_OG_NATIVE;
    return ExpCeRules_ValidateMatch(match,err);
}

int ExpCeRules_ValidateTable(const ExpCeTableRules *t,ExpCeRuleError *err)
{
    uint8_t i,seen[EXP_CE_TB_COUNT];
    if(!t)return rule_fail(err,"table","null");
    if(t->provenance==EXP_CE_PROV_MISSING)
        return rule_fail(err,"table","no verified source");
    if(!t->win || t->win>9u)return rule_fail(err,"table.points[0]","win 1..9");
    if(t->draw>=t->win)return rule_fail(err,"table.points[1]","draw < win");
    if(t->loss>t->draw)return rule_fail(err,"table.points[2]","loss <= draw");
    if(t->draw_penalties &&
       !(t->draw_pen_win<t->win && t->draw_pen_win>=t->draw_pen_loss &&
         t->draw_pen_loss>=t->loss && t->draw_pen_win>t->draw_pen_loss))
        return rule_fail(err,"table.draw_penalties","win > pen-win > pen-loss >= loss");
    if(!t->draw_penalties && (t->draw_pen_win || t->draw_pen_loss))
        return rule_fail(err,"table.draw_penalties","points set without the flag");
    if(!t->ntb || t->ntb>EXP_CE_MAX_TIEBREAK)
        return rule_fail(err,"table.tiebreak","1..10 criteria required");
    memset(seen,0,sizeof seen);
    for(i=0;i<t->ntb;++i) {
        const uint8_t tb=t->tiebreak[i];
        if(tb==EXP_CE_TB_END || tb>=EXP_CE_TB_COUNT)
            return rule_fail(err,"table.tiebreak","unknown criterion");
        if(seen[tb])return rule_fail(err,"table.tiebreak","duplicate criterion");
        seen[tb]=1u;
        if((tb==EXP_CE_TB_NAME || tb==EXP_CE_TB_LOT) && i+1u!=t->ntb)
            return rule_fail(err,"table.tiebreak","NAME/LOT must be last");
    }
    /* The last criterion must always separate two clubs, otherwise a final
     * table position (title, relegation, qualification) is undefined. */
    if(t->tiebreak[t->ntb-1u]!=EXP_CE_TB_NAME &&
       t->tiebreak[t->ntb-1u]!=EXP_CE_TB_LOT)
        return rule_fail(err,"table.tiebreak",
                         "last criterion must be NAME or LOT (total order)");
    return 1;
}
int ExpCeRules_ValidateMatch(const ExpCeMatchRules *m,ExpCeRuleError *err)
{
    if(!m)return rule_fail(err,"match","null");
    if(m->provenance==EXP_CE_PROV_MISSING)
        return rule_fail(err,"match","no verified source");
    if(m->legs<1u || m->legs>2u)return rule_fail(err,"match.legs","1 or 2");
    if(m->bench>EXP_CE_OG_MAX_BENCH)
        return rule_fail(err,"match.bench","exceeds the 15-seat bench");
    if(m->subs>m->bench)return rule_fail(err,"match.subs","subs > bench");
    if(m->subs+m->et_extra_subs>m->bench)
        return rule_fail(err,"match.et_extra_subs","total subs > bench");
    if(m->windows && m->windows>m->subs)
        return rule_fail(err,"match.windows","windows > subs");
    if(!m->windows && (m->halftime_free || m->et_extra_windows))
        return rule_fail(err,"match.windows",
                         "half-time/ET window rules need a window limit");
    if(m->discipline>1u)return rule_fail(err,"match.discipline","0 or 1");
    if(!m->needs_winner && (m->extra_time || m->penalties || m->away_goals))
        return rule_fail(err,"match.needs_winner",
                         "ET/penalties/away goals only for a knockout tie");
    if(m->away_goals && m->legs!=2u)
        return rule_fail(err,"match.away_goals","only with 2 legs");
    /* A knockout tie must ALWAYS end with a winner: penalties are the only
     * OG terminal decider (replays are not supported). */
    if(m->needs_winner && !m->penalties && !m->rank_tiebreak)
        return rule_fail(err,"match.penalties",
                         "knockout without penalties can end level (no replay)");
    if(m->rank_tiebreak && (!m->needs_winner || m->penalties))
        return rule_fail(err,"match.rank_tiebreak",
                         "only for a knockout tie decided without penalties");
    return 1;
}

static int json_u8(const char *json,int len,const char *path,
                   unsigned lo,unsigned hi,uint8_t *out,int *present,
                   ExpCeRuleError *err,const char *field)
{
    double v=0.0;
    int tok;
    const char *p=NULL;
    int plen=0;
    tok=mjson_find(json,len,path,&p,&plen);
    if(tok==MJSON_TOK_INVALID) { if(present)*present=0; return 1; }
    if(present)*present=1;
    if(tok==MJSON_TOK_TRUE) { v=1.0; }
    else if(tok==MJSON_TOK_FALSE) { v=0.0; }
    else if(tok!=MJSON_TOK_NUMBER || mjson_get_number(json,len,path,&v)<=0)
        return rule_fail(err,field,"not a number/bool");
    if(v<(double)lo || v>(double)hi || v!=(double)(unsigned)v)
        return rule_fail(err,field,"out of range");
    *out=(uint8_t)v;
    return 1;
}
int ExpCeRules_ParseTable(const char *json,int len,const ExpCeTableRules *base,
                          ExpCeTableRules *out,ExpCeRuleError *err)
{
    const char *arr=NULL;
    int alen=0,present=0;
    ExpCeTableRules t;
    char prov[24];
    if(!json || !out)return rule_fail(err,"table","null");
    if(base)t=*base; else memset(&t,0,sizeof t);
    if(mjson_find(json,len,"$.points",&arr,&alen)==MJSON_TOK_ARRAY) {
        double v[3];
        if(mjson_get_number(arr,alen,"$[0]",&v[0])<=0 ||
           mjson_get_number(arr,alen,"$[1]",&v[1])<=0 ||
           mjson_get_number(arr,alen,"$[2]",&v[2])<=0 ||
           v[0]<0 || v[0]>9 || v[1]<0 || v[1]>9 || v[2]<0 || v[2]>9)
            return rule_fail(err,"table.points","[win,draw,loss] 0..9");
        t.win=(uint8_t)v[0];t.draw=(uint8_t)v[1];t.loss=(uint8_t)v[2];
    }
    /* v309: "draw_penalties": [penalty-winner points, penalty-loser points] */
    if(mjson_find(json,len,"$.draw_penalties",&arr,&alen)==MJSON_TOK_ARRAY) {
        double w,l;
        if(mjson_get_number(arr,alen,"$[0]",&w)<=0 || mjson_get_number(arr,alen,"$[1]",&l)<=0 ||
           w<0 || w>9 || l<0 || l>9)
            return rule_fail(err,"table.draw_penalties","[winner points,loser points] 0..9");
        t.draw_penalties=1u;t.draw_pen_win=(uint8_t)w;t.draw_pen_loss=(uint8_t)l;
    }
    if(mjson_find(json,len,"$.tiebreak",&arr,&alen)==MJSON_TOK_ARRAY) {
        int off=0,ko,kl,vo,vl,vt;
        t.ntb=0u;
        while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
            char name[24];
            uint8_t tb=0;
            if(vt!=MJSON_TOK_STRING || vl<2 || vl-2>=(int)sizeof name ||
               t.ntb>=EXP_CE_MAX_TIEBREAK)
                return rule_fail(err,"table.tiebreak","string list (max 10)");
            memcpy(name,arr+vo+1,(size_t)(vl-2));name[vl-2]=0;
            if(!ExpCeRules_TieBreakFromName(name,&tb))
                return rule_fail(err,"table.tiebreak","unknown criterion name");
            t.tiebreak[t.ntb++]=tb;
        }
        if(off<0)return rule_fail(err,"table.tiebreak","malformed");
    }
    (void)present;
    if(mjson_get_string(json,len,"$.provenance",prov,sizeof prov)>0) {
        if(strcmp(prov,"VERIFIED")==0)t.provenance=EXP_CE_PROV_JSON_VERIFIED;
        else if(strcmp(prov,"GAME_TEMPLATE")==0)t.provenance=EXP_CE_PROV_JSON_TEMPLATE;
        else if(strcmp(prov,"OG_NATIVE")==0)t.provenance=EXP_CE_PROV_OG_NATIVE;
        else return rule_fail(err,"table.provenance",
                              "VERIFIED|GAME_TEMPLATE|OG_NATIVE");
    }
    if(!ExpCeRules_ValidateTable(&t,err))return 0;
    *out=t;
    return 1;
}
int ExpCeRules_ParseMatch(const char *json,int len,const ExpCeMatchRules *base,
                          ExpCeMatchRules *out,ExpCeRuleError *err)
{
    ExpCeMatchRules m;
    char prov[24];
    if(!json || !out)return rule_fail(err,"match","null");
    if(base)m=*base; else memset(&m,0,sizeof m);
#define U8(path,lo,hi,field,name) \
    if(!json_u8(json,len,path,lo,hi,&m.field,NULL,err,name))return 0
    U8("$.legs",1,2,legs,"match.legs");
    U8("$.knockout",0,1,needs_winner,"match.knockout");
    U8("$.extra_time",0,1,extra_time,"match.extra_time");
    U8("$.penalties",0,1,penalties,"match.penalties");
    U8("$.away_goals",0,1,away_goals,"match.away_goals");
    U8("$.rank_tiebreak",0,2,rank_tiebreak,"match.rank_tiebreak");
    U8("$.bench",0,255,bench,"match.bench");
    U8("$.subs",0,255,subs,"match.subs");
    U8("$.windows",0,255,windows,"match.windows");
    U8("$.halftime_free",0,1,halftime_free,"match.halftime_free");
    U8("$.et_extra_subs",0,255,et_extra_subs,"match.et_extra_subs");
    U8("$.et_extra_windows",0,255,et_extra_windows,"match.et_extra_windows");
    U8("$.discipline",0,1,discipline,"match.discipline");
#undef U8
    if(mjson_get_string(json,len,"$.provenance",prov,sizeof prov)>0) {
        if(strcmp(prov,"VERIFIED")==0)m.provenance=EXP_CE_PROV_JSON_VERIFIED;
        else if(strcmp(prov,"GAME_TEMPLATE")==0)m.provenance=EXP_CE_PROV_JSON_TEMPLATE;
        else if(strcmp(prov,"OG_NATIVE")==0)m.provenance=EXP_CE_PROV_OG_NATIVE;
        else return rule_fail(err,"match.provenance",
                              "VERIFIED|GAME_TEMPLATE|OG_NATIVE");
    }
    if(!ExpCeRules_ValidateMatch(&m,err))return 0;
    *out=m;
    return 1;
}
uint64_t ExpCeRules_Hash(const ExpCeTableRules *t,const ExpCeMatchRules *m)
{
    uint64_t h=UINT64_C(14695981039346656037);
    uint8_t buf[64];
    size_t n=0,i;
    memset(buf,0,sizeof buf);
    if(t) {
        buf[n++]=t->win;buf[n++]=t->draw;buf[n++]=t->loss;buf[n++]=t->ntb;
        /* only when used: existing rule sets keep their historical hash */
        if(t->draw_penalties){buf[n++]=0xD7u;buf[n++]=t->draw_pen_win;buf[n++]=t->draw_pen_loss;}
        for(i=0;i<EXP_CE_MAX_TIEBREAK;++i)buf[n++]=i<t->ntb?t->tiebreak[i]:0u;
    }
    if(m) {
        buf[n++]=m->legs;buf[n++]=m->needs_winner;buf[n++]=m->extra_time;
        buf[n++]=m->penalties;buf[n++]=m->away_goals;buf[n++]=m->bench;
        buf[n++]=m->subs;buf[n++]=m->windows;buf[n++]=m->halftime_free;
        buf[n++]=m->et_extra_subs;buf[n++]=m->et_extra_windows;
        buf[n++]=m->discipline;
        if(m->rank_tiebreak)buf[n++]=(uint8_t)(0xB6u+m->rank_tiebreak); /* only when used: historical hashes unchanged */
    }
    for(i=0;i<n;++i){h^=buf[i];h*=UINT64_C(1099511628211);}
    return h;
}
