#ifndef EXPANDED_FACILITY_RULES_H
#define EXPANDED_FACILITY_RULES_H
#include <stdint.h>
/* Indices match the original eight facility slots, not the visual card order. */
static inline unsigned Expanded_StaffWeekly(unsigned facility,unsigned level) {
    static const uint16_t weekly[8][5]={
      {35,75,145,250,420}, /* stadium maintenance */
      {45,95,190,345,565}, /* trainers */
      {35,85,165,290,480}, /* transport maintenance */
      {30,65,120,220,380}, /* club office staff */
      {35,80,150,265,435}, /* fitness coaches */
      {60,120,225,385,620},/* scouts */
      {65,135,250,430,680},/* medical personnel */
      {40,100,200,365,590} /* academy */
    };
    if(facility>=8) return 0;
    if(level<1) level=1;
    if(level>5) level=5;
    return weekly[facility][level-1];
}
static inline unsigned Expanded_MedicalPct(unsigned level) {
    if(level<1) level=1;
    if(level>5) level=5;
    return level*10u;
}
/* Ticket, fuel and hotel expenses per AWAY fixture; home is zero.
 * Higher comfort means higher cost but less fatigue. */
static inline unsigned Expanded_TravelMatchCost(unsigned level) {
    static const uint16_t price[5]={100,180,320,560,950};
    if(level<1) level=1;
    if(level>5) level=5;
    return price[level-1];
}
static inline unsigned Expanded_AwayFatigue(unsigned level) {
    static const uint8_t cost[5]={14,11,8,5,2};
    if(level<1) level=1;
    if(level>5) level=5;
    return cost[level-1];
}
static inline unsigned Expanded_FitnessRecovery(unsigned level) {
    if(level<1) level=1;
    if(level>5) level=5;
    return 2u*level;
}
static inline unsigned Expanded_TrainingGain(unsigned train,unsigned fitness) {
    if(train<1) train=1;
    if(train>5) train=5;
    if(fitness<1) fitness=1;
    if(fitness>5) fitness=5;
    return 2u*train + fitness;
}
static inline unsigned Expanded_YouthPriceBonus(unsigned level) {
    return level<1?0:level>5?8:(level-1)*2u;
}
static inline unsigned Expanded_OfficeSponsorPct(unsigned level) {
    return 100u+(level>5?5u:level)*8u;
}
/* Results encoded 0=loss, 1=draw, 3=win, neutral games 1.5pt.
 * Form is based on at most five REAL results and subtracts travel fatigue. */
static inline unsigned Expanded_RecentForm(const uint8_t *results,unsigned count,
                                            unsigned fatigue) {
    unsigned total=0;
    if(count>5) count=5;
    for(unsigned i=0;i<count;i++) total+=results[i]>3?3:results[i];
    unsigned pct=count?(total*100u)/(3u*count):50u;
    return fatigue>=pct?0:pct-fatigue;
}
#endif
