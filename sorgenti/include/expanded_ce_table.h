/* SWOS Expanded — Fase 2: standings and knockout decisions from the ONE CE
 * ledger, under the competition's own rules (expanded_ce_rules.h).
 * Readers only: never writes a score, never simulates. Pure C11.
 *
 * Stage encoding (CE fixture key, also used by the engine):
 *   stage = (phase << 8) | group      group 1..254 for a league/group table,
 *                                     group 0xFF for a knockout phase
 * (stage is never 0: ExpandedFixtureKey_Valid). Existing type0 league
 * fixtures use stage 1 = phase 0, group 1.
 */
#ifndef EXPANDED_CE_TABLE_H
#define EXPANDED_CE_TABLE_H
#include "expanded_ce_rules.h"
#include "expanded_universal_fixture_registry.h"

#define EXP_CE_STAGE(phase,group) ((((uint32_t)(phase))<<8)|(uint32_t)(group))
#define EXP_CE_STAGE_PHASE(stage) ((uint32_t)(stage)>>8)
#define EXP_CE_STAGE_GROUP(stage) ((uint32_t)(stage)&0xffu)
#define EXP_CE_KO_GROUP 0xffu

/* score.flags bits written by the OG adapters (witness). */
#define EXP_CE_SCORE_EXTRA_TIME 0x01u  /* extra time was played */
#define EXP_CE_SCORE_PENALTIES  0x02u  /* shoot-out was played */
#define EXP_CE_SCORE_START_HISTORY 0x08u /* verified pre-START, never an OG witness */
typedef struct ExpCeInitialDeduction { uint32_t club, points; } ExpCeInitialDeduction;
/* Immutable rules of a seeded edition; loaded from the same START JSON
 * on START and CONTINUE. Older unseeded CARs are not retroactively changed. */
void ExpCeTable_HistoryReset(void);
int ExpCeTable_HistoryAdd(ExpandedEditionKey edition,uint32_t played,
    const ExpCeInitialDeduction *deductions,uint32_t count);

typedef struct ExpCeTableRow {
    ExpandedClubKey club;
    uint32_t played,wins,draws,losses,goals_for,goals_against,away_goals_for;
    uint32_t away_wins;            /* v00.13.355 */
    uint32_t points;
    uint32_t rank; /* 1-based after sorting */
} ExpCeTableRow;

/* Optional OG name comparator (EXP_CE_TB_NAME): <0 if a sorts first. */
typedef int (*ExpCeNameCompare)(void *user,ExpandedClubKey a,ExpandedClubKey b);

/* Table of (edition, stage). Rows include every club of the stage, also
 * those that have not played yet. 1 ok, 0 invalid/CLAIMED/overflow. */
int ExpCeTable_Compute(const ExpandedUniversalRegistry *r,
                       ExpandedEditionKey edition,uint32_t stage,
                       const ExpCeTableRules *rules,
                       ExpCeNameCompare name_cmp,void *user,
                       ExpCeTableRow *rows,uint32_t capacity,uint32_t *used);
/* v00.13.355 data hooks of the generic criteria (set by the engine):
 *  access(user, edition, club) -> access-list rank (1 best), 0 unknown;
 *  discipline(user, event_id, &home, &away) -> 1 known points of a played
 *  fixture, 0 unknown (QUICK results carry no cards). */
typedef uint32_t (*ExpCeAccessRank)(void *user,ExpandedEditionKey edition,ExpandedClubKey club);
typedef int (*ExpCeDisciplineOf)(void *user,uint64_t event_id,uint32_t *home,uint32_t *away);
void ExpCeTable_SetHooks(ExpCeAccessRank access,ExpCeDisciplineOf discipline,void *user);
/* 1 = every fixture of (edition, stage) is PLAYED (no SCHEDULED/CLAIMED). */
int ExpCeTable_StageComplete(const ExpandedUniversalRegistry *r,
                             ExpandedEditionKey edition,uint32_t stage,
                             uint32_t *fixtures);

typedef enum ExpCeTieResult {
    EXP_CE_TIE_INVALID = -1,
    EXP_CE_TIE_PENDING = 0,     /* a leg is not played yet */
    EXP_CE_TIE_DECIDED = 1,
    EXP_CE_TIE_UNRESOLVED = 2   /* level although the rules need a winner */
} ExpCeTieResult;
/* Knockout tie (edition, stage, tie): legs from the rules, aggregate, away
 * goals only if the rules enable it, then the penalties stored with the
 * DECIDING leg. home/away of leg 1 define the pair. */
ExpCeTieResult ExpCeTie_Resolve(const ExpandedUniversalRegistry *r,
                                ExpandedEditionKey edition,uint32_t stage,
                                uint32_t tie,const ExpCeMatchRules *rules,
                                ExpandedClubKey *winner,ExpandedClubKey *loser);
/* For the second leg (or a single match) of a tie: what OG must do if the
 * score is level at the end of regulation. first-leg goals are returned
 * from the HOME side's point of view of the match being played. */
int ExpCeTie_MatchContext(const ExpandedUniversalRegistry *r,
                          const ExpandedUniversalFixture *f,
                          const ExpCeMatchRules *rules,
                          uint8_t *decides_tie,
                          uint16_t *home_first_leg_goals,
                          uint16_t *away_first_leg_goals);
#endif
