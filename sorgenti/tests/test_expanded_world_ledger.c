#include <assert.h>
#include <stdio.h>
#include "expanded_world_time.h"
#include "expanded_world_ledger.h"
static int calls;
static int result(const ExpandedWorldFixture *f, uint8_t *h, uint8_t *a, void *u) {
    (void)u;
    ++calls;
    *h=f->home==1 ? 2 : 0;
    *a=1;
    return 1;
}
static int fail(const ExpandedWorldFixture *f, uint8_t *h, uint8_t *a, void *u) {
    (void)f; (void)h; (void)a; (void)u;
    return 0;
}
int main(void) {
    ExpandedWorldDate d={2026,8,1};
    ExpandedWorldFixture storage[16];
    ExpandedWorldLedger l;
    ExpandedWorldStanding tab[3]={{.club=1},{.club=2},{.club=3}};
    int32_t start;
    assert(ExpandedWorld_DateToIndex(d,&start));
    assert(ExpandedWorldLedger_Init(&l,storage,16,start));
    assert(ExpandedWorldLedger_Add(&l,(ExpandedWorldFixture){
        .day=start+7,.id=2,.competition=14,.home=2,.away=3}));
    assert(ExpandedWorldLedger_Add(&l,(ExpandedWorldFixture){
        .day=start+2,.id=1,.competition=14,.home=1,.away=2}));
    assert(ExpandedWorldLedger_Add(&l,(ExpandedWorldFixture){
        .day=start+3,.id=1,.competition=15,.home=1,.away=3}));
    assert(!ExpandedWorldLedger_Add(&l,(ExpandedWorldFixture){
        .day=start+9,.id=1,.competition=14,.home=2,.away=3}));
    assert(l.fixtures[0].day==start+2 && l.fixtures[1].day==start+3);
    assert(ExpandedWorldLedger_Advance(&l,start+2,result,NULL));
    assert(l.played==1 && l.now==start+2 && calls==1);
    assert(ExpandedWorldLedger_Advance(&l,start+2,result,NULL));
    assert(l.played==1 && calls==1);
    assert(!ExpandedWorldLedger_Advance(&l,start+7,fail,NULL));
    assert(l.played==1 && l.now==start+3);
    assert(ExpandedWorldLedger_Advance(&l,start+7,result,NULL));
    assert(l.played==3 && calls==3);
    assert(ExpandedWorldLedger_NextPending(&l)==UINT32_MAX);
    assert(!ExpandedWorldLedger_Advance(&l,start+6,result,NULL));
    assert(ExpandedWorldLedger_Standings(&l,14,tab,3,3));
    assert(tab[0].played==1 && tab[0].wins==1 && tab[0].points==3);
    assert(tab[1].played==2 && tab[1].lost==2 && tab[1].drawn==0);
    assert(tab[2].played==1 && tab[2].wins==1);
    assert(l.fixtures[0].played && l.fixtures[0].goals_home==2 && l.fixtures[0].goals_away==1);
    {
        ExpandedWorldFixture input[2];
        ExpandedWorldLedger playing;
        assert(ExpandedWorldLedger_Init(&playing,input,2,start));
        assert(ExpandedWorldLedger_Add(&playing,(ExpandedWorldFixture){
            .day=start+5,.id=9,.competition=14,.home=1,.away=2}));
        assert(!ExpandedWorldLedger_Advance(&playing,start+5,fail,NULL));
        assert(playing.now==start+5 && playing.played==0);
        assert(ExpandedWorldLedger_Record(&playing,14,9,4,1));
        assert(!ExpandedWorldLedger_Record(&playing,14,9,5,1));
        assert(ExpandedWorldLedger_Advance(&playing,start+6,result,NULL));
        assert(playing.played==1 && input[0].goals_home==4);
    }
    {
        ExpandedWorldStanding table2[3]={{.club=1},{.club=2},{.club=3}};
        assert(ExpandedWorldLedger_Standings(&l,14,table2,3,2));
        assert(table2[0].points==2 && table2[2].points==2);
    }
    puts("OK: ordered fixtures, unique IDs, no replay, standings, 2/3 points");
    return 0;
}
