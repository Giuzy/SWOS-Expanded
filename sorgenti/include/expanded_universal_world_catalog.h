/* SWOS Expanded: JSON-first world-wide fixture/roster catalogue.
 * New Competition Engine generates deterministic club pairings from JSON
 * format and stable club IDs; OG remains ONLY a 50 Hz match executor.
 * No results, civil ticks, standings or CAR are written here yet.
 * v00.13.280: every round gets a real civil day from the CE date planner
 * and league_calendar.json; OG native phases no longer date CE fixtures
 * (EXP_CATALOG_DATE_OG_PHASE_FIRST_DAY kept only as a retired value).
 */
#ifndef EXPANDED_UNIVERSAL_WORLD_CATALOG_H
#define EXPANDED_UNIVERSAL_WORLD_CATALOG_H
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_fixture_registry.h"
#include "game_types.h"
#include <stdint.h>

enum {
    EXP_CATALOG_DATE_VERIFIED = 1,
    EXP_CATALOG_DATE_OG_PHASE_FIRST_DAY = 2, /* retired in v280 */
    EXP_CATALOG_DATE_CE_PLANNER = 3          /* league_calendar.json */
};
typedef struct ExpandedUniversalCatalogFixture {
    ExpandedUniversalFixture fixture;
    uint8_t native_phase;
    uint8_t date_provenance;
    uint8_t past_at_start;
    uint8_t date_collision;
} ExpandedUniversalCatalogFixture;
typedef struct ExpandedUniversalCatalogAudit {
    uint32_t clubs, league_definitions, cup_definitions;
    uint32_t fixtures, future_fixtures, prestart_unresolved;
    uint32_t unsupported_leagues, failed_clubs, bad_pairs;
    uint32_t day_collisions, missing_cup_fixtures;
    uint32_t unverified_native_order; /* retired diagnostic; stays 0 JSON-first */
    uint32_t json_rule_failures,native_rule_mismatches;
    uint8_t sealed, source_complete;
} ExpandedUniversalCatalogAudit;

/* Called ONCE at NEW CAREER START by all-country JSON/TEAM importer,
 * never after accepting a job or changing clubs. WORLD_9697 is untouched.
 */
int ExpandedUniversalCatalog_Begin(void);
void ExpandedUniversalCatalog_Reset(void);
int ExpandedUniversalCatalog_AddClub(const TeamFileEx *team);
/* v284: identity of a roster without touching the catalogue (CAR dry-run). */
int ExpandedUniversalCatalog_KeyOf(const TeamFileEx *team,ExpandedClubKey *out);
int ExpandedUniversalCatalog_AddLeague(uint8_t country, uint8_t native_comp_id,
                                       uint8_t division, uint8_t begin_phase,
                                       uint8_t end_phase, uint8_t legs);
int ExpandedUniversalCatalog_AddCup(uint8_t country, uint8_t native_comp_id,
                                    uint8_t native_type);
int ExpandedUniversalCatalog_Seal(void);
int ExpandedUniversalCatalog_Audit(ExpandedUniversalCatalogAudit *out);
const ExpandedUniversalCatalogFixture *ExpandedUniversalCatalog_Fixture(uint32_t i);
TeamFileEx *ExpandedUniversalCatalog_Roster(ExpandedClubKey key);
uint32_t ExpandedUniversalCatalog_RosterCount(void);
uint32_t ExpandedUniversalCatalog_DivisionClubs(uint8_t country,uint8_t division,
                                                ExpandedClubKey *out,uint32_t cap);
const TeamFileEx *ExpandedUniversalCatalog_RosterAt(uint32_t i);
/* This is deliberately not a CAR activation API: old OG CAREER/TSKC is still
 * resident. Moving these fixtures into CAR requires a single-owner cutover,
 * full KO/qualifier rules and persisted roster state.
 */
#endif
#endif
