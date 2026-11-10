#include <assert.h>
#include <stdio.h>
#include "expanded_debug.h"
int main(void) {
    uint16_t h=1,a=1;
    assert(ExpandedDebug_ResultMode()==0 && !ExpandedDebug_SackArmed());
    ExpandedDebug_ForceScore(1,&h,&a); assert(h==1 && a==1);
    ExpandedDebug_ToggleWin();
    assert(ExpandedDebug_ResultMode()==1);
    ExpandedDebug_ForceScore(1,&h,&a); assert(h==2 && a==0);
    ExpandedDebug_ForceScore(0,&h,&a); assert(h==0 && a==2);
    ExpandedDebug_ToggleLose();
    assert(ExpandedDebug_ResultMode()==2);
    ExpandedDebug_ForceScore(1,&h,&a); assert(h==0 && a==2);
    ExpandedDebug_ForceScore(0,&h,&a); assert(h==2 && a==0);
    ExpandedDebug_ToggleLose(); assert(ExpandedDebug_ResultMode()==0);
    ExpandedDebug_ToggleSack(); assert(ExpandedDebug_SackArmed());
    ExpandedDebug_ToggleSack(); assert(!ExpandedDebug_SackArmed());
    ExpandedDebug_ToggleSack(); assert(ExpandedDebug_ConsumeSack());
    assert(!ExpandedDebug_SackArmed() && !ExpandedDebug_ConsumeSack());
    puts("OK: debug toggles, mutually exclusive scores, one-shot sacking");
    return 0;
}
