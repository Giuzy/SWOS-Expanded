/* Link the exact SWOS source fixture tables in the pure-C living-world
 * regression harness. The actual Windows game provides this pointer table
 * from its original/OpenSWOS data and FixtureRounds_Install; this file
 * exists ONLY in tests and must never be linked into SWOS_Expanded.exe. */
#include "fixture_data.h"
const uint8_t *_fixtureSchedulePtrTable[] = {
    _fixtureSchedule2Teams,_fixtureSchedule3Teams,
    _fixtureSchedule4Teams,_fixtureSchedule5Teams,
    _fixtureSchedule6Teams,_fixtureSchedule7Teams,
    _fixtureSchedule8Teams,_fixtureSchedule9Teams,
    _fixtureSchedule10Teams,_fixtureSchedule11Teams,
    _fixtureSchedule12Teams,_fixtureSchedule13Teams,
    _fixtureSchedule14Teams,_fixtureSchedule15Teams,
    _fixtureSchedule16Teams,_fixtureSchedule17Teams,
    _fixtureSchedule18Teams,_fixtureSchedule19Teams,
    _fixtureSchedule20Teams,_fixtureSchedule21Teams,
    _fixtureSchedule22Teams,_fixtureSchedule23Teams,
    _fixtureSchedule24Teams
};
