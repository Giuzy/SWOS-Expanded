/* AUTO-GENERATED-LIKE baked lookup from country_season_cycles.json
 * prepared SHA 1d4f3cbab5b415b88f185c0935a32e16d7438663.
 * This is NOT a native OG lifecycle dispatcher. The JSON is the source;
 * static data work even when no external filesystem is available.
 * Edition exceptions (Bolivia 2026, MLS sprint 2027) are NOT representable
 * by a generic annual country default and need explicit validated data. */
#ifndef EXPANDED_COUNTRY_SEASON_BAKED_H
#define EXPANDED_COUNTRY_SEASON_BAKED_H
#include <stddef.h>
#include <stdint.h>
#include "expanded_country_season_year.h"
typedef struct ExpandedCountrySeasonBakedRow {
    uint16_t country;
    ExpandedCountrySeasonCycle cycle;
} ExpandedCountrySeasonBakedRow;
static const ExpandedCountrySeasonBakedRow kExpandedCountry2627Cycles[91]={
    {0u,{7u,1u,6u,30u}}, /* _albanianLeague JULY_JUNE_GAME_TEMPLATE */
    {1u,{7u,1u,6u,30u}}, /* _austrianLeague JULY_JUNE_GAME_TEMPLATE */
    {2u,{7u,1u,6u,30u}}, /* _belgianLeague JULY_JUNE_GAME_TEMPLATE */
    {3u,{7u,1u,6u,30u}}, /* _bulgarianLeague JULY_JUNE_GAME_TEMPLATE */
    {4u,{7u,1u,6u,30u}}, /* _croatianLeague JULY_JUNE_GAME_TEMPLATE */
    {5u,{7u,1u,6u,30u}}, /* _cypriotLeague JULY_JUNE_GAME_TEMPLATE */
    {6u,{7u,1u,6u,30u}}, /* _czechLeague JULY_JUNE_GAME_TEMPLATE */
    {7u,{7u,1u,6u,30u}}, /* _danishLeague JULY_JUNE_GAME_TEMPLATE */
    {8u,{7u,1u,6u,30u}}, /* _englishLeague JULY_JUNE_GAME_TEMPLATE */
    {10u,{1u,1u,12u,31u}}, /* _estonianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {11u,{1u,1u,12u,31u}}, /* _faroeIslesLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {12u,{1u,1u,12u,31u}}, /* _finnishLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {13u,{7u,1u,6u,30u}}, /* _frenchLeague JULY_JUNE_GAME_TEMPLATE */
    {14u,{7u,1u,6u,30u}}, /* _germanLeague JULY_JUNE_GAME_TEMPLATE */
    {15u,{7u,1u,6u,30u}}, /* _greekLeague JULY_JUNE_GAME_TEMPLATE */
    {16u,{7u,1u,6u,30u}}, /* _hungarianLeague JULY_JUNE_GAME_TEMPLATE */
    {17u,{1u,1u,12u,31u}}, /* _icelandicLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {18u,{1u,1u,12u,31u}}, /* _irishLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {19u,{7u,1u,6u,30u}}, /* _israeliLeague JULY_JUNE_GAME_TEMPLATE */
    {20u,{7u,1u,6u,30u}}, /* _italianLeague JULY_JUNE_GAME_TEMPLATE */
    {21u,{1u,1u,12u,31u}}, /* _latvianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {22u,{1u,1u,12u,31u}}, /* _lithuanianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {23u,{7u,1u,6u,30u}}, /* _luxembourgLeague JULY_JUNE_GAME_TEMPLATE */
    {24u,{7u,1u,6u,30u}}, /* _malteseLeague JULY_JUNE_GAME_TEMPLATE */
    {25u,{7u,1u,6u,30u}}, /* _dutchLeague JULY_JUNE_GAME_TEMPLATE */
    {26u,{7u,1u,6u,30u}}, /* _northernIrishLeague JULY_JUNE_GAME_TEMPLATE */
    {27u,{1u,1u,12u,31u}}, /* _norwegianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {28u,{7u,1u,6u,30u}}, /* _polishLeague JULY_JUNE_GAME_TEMPLATE */
    {29u,{7u,1u,6u,30u}}, /* _portugeseLeague JULY_JUNE_GAME_TEMPLATE */
    {30u,{7u,1u,6u,30u}}, /* _romanianLeague JULY_JUNE_GAME_TEMPLATE */
    {31u,{7u,1u,6u,30u}}, /* _russianLeague JULY_JUNE_GAME_TEMPLATE */
    {32u,{7u,1u,6u,30u}}, /* _sanMarinoLeague JULY_JUNE_GAME_TEMPLATE */
    {33u,{7u,1u,6u,30u}}, /* _scottishLeague JULY_JUNE_GAME_TEMPLATE */
    {34u,{7u,1u,6u,30u}}, /* _slovenianLeague JULY_JUNE_GAME_TEMPLATE */
    {35u,{7u,1u,6u,30u}}, /* _spanishLeague JULY_JUNE_GAME_TEMPLATE */
    {36u,{1u,1u,12u,31u}}, /* _swedishLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {37u,{7u,1u,6u,30u}}, /* _swissLeague JULY_JUNE_GAME_TEMPLATE */
    {38u,{7u,1u,6u,30u}}, /* _turkishLeague JULY_JUNE_GAME_TEMPLATE */
    {39u,{7u,1u,6u,30u}}, /* _ukrainianLeague JULY_JUNE_GAME_TEMPLATE */
    {40u,{7u,1u,6u,30u}}, /* _welshLeague JULY_JUNE_GAME_TEMPLATE */
    {41u,{7u,1u,6u,30u}}, /* _yugoslavianLeague JULY_JUNE_GAME_TEMPLATE */
    {42u,{7u,1u,6u,30u}}, /* _algerianLeague JULY_JUNE_GAME_TEMPLATE */
    {43u,{1u,1u,12u,31u}}, /* _argentinianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {44u,{10u,1u,9u,30u}}, /* _australianLeague AUSTRALIA_ALEAGUE */
    {45u,{1u,15u,1u,14u}}, /* _bolivianLeague BOLIVIA_PROVISIONAL */
    {46u,{1u,1u,12u,31u}}, /* _brasilianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {47u,{7u,1u,6u,30u}}, /* _worldAndLeague JULY_JUNE_GAME_TEMPLATE */
    {48u,{1u,1u,12u,31u}}, /* _chileanLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {49u,{1u,1u,12u,31u}}, /* _colombianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {50u,{1u,1u,12u,31u}}, /* _ecuadorLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {51u,{7u,1u,6u,30u}}, /* _elSalvadorLeague JULY_JUNE_GAME_TEMPLATE */
    {52u,{7u,1u,6u,30u}}, /* _worldArmLeague JULY_JUNE_GAME_TEMPLATE */
    {53u,{7u,1u,6u,30u}}, /* _worldAzeLeague JULY_JUNE_GAME_TEMPLATE */
    {54u,{7u,1u,6u,30u}}, /* _worldBihLeague JULY_JUNE_GAME_TEMPLATE */
    {55u,{8u,1u,7u,31u}}, /* _japaneseLeague JLEAGUE_MIGRATION */
    {56u,{1u,1u,12u,31u}}, /* _worldCanLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {57u,{1u,1u,12u,31u}}, /* _worldChnLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {58u,{7u,1u,6u,30u}}, /* _worldCrcLeague JULY_JUNE_GAME_TEMPLATE */
    {59u,{7u,1u,6u,30u}}, /* _worldEgyLeague JULY_JUNE_GAME_TEMPLATE */
    {60u,{7u,1u,6u,30u}}, /* _mexicanLeague JULY_JUNE_GAME_TEMPLATE */
    {61u,{1u,1u,12u,31u}}, /* _worldGeoLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {62u,{1u,1u,12u,31u}}, /* _newZealandLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {63u,{7u,1u,6u,30u}}, /* _worldGibLeague JULY_JUNE_GAME_TEMPLATE */
    {64u,{1u,1u,12u,31u}}, /* _paraguayanLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {65u,{1u,1u,12u,31u}}, /* _peruvianLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {66u,{7u,1u,6u,30u}}, /* _surinamLeague JULY_JUNE_GAME_TEMPLATE */
    {67u,{1u,1u,12u,31u}}, /* _taiwaneseLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {68u,{7u,1u,6u,30u}}, /* _worldGuaLeague JULY_JUNE_GAME_TEMPLATE */
    {69u,{7u,1u,6u,30u}}, /* _southAfricanLeague JULY_JUNE_GAME_TEMPLATE */
    {70u,{7u,1u,6u,30u}}, /* _worldHonLeague JULY_JUNE_GAME_TEMPLATE */
    {71u,{1u,1u,12u,31u}}, /* _uruguayanLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {73u,{1u,1u,12u,31u}}, /* _americanLeague MLS_TRANSITION */
    {74u,{7u,1u,6u,30u}}, /* _worldIrnLeague JULY_JUNE_GAME_TEMPLATE */
    {75u,{8u,1u,7u,31u}}, /* _indianLeague INDIA_GAME_TEMPLATE */
    {76u,{1u,1u,12u,31u}}, /* _belarussiaLleague CALENDAR_YEAR_GAME_TEMPLATE */
    {77u,{1u,1u,12u,31u}}, /* _venezuelanLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {78u,{7u,1u,6u,30u}}, /* _slovakianLeague JULY_JUNE_GAME_TEMPLATE */
    {79u,{7u,1u,6u,30u}}, /* _ghanaLeague JULY_JUNE_GAME_TEMPLATE */
    {86u,{1u,1u,12u,31u}}, /* _worldKazLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {87u,{1u,1u,12u,31u}}, /* _worldKorLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {88u,{7u,1u,6u,30u}}, /* _worldKosLeague JULY_JUNE_GAME_TEMPLATE */
    {89u,{7u,1u,6u,30u}}, /* _worldKsaLeague JULY_JUNE_GAME_TEMPLATE */
    {90u,{7u,1u,6u,30u}}, /* _worldLieLeague JULY_JUNE_GAME_TEMPLATE */
    {91u,{7u,1u,6u,30u}}, /* _worldMarLeague JULY_JUNE_GAME_TEMPLATE */
    {92u,{7u,1u,6u,30u}}, /* _worldMdaLeague JULY_JUNE_GAME_TEMPLATE */
    {93u,{7u,1u,6u,30u}}, /* _worldMkdLeague JULY_JUNE_GAME_TEMPLATE */
    {94u,{7u,1u,6u,30u}}, /* _worldMneLeague JULY_JUNE_GAME_TEMPLATE */
    {95u,{1u,1u,12u,31u}}, /* _worldPanLeague CALENDAR_YEAR_GAME_TEMPLATE */
    {96u,{7u,1u,6u,30u}}, /* _worldQatLeague JULY_JUNE_GAME_TEMPLATE */
    {97u,{7u,1u,6u,30u}}, /* _worldTunLeague JULY_JUNE_GAME_TEMPLATE */
    {98u,{7u,1u,6u,30u}}, /* _worldUaeLeague JULY_JUNE_GAME_TEMPLATE */
};
static inline int ExpandedCountrySeason_Baked2627(
    uint16_t country,ExpandedCountrySeasonCycle *out)
{
    if(!out || country>=256u)return 0;
    const ExpandedCountrySeasonCycle *found=NULL;
    for(size_t i=0u;i<sizeof kExpandedCountry2627Cycles/
                   sizeof kExpandedCountry2627Cycles[0];++i) {
        if(kExpandedCountry2627Cycles[i].country!=country)continue;
        if(found)return 0;
        found=&kExpandedCountry2627Cycles[i].cycle;
    }
    if(!found)return 0; /* never silently choose 01 July for unknown ID */
    *out=*found;
    return 1;
}
#endif
