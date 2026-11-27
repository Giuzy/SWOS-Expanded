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
/* TEAM.081 mislabels Zambia as REPUBLIC CONGO. Correct the stale name
 * only, by native file/ordinal (not physical array index). */
static inline int TeamSource_CorrectNationalName(uint8_t file, uint8_t ordinal,
                                                  int8_t name[17])
{
    static const char wrong[] = "REPUBLIC CONGO";
    static const char right[] = "ZAMBIA";
    unsigned i;
    if (file != 81u || ordinal != 32u || !name)
        return 0;
    for (i = 0; i < sizeof wrong; ++i)
        if (name[i] != (int8_t)wrong[i])
            return 0;
    for (i = 0; i < 17u; ++i)
        name[i] = i < sizeof right ? (int8_t)right[i] : 0;
    return 1;
}
#endif
