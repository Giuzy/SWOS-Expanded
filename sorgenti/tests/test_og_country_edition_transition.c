/* RG-014/027, 022/025/028: pure counter transition contract.
 * Isolated synthetic proof, not a Windows match or CAR replay test. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_og_edition_transition.h"
int main(void)
{
    uint32_t comparable=9999u;
    /* Verified OG 2026 Copa group64 retires; 2027 resident begins at 0.
     * Worldwide active cup count drops by exactly 64 and no OG score
     * callback occurs at the edition boundary. */
    assert(ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,0u,1,1421u,1357u,&comparable));
    assert(comparable==1421u);
    /* One genuine other-country match in same tick must still be counted. */
    assert(ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,0u,1,1421u,1358u,&comparable));
    assert(comparable==1422u);
    /* Negative: no invented certification; no unsupported group stage. */
    comparable=9999u;
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,63u,0u,1,1421u,1357u,&comparable));
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,0u,0,1421u,1357u,&comparable));
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,1u,1,1421u,1357u,&comparable));
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2028u,64u,64u,0u,1,1421u,1357u,&comparable));
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,0u,1,1421u,1356u,&comparable));
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,0u,1,63u,0u,&comparable));
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,0u,1,1421u,1357u,NULL));
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,0u,0u,0u,1,1421u,1421u,&comparable));
    assert(comparable==9999u);
    /* Never overflow the global counter when retiring an old edition. */
    assert(!ExpandedOGEditionTransition_CertifiedCounter(
        2026u,2027u,64u,64u,0u,1,1421u,UINT32_MAX,&comparable));
    puts("PASS certified OG country edition counter 64->0 and negative cases (ISO)");
    return 0;
}
