/* Pure regressions: historical CWC/UEFA viewer identity + edition. */
#include "expanded_classic_cup_view_route.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    static const int cwc=1,cwcCopy=2,uefa=3,uefaCopy=4,slot=5,other=6;
    /* An unemployed picker and a nonparticipating club see the same OG cup. */
    assert(Expanded_ClassicCupResidentViewerRoute(4,1,&cwc,&cwc,&cwcCopy,0,0));
    assert(Expanded_ClassicCupResidentViewerRoute(4,1,&cwcCopy,&cwc,&cwcCopy,0,0));
    assert(Expanded_ClassicCupResidentViewerRoute(4,1,&uefaCopy,&uefa,&uefaCopy,0,0));
    /* Qualified club still reads the resident instead of a duplicate. */
    assert(Expanded_ClassicCupResidentViewerRoute(4,1,&slot,&cwc,&cwcCopy,&slot,1));
    assert(!Expanded_ClassicCupResidentViewerRoute(4,1,&slot,&cwc,&cwcCopy,&slot,0));
    assert(!Expanded_ClassicCupResidentViewerRoute(4,1,&other,&cwc,&cwcCopy,&slot,1));
    assert(!Expanded_ClassicCupResidentViewerRoute(4,0,&cwcCopy,&cwc,&cwcCopy,0,0));
    assert(!Expanded_ClassicCupResidentViewerRoute(2,1,&cwcCopy,&cwc,&cwcCopy,0,0));
    assert(!Expanded_ClassicCupResidentViewerRoute(4,1,0,&cwc,&cwcCopy,0,0));
    assert(Expanded_ClassicCupCurrentEdition(1997,1997,1));
    assert(!Expanded_ClassicCupCurrentEdition(1996,1997,1));
    assert(!Expanded_ClassicCupCurrentEdition(1997,1997,0));
    puts("HISTORIC_CWC_UEFA_VIEW_ROUTE_AND_YEAR=PASS_ISOLATED_ONLY");
    return 0;
}
