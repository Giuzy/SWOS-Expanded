/* Fase 2 — validator + capability matrix over the REAL data of a world:
 * data_json/worlds/2627/competitions/{leagues,league_calendar,ce_competitions}.json
 * read by the SAME builder the runtime uses (expanded_ce_definitions.c).
 * Prints one matrix row per definition (file + field of every gap).
 * Static data check only: no match, no Windows, no gameplay. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_ce_engine.h"
#include "expanded_universal_json_leagues.h"
#include "world.h"
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
WorldId World_Current(void){return WORLD_2627;}
/* v311: definitions.c loads the shared pool (identity helpers of the native world) */
uint16_t _teamsCountryNumbers[256];
uint8_t World_FileCountry(uint8_t f){return f;}
uint16_t World_Slot(uint8_t f,uint8_t o){(void)f;return o;}
uint16_t World_SlotTeam(uint8_t c,uint16_t slot){return (uint16_t)(((uint16_t)c<<8)|slot);}
uint16_t World_FirstYear(void){return 2026u;}
const char *swos_data_path(const char *rel,char *buf,uint32_t sz)
{snprintf(buf,sz,"../%s",rel);return buf;}
static const char *prov(uint8_t p)
{return p==EXP_CE_PROV_OG_NATIVE?"OG_NATIVE":p==EXP_CE_PROV_JSON_VERIFIED?"VERIFIED":
        p==EXP_CE_PROV_JSON_TEMPLATE?"GAME_TEMPLATE":"MISSING";}
