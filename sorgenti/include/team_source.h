/* Shared rule for the four verified 1996/97 CAREER native TEAM files.
 * Pure path construction, no I/O: caller may fall back to OG TEAM.nnn.
 * CLASSIC/DIY/SEASON deliberately retain their generic native files. */
#ifndef SWOS_TEAM_SOURCE_H
#define SWOS_TEAM_SOURCE_H
#include "world.h"

static inline int TeamSource_HistoricCareerPath(WorldId world,
                                                 uint16_t game_type,
                                                 uint8_t file,
                                                 char *path,
                                                 uint32_t capacity)
{
    static const char original[] = "data/9697/TEAM.000";
    unsigned i;
    if (!path || capacity < (uint32_t)sizeof original ||
        world != WORLD_9697 || game_type != 4u ||
        (file != 12u && file != 13u && file != 38u && file != 73u))
        return 0;
    for (i = 0; i < sizeof original; ++i)
        path[i] = original[i];
    path[15] = (char)('0' + file / 100u);
    path[16] = (char)('0' + (file / 10u) % 10u);
    path[17] = (char)('0' + file % 10u);
    return 1;
}
#endif
