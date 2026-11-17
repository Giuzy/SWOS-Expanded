/* Regression: employed VIEW WORLD must not depend on club enrolment. */
#include "expanded_champions_view_route.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    static const int native=1, copy=2, unrelated=3;
    /* Same original tournament from world picker before/after hiring. */
    assert(Expanded_ChampionsResidentViewerRoute(4,1,&native,&native,&copy));
    /* The career dispatcher may have mapped the record to its copy. */
    assert(Expanded_ChampionsResidentViewerRoute(4,1,&copy,&native,&copy));
    /* The route must not capture other competitions or classic/modern. */
    assert(!Expanded_ChampionsResidentViewerRoute(4,1,&unrelated,&native,&copy));
    assert(!Expanded_ChampionsResidentViewerRoute(4,1,NULL,&native,&copy));
    assert(!Expanded_ChampionsResidentViewerRoute(4,0,&native,&native,&copy));
    assert(!Expanded_ChampionsResidentViewerRoute(2,1,&native,&native,&copy));
    puts("historic Champions resident route: PASS (original/copy, no modern crossover)");
    return 0;
}
