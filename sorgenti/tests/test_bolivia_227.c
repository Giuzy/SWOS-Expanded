/* ISO regression gate for 00.13.227; run with run_test_bolivia_227.sh.
 * No game/simulator/CAR writes: checks the exact representative date set,
 * real Copa collision calendar, and the manager -1 identity guard.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "bolivia_league_2026_phases.h"
#include "bolivia_manager_offset_gate.h"
#include "bolivia_cup_sequence.h"
#include "bolivia_cup_knockout.h"

static unsigned native_civil(uint8_t phase)
{
    static const uint8_t days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    const unsigned month=phase/8u;
    const unsigned eighth=phase%8u;
    assert(month<12u);
    return (month+1u)*100u+1u+(eighth*days[month])/8u;
}
int main(void)
{
    static const uint8_t expected[30]={
        24,26,29,30,32,34,35,36,52,53,54,55,57,58,59,
        62,63,65,67,68,75,76,78,79,82,86,87,88,89,91
    };
    assert(BoliviaLeague2026_ValidatePhases());
    for(unsigned i=0u;i<30u;++i)
        assert(kBoliviaLeague2026NativePhases[i]==expected[i]);
    assert(native_civil(expected[17])==904u); /* Liga J18: 4 Sept */
    assert(native_civil(expected[29])==1212u);
    BoliviaCupFixture group[64];
    assert(BoliviaCup_ChronologicalFixtures(group)==64u);
    assert(BoliviaKO_Validate());
    for(unsigned i=0u;i<30u;++i) {
        const unsigned leagueDay=native_civil(expected[i]);
        for(unsigned j=0u;j<64u;++j)
            assert(leagueDay!=100u*group[j].month+group[j].day);
        for(unsigned j=0u;j<BOLIVIA_KO_FIXTURES;++j)
            assert(leagueDay!=100u*kBoliviaKO2026[j].month+
                               kBoliviaKO2026[j].day);
    }
    assert(BoliviaManagerFixture_ValidOffsets(0,6,16,0));
    assert(BoliviaManagerFixture_ValidOffsets(6,0,16,0));
    assert(!BoliviaManagerFixture_ValidOffsets(-1,6,16,0));
    assert(BoliviaManagerFixture_ValidOffsets(-1,6,16,1));
    assert(BoliviaManagerFixture_ValidOffsets(6,-1,16,1));
    assert(!BoliviaManagerFixture_ValidOffsets(-1,-1,16,1));
    assert(!BoliviaManagerFixture_ValidOffsets(0,6,16,1));
    assert(!BoliviaManagerFixture_ValidOffsets(6,6,16,0));
    assert(!BoliviaManagerFixture_ValidOffsets(-2,6,16,1));
    assert(!BoliviaManagerFixture_ValidOffsets(16,-1,16,1));
    assert(!BoliviaManagerFixture_ValidOffsets(-1,0,0,1));
    puts("ISO Bolivia 227: 30 dates, no 2026 Cup same-day clash, manager guard PASS");
    return 0;
}
