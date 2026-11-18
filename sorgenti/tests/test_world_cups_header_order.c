/* Header include-order regression: world_cups.h must be valid when a
 * translation unit (frontend.c) includes it before game_types.h.
 * The reverse order must still accept the same TeamId ABI/signature.
 * Compile this file twice with and without
 * -DSWOS_TEST_REVERSE_TEAMID_INCLUDE=1; no runtime or OG simulation. */
#ifdef SWOS_TEST_REVERSE_TEAMID_INCLUDE
#include "game_types.h"
#include "world_cups.h"
#else
#include "world_cups.h"
#include "game_types.h"
#endif

_Static_assert(sizeof(TeamId) == 2, "OG TeamId remains two bytes");
_Static_assert(_Generic(&WorldCups_PreviewEntrants,
    int32_t (*)(const uint8_t *, TeamId *, uint16_t, int *): 1,
    default: 0), "WorldCups_PreviewEntrants TeamId signature");
