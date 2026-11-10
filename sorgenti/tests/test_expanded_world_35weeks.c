#include <assert.h>
#include <stdio.h>
#include "expanded_world_time.h"
#include "expanded_world_ledger.h"
static unsigned result_calls;
static int simulate(const ExpandedWorldFixture *f, uint8_t *h, uint8_t *a, void *unused) {
    (void)unused;
    ++result_calls;
    /* TEST STUB only: production requires the real OpenSWOS simulator. */
    *h=(uint8_t)((f->id*7u+f->home)%4u);
    *a=(uint8_t)((f->id*3u+f->away)%3u);
    return 1;
}
int main(void) {
    ExpandedWorldFixture store[180];
    ExpandedWorldLedger w;
    ExpandedWorldDate start={2026,8,1};
    ExpandedWorldStanding a[4]={{.club=1},{.club=2},{.club=3},{.club=4}};
    ExpandedWorldStanding b[4]={{.club=5},{.club=6},{.club=7},{.club=8}};
    int32_t d0;
    assert(ExpandedWorld_DateToIndex(start,&d0));
    assert(ExpandedWorldLedger_Init(&w,store,180,d0));
    for (uint32_t week=1; week<=35; ++week) {
        for (uint16_t league=0; league<2; ++league) {
            for (uint16_t m=0; m<2; ++m) {
                uint16_t offset=(uint16_t)(league*4);
                uint16_t home=(uint16_t)(offset+m*2+1);
                uint16_t away=(uint16_t)(home+1);
                assert(ExpandedWorldLedger_Add(&w,(ExpandedWorldFixture){
                    .day=d0+(int32_t)week*7,
                    .id=week*2u+m,
                    .competition=(uint16_t)(100+league),
                    .home=home,.away=away}));
            }
        }
    }
    assert(w.count==140 && w.played==0);
    assert(ExpandedWorldLedger_Advance(&w,d0+35*7,simulate,NULL));
    assert(w.played==140 && w.now==d0+35*7 && result_calls==140);
    assert(ExpandedWorldLedger_Standings(&w,100,a,4,3));
    assert(ExpandedWorldLedger_Standings(&w,101,b,4,3));
    for (int t=0; t<4; ++t) assert(a[t].played==35 && b[t].played==35);
    assert(ExpandedWorldLedger_Advance(&w,d0+35*7,simulate,NULL));
    assert(w.played==140 && result_calls==140);
    assert(a[0].played==35);
    puts("OK: 35 weeks, 140 TEST results, standings and immutable history");
    return 0;
}
