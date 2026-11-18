/* Isolated contract regression, NOT a Windows gameplay or CAR roundtrip test.
 * Build with the existing C compiler and -I sorgenti/include. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "team_source.h"
#include "expanded_competition_identity.h"

int main(void)
{
    char path[32];
    ExpandedFixtureLedgerEntry entries[5];
    ExpandedFixtureLedger ledger = { entries, 0u, 5u };
    ExpandedFixtureKey key = { {{WORLD_9697, 1u}, 100u, 0u}, 1u, 1u, 0u, 0u };
    ExpandedFixtureKey next;
    ExpandedFixtureScore score = {2u, 1u, 0u, 0u, 0u};
    ExpandedFixtureScore changed = {9u, 1u, 0u, 0u, 0u};
    ExpandedClubKey club96 = {WORLD_9697, 123u};
    ExpandedClubKey club26 = {WORLD_2627, 123u};
    unsigned i;
    const unsigned files[4] = {12u,13u,38u,73u};
    const char *names[4] = {
        "data/9697/TEAM.012","data/9697/TEAM.013",
        "data/9697/TEAM.038","data/9697/TEAM.073"
    };

    for (i = 0u; i < 4u; ++i) {
        assert(TeamSource_HistoricCareerPath(WORLD_9697, 4u, (uint8_t)files[i],
                                            path, (uint32_t)sizeof path));
        assert(strcmp(path, names[i]) == 0);
    }
    assert(!TeamSource_HistoricCareerPath(WORLD_9697, 2u, 13u, path, sizeof path));
    assert(!TeamSource_HistoricCareerPath(WORLD_2627, 4u, 13u, path, sizeof path));
    assert(!TeamSource_HistoricCareerPath(WORLD_9697, 4u, 14u, path, sizeof path));
    assert(!TeamSource_HistoricCareerPath(WORLD_9697, 4u, 13u, path, 5u));
    assert(ExpandedClubKey_Valid(club96));
    assert(ExpandedClubKey_Valid(club26));
    assert(!ExpandedClubKey_Equal(club96, club26));

    assert(ExpandedFixtureLedger_Schedule(&ledger, key) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Schedule(&ledger, key) == EXP_FIXTURE_BUSY);
    assert(ExpandedFixtureLedger_Finish(&ledger, key, score) == EXP_FIXTURE_BUSY);
    assert(ExpandedFixtureLedger_Begin(&ledger, key) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Begin(&ledger, key) == EXP_FIXTURE_BUSY);
    assert(ExpandedFixtureLedger_Finish(&ledger, key, score) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Schedule(&ledger, key) == EXP_FIXTURE_ALREADY_PLAYED);
    assert(ExpandedFixtureLedger_Begin(&ledger, key) == EXP_FIXTURE_ALREADY_PLAYED);
    assert(ExpandedFixtureLedger_Finish(&ledger, key, score) == EXP_FIXTURE_ALREADY_PLAYED);
    assert(ExpandedFixtureLedger_Finish(&ledger, key, changed) == EXP_FIXTURE_CONFLICT);

    next = key; next.edition.competition.world = WORLD_2627;
    assert(ExpandedFixtureLedger_Schedule(&ledger, next) == EXP_FIXTURE_OK);
    next = key; next.edition.start_day++;
    assert(ExpandedFixtureLedger_Schedule(&ledger, next) == EXP_FIXTURE_OK);
    next = key; next.replay++;
    assert(ExpandedFixtureLedger_Schedule(&ledger, next) == EXP_FIXTURE_OK);
    next = key; next.leg++;
    assert(ExpandedFixtureLedger_Schedule(&ledger, next) == EXP_FIXTURE_OK);
    next = key; next.tie++;
    assert(ExpandedFixtureLedger_Schedule(&ledger, next) == EXP_FIXTURE_FULL);
    assert(ledger.count == 5u);
    puts("POINT2_IDENTITY_AND_CLAIM=PASS_ISOLATED_ONLY");
    return 0;
}
