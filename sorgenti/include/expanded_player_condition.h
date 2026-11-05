#ifndef EXPANDED_PLAYER_CONDITION_H
#define EXPANDED_PLAYER_CONDITION_H
#include <stdint.h>
/* At 100% a player has 100% of his nominal performance; at the default
 * 50% he has 85%, and at 0% he still has 70%.  This avoids a punitive
 * half-strength team at the start of a new career. */
static inline unsigned Expanded_ConditionPerformancePct(unsigned condition) {
    if(condition>100) condition=100;
    return 70u+(30u*condition+50u)/100u;
}
static inline unsigned Expanded_ConditionAdjustedSkill(unsigned skill,unsigned condition) {
    return (skill*Expanded_ConditionPerformancePct(condition)+50u)/100u;
}
static inline uint8_t Expanded_ConditionChange(unsigned current,int delta) {
    int next=(int)(current>100?100:current)+delta;
    return (uint8_t)(next<0?0:next>100?100:next);
}
#endif
