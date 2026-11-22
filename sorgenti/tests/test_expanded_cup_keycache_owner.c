/* RG-014 regression: native Copa Bolivia type1 KO has no local CAREER cup cache.
 * Negative witness: ZIP 00.13.250, after KO serial=7 round0->1, legacy
 * DIY_RecordCupBracketProgress hit PANIC 0x6666. Only OG resident advances. */
#include "expanded_cup_keycache_owner.h"
#include <assert.h>
#include <stdio.h>

static int legacy_cache_lookup(int hasLocalEntry)
{ return hasLocalEntry ? 1 : -0x6666; }

static int cup_cache_decision(unsigned gameType, int modalActive,
                              int hasLocalEntry)
{
    if (ExpandedCupKeyCache_SkipNativeModal(gameType,modalActive))
        return 0; /* do not write the unrelated local CAREER keycache */
    return legacy_cache_lookup(hasLocalEntry);
}

int main(void)
{
    /* The original failure remains reproducible in the model. */
    assert(legacy_cache_lookup(0)==-0x6666);
    /* The same OG callback is used for QUARTERS -> SEMIS -> FINAL;
     * no local CAREER cup slot exists in any of these three rounds. */
    for(unsigned nextRound=0u;nextRound<3u;++nextRound)
        assert(cup_cache_decision(4u,1,0)==0);

    /* The save/clock delegate suspends the modal, so no bypass there. */
    assert(cup_cache_decision(4u,0,1)==1);
    assert(cup_cache_decision(4u,0,0)==-0x6666);

    /* Classic/DIY/Season and all non-career modes are unchanged. */
    for (unsigned mode=0; mode<=5; ++mode) {
        if(mode==4u)continue;
        assert(cup_cache_decision(mode,1,1)==1);
        assert(cup_cache_decision(mode,1,0)==-0x6666);
    }
    assert(cup_cache_decision(4u,-1,0)==0);
    assert(ExpandedCupKeyCache_SkipNativeModal(4u,0)==0);
    assert(ExpandedCupKeyCache_SkipNativeModal(4u,1)==1);
    puts("PASS RG-014 native Copa KO vs CAREER keycache owner (isolated)");
    return 0;
}
