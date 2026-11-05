#include <assert.h>
#include <stdio.h>
#include "expanded_player_condition.h"
#include "expanded_squad_layout.h"
int main(void) {
    assert(Expanded_ConditionPerformancePct(0)==70);
    assert(Expanded_ConditionPerformancePct(50)==85);
    assert(Expanded_ConditionPerformancePct(100)==100);
    assert(Expanded_ConditionPerformancePct(255)==100);
    for(unsigned i=0;i<100;i++)
        assert(Expanded_ConditionPerformancePct(i+1)>=Expanded_ConditionPerformancePct(i));
    assert(Expanded_ConditionAdjustedSkill(10,50)==9);
    assert(Expanded_ConditionAdjustedSkill(10,100)==10);
    assert(Expanded_ConditionChange(50,10)==60);
    assert(Expanded_ConditionChange(95,20)==100);
    assert(Expanded_ConditionChange(3,-20)==0);
    assert(Expanded_ConditionChange(50,-11)==39);
    for(unsigned row=0;row<17;row++) {
        int focus=Expanded_SquadRowY(row), popup=Expanded_SquadTooltipY(row);
        assert(popup>=0 && popup+EXP_SQUAD_TOOLTIP_H<=232);
        assert(popup>=focus+EXP_SQUAD_ROW_H ||
               popup+EXP_SQUAD_TOOLTIP_H<=focus);
    }
    puts("PASS 0.10.20: condition 0..100, default 50 => 85% performance, clamp, recovery, fatigue");
    return 0;
}
