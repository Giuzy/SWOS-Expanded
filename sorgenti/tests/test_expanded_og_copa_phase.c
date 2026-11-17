#include "expanded_og_copa_phase.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main(void)
{
    /* These are the real OpenSWOS native-key boundaries from the 96/97
     * world calendar. Verify no fixture before March 1997. */
    for(unsigned key=0;key<112u;++key)
        assert(Expanded_OGCopa1996_FixturePhase((uint8_t)key)<=16u);
    assert(Expanded_OGCopa1996_FixturePhase(0u)==0u);  /* setup only */
    assert(Expanded_OGCopa1996_FixturePhase(48u)==0u); /* Jul 96 START */
    assert(Expanded_OGCopa1996_FixturePhase(95u)==0u); /* Dec 96 */
    assert(Expanded_OGCopa1996_FixturePhase(96u)==0u); /* Jan 97 */
    assert(Expanded_OGCopa1996_FixturePhase(111u)==15u); /* Feb 97 */
    assert(Expanded_OGCopa1996_FixturePhase(112u)==16u); /* Mar 97 */
    assert(Expanded_OGCopa1996_FixturePhase(113u)==17u); /* March games may begin */
    assert(Expanded_OGCopa1996_FixturePhase(136u)==40u); /* Jun 97 */
    assert(Expanded_OGCopa1996_FixturePhase(143u)==47u); /* final Jun tick */
    for(unsigned key=97;key<=143u;++key)
        assert(Expanded_OGCopa1996_FixturePhase((uint8_t)key)==key-96u);
    puts("PASS isolated: original Copa type-2 phase 16..40 maps to March-June after July 96 START");
    return 0;
}
