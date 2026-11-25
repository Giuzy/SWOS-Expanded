/* Deterministic round-robin pairing kernel for the JSON Competition Engine.
 * Inputs: validated JSON participant count and meeting cycles, plus the
 * catalogue's already-sorted immutable club keys. NO OG schedule/TSKC/RNG.
 * The caller owns civil dates, fixture IDs, results and CAR.
 */
#ifndef EXPANDED_UNIVERSAL_PAIRINGS_H
#define EXPANDED_UNIVERSAL_PAIRINGS_H
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
typedef int (*ExpandedUniversalPairConsumer)(
    void *user, uint32_t cycle, uint32_t round, uint32_t serial,
    uint32_t home_index, uint32_t away_index);
/* 1/2/3/4 meetings, 2..N clubs, including odd N with a BYE.
 * Return 0 on invalid inputs or caller rejection.
 * No match pair is repeated WITHIN one cycle, and every odd cycle is the
 * exact reverse (home/away) of the previous one.
 *
 * v303 home/away rule (replaces the old parity walk that gave a club up to
 * n-1 consecutive away games). Circle method: clubs 0..slots-2 rotate,
 * club slots-1 is fixed (the bye when `clubs` is odd). Round r pairs i
 * with j=(r-i) mod m, m=slots-1; i==j meets the fixed club. The lower
 * index hosts when (r even) XOR (i+j>=m); the pair with the fixed club
 * is hosted by i in even rounds. For an even number of clubs every club
 * alternates home/away with the minimum possible number of breaks
 * (n-2 in total, longest run 2) and home games differ by at most 1. */
static inline int ExpandedUniversal_GenerateRoundRobin(
    uint32_t clubs, uint8_t meetings,
    ExpandedUniversalPairConsumer emit, void *user)
{
    uint32_t slots, m, cycle, round, i, serial;
    if(!emit || clubs<2u || meetings<1u || meetings>4u ||
       clubs==UINT32_MAX)return 0;
    if(clubs>UINT32_MAX/(clubs-1u) ||
       (clubs*(clubs-1u)/2u)>UINT32_MAX/meetings)
        return 0;
    slots=clubs+(clubs&1u);
    m=slots-1u;
    for(cycle=0;cycle<(uint32_t)meetings;++cycle) {
        serial=0u; /* cumulative per cycle: unique serial per fixture */
        for(round=0;round<m;++round) {
            for(i=0;i<m;++i) {
                const uint32_t j=(round+m-i)%m;
                uint32_t home,away;
                if(i>j)continue;
                if(i==j) {
                    const uint32_t fixed=slots-1u;
                    if(fixed>=clubs)continue; /* bye */
                    if((round&1u)==0u){home=i;away=fixed;}
                    else {home=fixed;away=i;}
                } else if(((((round&1u)==0u)?1u:0u)^((i+j>=m)?1u:0u))!=0u) {
                    home=i;away=j;
                } else {home=j;away=i;}
                if(cycle&1u){const uint32_t t=home;home=away;away=t;}
                if(!emit(user,cycle,round,serial++,home,away))return 0;
            }
        }
    }
    return 1;
}
#endif
