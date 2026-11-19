/* Display-only selection: the OG key describes the NEXT fixture,
 * not the current world day. No game time, fixtures or save are modified. */
#ifndef EXPANDED_CAREER_DATE_H
#define EXPANDED_CAREER_DATE_H
#include "expanded_world_time.h"
static inline int Expanded_CareerDate_Select(int isCareer,int hasWorldToday,
    ExpandedWorldDate worldToday,uint16_t seasonYear,uint8_t nextNativeKey,
    ExpandedWorldDate *out)
{
    int32_t dayIndex=0;
    if(!out)return 0;
    if(isCareer) {
        if(!hasWorldToday ||
           !ExpandedWorld_DateToIndex(worldToday,&dayIndex))return 0;
        *out=worldToday;
        return 1;
    }
    return ExpandedWorld_NativePhaseToDate(seasonYear,nextNativeKey,out);
}
#endif
