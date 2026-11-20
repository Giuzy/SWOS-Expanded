/*
 * Copa Bolivia 2026: immutable 8-matchday fixture sequence (16 clubs).
 *
 * Produces NO goals, standings, winners, simulated results or independent time.
 * IDs are in the original 2026/27 TEAM.045 order. The sequence is a native
 * fixture source for an OG match executor, NOT an alternate match engine.
 * The six intra-group rounds are a deterministic RULES-VALID template;
 * the authoritative historical 2026 per-match date/venue mapping is still
 * required before this can be played as the first edition.
 */
#ifndef BOLIVIA_CUP_SEQUENCE_H
#define BOLIVIA_CUP_SEQUENCE_H
#include <stdint.h>

enum {
    BOLIVIA_CUP_TEAMS = 16,
    BOLIVIA_CUP_GROUPS = 4,
    BOLIVIA_CUP_MATCHDAYS = 8,
    BOLIVIA_CUP_GROUP_FIXTURES = 64,
    BOLIVIA_CUP_KNOCKOUT_FIXTURES = 14,
    BOLIVIA_CUP_TOTAL_FIXTURES = 78
};

typedef struct {
    uint16_t fixtureId; /* 1..64; identity key is world+edition+fixtureId */
    uint8_t matchday;   /* 1..8; only 2 and 4 are cross-group */
    uint8_t home;       /* TEAM.045 ordinal, never a global selectedTeams offset */
    uint8_t away;
    uint8_t groupHome;  /* 0..3 */
    uint8_t groupAway;
    uint8_t isDerby;
} BoliviaCupFixture;

/* Real 22 July 2026 draw, by team-file ordinal. */
static const uint8_t kBoliviaCupGroups[4][4] = {
    { 3, 14, 9, 15 },  /* San Antonio Bulo Bulo, ABB, Aurora, Real Potosi */
    { 0, 12, 10, 6 },  /* Always Ready, Real Oruro, Tomayapo, Nacional Potosi */
    { 7, 11, 2, 13 },  /* Guabira, Oriente, The Strongest, Universitario Vinto */
    { 1, 8, 4, 5 }     /* Bolivar, GV San Jose, Blooming, Independiente */
};
/* Verified reciprocal interseries opponent PAIRS. The template alternates
 * home/away across derby days 2 and 4; the exact historical first-home
 * order/dates still need an authenticated match-by-match source. There
 * are eight opponent pairs, yielding sixteen interseries fixtures. */
static const uint8_t kBoliviaCupDerbies[8][2] = {
    { 14, 0 }, { 1, 2 }, { 4, 11 }, { 8, 12 },
    { 6, 15 }, { 13, 9 }, { 3, 7 }, { 10, 5 }
};

static inline int BoliviaCup_GroupOf(uint8_t ordinal)
{
    for(unsigned g=0;g<4u;++g)
        for(unsigned k=0;k<4u;++k)
            if(kBoliviaCupGroups[g][k]==ordinal)return (int)g;
    return -1;
}

/* 8 days x 8 games. Native OG will resolve/play these encounters later;
 * there is deliberately no match result type anywhere in this interface. */
static inline unsigned BoliviaCup_BuildGroupFixtures(
    BoliviaCupFixture out[BOLIVIA_CUP_GROUP_FIXTURES])
{
    static const uint8_t pair[3][2][2] = {
        {{0,3},{1,2}}, {{0,2},{3,1}}, {{0,1},{2,3}}
    };
    unsigned count=0u, regular=0u;
    if(!out)return 0u;
    for(unsigned d=0;d<8u;++d) {
        if(d==1u || d==3u) {
            for(unsigned i=0;i<8u;++i) {
                uint8_t a=kBoliviaCupDerbies[i][0],b=kBoliviaCupDerbies[i][1];
                if(d==3u){uint8_t temp=a;a=b;b=temp;}
                const int ga=BoliviaCup_GroupOf(a),gb=BoliviaCup_GroupOf(b);
                if(ga<0 || gb<0 || ga==gb)return 0u;
                out[count]=(BoliviaCupFixture){
                    (uint16_t)(count+1u),(uint8_t)(d+1u),a,b,
                    (uint8_t)ga,(uint8_t)gb,1u
                };
                ++count;
            }
        } else {
            const unsigned round=regular%3u,back=regular>=3u;
            for(unsigned g=0;g<4u;++g)for(unsigned i=0;i<2u;++i) {
                uint8_t a=kBoliviaCupGroups[g][pair[round][i][0]];
                uint8_t b=kBoliviaCupGroups[g][pair[round][i][1]];
                if(back){uint8_t temp=a;a=b;b=temp;}
                out[count]=(BoliviaCupFixture){
                    (uint16_t)(count+1u),(uint8_t)(d+1u),a,b,
                    (uint8_t)g,(uint8_t)g,0u
                };
                ++count;
            }
            ++regular;
        }
    }
    return count==64u && regular==6u?count:0u;
}

/* Fail closed before handing a plan to the native OG executor. Every team
 * must play eight fixtures (4 home, 4 away); each head-to-head host/guest
 * identity is unique and the derby matches cannot be duplicated. */
static inline int BoliviaCup_ValidateGroupFixtures(
    const BoliviaCupFixture *fx,unsigned count)
{
    uint8_t homes[16]={0},aways[16]={0};
    uint8_t seen[16][16]={{0}};
    unsigned derby=0u,inGroup=0u;
    if(!fx || count!=BOLIVIA_CUP_GROUP_FIXTURES)return 0;
    for(unsigned i=0;i<count;++i) {
        const BoliviaCupFixture *f=&fx[i];
        if(f->fixtureId!=i+1u || f->matchday!=i/8u+1u ||
           f->home>=16u || f->away>=16u || f->home==f->away ||
           f->groupHome!=(uint8_t)BoliviaCup_GroupOf(f->home) ||
           f->groupAway!=(uint8_t)BoliviaCup_GroupOf(f->away) ||
           seen[f->home][f->away]++)return 0;
        ++homes[f->home];++aways[f->away];
        if(f->isDerby) {
            if(f->groupHome==f->groupAway ||
               (f->matchday!=2u && f->matchday!=4u))return 0;
            ++derby;
        } else {
            if(f->groupHome!=f->groupAway ||
               f->matchday==2u || f->matchday==4u)return 0;
            ++inGroup;
        }
    }
    if(derby!=16u || inGroup!=48u)return 0;
    for(unsigned i=0;i<16u;++i)
        if(homes[i]!=4u || aways[i]!=4u)return 0;
    return 1;
}
#endif /* BOLIVIA_CUP_SEQUENCE_H */
