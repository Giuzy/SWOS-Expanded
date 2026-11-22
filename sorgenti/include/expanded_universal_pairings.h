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
 * Return 0 on invalid inputs, allocation errors or caller rejection.
 * No match pair is repeated WITHIN one cycle, and for two cycles
 * exactly the reverse home/away game is scheduled on cycle 2.
 */
static inline int ExpandedUniversal_GenerateRoundRobin(
    uint32_t clubs, uint8_t meetings,
    ExpandedUniversalPairConsumer emit, void *user)
{
    uint32_t slots, cycles, rounds, cycle, round, pos, serial;
    uint32_t *order;
    if(!emit || clubs<2u || meetings<1u || meetings>4u ||
       clubs==UINT32_MAX)return 0;
    slots=clubs+(clubs&1u);
    rounds=slots-1u;
#if SIZE_MAX < UINT64_MAX
    if((size_t)slots>SIZE_MAX/sizeof *order)return 0;
#endif
    if(clubs>UINT32_MAX/(clubs-1u) ||
       (clubs*(clubs-1u)/2u)>UINT32_MAX/meetings)
        return 0;
    order=(uint32_t *)malloc((size_t)slots*sizeof *order);
    if(!order)return 0;
    cycles=(uint32_t)meetings;
    for(cycle=0;cycle<cycles;++cycle) {
        for(pos=0;pos<slots;++pos)order[pos]=pos;
        serial=0u;
        for(round=0;round<rounds;++round) {
            for(pos=0;pos<slots/2u;++pos) {
                uint32_t a=order[pos],b=order[slots-1u-pos];
                uint32_t home,away;
                if(a>=clubs || b>=clubs)continue; /* odd team bye */
                if(((round+pos+cycle)&1u)!=0u) {
                    home=b;away=a;
                } else {home=a;away=b;}
                if(!emit(user,cycle,round,serial++,home,away)) {
                    free(order);return 0;
                }
            }
            /* Circle rotation: index 0 remains fixed; the rest rotate.
             * Every unordered pair occurs once per meeting cycle. */
            if(slots>2u) {
                const uint32_t end=order[slots-1u];
                for(pos=slots-1u;pos>1u;--pos)order[pos]=order[pos-1u];
                order[1u]=end;
            }
        }
    }
    free(order);
    return 1;
}
#endif
