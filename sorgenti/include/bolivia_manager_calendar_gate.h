/* Copa Bolivia career: purely chronological gate for a REAL OG event.
 * This is NOT a match generator or calendar owner: caller passes two dates
 * obtained from the native resident fixture and current career keycache.
 * -2 overdue (cannot replay); -1 same-day conflict/uncertified collision;
 *  0 native manager league first; 1 OG Copa fixture first.
 * Same-day rule is deliberately fail-closed pending the global date audit.
 */
#ifndef BOLIVIA_MANAGER_CALENDAR_GATE_H
#define BOLIVIA_MANAGER_CALENDAR_GATE_H
#include <stdint.h>
static inline int BoliviaManagerCalendar_Gate(
    int32_t today,int32_t cup,int hasNative,int32_t native)
{
    if(today>cup)return -2;
    if(hasNative && native==cup)return -1;
    if(hasNative && native<cup)return 0;
    return 1;
}
#endif
