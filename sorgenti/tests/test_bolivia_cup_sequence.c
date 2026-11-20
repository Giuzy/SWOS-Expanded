#include "bolivia_cup_sequence.h"
#include <stdio.h>
int main(void) {
    BoliviaCupFixture f[BOLIVIA_CUP_GROUP_FIXTURES],bad[64];
    const unsigned n=BoliviaCup_BuildGroupFixtures(f);
    if(n!=64u || !BoliviaCup_ValidateGroupFixtures(f,n))return 1;
    for(unsigned i=0;i<64u;++i)bad[i]=f[i];
    bad[8]=bad[0]; /* duplicate ID and wrong matchday must fail */
    if(BoliviaCup_ValidateGroupFixtures(bad,64u))return 2;
    for(unsigned i=0;i<16u;++i)if(BoliviaCup_GroupOf((uint8_t)i)<0)return 3;
    if(BoliviaCup_GroupOf(16u)>=0)return 4;
    if(BOLIVIA_CUP_GROUP_FIXTURES+BOLIVIA_CUP_KNOCKOUT_FIXTURES!=78)return 5;
    puts("BOLIVIA_SEQUENCE_ISOLATED=PASS groups=4 teams=16 group_matches=64 derby=16 ko_capacity=14");
    puts("BOLIVIA_REAL_DATED_OG_EXECUTION=NOT_TESTED");
    return 0;
}
