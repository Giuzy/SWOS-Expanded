/* No game engine or CAR: isolated identity+delta checks. */
#include <assert.h>
#include <stdio.h>
#include "expanded_og_league_bridge.h"
int main(void){
    const uint16_t resident[4]={0x101u,0x102u,0x103u,0x104u};
    const int32_t resident_rows[4]={0,1,2,3};
    const uint16_t selected[3]={0x102u,0x103u,0x104u};
    int32_t local_rows[4]={2,-1,0,1};
    uint16_t native[4],rows[4];
    assert(ExpandedOGLeagueBridge_Map(resident,4u,resident_rows,
        selected,3u,0x101u,local_rows,4u,native,rows));
    assert(native[0]==3u&&native[1]==0u&&native[2]==1u&&native[3]==2u);
    ExpandedOGLeagueTotals before[4]={
        {2,1,1,0,3,2,4},{2,0,1,1,1,2,1},{0},{0}
    };
    ExpandedOGLeagueTotals after[4]={
        before[3],before[0],before[1],before[2]
    };
    after[1].played++;after[1].wins++;after[1].for_goals+=2u;
    after[1].against_goals++;after[1].points+=3u;
    after[2].played++;after[2].losses++;after[2].for_goals++;
    after[2].against_goals+=2u;
    assert(ExpandedOGLeagueBridge_CheckStep(before,after,native,
        4u,0u,1u,2u,1u,3u));
    after[3].points++;
    assert(!ExpandedOGLeagueBridge_CheckStep(before,after,native,
        4u,0u,1u,2u,1u,3u));
    local_rows[1]=0;
    assert(!ExpandedOGLeagueBridge_Map(resident,4u,resident_rows,
        selected,3u,0x101u,local_rows,4u,native,rows));
    local_rows[1]=-1;local_rows[3]=-1;
    assert(!ExpandedOGLeagueBridge_Map(resident,4u,resident_rows,
        selected,3u,0x101u,local_rows,4u,native,rows));
    /* Scotland 2026/27: country 33, divisions 1-3 have ten clubs,
     * four meetings and NO stage transitions (SP_TAB[7]==0).
     * The Premiership has a true stage and stays protected. */
    const uint8_t scot_lower[16]={0,33,3,10,4,36,0,0,16,0,16,0,16,0,16,0};
    const uint8_t scot_premier[16]={0,33,0,12,4,33,5,1,44,0,50,0,60,0,40,1};
    const uint8_t scot_bad[16]={0,33,3,12,4,36,0,0,16,0,16,0,16,0,16,0};
    assert(!ExpandedOGLeagueBridge_NeedsStageBridge(scot_lower,10u,4u));
    assert(ExpandedOGLeagueBridge_NeedsStageBridge(scot_premier,12u,4u));
    assert(ExpandedOGLeagueBridge_NeedsStageBridge(scot_bad,10u,4u));
    assert(ExpandedOGLeagueBridge_NeedsStageBridge(scot_lower,10u,2u));
    assert(ExpandedOGLeagueBridge_NeedsStageBridge(NULL,10u,4u));
    /* Fifteen already played games are a valid partial checkpoint, not
     * completed 180-match season and not fixtures to replay. */
    assert(!ExpandedOGLeagueBridge_SeasonComplete(10u,4u,15u,15u));
    assert(ExpandedOGLeagueBridge_SeasonComplete(10u,4u,180u,180u));
    assert(!ExpandedOGLeagueBridge_SeasonComplete(10u,4u,181u,180u));
    /* Windows 00.13.189 diagnostics: both worlds ended at 552/552,
     * followed by a phantom 553rd local match. */
    assert(ExpandedOGLeagueBridge_SeasonComplete(24u,2u,552u,552u));
    assert(!ExpandedOGLeagueBridge_SeasonComplete(24u,2u,553u,552u));
    assert(!ExpandedOGLeagueBridge_SeasonComplete(24u,2u,552u,551u));
    assert(!ExpandedOGLeagueBridge_SeasonComplete(24u,2u,551u,552u));
    assert(!ExpandedOGLeagueBridge_SeasonComplete(24u,2u,0u,0u));
    assert(ExpandedOGLeagueBridge_SeasonComplete(24u,4u,1104u,1104u));
    assert(ExpandedOGLeagueBridge_SeasonComplete(18u,2u,306u,306u));
    assert(!ExpandedOGLeagueBridge_SeasonComplete(24u,1u,276u,276u));
    assert(!ExpandedOGLeagueBridge_SeasonComplete(0u,2u,0u,0u));
    puts("OG_MANAGER_COMPLETED_LEAGUE_NO_EXTRA_FIXTURE=PASS_ISOLATED");
    puts("OG_LEAGUE_BRIDGE_REMAP_AND_SCORE=PASS_ISOLATED");
    puts("OG_SCOTTISH_FOUR_CYCLES_STAGE0_GUARD=PASS_ISOLATED");
    return 0;
}
