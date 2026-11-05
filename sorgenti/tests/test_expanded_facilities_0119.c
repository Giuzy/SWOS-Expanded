#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_facility_rules.h"
int main(void) {
    unsigned prev=0;
    for(unsigned f=0;f<8;f++) {
        prev=0;
        for(unsigned lv=1;lv<=5;lv++) {
            unsigned cost=Expanded_StaffWeekly(f,lv);
            assert(cost>prev); prev=cost;
        }
    }
    for(unsigned lv=1;lv<=5;lv++) {
        assert(Expanded_MedicalPct(lv)==lv*10);
        assert(Expanded_FitnessRecovery(lv)==2*lv);
        if(lv>1) assert(Expanded_AwayFatigue(lv)<Expanded_AwayFatigue(lv-1));
        if(lv>1) assert(Expanded_YouthPriceBonus(lv)>Expanded_YouthPriceBonus(lv-1));
        if(lv>1) assert(Expanded_TrainingGain(lv,lv)>Expanded_TrainingGain(lv-1,lv-1));
    }
    uint8_t last5[]={3,3,1,0,3};
    assert(Expanded_RecentForm(last5,5,0)==66);
    assert(Expanded_RecentForm(last5,5,15)==51);
    assert(Expanded_RecentForm(last5,5,70)==0);
    assert(Expanded_RecentForm(last5,0,0)==50);
    puts("PASS: 8 facilities have increasing wages, 10-50% medical, training/fitness, travel, youth, last-five form");
    return 0;
}
