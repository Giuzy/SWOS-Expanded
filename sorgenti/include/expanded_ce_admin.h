/* v00.13.301: administrative results for temporary TEST clubs.
 * GENERIC and DATA-DRIVEN (data_json/worlds/<w>/competitions/test_clubs.json):
 * a club listed there is a placeholder with a TEST roster (real roster not yet
 * inserted). Its matches are not played by OG; the result comes from a policy
 * written in the same JSON, travels through the SAME CE ledger / scheduler /
 * knockout update / CAR as every other result, and carries the persisted
 * score flag EXP_CE_SCORE_ADMIN_TEST so it can never be mistaken for a match
 * OG really played. Removing a club from the file ends the exception.
 *
 *  - TEST vs real club: the real club wins (policy.test_vs_real goals).
 *  - TEST vs TEST: ONE winner per PAIR chosen deterministically from the
 *    pair and the edition (never from fixture position), so both legs of a
 *    tie have the same winner and no team is locked as "always first".
 * PURE module: no OG, no globals of the game, no I/O. */
#ifndef EXPANDED_CE_ADMIN_H
#define EXPANDED_CE_ADMIN_H
#include <stdint.h>
#include "expanded_universal_fixture_registry.h"

#define EXP_CE_SCORE_ADMIN_TEST 0x04u   /* result assigned by the TEST policy */

void ExpCeAdmin_Reset(void);
/* Reads data_json/worlds/2627/competitions/test_clubs.json (defined in
 * expanded_ce_definitions.c, runtime only). Safe to call repeatedly. */
int ExpCe_LoadTestClubs(uint16_t world);
/* Loads the policy and the TEST club list. Returns the number of clubs, or
 * -1 when the file is malformed (then NOTHING is loaded: no half policy). */
int ExpCeAdmin_LoadJson(const char *json,int len,uint16_t world);
uint32_t ExpCeAdmin_Count(void);
int ExpCeAdmin_IsTest(ExpandedClubKey club);
/* 1: administrative result written to *score (EXP_CE_SCORE_ADMIN_TEST set,
 * *reason names the rule); 0: not an administrative match (real OG result). */
int ExpCeAdmin_Decide(const ExpandedUniversalFixture *fixture,
                      ExpandedFixtureScore *score,const char **reason);
#endif
