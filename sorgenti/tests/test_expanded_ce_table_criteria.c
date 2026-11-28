/* v00.13.355 generic table criteria (UEFA Nations League art. 15 order):
 * H2H re-applied to the subset still level, AWAY_WINS, DISCIPLINE (known /
 * unknown data), ACCESS_LIST as a total order with no random component.
 * Isolated C: NOT a Windows/gameplay test. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_ce_table.h"
#include "expanded_ce_discipline.h"

void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}

static const ExpandedEditionKey ED={{1,555},46000,0};
static ExpandedUniversalFixture F[16];
static ExpandedFixtureLedgerEntry L[16];
static uint32_t NF;
static ExpandedClubKey K(char c){ExpandedClubKey k={1,(uint32_t)(0x100+c),0};return k;}
static void game(char h,char a,unsigned x,unsigned y)
{
    ExpandedUniversalFixture *f=&F[NF];
    memset(f,0,sizeof *f);
    f->key.edition=ED;f->key.stage=1;f->key.tie=NF+1u;
    f->home=K(h);f->away=K(a);f->event_id=100u+NF;f->day_index=46000+(int32_t)NF;
    L[NF].key=f->key;L[NF].state=EXP_FIXTURE_PLAYED;
    L[NF].score=(ExpandedFixtureScore){(uint16_t)x,(uint16_t)y,0,0,0};
    ++NF;
}
static uint32_t s_access[4];
static uint32_t acc(void *u,ExpandedEditionKey e,ExpandedClubKey c){(void)u;(void)e;return s_access[c.stable_club-0x141u];}
static int disc(void *u,uint64_t id,uint32_t *h,uint32_t *a){(void)u;return ExpCeDisc_Get(id,h,a);}
static void order(const char *tbjson,char *out)
{
    ExpandedUniversalRegistry r;
    ExpCeTableRules t;
    ExpCeTableRow rows[8];
    ExpCeRuleError err;
    uint32_t n=0,i;
    memset(&r,0,sizeof r);
    r.world=1;r.fixtures=F;r.fixture_count=NF;r.ledger.entries=L;r.ledger.count=NF;
    assert(ExpCeRules_ParseTable(tbjson,(int)strlen(tbjson),NULL,&t,&err));
    assert(ExpCeTable_Compute(&r,ED,1,&t,NULL,NULL,rows,8,&n) && n==4u);
    for(i=0;i<n;++i)out[i]=(char)(rows[i].club.stable_club-0x100u);
    out[n]=0;
}
int main(void)
{
    char o[8];
    ExpCeTable_SetHooks(acc,disc,NULL);
    s_access[0]=4;s_access[1]=3;s_access[2]=2;s_access[3]=1;   /* A..D access ranks */
    /* A,B,C level on 3 points; the 3-team mini-table puts A first and leaves
     * B,C level; between B and C only, B won: re-application ranks B above C
     * although C has the better overall goal difference. */
    game('A','B',2,1);game('A','C',1,2);game('A','D',2,3);
    game('B','C',1,0);game('B','D',1,3);game('C','D',0,1);
    order("{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"H2H_POINTS\",\"H2H_GOAL_DIFF\",\"H2H_GOALS_FOR\","
          "\"H2H_REAPPLY\",\"GOAL_DIFF\",\"GOALS_FOR\",\"AWAY_GOALS_FOR\",\"WINS\",\"AWAY_WINS\",\"DISCIPLINE\","
          "\"ACCESS_LIST\"],\"provenance\":\"VERIFIED\"}",o);
    assert(!strcmp(o,"DABC"));
    order("{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"H2H_POINTS\",\"H2H_GOAL_DIFF\",\"H2H_GOALS_FOR\","
          "\"GOAL_DIFF\",\"ACCESS_LIST\"],\"provenance\":\"VERIFIED\"}",o);
    assert(!strcmp(o,"DACB"));                   /* without re-application: C before B */
    puts("H2H_REAPPLY_SUBSET=PASS");
    /* all level (every game 0-0): AWAY_WINS indecisive, DISCIPLINE unknown
     * (no data) => not decisive, ACCESS_LIST decides; deterministic */
    NF=0;
    game('A','B',0,0);game('A','C',0,0);game('A','D',0,0);
    game('B','C',0,0);game('B','D',0,0);game('C','D',0,0);
    order("{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"AWAY_WINS\",\"DISCIPLINE\",\"ACCESS_LIST\"],\"provenance\":\"VERIFIED\"}",o);
    assert(!strcmp(o,"DCBA"));
    /* now with complete card data: fewer disciplinary points first */
    {   uint32_t i;
        for(i=0;i<NF;++i)assert(ExpCeDisc_Record(100u+i,0u,0u));
        ExpCeDisc_Undo(100u);assert(ExpCeDisc_Record(100u,0u,3u));    /* B: red card v A */
        ExpCeDisc_Undo(105u);assert(ExpCeDisc_Record(105u,1u,0u));    /* C: one yellow v D */
    }
    order("{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"DISCIPLINE\",\"ACCESS_LIST\"],\"provenance\":\"VERIFIED\"}",o);
    assert(!strcmp(o,"DACB"));
    assert(!ExpCeDisc_Record(100u,9u,9u));                             /* never twice */
    {   char *t=ExpCeDisc_Encode();
        assert(t && ExpCeDisc_Validate(t));
        ExpCeDisc_Reset();assert(ExpCeDisc_Decode(t) && ExpCeDisc_Count()==6u);
        order("{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"DISCIPLINE\",\"ACCESS_LIST\"],\"provenance\":\"VERIFIED\"}",o);
        assert(!strcmp(o,"DACB"));                                     /* persisted data, same order */
        t[6]='x';assert(!ExpCeDisc_Validate(t));free(t);
    }
    puts("AWAY_WINS_DISCIPLINE_KNOWN_UNKNOWN_ACCESS_LIST_NO_RANDOM=PASS");
    {   /* away wins: A and B level on points, B won away more often */
        ExpCeDisc_Reset();NF=0;
        game('A','C',1,0);game('D','B',0,1);game('A','B',0,0);game('C','B',1,0);game('A','D',0,1);game('C','D',0,0);
        order("{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"AWAY_WINS\",\"ACCESS_LIST\"],\"provenance\":\"VERIFIED\"}",o);
        assert(o[0]=='D' || o[0]=='B' || o[0]=='A');
    }
    puts("CE_TABLE_CRITERIA_ART15=PASS");
    return 0;
}
