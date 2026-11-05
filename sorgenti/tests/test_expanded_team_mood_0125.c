#include <assert.h>
#include <stdio.h>
#include "expanded_team_mood.h"
int main(void) {
    assert(Expanded_FaceTier(0)==0 && Expanded_FaceTier(19)==0);
    assert(Expanded_FaceTier(20)==1 && Expanded_FaceTier(39)==1);
    assert(Expanded_FaceTier(40)==2 && Expanded_FaceTier(59)==2);
    assert(Expanded_FaceTier(60)==3 && Expanded_FaceTier(79)==3);
    assert(Expanded_FaceTier(80)==4 && Expanded_FaceTier(100)==4);
    assert(Expanded_TeamPerformancePct(65,65)==100);
    assert(Expanded_TeamPerformancePct(100,100)==120);
    assert(Expanded_TeamPerformancePct(20,20)==80);
    assert(Expanded_TeamPerformancePct(0,0)==80);
    assert(Expanded_TeamPerformancePct(100,30)==100);
    for(unsigned v=0;v<=100;v++) {
        unsigned p=Expanded_TeamPerformancePct(v,v);
        assert(p>=80 && p<=120);
        if(v)assert(p>=Expanded_TeamPerformancePct(v-1,v-1));
    }
    puts("PASS 0.10.25: team morale/form +/-20%, neutral at 65, 5 faces");
}
