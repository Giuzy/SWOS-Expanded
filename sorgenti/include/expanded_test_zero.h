#ifndef EXPANDED_TEST_ZERO_H
#define EXPANDED_TEST_ZERO_H
#include "game_types.h"
#include <string.h>
static inline void ExpCeTestZero_Materialize(uint16_t raw,uint16_t global,const char *name,TeamFileEx *t)
{
    /* One TEST_ZERO source copied by value; positions and shirt numbers
     * are mechanical OG roster metadata, not fabricated skill. */
    static const uint8_t roles[16]={0,1,2,3,3,4,5,6,6,7,7,0,1,3,6,7};
    PlayerFile template_player={0};
    unsigned i;
    memset(t,0,sizeof *t);
    t->team.countryNumber=(uint8_t)(raw>>8);t->team.teamOrdinal=(uint8_t)raw;
    t->team.globalTeamNumber=(int16_t)global;
    t->team.league=255u; /* outside league; CE membership is sole writer */
    {   size_t nl0=strlen(name);   /* v342: 16 chars max, the 17th byte is the terminator */
        if(nl0>sizeof t->team.teamName-1u)nl0=sizeof t->team.teamName-1u;
        memcpy(t->team.teamName,name,nl0); }
    t->team.prShirtType=t->team.secShirtType=1u;
    t->team.prBasicColor=2u;t->team.secBasicColor=0u;
    t->team.prShortsColor=2u;t->team.secShortsColor=0u;
    t->team.prSocksColor=2u;t->team.secSocksColor=0u;
    t->team.playerValuesDirty=TEAM_VALUES_DIRTY;
    memcpy(template_player.playerName,"TEST",4u);
    for(i=0;i<16u;++i) {
        t->players[i]=template_player;
        t->players[i].shirtNumber=(int8_t)(i+1u);
        t->players[i].positionAndFace=(uint8_t)(roles[i]<<PF_POSITION_SHIFT);
        t->team.playerNumbers[i]=(uint8_t)i;
    }
}
#endif
