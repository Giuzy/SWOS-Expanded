#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_midseason_join.h"
/* The no-SDL fallback reads this native career global. The stand-alone
 * calendar unit test intentionally does not link the full game. */
uint8_t _lastFinancialWeekIndex=0;
int main(void) {
    uint8_t week=99;
    assert(Expanded_MidseasonJoin_Cutoff(1996,0,&week) && week==0);
    assert(Expanded_MidseasonJoin_Cutoff(1996,4,&week) && week==0);
    /* Stop at the first phase on/after signing; ceil fractional
     * months to include all earlier fixtures, not just full past phases.
     * Native sortKey: January=0, March=16, August=56; yearly +0x60. */
    assert(Expanded_MidseasonJoin_Cutoff(1996,5,&week) && week==58);
    assert(Expanded_MidseasonJoin_Cutoff(1996,10,&week) && week==67);
    assert(Expanded_MidseasonJoin_Cutoff(1996,35,&week) && week==113);
    assert(Expanded_MidseasonJoin_Cutoff(2026,35,&week) && week==113);
    assert(Expanded_MidseasonJoin_Cutoff(1996,45,&week) && week==131);
    assert(Expanded_MidseasonJoin_Cutoff(1996,51,&week) && week==142);
    assert(!Expanded_MidseasonJoin_Cutoff(1996,52,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,4000,&week));
    assert(!Expanded_MidseasonJoin_Cutoff(1996,10,NULL));
    /* Multi-year hire dates must use the current world season, not 1996.
     * 260 weeks is still JUNE 2001 (2000/01 season); 273 is SEP 2001. */
    uint16_t season=0;
    uint8_t key=0;
    assert(Expanded_MidseasonJoin_AbsoluteCutoff(1996,43,&season,&key));
    assert(season==1996 && key==128);
    assert(Expanded_MidseasonJoin_AbsoluteCutoff(1996,260,&season,&key));
    assert(season==2000 && key>0);
    assert(Expanded_MidseasonJoin_AbsoluteCutoff(1996,261,&season,&key));
    assert(season==2001 && key==0);
    assert(Expanded_MidseasonJoin_AbsoluteCutoff(1996,273,&season,&key));
    assert(season==2001 && key>56);
    assert(!Expanded_MidseasonJoin_AbsoluteCutoff(1996,273,NULL,&key));
    assert(!Expanded_MidseasonJoin_AbsoluteCutoff(1996,273,&season,NULL));
    /* Reproduce the actual July 2001 Australian contract. A northern
     * calendar remains at 0; a league whose OG native rounds begin in
     * January must use the actual July 30 phase, not artificial preseason. */
    assert(Expanded_MidseasonJoin_AbsoluteCutoff(1996,265,&season,&key));
    assert(season==2001 && key==0u);
    assert(Expanded_MidseasonJoin_SouthernCutoff(1996,265,16u,
                                                 &season,&key));
    assert(season==2001 && key==56u);
    assert(!Expanded_MidseasonJoin_SouthernCutoff(1996,265,56u,
                                                  &season,&key));
    assert(!Expanded_MidseasonJoin_SouthernCutoff(1996,273,16u,
                                                  &season,&key));
    assert(!Expanded_MidseasonJoin_SouthernCutoff(1996,265,16u,
                                                  NULL,&key));
    /* The former early-success bug: target 143 but actual 96 on the
     * user's Hartlepool run MUST be rejected, even on season-end menus. */
    assert(!Expanded_MidseasonJoin_HandoffReached(143u,96u));
    assert(!Expanded_MidseasonJoin_HandoffReached(143u,97u));
    assert(Expanded_MidseasonJoin_HandoffReached(143u,143u));
    assert(Expanded_MidseasonJoin_HandoffReached(92u,93u));
    assert(Expanded_MidseasonJoin_HandoffReached(0u,0u));
    puts("OG_CATCHUP_EARLY_FINISH_REJECTION_RUNTIME=PASS");
    puts("SOUTHERN_JULY_NATIVE_PHASE_RUNTIME=PASS");
    puts("OK: native month-phase cutover, including hires after 5 years");
    return 0;
}
