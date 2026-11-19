/* SWOS Expanded Competition Coordinator / Phase A (00.13.198).
 * Only one civil clock and one native OG score-producing executor.
 * The coordinator does not simulate a match, draw a cup or run a
 * separate scheduler. Manager fixture ownership and CAR transactions
 * beyond this scope remain OPEN gates; see docs/COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md.
 */
#ifndef EXPANDED_COMPETITION_COORDINATOR_H
#define EXPANDED_COMPETITION_COORDINATOR_H
#include "expanded_world_time.h"
#ifdef SWOS_SDL_BUILD
int Expanded_CompetitionCoordinator_Start(void);
int Expanded_CompetitionCoordinator_Advance(ExpandedWorldDate target,const char *origin);
int Expanded_CompetitionCoordinator_PreflightBind(uint16_t season);
int Expanded_CompetitionCoordinator_ValidateRestore(void);
int Expanded_CompetitionCoordinator_CanSave(void);
int Expanded_CompetitionCoordinator_PreflightFixture(const char *kind);
void Expanded_CompetitionCoordinator_ReportFixtureFault(const char *reason);
int Expanded_CompetitionCoordinator_RecoveryRequired(void);
void Expanded_CompetitionCoordinator_ReportWorldFault(const char *reason);
#endif
#endif
