/* Real world-wide 2627 importer kernel test with native OG pair table.
 * Mock world/file storage only. NO OG 50Hz, CAR disk, or live career.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "expanded_universal_world_catalog.h"
#include "world.h"

static WorldId s_world=WORLD_2627;
WorldId World_Current(void){return s_world;}
const char *World_Tag(void){return s_world==WORLD_2627?"2627":"9697";}
uint16_t World_FirstYear(void){return s_world==WORLD_2627?2026u:1996u;}
uint8_t World_FileCountry(uint8_t file){return file;}
uint16_t World_Slot(uint8_t file,uint8_t ordinal)
{(void)file;return ordinal;}
uint16_t World_SlotTeam(uint8_t country,uint16_t slot)
{return (uint16_t)(((uint16_t)country<<8)|slot);}
static const uint8_t pairs4[12]={
    0,1, 2,3,
    0,2, 1,3,
    0,3, 1,2
};
const uint8_t *_fixtureSchedulePtrTable[24]={
    [2]=pairs4
};
int main(void)
{
    ExpandedUniversalCatalogAudit audit={0};
    TeamFileEx t={0};
    const ExpandedUniversalCatalogFixture *f;
    ExpandedClubKey club={WORLD_2627,(45u<<16)|1u};
    ExpandedWorldDate first;
    int32_t expected_day;
    assert(ExpandedUniversalCatalog_Begin());
    for(unsigned i=0;i<4u;++i) {
        memset(&t,0,sizeof t);
        t.team.countryNumber=45u;
        t.team.teamOrdinal=(uint8_t)i;
        t.team.league=0u;
        t.players[0].price=(int8_t)(i+1u);
        assert(ExpandedUniversalCatalog_AddClub(&t));
    }
    memset(&t,0,sizeof t); /* importer file buffer is now overwritten */
    assert(ExpandedUniversalCatalog_Roster(club)!=NULL);
    assert(ExpandedUniversalCatalog_Roster(club)->players[0].price==1);
    assert(ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(!ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(ExpandedUniversalCatalog_AddCup(45u,164u,1u));
    assert(ExpandedUniversalCatalog_Seal());
    assert(ExpandedUniversalCatalog_Audit(&audit));
    assert(audit.clubs==4u && audit.league_definitions==1u);
    assert(audit.fixtures==12u && audit.future_fixtures==12u);
    assert(audit.unsupported_leagues==0u && audit.day_collisions==0u);
    assert(audit.cup_definitions==1u && audit.missing_cup_fixtures==1u);
    assert(audit.unverified_native_order==1u && !audit.source_complete);
    f=ExpandedUniversalCatalog_Fixture(0u);
    assert(f && f->fixture.key.edition.competition.world==WORLD_2627);
    assert(f->date_provenance==EXP_CATALOG_DATE_OG_PHASE_FIRST_DAY);
    assert(f->native_phase==56u && f->past_at_start==0u);
    assert(ExpandedWorld_NativePhaseToDate(2026u,56u,&first));
    assert(ExpandedWorld_DateToIndex(first,&expected_day));
    assert(f->fixture.day_index==expected_day);
    assert(ExpandedUniversalCatalog_Fixture(12u)==NULL);
    s_world=WORLD_9697;
    assert(ExpandedUniversalCatalog_Begin()); /* no interference with 9697 */
    assert(!ExpandedUniversalCatalog_Audit(&audit));
    ExpandedUniversalCatalog_Reset();
    puts("UNIVERSAL_2627_CATALOG_C11=PASS isolated, native OG pairs/dates, stable rosters, unknown KO");
    puts("OG_50HZ_AND_CAR_CUTOVER=NOT_TESTED");
    return 0;
}
