/* Real 14 KO date table, type1 native cursors and manager/CPU offsets.
 * Scope: standalone C11; Windows OG/50Hz/CAR still NOT_TESTED.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_bolivia_ko_modal_invariants.h"
int main(void)
{
    assert(BoliviaKO_Validate());
    for(unsigned i=0u;i<BOLIVIA_KO_FIXTURES;++i) {
        const BoliviaKODate *d=&kBoliviaKO2026[i];
        const unsigned per=4u>>d->round;
        assert(ExpandedBoliviaKO_ModalCursor(i,d->round,d->leg,
                                              d->match,i,per));
        assert(!ExpandedBoliviaKO_ModalCursor(i,d->round,d->leg,
                                               d->match,i+1u,per));
        assert(!ExpandedBoliviaKO_ModalCursor(i,d->round+1u,d->leg,
                                               d->match,i,per));
        assert(!ExpandedBoliviaKO_ModalCursor(i,d->round,d->leg+1u,
                                               d->match,i,per));
        assert(!ExpandedBoliviaKO_ModalCursor(i,d->round,d->leg,
                                               d->match+1u,i,per));
        assert(!ExpandedBoliviaKO_ModalCursor(i,d->round,d->leg,
                                               d->match,i,per+1u));
    }
    assert(!ExpandedBoliviaKO_ModalCursor(14,0,0,0,14,1));
    assert(ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d01,0x2d02,-1,3,8));
    assert(ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d02,0x2d01,3,-1,8));
    assert(ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d02,0x2d03,3,4,8));
    assert(!ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d01,0x2d02,0,3,8));
    assert(!ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d02,0x2d03,-1,4,8));
    assert(!ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d01,0x2d02,-1,-1,8));
    assert(!ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d01,0x2d01,-1,3,8));
    assert(!ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d01,0x2d02,-2,3,8));
    assert(!ExpandedBoliviaKO_ModalOffsets(0x2d01,0x2d02,0x2d03,3,9,8));
    assert(ExpandedBoliviaModal_WitnessGate(12,13,31,32,49,50));
    assert(!ExpandedBoliviaModal_WitnessGate(12,12,31,32,49,50));
    assert(!ExpandedBoliviaModal_WitnessGate(12,13,31,33,49,50));
    puts("RG-014 KO_MODAL_ISO PASS: 14 x native cursor + manager/CPU negatives");
    puts("WINDOWS_OG_CAR_KO: NOT_TESTED");
    return 0;
}
