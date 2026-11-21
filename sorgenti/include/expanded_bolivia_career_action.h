/* Pure UX gate for the SAME OG Bolivia manager fixture and civil clock.
 * This classifies a verified event; it never schedules or plays it.
 * Unknown/overdue dates must not be disguised as a playable match.
 */
#ifndef EXPANDED_BOLIVIA_CAREER_ACTION_H
#define EXPANDED_BOLIVIA_CAREER_ACTION_H
#include "expanded_world_time.h"
enum {
    EXP_BOLIVIA_ACTION_BLOCKED=-1,
    EXP_BOLIVIA_ACTION_FORWARD=0,
    EXP_BOLIVIA_ACTION_PLAY=1
};
static inline int ExpandedBoliviaCareer_Action(
    ExpandedWorldDate today,ExpandedWorldDate fixture)
{
    int32_t now=0,day=0;
    if(!ExpandedWorld_DateToIndex(today,&now) ||
       !ExpandedWorld_DateToIndex(fixture,&day) ||
       day<now)return EXP_BOLIVIA_ACTION_BLOCKED;
    return day==now?EXP_BOLIVIA_ACTION_PLAY:
                    EXP_BOLIVIA_ACTION_FORWARD;
}
#endif
