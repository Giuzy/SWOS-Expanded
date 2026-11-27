#ifndef EXPANDED_FIFA_RANKING_H
#define EXPANDED_FIFA_RANKING_H
/* FIFA men's national team ranking, WORLD 2627 only (never 9697).
 *
 * One process-wide state, owned by the CE result path:
 *  - seed: data_json/worlds/2627/rankings/fifa_nations.json (user-approved
 *    1 July 2026 projection, 211 nations, raw double precision);
 *  - updates: ExpFifa_Apply() is called ONLY from the single CE completion
 *    writer (expanded_universal_car.c), same path for CPU QUICK / PLAY /
 *    COACH; no simulator, calendar, clock or result writer of its own;
 *  - anti double counting: every applied CE event id is remembered and
 *    persisted in the same atomic CAR as the fixture result;
 *  - week: Monday-based CE civil week (day_index / 7, ExpandedWorld_Weekday),
 *    rolled LAZILY: the first result of a new week freezes the reference
 *    points/positions (identical to a rollover at the boundary, because
 *    nothing can change the ranking in between). Readers derive zero
 *    weekly deltas when the stored week is older: viewing never mutates;
 *  - draw snapshots: frozen points of the entrants of one draw, keyed by
 *    edition + phase, reused for every later evaluation of that draw.
 * Points are never reset at a year or season change. */
#include <stdint.h>
#include <stddef.h>
#include "expanded_ce_ranking.h"

#define EXP_FIFA_NATIONS 211u
#define EXP_FIFA_FLAG_W 12
#define EXP_FIFA_FLAG_H 8
#define EXP_FIFA_MAX_SNAPSHOTS 256u
#define EXP_FIFA_SNAPSHOT_CLUBS 128u

typedef struct ExpFifaNation {
    char code[4];                  /* FIFA trigramme, stable external key */
    char name[24];                 /* game national DB name (2627 files) */
    uint32_t stable_club;          /* NATIONAL participant key: (file<<8|ord)+1 */
    uint16_t seed_index;           /* order in the seed (tie break) */
    double points, week_points;    /* current / at the start of week_id */
    uint16_t week_pos;             /* position at the start of week_id */
    char flag[EXP_FIFA_FLAG_W*EXP_FIFA_FLAG_H+1]; /* colour letters */
} ExpFifaNation;

typedef struct ExpFifaViewRow {
    const ExpFifaNation *nation;
    uint16_t pos;                  /* current shared position (1..211) */
    int16_t move;                  /* >0 places gained this week, <0 lost */
    double week_delta;             /* points - week_points, this week only */
} ExpFifaViewRow;

typedef struct ExpFifaUndo {
    uint8_t valid;
    int idx_a, idx_b;
    double old_a, old_b, old_wa, old_wb;
    uint16_t old_wpos[EXP_FIFA_NATIONS];
    int32_t old_week;
    uint32_t old_events;
} ExpFifaUndo;

/* ---- state ---- */
void ExpFifa_Reset(void);
int ExpFifa_Active(void);
/* Parse the seed JSON (schema 1, world 2627, 211 rows). 1 ok (state replaced),
 * 0 malformed (state untouched). */
int ExpFifa_LoadSeedJson(const char *json, int len, int32_t start_day);
/* Load the seed file from data_json (2627 only). */
int ExpFifa_LoadSeedFile(int32_t start_day);
uint32_t ExpFifa_Count(void);
int ExpFifa_FindClub(uint32_t stable_club);           /* index or -1 */
const ExpFifaNation *ExpFifa_Nation(uint32_t index);
int32_t ExpFifa_WeekId(void);
uint32_t ExpFifa_EventCount(void);
int ExpFifa_EventApplied(uint64_t event_id);

/* Read-only ranking view at civil day `today` (rank order, 0-based). */
int ExpFifa_ViewRow(uint32_t rank, int32_t today, ExpFifaViewRow *out);

/* ---- one eligible A-match (both teams use PRE-match points) ----
 * 1 applied, 0 already applied (event id), -1 invalid input. */
int ExpFifa_Apply(uint64_t event_id, int32_t day, uint32_t club_a,
                  uint32_t club_b, int importance, int protect,
                  ExpCeRankResult result, ExpFifaUndo *undo);
void ExpFifa_Undo(const ExpFifaUndo *undo);
/* Score of one match -> FIFA result. A shoot-out counts (0.75/0.5) only when
 * THIS match ended level (single match or a drawn second leg); a non-drawn
 * leg decided on aggregate penalties keeps its own win/loss. */
ExpCeRankResult ExpFifa_ResultFromScore(unsigned home, unsigned away,
                                        unsigned pen_home, unsigned pen_away);
/* Allowed FIFA importance (5,10,15,25,35,40,50,60). */
int ExpFifa_ImportanceValid(int importance);

/* ---- draw snapshot: sort clubs (best first) by frozen points ----
 * Key = edition (competition, start_day, sequence) + phase. Frozen at the
 * first call; later calls reuse it (no later ranking change alters it).
 * 1 sorted, 0 a club is not a ranked nation / capacity. */
int ExpFifa_SeedOrder(uint32_t competition, int32_t start_day, uint16_t sequence,
                      uint32_t phase, int32_t day, uint32_t *clubs, uint32_t n);
uint32_t ExpFifa_SnapshotCount(void);

/* ---- persistence (same atomic CAR, "fifa_ranking":"FR01:...") ---- */
char *ExpFifa_Encode(void);                            /* malloc'd, NULL if inactive */
int ExpFifa_Validate(const char *text);                /* dry run */
int ExpFifa_Decode(const char *text);                  /* transactional */
#endif
