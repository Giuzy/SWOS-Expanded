/* RG-014: native Copa Bolivia modal has no local CAREER cup keycache.
 * One OG resident owns its bracket; keycache is ONLY a five-slot career view.
 * This pure gate is shared by the production guard and its isolated tests. */
#ifndef EXPANDED_CUP_KEYCACHE_OWNER_H
#define EXPANDED_CUP_KEYCACHE_OWNER_H

static inline int ExpandedCupKeyCache_SkipNativeModal(
    unsigned gameType, int nativeModalActive)
{
    /* CAREER is 4 in OG. During the modal save/world delegate the native
     * modal is suspended (active=0), and the ordinary career cache is valid. */
    return gameType == 4u && nativeModalActive != 0;
}
#endif /* EXPANDED_CUP_KEYCACHE_OWNER_H */
