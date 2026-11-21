/* RG-014: pure KO type1 native cursor contract.
 * No manager score, bracket result, date, or CAR is created here.
 */
#ifndef EXPANDED_BOLIVIA_KO_MODAL_INVARIANTS_H
#define EXPANDED_BOLIVIA_KO_MODAL_INVARIANTS_H
#include "bolivia_cup_knockout.h"
#include "bolivia_manager_offset_gate.h"
#include "expanded_bolivia_modal_invariants.h"
static inline int ExpandedBoliviaKO_ModalCursor(
    unsigned serial,unsigned round,unsigned leg,unsigned match,
    unsigned matchCount,unsigned totalMatchesInRound)
{
    if(serial>=BOLIVIA_KO_FIXTURES || matchCount!=serial)return 0;
    const BoliviaKODate *d=&kBoliviaKO2026[serial];
    return round==d->round && leg==d->leg && match==d->match &&
           totalMatchesInRound==(4u>>d->round);
}
/* The manager's temporary -1 must identify their own enrolled club
 * and cannot be present on an OG CPU/CPU tie. */
static inline int ExpandedBoliviaKO_ModalOffsets(
    uint16_t manager,uint16_t home,uint16_t away,
    int32_t homeOffset,int32_t awayOffset,uint16_t count)
{
    const int mode=ExpandedBoliviaModal_ManagerMode(manager,home,away);
    return mode>=0 &&
           BoliviaManagerFixture_ValidOffsets(homeOffset,awayOffset,count,mode);
}
#endif
