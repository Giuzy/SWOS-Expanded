/* SWOS Expanded: world-wide native fixture/roster catalogue.
 * The catalogue is an INPUT STAGE, not a second competition simulator.
 * No results, civil ticks, standings or CAR are written here.
 * Source: original country/club tables, round pairings and OG phase keys.
 * Data with no authoritative pairing/phase remains explicitly unresolved.
 */
#ifndef EXPANDED_UNIVERSAL_WORLD_CATALOG_H
#define EXPANDED_UNIVERSAL_WORLD_CATALOG_H
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_fixture_registry.h"
#include "game_types.h"
#include <stdint.h>

enum {
    EXP_CATALOG_DATE_VERIFIED = 1,
    EXP_CATALOG_DATE_OG_PHASE_FIRST_DAY = 2
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
    uint32_t unverified_native_order; /* OG team seed order not yet witnessed */
    uint8_t sealed, source_complete;
} ExpandedUniversalCatalogAudit;

/* Called ONCE at NEW CAREER START by the existing all-country team importer,
 * never after accepting a job or changing clubs. WORLD_9697 is untouched.
 */
int ExpandedUniversalCatalog_Begin(void);
void ExpandedUniversalCatalog_Reset(void);
int ExpandedUniversalCatalog_AddClub(const TeamFileEx *team);
int ExpandedUniversalCatalog_AddLeague(uint8_t country, uint8_t native_comp_id,
                                       uint8_t division, uint8_t begin_phase,
                                       uint8_t end_phase, uint8_t legs);
int ExpandedUniversalCatalog_AddCup(uint8_t country, uint8_t native_comp_id,
                                    uint8_t native_type);
int ExpandedUniversalCatalog_Seal(void);
int ExpandedUniversalCatalog_Audit(ExpandedUniversalCatalogAudit *out);
const ExpandedUniversalCatalogFixture *ExpandedUniversalCatalog_Fixture(uint32_t i);
TeamFileEx *ExpandedUniversalCatalog_Roster(ExpandedClubKey key);
/* This is deliberately not a CAR activation API: old OG CAREER/TSKC is still
 * resident. Moving these fixtures into CAR requires a single-owner cutover,
 * full KO/qualifier rules and persisted roster state.
 */
#endif
#endif
