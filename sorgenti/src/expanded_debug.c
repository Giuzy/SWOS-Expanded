#include "expanded_debug.h"
/* Always OFF at process start. These debug values are not in the save file. */
static int s_result_mode;
static int s_sack_on_next;
int ExpandedDebug_ResultMode(void) { return s_result_mode; }
void ExpandedDebug_ToggleWin(void) { s_result_mode=(s_result_mode==1)?0:1; }
void ExpandedDebug_ToggleLose(void) { s_result_mode=(s_result_mode==2)?0:2; }
void ExpandedDebug_ToggleDraw(void) { s_result_mode=(s_result_mode==4)?0:4; }
void ExpandedDebug_ToggleSack(void) { s_sack_on_next=!s_sack_on_next; }
int ExpandedDebug_SackArmed(void) { return s_sack_on_next; }
int ExpandedDebug_ConsumeSack(void) {
    int armed=s_sack_on_next;
    s_sack_on_next=0;
    return armed;
}
void ExpandedDebug_ForceScore(int career_home, uint16_t *home, uint16_t *away) {
    if (!home || !away || !s_result_mode) return;
    if (s_result_mode==4) { *home=1; *away=1; return; }
    /* Official match-result record only. The match engine itself is unchanged.
     * Standard SWOS processing applies standings and all career stats. */
    if ((s_result_mode==1) == !!career_home) { *home=2; *away=0; }
    else { *home=0; *away=2; }
}

#ifdef SWOS_SDL_BUILD
#include "expanded_debug_offer_week.h"
#include "expanded_world_live.h"
#include "expanded_world_time.h"
#include "world.h"
#include <stdio.h>
/* Transient developer request, not saved; one civil clock, one OG world. */
extern uint16_t _gameType;
extern uintptr_t _currentMenuPtr;
extern uint8_t _contestMenu[];
extern uint8_t *Expanded_UnemployedCalendarMenu(void);
static ExpandedDebugOfferWeek s_offerWeek;
static int s_debugNativeOfferGeneration;
int ExpandedDebug_OfferWeekArmed(void) { return s_offerWeek.armed!=0; }
void ExpandedDebug_OfferWeekReset(void)
{
    ExpandedDebugOffer_Reset(&s_offerWeek);
    s_debugNativeOfferGeneration=0;
}
int ExpandedDebug_OfferWeekToggle(void)
{
    ExpandedLiveCalendarState live;
    int32_t day=0;
    const uintptr_t unemployed=(uintptr_t)Expanded_UnemployedCalendarMenu();
    if(_gameType!=4 || (_currentMenuPtr!=(uintptr_t)_contestMenu &&
                         _currentMenuPtr!=unemployed) ||
       !ExpandedWorldLive_CalendarState(&live) ||
       !ExpandedWorld_DateToIndex(live.today,&day)) {
        fprintf(stderr,"[debug-offer-week] ARM_BLOCKED "
                       "reason=NO_ACTIVE_CAREER_MENU_OR_VALID_DATE\n");
        return -1;
    }
    const int result=ExpandedDebugOffer_Toggle(
        &s_offerWeek,(uint16_t)World_Current(),day);
    fprintf(stderr,"[debug-offer-week] %s world=%s "
        "today=%04u-%02u-%02u due-index=%ld "
        "source=ONE_CIVIL_CLOCK no-CAR=1\n",
        result==1?"ARMED":result==0?"CANCELLED":"INVALID",
        World_Tag(),(unsigned)live.today.year,(unsigned)live.today.month,
        (unsigned)live.today.day,(long)s_offerWeek.due);
    return result;
}
/* Call ONLY after a successfully committed OG/civil forward and when
 * the real contest/unemployed UI can safely publish an OG offer. */
int ExpandedDebug_OfferWeekTake(void)
{
    ExpandedLiveCalendarState live;
    int32_t now=0;
    if(!s_offerWeek.armed || _gameType!=4 ||
       !ExpandedWorldLive_CalendarState(&live) ||
       !ExpandedWorld_DateToIndex(live.today,&now))return 0;
    return ExpandedDebugOffer_Take(
        &s_offerWeek,(uint16_t)World_Current(),now);
}
void ExpandedDebug_OfferNativeForce(int enabled)
{
    s_debugNativeOfferGeneration=!!enabled;
}
int ExpandedDebug_OfferNativeForced(void)
{
    return s_debugNativeOfferGeneration;
}
#endif /* SWOS_SDL_BUILD */
