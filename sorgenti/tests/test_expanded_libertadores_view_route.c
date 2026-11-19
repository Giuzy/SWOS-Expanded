/* Regression: historical Copa viewer remains observable without enrolment. */
#include "expanded_libertadores_view_route.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    static const int native=1,copy=2,clubSlot=3,unrelated=4;
    /* Unemployed picker, employed spectator, qualified club slot3. */
    assert(Expanded_LibertadoresResidentViewerRoute(4,1,&native,&native,&copy,NULL,0));
    assert(Expanded_LibertadoresResidentViewerRoute(4,1,&copy,&native,&copy,NULL,0));
    assert(Expanded_LibertadoresResidentViewerRoute(4,1,&clubSlot,&native,&copy,&clubSlot,1));
    /* Slot3 of a different cup is not Copa; no modern/classic crossover. */
    assert(!Expanded_LibertadoresResidentViewerRoute(4,1,&clubSlot,&native,&copy,&clubSlot,0));
    assert(!Expanded_LibertadoresResidentViewerRoute(4,1,&unrelated,&native,&copy,NULL,1));
    assert(!Expanded_LibertadoresResidentViewerRoute(4,0,&copy,&native,&copy,NULL,0));
    assert(!Expanded_LibertadoresResidentViewerRoute(2,1,&copy,&native,&copy,NULL,0));
    assert(!Expanded_LibertadoresResidentViewerRoute(4,1,&native,NULL,&copy,NULL,0));
    assert(!Expanded_LibertadoresResidentViewerRoute(4,1,NULL,&native,&copy,NULL,0));
    puts("HISTORIC_LIBERTADORES_VIEW_ROUTE=PASS_ISOLATED_ONLY");
    return 0;
}
