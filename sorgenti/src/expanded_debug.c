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