int main(void)
{
    uint32_t i,ready=0,missing=0,leagues=0,json=0,chain_ok=0,chain_bad=0;
    int n;
    assert(ExpandedUniversalJsonLeagues_Load());
    n=ExpCe_BuildWorldDefinitions(WORLD_2627);
    assert(n>0);
    printf("| id | origin | type | name | phases | table | match (bench/subs/win/disc) | rollover | status |\n");
    printf("|---|---|---|---|---|---|---|---|---|\n");
    for(i=0;i<ExpCe_DefinitionCount();++i) {
        const ExpCeDef *d=ExpCe_DefinitionAt(i);
        const ExpCePhaseDef *p0=&d->phases[0];
        const char *roll="-";
        if(d->origin==EXP_CE_ORIGIN_LEAGUE) {
            ++leagues;
            if(d->division==0u) {
                if(ExpCe_LeagueFamilyChainOk(d->country,d->comp_id)){++chain_ok;}
                else ++chain_bad;
            }
            roll=ExpCe_LeagueFamilyChainOk(d->country,d->comp_id)?"CHAIN_OK":
                 "DISTRIBUTION_RULE_MISSING";
        } else ++json;
        if(d->missing[0])++missing; else ++ready;
        printf("| %lu | %s | %u | %s | %u | %s %u-%u-%u | %u/%u/%u/%u %s | %s | %s |\n",
            (unsigned long)d->id,d->origin==EXP_CE_ORIGIN_LEAGUE?"leagues.json":d->source_file,
            (unsigned)d->type,d->name,(unsigned)d->nphases,prov(p0->table.provenance),
            (unsigned)p0->table.win,(unsigned)p0->table.draw,(unsigned)p0->table.loss,
            (unsigned)p0->match.bench,(unsigned)p0->match.subs,(unsigned)p0->match.windows,
            (unsigned)p0->match.discipline,prov(p0->match.provenance),roll,
            d->missing[0]?d->missing:"READY");
    }
    printf("\nSUMMARY definitions=%lu leagues=%lu json=%lu ready=%lu missing=%lu "
           "league_families_chain_ok=%lu league_families_need_rule=%lu\n",
           (unsigned long)ExpCe_DefinitionCount(),(unsigned long)leagues,
           (unsigned long)json,(unsigned long)ready,(unsigned long)missing,
           (unsigned long)chain_ok,(unsigned long)chain_bad);
    assert(leagues==134u);      /* every division of leagues.json */
    {   /* v309: the REAL Serie B definition: play-off for the 3rd promotion from the data */
        const ExpCeDef *sb=ExpCe_Definition(ExpCe_LeagueCompetitionId(20u,61u,1u));
        const ExpCeDef *sa=ExpCe_Definition(ExpCe_LeagueCompetitionId(20u,61u,0u));
        assert(sb && !sb->missing[0] && sb->promotion_playoff==1u && sb->nphases==4u &&
               sb->phases[0].qualify==8u && sb->phases[0].qualify_from==3u &&
               sb->phases[1].byes==2u && sb->phases[1].nrel==1u && sb->phases[1].rel[0]==4u &&
               sb->phases[2].nrel==2u && sb->phases[2].rel[1]==11u &&
               sb->phases[3].rel[0]==16u && sb->phases[3].rel[1]==21u &&
               sb->phases[1].match.rank_tiebreak==1u && sb->phases[1].match.extra_time==1u &&
               !sb->phases[1].match.penalties && sb->promoted==3u);
        assert(sa && sa->nphases==1u && !sa->promotion_playoff);   /* Serie A is unchanged */
        /* v311: Serie C (three parallel groups) promotes 4 for now: B relegates 3 direct + the
         * play-out loser, the play-out is ACTIVE, and the conditional gap rules are loaded */
        assert(sb->relegated==4u && sb->par_n==3u && sb->par_groups[0]==2u && sb->par_groups[2]==4u &&
               sb->par_per_group==1u && sb->par_extra==1u);
        assert(sb->has_playout==1u && sb->playout_hi==16u && sb->playout_lo==17u &&
               sb->playout_relegated_total==4u && ExpCe_PlayoutActive(sb) &&
               sb->playout.nrel==2u && sb->playout.rel[0]==7u && sb->playout.rel[1]==14u &&
               sb->playout.match.rank_tiebreak==2u && sb->playout_gap_min==4u);
        assert(sb->fin_gap_hi==3u && sb->fin_gap_lo==4u && sb->fin_gap_min==14u &&
               sb->phases[3].match.rank_tiebreak==2u);
        puts("SERIE_B_PLAYOFF_DEFINITION_FROM_REAL_JSON=PASS");
    }
    {   /* v311: regional finals series of the three playable Australian leagues from the real JSON */
        const ExpCeDef *nsw=ExpCe_Definition(ExpCe_LeagueCompetitionId(44u,88u,1u));
        const ExpCeDef *vic=ExpCe_Definition(ExpCe_LeagueCompetitionId(44u,88u,2u));
        const ExpCeDef *qld=ExpCe_Definition(ExpCe_LeagueCompetitionId(44u,88u,3u));
        assert(nsw && vic && qld && !nsw->missing[0] && !vic->missing[0] && !qld->missing[0]);
        assert(nsw->nphases==4u && vic->nphases==4u && qld->nphases==4u);
        assert(nsw->phases[0].qualify==6u && nsw->phases[1].byes==2u && nsw->phases[1].rel[0]==3u &&
               nsw->phases[2].rel[0]==7u && nsw->phases[3].rel[0]==14u);
        assert(qld->phases[0].qualify==4u && qld->phases[1].pairing==EXP_CE_PAIR_SOURCES &&
               qld->phases[1].nties==2u && qld->phases[2].tsrc[0][0][0]==3u &&   /* loser of QF */
               qld->phases[2].tsrc[0][1][0]==2u && qld->phases[3].nties==1u);     /* winner of EF */
        /* v311: bottom pools of the three pyramids from the shared pool (data: bottom_pools.json) */
        assert(nsw->pool_direct==1u && nsw->pool_playoff==1u && nsw->pool_n==3u && ExpCe_PlayoutActive(nsw) &&
               nsw->playout.rel[0]==6u && nsw->playout.rel[1]==13u && nsw->playout.higher_home==2u);
        assert(vic->pool_direct==3u && vic->pool_playoff==0u && vic->pool_n==4u);
        assert(qld->pool_direct==2u && qld->pool_playoff==0u && qld->pool_n==3u);
        {   uint32_t z;
            for(z=0u;z<nsw->pool_n;++z)assert((nsw->pool_clubs[z]>>16)==44u && (nsw->pool_clubs[z]&0xffffu)>=80u);
        }
        puts("AUSTRALIAN_REGIONAL_FINALS_FROM_REAL_JSON_NSW_VIC_SIX_TEAMS_QLD_PAGE_BRACKET=PASS");
    }
    {   /* v311: the Australian Championship is defined from the real JSON but BLOCKED by the OG bench
         * limit (approved bench 7 > 5): no rule is altered to bypass it */
        const ExpCeDef *ch=ExpCe_Definition(2884098u);
        assert(ch && ch->type==2u && ch->missing[0]);
        assert(strstr(ch->missing,"bench")!=NULL);
        puts("AUSTRALIAN_CHAMPIONSHIP_DEFINED_BLOCKED_BY_OG_BENCH_LIMIT_ONLY=PASS");
    }
    assert(json==5u);           /* Copa Bolivia, Libertadores, Supercoppa, Australia Cup, Australian Championship */
    assert(chain_ok==90u && chain_bad==1u); /* Italy: parallel groups */
    puts("CE_REAL_DATA_VALIDATOR_AND_MATRIX=PASS_STATIC_DATA_ONLY");
    return 0;
}
