/*
 * Copa Bolivia 2026 knockout calendar and group-to-bracket positions.
 * This is fixture identity and CIVIL DATES only: OG owns every result,
 * tie winner, aggregate, trophy and qualification. No synthetic scores.
 *
 * Four quarter-final ties, two semi-final ties, one final:
 * each is two legs = 8 + 4 + 2 = 14 original OG cup matches.
 *
 * The date slots follow the FBF 2026 QF schedule (Unitel 09/10/2026), allowing
 * new-world qualifiers to differ from actual-life clubs.
 */
#ifndef BOLIVIA_CUP_KNOCKOUT_H
#define BOLIVIA_CUP_KNOCKOUT_H
#include <stdint.h>
enum { BOLIVIA_KO_TEAMS=8, BOLIVIA_KO_FIXTURES=14,
       BOLIVIA_KO_QUARTER_LEGS=8, BOLIVIA_KO_SEMI_LEGS=4 };
typedef struct {
    uint8_t round, leg, match, month, day;
} BoliviaKODate;
/* Play order is the *same* OG cup bracket's native tie order. */
static const BoliviaKODate kBoliviaKO2026[BOLIVIA_KO_FIXTURES]={
    {0,0,0,10,20},{0,0,1,10,21},{0,0,2,10,21},{0,0,3,10,21},
    {0,1,0,11,2}, {0,1,1,11,4}, {0,1,2,11,4}, {0,1,3,11,5},
    {1,0,0,11,11},{1,0,1,11,12},
    {1,1,0,11,15},{1,1,1,11,15},
    {2,0,0,11,18},{2,1,0,11,25}
};
/* Group A=0 B=1 C=2 D=3, each has [winner,runner-up].
 * Actual Copa 2026 knockout CROSS-GROUP structure, not participants.
 * Quarter 1 D2-A1; quarter 2 A2-D1;
 * quarter 3 C2-B1; quarter 4 B2-C1.
 * Adjacent quarters feed the two semis via ORIGINAL OG bracket.
 */
static const uint8_t kBoliviaKOGroups[8][2]={
    {3,1},{0,0},{0,1},{3,0},
    {2,1},{1,0},{1,1},{2,0}
};
static inline int BoliviaKO_Validate(void)
{
    uint8_t seen[4][2]={{0}};
    for(unsigned i=0;i<8u;++i) {
        const unsigned g=kBoliviaKOGroups[i][0];
        const unsigned rank=kBoliviaKOGroups[i][1];
        if(g>=4u || rank>=2u || seen[g][rank]++)return 0;
        if(i&1u && kBoliviaKOGroups[i-1u][0]==g)return 0;
    }
    for(unsigned i=0;i<14u;++i) {
        const BoliviaKODate *d=&kBoliviaKO2026[i];
        const unsigned round=i<8u?0u:i<12u?1u:2u;
        const unsigned leg=i<8u?(i>=4u):i<12u?(i>=10u):(i>=13u);
        const unsigned tie=i<8u?(i%4u):i<12u?((i-8u)%2u):0u;
        if(d->round!=round || d->leg!=leg || d->match!=tie ||
           d->month<10u || d->month>11u ||
           !d->day || d->day>30u)return 0;
        if(i && (unsigned)kBoliviaKO2026[i-1u].month*100u+
                kBoliviaKO2026[i-1u].day >
                (unsigned)d->month*100u+d->day)return 0;
    }
    return 1;
}
#endif
