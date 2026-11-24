/* SWOS Expanded — Fase 2: Competition Engine definitions of a world.
 * League divisions from leagues.json + league_calendar.json (OG native
 * rules) and cups / multi-phase competitions from ce_competitions.json.
 * Shared by the runtime (START/CONTINUE) and the data validator test.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_ce_engine.h"
#include "expanded_universal_json_leagues.h"
#include "swos_io.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definitions: one per league division (leagues.json + league_calendar.json,
 * OG native rules) + data_json/worlds/2627/competitions/ce_competitions.json
 * (cups / multi-phase). Same builder at START and at CONTINUE. */
int ExpCe_BuildWorldDefinitions(uint16_t world)
{
    uint32_t i,leagues=0u;
    int json=0;
    ExpandedUniversalJsonLeague row;
    ExpCe_Reset();
    for(i=0;ExpandedUniversalJsonLeagues_At(i,&row);++i) {
        ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
        ExpCeWindow w;
        uint32_t b;
        if(!d)return 0;
        memset(&w,0,sizeof w);
        if(row.has_calendar) {
            w.first_m=row.first_month;w.first_d=row.first_day;
            w.last_m=row.last_month;w.last_d=row.last_day;
            w.last_year_offset=row.last_year_offset;
            w.weekdays=row.match_weekdays;w.extra_weekdays=row.extra_weekdays;
            w.rest=row.min_rest_days;
            for(b=0;b<row.break_count && b<EXP_CE_MAX_BREAKS;++b) {
                w.breaks[b][0]=row.breaks[b].from_month;w.breaks[b][1]=row.breaks[b].from_day;
                w.breaks[b][2]=row.breaks[b].to_month;w.breaks[b][3]=row.breaks[b].to_day;
            }
            w.nbreaks=(uint8_t)b;
        }
        (void)ExpCe_LeagueDefinition(world,row.country,
            row.comp_id,row.division,row.divisions,row.declared_clubs,
            row.meetings,row.has_rules?row.points_for_win:0u,row.subs,
            row.bench,row.promoted,row.relegated,row.promoted_po,
            row.relegated_po,row.has_calendar?&w:NULL,d);
        if(!row.has_rules)
            snprintf(d->missing,sizeof d->missing,
                     "leagues.json: points_for_win/substitutes/max_subs");
        if(ExpCe_AddDefinition(d))++leagues;
        free(d);
    }
    {
        char path[1024];
        const char *rel="data_json/worlds/2627/competitions/ce_competitions.json";
        FILE *file=fopen(swos_data_path(rel,path,(uint32_t)sizeof path),"rb");
        if(file) {
            long size=0;
            char *bytes=NULL;
            if(fseek(file,0,SEEK_END)==0 && (size=ftell(file))>0 &&
               size<16L*1024L*1024L && fseek(file,0,SEEK_SET)==0 &&
               (bytes=(char *)malloc((size_t)size+1u))!=NULL &&
               fread(bytes,1,(size_t)size,file)==(size_t)size) {
                bytes[size]=0;
                json=ExpCe_LoadJsonDefinitions(bytes,(int)size,
                    world,"ce_competitions.json");
            } else json=-1;
            free(bytes);fclose(file);
        }
    }
    fprintf(stderr,"[ce-engine] DEFINITIONS world=%s leagues=%lu json=%d "
        "source=leagues.json+league_calendar.json+ce_competitions.json\n",
        world==WORLD_2627?"2627":"9697",(unsigned long)leagues,json);
    if(json<0)fprintf(stderr,"[ce-engine] CE_COMPETITIONS_JSON_MALFORMED "
        "cups/multiphase=NOT_LOADED leagues-unaffected=1\n");
    return json<0?json:(int)leagues;
}
#endif
