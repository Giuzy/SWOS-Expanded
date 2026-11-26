/* SWOS Expanded — Fase 2: universal, versioned competition rules.
 *
 * ONE rules model for every competition of every world (type0 leagues,
 * type1 cups, type2 multi-phase). Pure C11: no SDL, no OG globals, so the
 * same code is used by the runtime CE and by the isolated tests.
 *
 * Two families of rules:
 *  - table rules  (points, ordered tie-breakers) for league/group phases;
 *  - match rules  (legs, winner required, extra time, penalties, away goals,
 *                  bench size, substitutions, substitution windows, half-time
 *                  and extra-time allowances, OG discipline mode).
 *
 * OG mapping (V, source audit 23/11/2026):
 *   bench       -> InitializeInGameTeamsAndStartGame maxSubs -> _gameMaxSubstitutes
 *   subs        -> InitializeInGameTeamsAndStartGame minSubs -> _maxSubsPerTeam
 *   discipline  -> paramD3 (competition byte 0x3C): 1 domestic, 0 continental/
 *                  national (checkEuropeanCupsAndClear3C); player_injury_status
 *   extra_time  -> _extraTimeState=1, penalties -> _penaltiesState=1,
 *   away_goals  -> _playing2ndGame=1 on the second leg (game_state.c).
 * Native league record byte subsAndMaxSubs: hi nibble subs, lo nibble bench
 * (0x35 = 3 from 5). Windows/half-time/extra-time allowances do not exist in
 * OG: they are applied by a CE-only adapter during CE matches.
 *
 * Nothing here invents a rule: a missing value is MISSING (validator error),
 * never a European default. The OG native provenance is used only where the
 * value really comes from the OG record of that competition.
 */
#ifndef EXPANDED_CE_RULES_H
#define EXPANDED_CE_RULES_H
#include <stddef.h>
#include <stdint.h>

#define EXP_CE_MAX_TIEBREAK 10u
/* OG roster: 16 players, 11 starters -> at most 5 on the bench. */
#define EXP_CE_OG_MAX_BENCH 15u   /* v322: 11+15 match squad (extended bench, RG-MATCH-BENCH-EXT) */

typedef enum ExpCeProvenance {
    EXP_CE_PROV_MISSING = 0,
    EXP_CE_PROV_OG_NATIVE = 1,      /* the OG record of this competition */
    EXP_CE_PROV_JSON_VERIFIED = 2,  /* JSON with a verified real source */
    EXP_CE_PROV_JSON_TEMPLATE = 3   /* JSON game template (not official) */
} ExpCeProvenance;

typedef enum ExpCeTieBreak {
    EXP_CE_TB_END = 0,
    EXP_CE_TB_POINTS,
    EXP_CE_TB_GOAL_DIFF,
    EXP_CE_TB_GOALS_FOR,
    EXP_CE_TB_WINS,
    EXP_CE_TB_AWAY_GOALS_FOR,
    EXP_CE_TB_H2H_POINTS,      /* mini-table among the tied clubs only */
    EXP_CE_TB_H2H_GOAL_DIFF,
    EXP_CE_TB_H2H_GOALS_FOR,
    EXP_CE_TB_H2H_AWAY_GOALS,
    EXP_CE_TB_NAME,            /* OG: alphabetical club name (native sort) */
    EXP_CE_TB_LOT,             /* deterministic game draw (CE, documented) */
    EXP_CE_TB_COUNT
} ExpCeTieBreak;

typedef struct ExpCeTableRules {
    uint8_t win, draw, loss;
    /* v309: drawn match decided on penalties (draw_penalties=1): the winner
     * of the shoot-out gets draw_pen_win points, the loser draw_pen_loss; the
     * regulation result is still a draw. 0 = ordinary draw points. */
    uint8_t draw_penalties, draw_pen_win, draw_pen_loss;
    uint8_t tiebreak[EXP_CE_MAX_TIEBREAK];
    uint8_t ntb;
    uint8_t provenance;
} ExpCeTableRules;

typedef struct ExpCeMatchRules {
    uint8_t legs;            /* 1 single match, 2 home/away aggregate */
    uint8_t needs_winner;    /* knockout tie: a level result is not final */
    uint8_t extra_time;      /* after a level single match / level aggregate */
    uint8_t penalties;       /* after extra time (or regulation if no ET) */
    uint8_t away_goals;      /* aggregate tie-break BEFORE extra time */
    /* v309: a tie still level after the played decision (aggregate, extra
     * time) goes to the club with the better REGULAR-SEASON rank. By
     * construction that club hosts the deciding leg (higher_home), so no
     * table is read. Replaces penalties as terminal decider (Serie B play-off). */
    /* 1 = rank decides a level tie; 2 = rank decides UNLESS the two clubs finished the
     * regular season level on points, then the deciding match goes to extra time and
     * penalties (decided by the CE before the match, from the regular table) */
    uint8_t rank_tiebreak;
    uint8_t bench;           /* players on the bench (OG maxSubs) */
    uint8_t subs;            /* substitutions allowed (OG minSubs) */
    uint8_t windows;         /* 0 = no window limit (OG behaviour) */
    uint8_t halftime_free;   /* a half-time session does not use a window */
    uint8_t et_extra_subs;   /* additional substitutions in extra time */
    uint8_t et_extra_windows;/* additional windows in extra time */
    uint8_t discipline;      /* OG paramD3: 1 domestic, 0 continental */
    uint8_t provenance;
} ExpCeMatchRules;

/* Validation errors name the field; `where` is caller text (file/path). */
typedef struct ExpCeRuleError {
    char field[48];
    char reason[96];
} ExpCeRuleError;

/* OG native: the values encoded in the OG league record. */
int ExpCeRules_FromOgLeague(uint8_t points_for_win, uint8_t subs,
                            uint8_t bench, ExpCeTableRules *table,
                            ExpCeMatchRules *match);
/* OG native cup round flags (roundLegs/roundExtraTime/roundPenalties):
 * value 1 = always, >1 = only on replay (not supported -> error). */
int ExpCeRules_FromOgCupRound(uint8_t subs, uint8_t bench, uint8_t legs,
                              uint8_t extra_time, uint8_t penalties,
                              uint8_t away_goals_mode, uint8_t discipline,
                              ExpCeMatchRules *match, ExpCeRuleError *err);

int ExpCeRules_ValidateTable(const ExpCeTableRules *t, ExpCeRuleError *err);
int ExpCeRules_ValidateMatch(const ExpCeMatchRules *m, ExpCeRuleError *err);

/* JSON objects (mjson). `base` (may be NULL) provides inherited values for
 * phase overrides: only keys present in the object change the copy. A key
 * absent with base==NULL stays MISSING and fails validation. */
int ExpCeRules_ParseTable(const char *json, int len,
                          const ExpCeTableRules *base,
                          ExpCeTableRules *out, ExpCeRuleError *err);
int ExpCeRules_ParseMatch(const char *json, int len,
                          const ExpCeMatchRules *base,
                          ExpCeMatchRules *out, ExpCeRuleError *err);
int ExpCeRules_TieBreakFromName(const char *name, uint8_t *out);
const char *ExpCeRules_TieBreakName(uint8_t tb);

/* Stable 64-bit fingerprint of a rule set (edition snapshot/version). */
uint64_t ExpCeRules_Hash(const ExpCeTableRules *t, const ExpCeMatchRules *m);
#endif
