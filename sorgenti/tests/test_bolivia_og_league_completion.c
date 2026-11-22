/* RG-001/014/027: OG Bolivia completed-state proof, isolated only.
 * Synthetic standings exercise the contract, not Windows gameplay/CAR. */
#include <assert.h>
#include <stdio.h>
#include "bolivia_og_league_completion.h"
int main(void)
{
    BoliviaOGLeagueProofRow rows[BOLIVIA_OG_LEAGUE_TEAMS];
    for(unsigned i=0;i<BOLIVIA_OG_LEAGUE_TEAMS;++i)
        rows[i]=(BoliviaOGLeagueProofRow){(int32_t)i,30,15,0,15,30,30};
    assert(BoliviaOGLeague_CompletedFromOG(16,240,rows));
    assert(!BoliviaOGLeague_CompletedFromOG(16,239,rows));
    assert(!BoliviaOGLeague_CompletedFromOG(15,240,rows));
    assert(!BoliviaOGLeague_CompletedFromOG(16,240,NULL));
    rows[0].gamesPlayed=29;
    assert(!BoliviaOGLeague_CompletedFromOG(16,240,rows));
    rows[0].gamesPlayed=30;
    rows[0].teamIndex=1;
    assert(!BoliviaOGLeague_CompletedFromOG(16,240,rows));
    rows[0].teamIndex=0;
    rows[0].wins=16;
    assert(!BoliviaOGLeague_CompletedFromOG(16,240,rows));
    rows[0].wins=15;
    rows[0].goalsFor=31;
    assert(!BoliviaOGLeague_CompletedFromOG(16,240,rows));
    puts("PASS Bolivia OG league completion proof 16x30/240 (ISO)");
    return 0;
}
