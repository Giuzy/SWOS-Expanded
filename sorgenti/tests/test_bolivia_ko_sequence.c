/* Static fixture audit only. Real scores and progression are owned by OG. */
#include "bolivia_cup_knockout.h"
#include "bolivia_cup_sequence.h"
#include <stdio.h>

int main(void)
{
    static const uint8_t months[14]={
        10,10,10,10,11,11,11,11,11,11,11,11,11,11
    };
    static const uint8_t days[14]={
        20,21,21,21,2,4,4,5,11,12,15,15,18,25
    };
    static const uint8_t groups[8][2]={
        {3,1},{0,0},{0,1},{3,0},{2,1},{1,0},{1,1},{2,0}
    };
    unsigned seen[4][2]={{0}};
    if(!BoliviaKO_Validate())return 1;
    if(BOLIVIA_CUP_GROUP_FIXTURES!=64 ||
       BOLIVIA_KO_FIXTURES!=14 ||
       (int)BOLIVIA_CUP_KNOCKOUT_FIXTURES!=(int)BOLIVIA_KO_FIXTURES)
       return 2;
    for(unsigned i=0;i<8u;i++) {
        const unsigned g=kBoliviaKOGroups[i][0];
        const unsigned p=kBoliviaKOGroups[i][1];
        if(g>=4u || p>=2u || seen[g][p]++ ||
           g!=groups[i][0] || p!=groups[i][1])return 3;
    }
    for(unsigned g=0;g<4u;g++)
       for(unsigned p=0;p<2u;p++)if(seen[g][p]!=1u)return 4;
    for(unsigned i=0;i<14u;i++) {
        const BoliviaKODate *d=&kBoliviaKO2026[i];
        if(d->month!=months[i] || d->day!=days[i])return 5;
        if(d->round!=(i<8u?0u:i<12u?1u:2u))return 6;
        if(d->leg!=(i<8u?(i>=4u):i<12u?(i>=10u):(i>=13u)))
            return 7;
        if(d->match!=(i<8u?i%4u:i<12u?(i-8u)%2u:0u))return 8;
    }
    for(unsigned i=0;i<14u;i++)
       for(unsigned j=0;j<i;j++) {
          const BoliviaKODate *a=&kBoliviaKO2026[i],*b=&kBoliviaKO2026[j];
          if(a->month==b->month && a->day==b->day &&
             a->round==b->round && a->match==b->match)
             return 9; /* one tie cannot play twice on the same day */
       }
    puts("BOLIVIA_KO_SEQUENCE_C11=PASS teams=8 qf=8 sf=4 final=2 total=14");
    puts("BOLIVIA_KO_DATES_SOURCE=FBF_2026_REPORTS");
    puts("BOLIVIA_KO_OG_GAMEPLAY=NOT_TESTED");
    puts("BOLIVIA_KO_MANAGER_BRIDGE=NOT_TESTED");
    return 0;
}
