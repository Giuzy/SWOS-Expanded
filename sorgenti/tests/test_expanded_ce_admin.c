/* v00.13.301 RG-T: administrative results of TEST clubs (pure module). */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_ce_admin.h"
#include <stdlib.h>
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
void *swos_io_alloc(uint32_t n){return malloc(n);}
void swos_io_free(void *p){free(p);}

static const char *GOOD=
"{\"schema\":1,\"policy\":{"
"\"test_vs_real\":{\"winner_goals\":3,\"loser_goals\":0},"
"\"test_vs_test\":{\"winner_goals\":1,\"loser_goals\":0,\"selection\":\"PAIR_HASH\"}},"
"\"clubs\":[\"44:55\",\"44:56\",\"44:57\",\"44:58\"]}";
static ExpandedClubKey K(unsigned c,unsigned s)
{return (ExpandedClubKey){2627u,(c<<16)|s,EXP_PARTICIPANT_CLUB};}
static ExpandedUniversalFixture fx(ExpandedClubKey h,ExpandedClubKey a,uint32_t comp,
                                   int32_t start,uint16_t leg)
{
    ExpandedUniversalFixture f;
    memset(&f,0,sizeof f);
    f.home=h;f.away=a;f.key.edition.competition=(ExpandedCompetitionKey){2627u,comp};
    f.key.edition.start_day=start;f.key.edition.sequence=1u;f.key.leg=leg;
    return f;
}
int main(void)
{
    ExpandedFixtureScore s;
    const char *why=NULL;
    int i,low=0,high=0;
    /* nothing loaded: every match is a real OG match */
    ExpCeAdmin_Reset();
    { ExpandedUniversalFixture f=fx(K(44,55),K(44,1),1u,100,0);
      assert(!ExpCeAdmin_Decide(&f,&s,&why) && ExpCeAdmin_Count()==0u); }
    assert(ExpCeAdmin_LoadJson(GOOD,(int)strlen(GOOD),2627u)==4);
    assert(ExpCeAdmin_Count()==4u && ExpCeAdmin_IsTest(K(44,57)) &&
           !ExpCeAdmin_IsTest(K(44,1)) && !ExpCeAdmin_IsTest(K(44,59)));
    { ExpandedClubKey other=K(44,55);other.world=9697u;
      assert(!ExpCeAdmin_IsTest(other)); }                 /* never across worlds */
    /* real vs real: not administrative */
    { ExpandedUniversalFixture f=fx(K(44,1),K(44,2),1u,100,0);
      assert(!ExpCeAdmin_Decide(&f,&s,&why)); }
    /* TEST vs real, either orientation: the real club wins 3-0, flagged */
    { ExpandedUniversalFixture f=fx(K(44,55),K(44,1),1u,100,0);
      assert(ExpCeAdmin_Decide(&f,&s,&why) && s.home==0 && s.away==3 &&
             (s.flags&EXP_CE_SCORE_ADMIN_TEST) && !(s.flags&3u) &&
             s.penalties_home==0 && s.penalties_away==0 &&
             !strcmp(why,"TEST_VS_REAL_REAL_WINS"));
      f=fx(K(44,1),K(44,55),1u,100,1);
      assert(ExpCeAdmin_Decide(&f,&s,&why) && s.home==3 && s.away==0); }
    /* TEST vs TEST: both legs same winner, 1-0, independent of position */
    for(i=0;i<200;++i) {
        const uint32_t comp=1000u+(uint32_t)i;
        ExpandedUniversalFixture a=fx(K(44,55),K(44,56),comp,100,0),
                                 b=fx(K(44,56),K(44,55),comp,100,1);
        ExpandedFixtureScore sa,sb;
        int a_home_wins,b_home_wins;
        assert(ExpCeAdmin_Decide(&a,&sa,&why) && !strcmp(why,"TEST_VS_TEST_PAIR_WINNER"));
        assert(ExpCeAdmin_Decide(&b,&sb,&why));
        assert((sa.home==1&&sa.away==0)||(sa.home==0&&sa.away==1));
        assert((sb.home==1&&sb.away==0)||(sb.home==0&&sb.away==1));
        a_home_wins=sa.home==1;b_home_wins=sb.home==1;
        /* leg 1 home = 44:55, leg 2 home = 44:56: same winner means the home
         * flags are opposite */
        assert(a_home_wins!=b_home_wins);
        if(a_home_wins)low++;else high++; /* winner 44:55 (lower) or 44:56 */
    }
    assert(low>40 && high>40);                 /* not locked on one position/key */
    /* rejections: no partial policy, no bad club, no duplicate */
#define BAD(js) do{ ExpCeAdmin_Reset(); assert(ExpCeAdmin_LoadJson((js),(int)strlen(js),2627u)==-1 && ExpCeAdmin_Count()==0u);}while(0)
    BAD("{\"schema\":1,\"clubs\":[\"44:55\"]}");
    BAD("{\"schema\":2,\"policy\":{},\"clubs\":[]}");
    BAD("{\"schema\":1,\"policy\":{\"test_vs_real\":{\"winner_goals\":0,\"loser_goals\":3},"
        "\"test_vs_test\":{\"winner_goals\":1,\"loser_goals\":0,\"selection\":\"PAIR_HASH\"}},\"clubs\":[]}");
    BAD("{\"schema\":1,\"policy\":{\"test_vs_real\":{\"winner_goals\":3,\"loser_goals\":0},"
        "\"test_vs_test\":{\"winner_goals\":1,\"loser_goals\":0,\"selection\":\"FIRST\"}},\"clubs\":[]}");
    BAD("{\"schema\":1,\"policy\":{\"test_vs_real\":{\"winner_goals\":3,\"loser_goals\":0},"
        "\"test_vs_test\":{\"winner_goals\":1,\"loser_goals\":0,\"selection\":\"PAIR_HASH\"}},\"clubs\":[\"44:55\",\"x\"]}");
    BAD("{\"schema\":1,\"policy\":{\"test_vs_real\":{\"winner_goals\":3,\"loser_goals\":0},"
        "\"test_vs_test\":{\"winner_goals\":1,\"loser_goals\":0,\"selection\":\"PAIR_HASH\"}},\"clubs\":[\"44:55\",\"44:55\"]}");
    /* a failed reload never leaves the previous policy half-applied */
    assert(ExpCeAdmin_LoadJson(GOOD,(int)strlen(GOOD),2627u)==4);
    assert(ExpCeAdmin_LoadJson("{\"schema\":1}",12,2627u)==-1 && ExpCeAdmin_Count()==4u);
    puts("EXP_CE_ADMIN_TEST_CLUBS=PASS_ISOLATED_ONLY");
    puts("OG_NATIVE_HOOK_GAMEPLAY_WINDOWS=NOT_TESTED_HERE");
    return 0;
}
