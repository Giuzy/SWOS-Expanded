/* RG-014 manager Copa Bolivia: permit exactly ONE temporary -1 identity
 * only while the real native interactive manager callback owns this fixture.
 * World CPU execution never receives -1. Never persist this sentinel.
 * Pure gate shared by original OG league game writer and isolated QA.
 */
#ifndef BOLIVIA_MANAGER_OFFSET_GATE_H
#define BOLIVIA_MANAGER_OFFSET_GATE_H
#include <stdint.h>
static inline int BoliviaManagerFixture_ValidOffsets(
    int32_t first,int32_t second,uint16_t teamCount,int interactive)
{
    const int a=(first>=0 && (uint32_t)first<teamCount) ||
                 (interactive && first==-1);
    const int b=(second>=0 && (uint32_t)second<teamCount) ||
                 (interactive && second==-1);
    if(!a || !b || first==second)return 0;
    /* No sentinel outside the one manager-involved original OG fixture. */
    if(interactive && ((first==-1)==(second==-1)))return 0;
    return 1;
}
#endif
