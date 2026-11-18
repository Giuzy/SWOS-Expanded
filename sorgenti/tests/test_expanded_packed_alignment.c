/* Regression for alignment-safe access, independent of CAR/OG simulation.
 * Uses real game_types.h ABI asserts; run with the project's existing C compiler.
 * No World fixture is advanced; this cannot certify Windows gameplay. */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "game_types.h"

_Static_assert(_Alignof(DIYCompetitionEx) >= _Alignof(DIYTeamStats),
               "standings require aligned parent");
_Static_assert(offsetof(DIYCompetitionEx,leagueStandings) %
                   _Alignof(DIYTeamStats) == 0,
               "standings must start at aligned offset");

int main(void)
{
    DIYCompetitionEx live = {0};
    DIYTeamStats *r = (DIYTeamStats *)((uint8_t *)&live +
        offsetof(DIYCompetitionEx,leagueStandings) +
        (size_t)3 * sizeof(DIYTeamStats));
    r->teamSelOffset = 73;
    r->gamesPlayed = 12;
    assert(live.leagueStandings[3].teamSelOffset == 73);
    assert(live.leagueStandings[3].gamesPlayed == 12);

    SeasonInformations season = {0};
    uint8_t name[] = { 'S','W','O','S',0 };
    uint8_t *p = name, *q = NULL;
    memcpy((uint8_t *)&season + offsetof(SeasonInformations,strPtr2),
           &p, sizeof p);
    memcpy(&q, (const uint8_t *)&season +
               offsetof(SeasonInformations,strPtr2), sizeof q);
    assert(q == name);

    CompetitionLeague league;
    memset(&league, 0, sizeof league);
    char club[] = "LEAGUE";
    char *names[2] = {club, club};
    memcpy((uint8_t *)&league + offsetof(CompetitionLeague,name),
           names, sizeof names);
    char *out[2] = {0,0};
    memcpy(out, (const uint8_t *)&league +
                 offsetof(CompetitionLeague,name), sizeof out);
    assert(out[0] == club && out[1] == club);

    QualifyingGroup group;
    memset(&group, 0, sizeof group);
    Competition *expected = (Competition *)(uintptr_t)0x12345678u;
    Competition *actual = NULL;
    memcpy((uint8_t *)&group + offsetof(QualifyingGroup,compCup) +
               2 * sizeof expected, &expected, sizeof expected);
    memcpy(&actual, (const uint8_t *)&group +
                    offsetof(QualifyingGroup,compCup) +
                    2 * sizeof actual, sizeof actual);
    assert(actual == expected);

    int32_t input = -19, output = 0;
    memcpy((uint8_t *)&live +
           offsetof(DIYCompetitionEx,bracketScratch.teamOffsetTable) +
           7 * sizeof input, &input, sizeof input);
    memcpy(&output, (const uint8_t *)&live +
            offsetof(DIYCompetitionEx,bracketScratch.teamOffsetTable) +
            7 * sizeof output, sizeof output);
    assert(output == input);

    puts("PACKED_ALIGNMENT_CONTRACT=PASS_ISOLATED_ONLY");
    return 0;
}
