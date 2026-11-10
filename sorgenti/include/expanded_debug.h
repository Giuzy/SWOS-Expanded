#ifndef EXPANDED_DEBUG_H
#define EXPANDED_DEBUG_H
/* Development test toggles. Not saved and do not alter match physics. */
#include "swos_types.h"
int ExpandedDebug_ResultMode(void); /* 0 normal, 1 force win, 2 force loss */
void ExpandedDebug_ToggleWin(void);
void ExpandedDebug_ToggleLose(void);
void ExpandedDebug_ToggleSack(void);
int ExpandedDebug_SackArmed(void);
int ExpandedDebug_ConsumeSack(void);
void ExpandedDebug_ForceScore(int career_home, uint16_t *home, uint16_t *away);
#endif
