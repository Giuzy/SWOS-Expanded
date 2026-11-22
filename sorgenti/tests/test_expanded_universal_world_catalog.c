/* Isolated WORLD2627 JSON-first catalog/pairing regression contract.
 * Mock of the JSON reader only; C11 catalog + circle algorithm are real.
 * This is NOT Windows gameplay, OG50Hz, full JSON parsing or CAR.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "expanded_universal_world_catalog.h"
#include "expanded_universal_pairings.h"
#include "expanded_universal_json_leagues.h"
#include "world.h"
static WorldId s_world=WORLD_2627;
static int s_json_ok=1;
WorldId World_Current(void){return s_world;}
const char *World_Tag(void){return s_world==WORLD_2627?"2627":"9697";}
uint16_t World_FirstYear(void){return s_world==WORLD_2627?2026u:1996u;}
uint8_t World_FileCountry(uint8_t file){return file;}
uint16_t World_Slot(uint8_t file,uint8_t ordinal)
{(void)file;return ordinal;}
uint16_t World_SlotTeam(uint8_t country,uint16_t slot)
{return (uint16_t)(((uint16_t)country<<8)|slot);}
/* Stub validates exactly the rule fields expected from actual JSON. */
void ExpandedUniversalJsonLeagues_Reset(void){}
int ExpandedUniversalJsonLeagues_Load(void){return s_json_ok;}
uint32_t ExpandedUniversalJsonLeagues_Count(void){return s_json_ok?3u:0u;}
int ExpandedUniversalJsonLeagues_Find(uint8_t country,uint8_t comp,
    uint8_t division,ExpandedUniversalJsonLeague *out)
{
    unsigned n=0,cycles=0;
    if(!s_json_ok || !out)return 0;
    if(country==45u && comp==89u && division==0u){n=4;cycles=2;}
    else if(country==45u && comp==89u && division==1u){n=5;cycles=1;}
    else if(country==44u && comp==77u && division==0u){n=30;cycles=1;}
    else return 0;
    *out=(ExpandedUniversalJsonLeague){
        country,comp,division,56u,88u,(uint8_t)cycles,(uint16_t)n
    };
    return 1;
}
static void add(uint8_t file,uint8_t ordinal,uint8_t division)
{
    TeamFileEx t={0};
    t.team.countryNumber=file;
    t.team.teamOrdinal=ordinal;
    t.team.league=division;
    t.players[0].price=(int8_t)(ordinal+1u);
    assert(ExpandedUniversalCatalog_AddClub(&t));
}
/* RG-T: genuine C circle algorithm, all even/odd n=2..48 and 1..4
 * independent cycles. No OG fixture data required or permitted.
 */
typedef struct PairCheck {
    uint8_t orient[4][48][48];
    uint8_t in_round[4][48][48];
    uint32_t per_cycle[4];
    unsigned n;
} PairCheck;
static int check_pair(void *user,uint32_t c,uint32_t round,
                      uint32_t serial,uint32_t home,uint32_t away)
{
    PairCheck *p=(PairCheck *)user;
    (void)serial;
    assert(c<4u && round<48u && home<p->n && away<p->n &&
           home!=away);
    assert(!p->orient[c][home][away] &&
           !p->orient[c][away][home]);
    assert(!p->in_round[c][round][home] &&
           !p->in_round[c][round][away]);
    p->orient[c][home][away]=1u;
    p->orient[c][away][home]=2u;
    p->in_round[c][round][home]=1u;
    p->in_round[c][round][away]=1u;
    ++p->per_cycle[c];
    return 1;
}
static void test_pairings_all(void)
{
    PairCheck p;
    unsigned n,legs,i,j,c;
    for(n=2u;n<=48u;++n)for(legs=1u;legs<=4u;++legs) {
        memset(&p,0,sizeof p);
        p.n=n;
        assert(ExpandedUniversal_GenerateRoundRobin(
            n,(uint8_t)legs,check_pair,&p));
        for(c=0u;c<legs;++c) {
            assert(p.per_cycle[c]==n*(n-1u)/2u);
            for(i=0u;i<n;++i)for(j=i+1u;j<n;++j)
                assert(p.orient[c][i][j]!=0u);
        }
        if(legs>=2u)
            for(i=0u;i<n;++i)for(j=i+1u;j<n;++j)
                assert(p.orient[0][i][j]!=p.orient[1][i][j]);
    }
}
int main(void)
{
    ExpandedUniversalCatalogAudit audit={0};
    const ExpandedUniversalCatalogFixture *f=NULL;
    ExpandedClubKey first={WORLD_2627,(45u<<16)|1u};
    ExpandedWorldDate date={0};
    int32_t expected=0;
    test_pairings_all(); /* 188 n/meeting configurations, actual C11 */
    assert(ExpandedUniversalCatalog_Begin());
    for(uint8_t i=0;i<4u;++i)add(45u,i,0u);
    for(uint8_t i=4u;i<9u;++i)add(45u,i,1u);
    for(uint8_t i=0;i<30u;++i)add(44u,i,0u);
    assert(ExpandedUniversalCatalog_Roster(first)!=NULL);
    assert(ExpandedUniversalCatalog_Roster(first)->players[0].price==1);
    assert(ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(ExpandedUniversalCatalog_AddLeague(45u,89u,1u,56u,88u,1u));
    assert(ExpandedUniversalCatalog_AddLeague(44u,77u,0u,56u,88u,1u));
    assert(!ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(ExpandedUniversalCatalog_AddCup(45u,164u,1u));
    assert(ExpandedUniversalCatalog_Seal());
    assert(ExpandedUniversalCatalog_Audit(&audit));
    assert(audit.clubs==39u && audit.league_definitions==3u);
    assert(audit.fixtures==457u && audit.future_fixtures==457u);
    assert(audit.unsupported_leagues==0u && audit.day_collisions==0u);
    assert(audit.missing_cup_fixtures==1u && !audit.source_complete);
    assert(audit.unverified_native_order==0u && audit.json_rule_failures==0u);
    f=ExpandedUniversalCatalog_Fixture(0u);
    assert(f && f->fixture.key.edition.competition.world==WORLD_2627);
    assert(f->fixture.home.stable_club==((45u<<16)|1u));
    assert(f->fixture.away.stable_club==((45u<<16)|4u));
    assert(f->native_phase==56u && f->past_at_start==0u);
    assert(f->date_provenance==EXP_CATALOG_DATE_OG_PHASE_FIRST_DAY);
    assert(ExpandedWorld_NativePhaseToDate(2026u,56u,&date));
    assert(ExpandedWorld_DateToIndex(date,&expected));
    assert(f->fixture.day_index==expected);
    assert(ExpandedUniversalCatalog_Fixture(457u)==NULL);
    /* JSON absent: never fall back to original OG pair tables. */
    s_json_ok=0;
    assert(ExpandedUniversalCatalog_Begin());
    assert(!ExpandedUniversalCatalog_AddLeague(45u,89u,0u,56u,88u,2u));
    assert(ExpandedUniversalCatalog_Seal());
    assert(ExpandedUniversalCatalog_Audit(&audit));
    assert(audit.json_rule_failures>=1u && audit.fixtures==0u);
    s_world=WORLD_9697;
    assert(ExpandedUniversalCatalog_Begin());
    assert(!ExpandedUniversalCatalog_Audit(&audit));
    ExpandedUniversalCatalog_Reset();
    puts("UNIVERSAL_JSON_FIRST_CATALOG=PASS_ISOLATED_ONLY");
    puts("OG_NATIVE_MATCH/CAR/GP/WIN=NOT_TESTED");
    return 0;
}
