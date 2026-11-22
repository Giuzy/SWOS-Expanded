/* RG-027: exact ordinary TYPE0 native completion, not cup finished flag. */
#include <stdio.h>
#include <string.h>
#include "expanded_og_type0_completion.h"
static int checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"FAIL case=%d line=%d\n",checks,__LINE__);return 1;}}while(0)
int main(void) {
    ExpandedOGType0ProofRow ok[4]={
      {0,0x1200,6,2,2,2,9,9},{1,0x1201,6,2,2,2,9,9},
      {2,0x1202,6,2,2,2,9,9},{3,0x1203,6,2,2,2,9,9}
    };
    ExpandedOGType0ProofRow x[4];
    CHECK(ExpandedOGType0_CompletedFromOG(4,2,12,ok)); /* no finish flag */
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,11,ok));
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,13,ok));
    CHECK(!ExpandedOGType0_CompletedFromOG(4,0,12,ok));
    CHECK(!ExpandedOGType0_CompletedFromOG(4,5,12,ok));
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,NULL));
    CHECK(!ExpandedOGType0_CompletedFromOG(1,2,12,ok));
    memcpy(x,ok,sizeof x); x[0].games=5;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    memcpy(x,ok,sizeof x); x[0].wins=3;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    memcpy(x,ok,sizeof x); x[0].goals_for++;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    memcpy(x,ok,sizeof x); x[1].club_id=x[0].club_id;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    memcpy(x,ok,sizeof x); x[1].team_index=0;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    memcpy(x,ok,sizeof x); x[1].team_index=-1;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    memcpy(x,ok,sizeof x); x[1].team_index=4;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    memcpy(x,ok,sizeof x); x[1].club_id=UINT16_MAX;
    CHECK(!ExpandedOGType0_CompletedFromOG(4,2,12,x));
    /* A legitimate single-leg ordinary league can also complete. */
    ExpandedOGType0ProofRow single[2]={
      {0,0x2001,1,1,0,0,2,0},{1,0x2002,1,0,0,1,0,2}
    };
    CHECK(ExpandedOGType0_CompletedFromOG(2,1,1,single));
    single[1].goals_against=1;
    CHECK(!ExpandedOGType0_CompletedFromOG(2,1,1,single));
    printf("PASS RG-027 ordinary OG TYPE0 completion: %d assertions\n",checks);
    return 0;
}
