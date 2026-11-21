/* RG-014: pure, no-OG-state gates shared by the native Copa modal.
 * Actual team pairs/dates/results stay owned by resident OpenSWOS.
 * Never mutate a fixture, score, or civil clock from this header. */
#ifndef EXPANDED_BOLIVIA_MODAL_INVARIANTS_H
#define EXPANDED_BOLIVIA_MODAL_INVARIANTS_H
#include <stdint.h>
#include <stddef.h>

enum {
    EXP_BOLIVIA_MODAL_DUE = 1,
    EXP_BOLIVIA_MODAL_LEAGUE_FIRST = -1,
    EXP_BOLIVIA_MODAL_OVERDUE = -2
};
/* Must be tested with live OG manager league witness, not an estimated
 * fixture count. A same-day collision is not silently approved. */
static inline int ExpandedBoliviaModal_DateGate(
    int32_t today,int32_t fixture,int hasLeague,int32_t nextLeague)
{
    if(fixture<today)return EXP_BOLIVIA_MODAL_OVERDUE;
    if(hasLeague && nextLeague<=fixture)
        return EXP_BOLIVIA_MODAL_LEAGUE_FIRST;
    return EXP_BOLIVIA_MODAL_DUE;
}
/* A whole Copa menu can include the manager while the current
 * ORIGINAL OG fixture has two CPU sides. Never apply a session-wide
 * interactive sentinel requirement to CPU/CPU fixtures. */
static inline int ExpandedBoliviaModal_ManagerMode(
    uint16_t manager,uint16_t home,uint16_t away)
{
    if(manager==0xffffu || home==0xffffu || away==0xffffu ||
       home==away)return -1;
    return (home==manager || away==manager)?1:0;
}
/* One original OG callback may publish exactly one score. A cancelled
 * callback does not call this; a multiple-witness delta is never PASS. */
static inline int ExpandedBoliviaModal_WitnessGate(
    uint32_t beforeSerial,uint32_t afterSerial,
    size_t beforeWitness,size_t afterWitness,
    uint64_t beforeCup,uint64_t afterCup)
{
    return beforeSerial<UINT32_MAX &&
           afterSerial==beforeSerial+1u &&
           afterWitness>beforeWitness &&
           afterWitness-beforeWitness==1u &&
           afterCup>beforeCup &&
           afterCup-beforeCup==1u;
}
#endif /* EXPANDED_BOLIVIA_MODAL_INVARIANTS_H */
