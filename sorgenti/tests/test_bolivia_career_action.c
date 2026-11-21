/* RG-014: isolated UX gate on the REAL civil date library.
 * Does not prove OG native menu, manager witness, Windows CAR or input.
 */
#include <assert.h>
#include <stdio.h>
#include "expanded_bolivia_career_action.h"
static ExpandedWorldDate at(unsigned y,unsigned m,unsigned d)
{
    return (ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d};
}
int main(void)
{
    assert(ExpandedBoliviaCareer_Action(at(2026,8,28),at(2026,9,3))==
           EXP_BOLIVIA_ACTION_FORWARD);
    assert(ExpandedBoliviaCareer_Action(at(2026,9,2),at(2026,9,3))==
           EXP_BOLIVIA_ACTION_FORWARD);
    assert(ExpandedBoliviaCareer_Action(at(2026,9,3),at(2026,9,3))==
           EXP_BOLIVIA_ACTION_PLAY);
    assert(ExpandedBoliviaCareer_Action(at(2026,9,4),at(2026,9,3))==
           EXP_BOLIVIA_ACTION_BLOCKED);
    assert(ExpandedBoliviaCareer_Action(at(2026,12,31),at(2027,1,1))==
           EXP_BOLIVIA_ACTION_FORWARD);
    assert(ExpandedBoliviaCareer_Action(at(2027,1,1),at(2027,1,1))==
           EXP_BOLIVIA_ACTION_PLAY);
    assert(ExpandedBoliviaCareer_Action(at(2027,1,2),at(2027,1,1))==
           EXP_BOLIVIA_ACTION_BLOCKED);
    assert(ExpandedBoliviaCareer_Action(at(2026,2,30),at(2026,3,1))==
           EXP_BOLIVIA_ACTION_BLOCKED);
    assert(ExpandedBoliviaCareer_Action(at(2026,9,3),at(2026,13,1))==
           EXP_BOLIVIA_ACTION_BLOCKED);
    assert(ExpandedBoliviaCareer_Action(at(1899,1,1),at(2026,9,3))==
           EXP_BOLIVIA_ACTION_BLOCKED);
    printf("RG-014 ACTION_DATE_ISO PASS: 10 positive/negative date cases\n");
    printf("OG_GAMEPLAY_CAR: NOT_TESTED\n");
    return 0;
}
