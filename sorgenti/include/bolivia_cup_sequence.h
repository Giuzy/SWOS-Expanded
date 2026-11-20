/*
 * 2026 Copa Bolivia (Copa Paceña): actual 64 first-edition group fixtures.
 * Source for each dated pairing: Bolivia.com 2026 Copa Bolivia results,
 * https://www.bolivia.com/futbol/liga-boliviana/copa-bolivia/resultados
 *
 * The 2026 edition began on August 18, not August 19. Fixture dates
 * are schedules/real-world metadata only: goals, tables, winners and
 * match execution belong exclusively to the native OpenSWOS engine.
 * The game must NEVER import real-world scores into a new 1 July career.
 * Some postponed matches have a later civil date than later matchdays.
 */
#ifndef BOLIVIA_CUP_SEQUENCE_H
#define BOLIVIA_CUP_SEQUENCE_H
#include <stdint.h>

enum {
    BOLIVIA_CUP_TEAMS=16, BOLIVIA_CUP_GROUPS=4,
    BOLIVIA_CUP_MATCHDAYS=8, BOLIVIA_CUP_GROUP_FIXTURES=64,
    BOLIVIA_CUP_KNOCKOUT_FIXTURES=14,
    BOLIVIA_CUP_TOTAL_FIXTURES=78
};

typedef struct {
    uint16_t fixtureId; /* 1..64, stable in edition; not a global ordinal */
    uint8_t matchday;   /* 1..8, 2 and 4 are cross-group classics */
    uint8_t home;       /* TEAM.045 ordinal */
    uint8_t away;
    uint8_t groupHome;
    uint8_t groupAway;
    uint8_t isDerby;
    uint8_t month;      /* 2026 civil month, 8..10 */
    uint8_t day;        /* 2026 civil day; no score or simulation */
} BoliviaCupFixture;

static const uint8_t kBoliviaCupGroups[4][4] = {
    { 3,14, 9,15 }, /* San Antonio, ABB, Aurora, Real Potosi */
    { 0,12,10, 6 }, /* Always Ready, Real Oruro, Tomayapo, Nacional */
    { 7,11, 2,13 }, /* Guabira, Oriente, Strongest, Universitario */
    { 1, 8, 4, 5 }  /* Bolivar, GV San Jose, Blooming, Independiente */
};

typedef struct { uint8_t home,away,month,day; } BoliviaCupRealPair;
/* One authoritative initial 2026 dated pair table. Not results. */
static const BoliviaCupRealPair kBoliviaCup2026[8][8] = {
    /* Matchday 1: eight matches, some rescheduled onto later dates. */
    { {  3,  9, 8, 18 }, {  7, 13, 8, 18 }, {  2, 11, 8, 18 }, { 14, 15, 8, 19 }, {  5,  4, 8, 19 }, {  0, 12, 8, 20 }, {  8,  1, 9,  2 }, {  6, 10, 9,  3 } },
    /* Matchday 2: eight matches, some rescheduled onto later dates. */
    { {  8, 12, 8, 29 }, { 14,  0, 8, 30 }, {  1,  2, 8, 30 }, {  4, 11, 8, 30 }, {  3,  7, 8, 31 }, { 10,  5, 8, 31 }, { 13,  9, 9,  1 }, {  6, 15, 9, 22 } },
    /* Matchday 3: eight matches, some rescheduled onto later dates. */
    { { 14,  3, 9,  8 }, {  9, 15, 9,  8 }, { 11,  7, 9,  8 }, { 12,  6, 9,  9 }, { 13,  2, 9,  9 }, {  4,  8, 9,  9 }, { 10,  0, 9, 10 }, {  1,  5, 9, 10 } },
    /* Matchday 4: eight matches, some rescheduled onto later dates. */
    { {  0, 14, 9, 19 }, {  9, 13, 9, 19 }, { 15,  6, 9, 19 }, { 12,  8, 9, 20 }, {  2,  1, 9, 20 }, { 11,  4, 9, 20 }, {  7,  3, 9, 21 }, {  5, 10, 9, 21 } },
    /* Matchday 5: eight matches, some rescheduled onto later dates. */
    { {  3, 15, 9, 25 }, { 14,  9, 9, 25 }, {  6,  0, 9, 25 }, {  2,  7, 9, 26 }, { 13, 11, 9, 26 }, { 10, 12, 9, 27 }, {  8,  5, 9, 27 }, {  1,  4, 9, 27 } },
    /* Matchday 6: eight matches, some rescheduled onto later dates. */
    { { 15, 14, 9, 29 }, {  9,  3, 9, 29 }, { 13,  7, 9, 29 }, { 12,  0, 9, 30 }, { 10,  6, 9, 30 }, { 11,  2, 9, 30 }, {  1,  8, 10,  1 }, {  4,  5, 10,  1 } },
    /* Matchday 7: eight matches, some rescheduled onto later dates. */
    { {  3, 14, 10,  2 }, { 15,  9, 10,  2 }, {  0, 10, 10,  3 }, {  6, 12, 10,  3 }, {  2, 13, 10,  3 }, {  8,  4, 10,  4 }, {  7, 11, 10,  4 }, {  5,  1, 10,  4 } },
    /* Matchday 8: eight matches, some rescheduled onto later dates. */
    { { 15,  3, 10,  6 }, {  9, 14, 10,  6 }, { 12, 10, 10,  6 }, {  7,  2, 10,  7 }, { 11, 13, 10,  7 }, {  5,  8, 10,  7 }, {  0,  6, 10,  8 }, {  4,  1, 10,  8 } }
};

static inline int BoliviaCup_GroupOf(uint8_t ordinal) {
    for(unsigned g=0;g<4u;++g)
        for(unsigned k=0;k<4u;++k)
            if(kBoliviaCupGroups[g][k]==ordinal)return (int)g;
    return -1;
}

static inline unsigned BoliviaCup_BuildGroupFixtures(
    BoliviaCupFixture out[BOLIVIA_CUP_GROUP_FIXTURES])
{
    if(!out)return 0u;
    for(unsigned d=0;d<8u;++d)for(unsigned i=0;i<8u;++i) {
        const BoliviaCupRealPair *p=&kBoliviaCup2026[d][i];
        const int gh=BoliviaCup_GroupOf(p->home);
        const int ga=BoliviaCup_GroupOf(p->away);
        if(gh<0 || ga<0)return 0u;
        const unsigned k=d*8u+i;
        out[k]=(BoliviaCupFixture){
            (uint16_t)(k+1u),(uint8_t)(d+1u),p->home,p->away,
            (uint8_t)gh,(uint8_t)ga,(uint8_t)(gh!=ga),
            p->month,p->day
        };
    }
    return 64u;
}

/* Validate that all 16 clubs play once on each of eight matchdays,
 * four home and four away, with 48 internal +16 reciprocal cross-group
 * games and no duplicate directed pair or duplicate derby day. */
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
           f->month<8u || f->month>10u || f->day<1u || f->day>31u ||
           seen[f->home][f->away]++)return 0;
        const unsigned d=i/8u;
        for(unsigned j=d*8u;j<i;++j)
            if(fx[j].home==f->home || fx[j].away==f->home ||
               fx[j].home==f->away || fx[j].away==f->away)
                return 0;
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
