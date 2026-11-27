#ifndef EXPANDED_CE_CALENDAR_VALIDATOR_H
#define EXPANDED_CE_CALENDAR_VALIDATOR_H
/* Independent CE calendar validator (v00.13.354).
 *
 * Reads fixtures ONLY (never the generator's counters) and compares them
 * with expectations derived from the regulation: participants, cycles,
 * extra rounds, limits. It never writes a fixture, a date or a result and
 * never repairs anything: it reports, per criterion ID of the 50-criteria
 * catalogue (docs/CE_CALENDAR_GENERATOR.md), PASS / FAIL / N/A / UNKNOWN
 * with the expected and the found value. Pure C, no OG/SDL/CE state. */
#include <stdint.h>
#include <stddef.h>
#include "expanded_universal_fixture_registry.h"

#define EXP_CEV_CRITERIA 50u

typedef enum ExpCevStatus {
    EXP_CEV_NA = 0,        /* criterion not applicable / not checked here */
    EXP_CEV_PASS = 1,
    EXP_CEV_FAIL = 2,
    EXP_CEV_UNKNOWN = 3    /* regulation data missing: never a PASS */
} ExpCevStatus;

typedef struct ExpCevItem {
    uint8_t status, hard;  /* hard: 1 HARD violation blocks publication */
    int32_t expected, found;
} ExpCevItem;

typedef struct ExpCevReport {
    ExpCevItem c[EXP_CEV_CRITERIA + 1u];   /* index = criterion ID 1..50 */
    uint32_t fixtures, hard_failures;
    uint64_t first_bad_event;               /* reproduction hint */
} ExpCevReport;

/* Expected shape of one round-robin stage (group), from the regulation. */
typedef struct ExpCevRoundRobin {
    const uint32_t *clubs;      /* stable_club of the participants */
    uint32_t nclubs;
    uint8_t cycles;             /* meetings per pair (1..4) */
    uint8_t extra_rounds;       /* extra rounds (each club plays one more) */
    uint8_t min_return_gap;     /* 04: logical rounds, 0 = not regulated */
    uint8_t max_run;            /* 07: max consecutive H or A, 0 = not regulated */
    uint8_t mirror;             /* 03: 0 not regulated, 1 MIRROR, 2 ASYMMETRIC */
} ExpCevRoundRobin;

void ExpCev_Init(ExpCevReport *r);
/* 01 02 03 04 05 06 07 27(stage) on the fixtures of ONE stage (all others
 * in `f` are ignored): stage = ExpandedFixtureKey.stage of the group. */
void ExpCev_RoundRobin(const ExpandedUniversalFixture *f, uint32_t n,
                       ExpandedEditionKey edition, uint32_t stage,
                       const ExpCevRoundRobin *exp, ExpCevReport *r);
/* 27 28 across EVERY competition: one fixture per club per civil day, and
 * the minimum civil-day distance between two fixtures of the same club
 * (min_rest 0 = only measured, reported as found). Unique event ids. */
void ExpCev_Global(const ExpandedUniversalFixture *f, uint32_t n,
                   uint32_t min_rest, ExpCevReport *r);
/* 15: fixtures per weekday inside each round (same stage, same logical
 * round = same `leg` cycle and tie group of `round_size` fixtures is NOT
 * assumed): counts[7] expected per round weekday Monday=0..Sunday=6 for
 * rounds whose fixtures are all within [anchor-3, anchor+3]. Simple form:
 * the caller passes the fixtures of one round. */
void ExpCev_DayProfile(const ExpandedUniversalFixture *f, uint32_t n,
                       const uint8_t expected[7], ExpCevReport *r);
const char *ExpCev_StatusName(uint8_t s);
/* One concise log line per non-N/A criterion; returns hard failures. */
uint32_t ExpCev_Log(const ExpCevReport *r, const char *tag);
#endif
