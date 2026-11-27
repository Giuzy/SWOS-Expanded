/* Standalone arithmetic regression; not runtime or CAR integration. */
#include "expanded_ce_ranking.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    double a=0,b=0;uint32_t p=0;uint64_t sum=0;
    ExpCeUefaMatchPoints m;
    ExpCeUefaSeasonValue v[]={{2020,500,1},{2021,1000,1},{2022,2000,1},{2023,3000,1},{2024,4000,1},{2025,5000,1},{2026,6000,1}};
    uint32_t x[5]={1000,2000,3000,4000,5000},y[5]={1000,1999,9000,9000,9000};
    assert(ExpCeFifa_Sum(1300,1500,25,EXP_RANK_A_WIN,0,&a,&b));
    assert(fabs(a-1317.0746)<.01&&fabs(b-1482.9254)<.01);
    assert(ExpCeFifa_Sum(1500,1500,60,EXP_RANK_A_PSO,1,&a,&b)&&a==1515&&b==1500);
    assert(ExpCeFifa_Sum(1500,1500,60,EXP_RANK_A_WIN,1,&a,&b)&&a==1530&&b==1500);
    assert(ExpCeFifa_Sum(1500,1500,25,EXP_RANK_B_WIN,0,&a,&b)&&a==1487.5&&b==1512.5);
    assert(!ExpCeFifa_Sum(1500,1500,12,EXP_RANK_DRAW,0,&a,&b)&&a==1487.5);
    assert(!ExpCeFifa_Sum(NAN,1500,25,EXP_RANK_DRAW,0,&a,&b));
    assert(ExpCeUefa_Match2627(EXP_UEFA_QUALIFYING,0,EXP_RANK_A_WIN,&m)&&m.club_a==0&&m.association_a==1000);
    assert(ExpCeUefa_Match2627(EXP_UEFA_KO_PLAYOFF,0,EXP_RANK_DRAW,&m)&&m.club_a==0&&m.association_a==1000);
    assert(ExpCeUefa_Match2627(EXP_UEFA_FINAL,0,EXP_RANK_A_PSO,&m)&&m.club_a==1000&&m.club_b==1000);
    assert(ExpCeUefa_Match2627(EXP_UEFA_KO,1,EXP_RANK_A_WIN,&m)&&m.club_a==3000&&m.club_b==1000);
    assert(!ExpCeUefa_Match2627(EXP_UEFA_FINAL,1,EXP_RANK_A_WIN,&m));
    assert(ExpCeUefa_LeagueBonus2627(EXP_UEFA_UCL,1,&p)&&p==12000);
    assert(ExpCeUefa_LeagueBonus2627(EXP_UEFA_UCL,36,&p)&&p==6000);
    assert(ExpCeUefa_LeagueBonus2627(EXP_UEFA_UEL,24,&p)&&p==250);
    assert(ExpCeUefa_LeagueBonus2627(EXP_UEFA_UECL,10,&p)&&p==1875);
    assert(ExpCeUefa_LeagueBonus2627(EXP_UEFA_UECL,24,&p)&&p==125);
    assert(ExpCeUefa_LeagueBonus2627(EXP_UEFA_UECL,25,&p)&&p==0);
    assert(!ExpCeUefa_LeagueBonus2627(EXP_UEFA_UCL,37,&p));
    assert(ExpCeUefa_ReachBonus2627(EXP_UEFA_UCL,&p)&&p==1500);
    assert(ExpCeUefa_ReachBonus2627(EXP_UEFA_UECL,&p)&&p==500);
    assert(ExpCeUefa_Eliminated2627(4,&p)&&p==2500);
    assert(ExpCeUefa_LeagueFloor2627(EXP_UEFA_UEL,1000,&p)&&p==2000);
    assert(ExpCeUefa_LeagueFloor2627(EXP_UEFA_UECL,3000,&p)&&p==0);
    assert(ExpCeUefa_AssociationSeason(10000,3,&p)&&p==3333);
    assert(!ExpCeUefa_AssociationSeason(10000,0,&p)&&p==3333);
    assert(ExpCeUefa_Five(v,7,2024,&sum)&&sum==10500);
    assert(ExpCeUefa_Five(v,7,2025,&sum)&&sum==15000);
    assert(ExpCeUefa_Five(v,7,2026,&sum)&&sum==20000);
    assert(!ExpCeUefa_Five(v,7,2027,&sum)&&sum==20000);
    v[2].known=0;assert(!ExpCeUefa_Five(v,7,2025,&sum));v[2].known=1;
    v[2].year=2023;assert(!ExpCeUefa_Five(v,7,2025,&sum));
    assert(ExpCeUefa_ClubModern(5000,30000)==6000);
    assert(ExpCeUefa_ClubModern(7000,30000)==7000);
    assert(ExpCeUefa_RecentCompare(x,y)>0);
    puts("CE 2627 ranking arithmetic PASS");return 0;
}
