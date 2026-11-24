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
    assert(json==2u);           /* Copa Bolivia, Libertadores */
    assert(chain_ok==90u && chain_bad==1u); /* Italy: parallel groups */
    puts("CE_REAL_DATA_VALIDATOR_AND_MATRIX=PASS_STATIC_DATA_ONLY");
    return 0;
}
