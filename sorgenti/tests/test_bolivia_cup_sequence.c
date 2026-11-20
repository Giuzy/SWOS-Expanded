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
    /* 2026 real fixture mapping: opening match 18 Aug, postponed derby
     * of matchday 2 only on 22 Sep, last group date 8 Oct. */
    if(f[0].home!=3u || f[0].away!=9u ||
       f[0].month!=8u || f[0].day!=18u)return 6;
    unsigned delayed=0u,closing=0u;
    for(unsigned i=0;i<64u;++i) {
        if(f[i].matchday==2u && f[i].home==6u && f[i].away==15u &&
           f[i].month==9u && f[i].day==22u)++delayed;
        if(f[i].matchday==8u && f[i].month==10u && f[i].day==8u)
            ++closing;
    }
    if(delayed!=1u || closing!=2u)return 7;
    for(unsigned i=0;i<64u;++i)bad[i]=f[i];
    bad[0].away=3u;
    if(BoliviaCup_ValidateGroupFixtures(bad,64u))return 8;
    /* Play ACTUAL dates in order, not the old matchday order. Delayed
     * round-1/round-2 games must not be played retrospectively. */
    BoliviaCupFixture sorted[64];
    if(BoliviaCup_ChronologicalFixtures(sorted)!=64u)return 9;
    uint8_t ids[65]={0};
    for(unsigned i=0u;i<64u;++i) {
        const unsigned id=sorted[i].fixtureId;
        if(!id || id>64u || ids[id]++)return 10;
        if(i>0u && (100u*sorted[i].month+sorted[i].day)<
                    (100u*sorted[i-1u].month+sorted[i-1u].day))
            return 11;
    }
    /* Inject a same-day conflict for San Antonio from matchday 2;
     * the dated validator must catch it even though rounds differ. */
    for(unsigned i=0;i<64u;++i)bad[i]=f[i];
    bad[12].month=f[0].month;bad[12].day=f[0].day;
    if(BoliviaCup_ValidateGroupFixtures(bad,64u))return 12;
    puts("BOLIVIA_SEQUENCE_ISOLATED=PASS groups=4 teams=16 group_matches=64 derby=16 dated-2026-fixtures=64 chronological=64 no-club-day-overlap=YES ko_capacity=14");
    puts("BOLIVIA_REAL_DATED_OG_EXECUTION=NOT_TESTED");
    return 0;
}
