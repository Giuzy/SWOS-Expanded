/* RG-014 experimental original OG modal: C11 pure event contract.
 * NOT a native game, career menu, CAR or Windows gameplay test. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "expanded_bolivia_modal_invariants.h"

int main(void)
{
    /* Dates are civil day indices from the ONE resident scheduler. */
    assert(ExpandedBoliviaModal_DateGate(100,105,1,110)==
           EXP_BOLIVIA_MODAL_DUE);
    assert(ExpandedBoliviaModal_DateGate(100,100,1,110)==
           EXP_BOLIVIA_MODAL_DUE);
    assert(ExpandedBoliviaModal_DateGate(100,105,1,105)==
           EXP_BOLIVIA_MODAL_LEAGUE_FIRST);
    assert(ExpandedBoliviaModal_DateGate(100,105,1,103)==
           EXP_BOLIVIA_MODAL_LEAGUE_FIRST);
    assert(ExpandedBoliviaModal_DateGate(101,100,0,0)==
           EXP_BOLIVIA_MODAL_OVERDUE);
    assert(ExpandedBoliviaModal_DateGate(100,120,0,0)==
           EXP_BOLIVIA_MODAL_DUE);
    /* One original fixture and exactly one observed OG witness. */
    assert(ExpandedBoliviaModal_WitnessGate(14,15,300,301,55,56));
    assert(!ExpandedBoliviaModal_WitnessGate(14,14,300,301,55,56));
    assert(!ExpandedBoliviaModal_WitnessGate(14,16,300,301,55,56));
    assert(!ExpandedBoliviaModal_WitnessGate(14,15,300,300,55,56));
    assert(!ExpandedBoliviaModal_WitnessGate(14,15,300,302,55,56));
    assert(!ExpandedBoliviaModal_WitnessGate(14,15,300,301,55,55));
    assert(!ExpandedBoliviaModal_WitnessGate(14,15,300,301,55,57));
    assert(!ExpandedBoliviaModal_WitnessGate(UINT32_MAX,0,300,301,55,56));
    assert(!ExpandedBoliviaModal_WitnessGate(14,15,(size_t)-1,0,55,56));
    assert(!ExpandedBoliviaModal_WitnessGate(14,15,300,301,UINT64_MAX,0));
    puts("RG-014 MODAL_C11_GATE_PASS pos=3 neg=13 nativeWindows=NOT_TESTED");
    return 0;
}
