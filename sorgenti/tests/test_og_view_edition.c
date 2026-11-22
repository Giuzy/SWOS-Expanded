/* RG-014/027: original OG viewer must resolve the TRUE COUNTRY edition,
 * never the oldest inserted pointer alias or an invented future season.
 * Isolated C11 positive/negative; no save, RNG, simulator or UI hooks. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_og_view_edition.h"
#define C(i,c,y,cur,p,id) ExpandedOGViewEdition_Consider(&v,i,c,y,cur,p,id)
#define HAS(i) do { uint32_t j=UINT32_MAX; assert(ExpandedOGViewEdition_Selected(&v,&j)); assert(j==(i)); ++passed; } while(0)
#define NOT() do { uint32_t j=UINT32_MAX; assert(!ExpandedOGViewEdition_Selected(&v,&j)); ++passed; } while(0)
#define RESET() do { v=(ExpandedOGViewEditionChoice){0}; } while(0)
int main(void)
{
    ExpandedOGViewEditionChoice v={0};
    unsigned passed=0u;
    /* Inserted 2026 first, but July 2027 must show new 2027 native OG. */
    C(0,45,2026,2027,1,1);
    C(1,45,2027,2027,1,1);
    HAS(1);
    RESET();
    /* Same result in reverse resident storage order. */
    C(0,45,2027,2027,1,1);
    C(1,45,2026,2027,1,1);
    HAS(0);
    RESET();
    /* 2027 never opened: newest actual historical fallback, not 2025. */
    C(0,45,2025,2027,1,1);
    C(1,45,2026,2027,1,1);
    HAS(1);
    RESET();
    /* A stale compId alias must not defeat the exact native pointer. */
    C(0,45,2027,2027,0,1);
    C(1,45,2027,2027,1,1);
    HAS(1);
    RESET();
    /* Current by-ID only is still newer than an older pointer record. */
    C(0,45,2026,2027,1,1);
    C(1,45,2027,2027,0,1);
    HAS(1);
    RESET();
    /* A future OG edition is NOT historical, even if allocated. */
    C(0,45,2028,2027,1,1);
    C(1,45,2026,2027,1,1);
    HAS(1);
    RESET();
    C(0,45,2028,2027,1,1);
    NOT();
    RESET();
    /* Country A and B share compId: never combine their standings. */
    C(0,45,2027,2027,0,1);
    C(1,46,2027,2027,0,1);
    NOT();
    RESET();
    /* Duplicate same country and same year is an OG data error. */
    C(0,45,2027,2027,1,1);
    C(1,45,2027,2027,1,1);
    NOT();
    RESET();
    /* Distinct-country conflict can be replaced by exact higher rank. */
    C(0,45,2026,2027,0,1);
    C(1,46,2026,2027,0,1);
    C(2,45,2027,2027,1,1);
    HAS(2);
    RESET();
    /* Historical world 9697 also needs current, not first copied year. */
    C(0,13,1996,1997,1,1);
    C(1,13,1997,1997,1,1);
    HAS(1);
    RESET();
    /* Without certified country year, no source may be invented. */
    C(0,45,2026,0,1,1);
    NOT();
    RESET();
    /* Nonmatching compId or invalid year is not a candidate. */
    C(0,45,2027,2027,0,0);
    C(1,45,0,2027,1,1);
    NOT();
    printf("PASS RG-014 isolated OG viewer edition selector: %u cases\n",
           passed);
    return 0;
}
