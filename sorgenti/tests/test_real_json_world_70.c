/* SWOS Expanded: 70 years using the real installed JSON club catalogue.
 *
 * Unlike the 216-club synthetic stress test, club IDs, league membership,
 * division IDs and names are parsed from the installed data_json catalogues. We require
 * every eligible record to be accepted by the real Living World scheduler.
 *
 * Coverage limitation: this standalone test does not load the native
 * CompetitionLeague records, OG cup callbacks, real player value ratings,
 * or promotion-playoff outcomes. It uses the original native fixture table
 * via FixtureRounds_Install, with default league timing and 3-point wins.
 * It must NEVER be reported as an OG managerial end-to-end test.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mjson.h"
#include "expanded_world_live.h"
#include "fixture_rounds.h"

static char *read_catalog(const char *path,long *length) {
    FILE *f=fopen(path,"rb");
    if(!f)return NULL;
    if(fseek(f,0,SEEK_END)) {fclose(f);return NULL;}
    long n=ftell(f);
    if(n<1 || n>16000000 || fseek(f,0,SEEK_SET)) {
        fclose(f);return NULL;
    }
    char *buf=(char *)malloc((size_t)n+1u);
    if(!buf) {fclose(f);return NULL;}
    if(fread(buf,1,(size_t)n,f)!=(size_t)n) {
        free(buf);fclose(f);return NULL;
    }
    fclose(f);buf[n]=0;*length=n;return buf;
}
static int team_number(const char *json,int length,unsigned index,
                       const char *key,long *result) {
    char path[120];
    snprintf(path,sizeof path,"$.teams[%u].team.%s",index,key);
    return mjson_get_long(json,length,path,result);
}
static int team_name(const char *json,int length,unsigned index,
                     char *name,int cap) {
    char path[120];
    snprintf(path,sizeof path,"$.teams[%u].team.team_name",index);
    return mjson_get_string(json,length,path,name,cap)>0;
}
/* Catalogues used as tournament entrants or custom pools, not regular
 * national leagues. Their original OG cup/qualifier logic is preserved:
 * treating them as a league would invent a 34-team African division. */
static int non_domestic_catalog(const char *path) {
    static const char *const names[]={
        "AFRICA.json","ASIA.json","EUROPE.json","NORTH_AMERICA.json",
        "OCEANIA.json","SOUTH_AMERICA.json",
        "EUROCUP.json","EUROCWC.json","UEFACUP.json",
        "CUSTOM.json","POOLPLYR.json"
    };
    const char *base=strrchr(path,'/');
    base=base?base+1:path;
    for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);++i)
        if(strcmp(base,names[i])==0) return 1;
    return 0;
}
static int test_country_file(const char *path,
                             unsigned *clubs,unsigned *leagues,
                             unsigned *out_of_league,
                             unsigned long long *club_seasons) {
    long length=0,count=0;
    char *json=read_catalog(path,&length);
    if(!json) {
        fprintf(stderr,"[real-json] cannot open %s\n",path);
        return 0;
    }
    if(!mjson_get_long(json,(int)length,"$.count",&count) ||
       count<0 || count>100000) {
        fprintf(stderr,"[real-json] invalid count in %s\n",path);
        free(json);
        return 0;
    }
    if(!ExpandedWorldLive_Begin(1996)) {
        fprintf(stderr,"[real-json] cannot initialize world for %s\n",path);
        free(json);return 0;
    }
    unsigned accepted=0,not_league=0;
    int ok=1;
    for(unsigned i=0;i<(unsigned)count;++i) {
        long country=0,ordinal=0,division=0;
        char name[96];
        if(!team_number(json,(int)length,i,"country_number",&country) ||
           !team_number(json,(int)length,i,"team_ordinal",&ordinal) ||
           !team_number(json,(int)length,i,"league",&division) ||
           !team_name(json,(int)length,i,name,sizeof name)) {
            fprintf(stderr,"[real-json] malformed record %u in %s\n",i,path);
            ok=0;break;
        }
        /* Outside-league and national entries have no native league
         * schedule. Explicitly account for them; never invent fixtures. */
        if(division<0 || division>=16) {
            ++not_league;continue;
        }
        if(country<0 || country>255 || ordinal<0 || ordinal>255) {
            fprintf(stderr,"[real-json] invalid club ID %u in %s\n",i,path);
            ok=0;break;
        }
        uint16_t id=(uint16_t)(((uint16_t)country<<8)|(uint8_t)ordinal);
        if(!ExpandedWorldLive_AddClub((uint8_t)country,(uint8_t)division,
                  id,200,name,3)) {
            fprintf(stderr,"[real-json] unscheduled club %04x div=%ld file=%s\n",
                    (unsigned)id,division,path);
            ok=0;break;
        }
        ++accepted;
    }
    free(json);
    if(ok && accepted) {
        unsigned n=(unsigned)ExpandedWorldLive_Leagues();
        if(!ExpandedWorldLive_Seal() ||
           !ExpandedWorldLive_AdvanceWeeks(3679u) ||
           ExpandedWorldLive_ArchivedLeagues()!=(uint32_t)(n*70u)) {
            fprintf(stderr,"[real-json] incomplete 70 seasons: %s "
                    "leagues=%u archived=%lu\n",path,n,
                    (unsigned long)ExpandedWorldLive_ArchivedLeagues());
            ok=0;
        } else {
            *clubs+=accepted;
            *leagues+=n;
            *club_seasons+=(unsigned long long)accepted*70u;
        }
    }
    if(ok) {
        *out_of_league+=not_league;
        printf("[real-json] %s imported=%u nonleague=%u "
               "scheduled_leagues=%lu seasons=%u\n",
               path,accepted,not_league,
               (unsigned long)ExpandedWorldLive_Leagues(),accepted?70u:0u);
    }
    ExpandedWorldLive_End();
    return ok;
}
int main(int argc,char **argv) {
    unsigned files=0,clubs=0,leagues=0,excluded=0,failures=0;
    unsigned other_catalogs=0,other_records=0;
    unsigned long long club_seasons=0;
    if(argc<2) {
        puts("REAL_JSON_DOMESTIC_70_SEASONS=FAIL");
        return 1;
    }
    FixtureRounds_Install();
    for(int i=1;i<argc;++i) {
        if(non_domestic_catalog(argv[i])) {
            long len=0,count=0;
            char *js=read_catalog(argv[i],&len);
            if(!js || !mjson_get_long(js,(int)len,"$.count",&count) ||
               count<0 || count>100000L) {
                fprintf(stderr,"[real-json] invalid OG cup/national pool: %s\n",
                        argv[i]);
                ++failures;
            } else {
                ++other_catalogs;
                other_records+=(unsigned)count;
                printf("[real-json] OG non-league catalogue: %s records=%ld "
                       "league simulation=NOT_APPLICABLE\n",argv[i],count);
            }
            free(js);
            continue;
        }
        if(test_country_file(argv[i],&clubs,&leagues,&excluded,&club_seasons))
            ++files;
        else
            ++failures;
    }
    printf("REAL_JSON_FILES=%u CLUBS_SIMULATED=%u "
           "LEAGUES_SIMULATED=%u OUTSIDE_NATIVE_LEAGUE=%u "
           "CLUB_SEASONS=%llu FAILED_FILES=%u\n",
           files,clubs,leagues,excluded,club_seasons,failures);
    printf("REAL_JSON_NONDOMESTIC_CATALOGS=%u RECORDS=%u\n",
           other_catalogs,other_records);
    puts("REAL_JSON_NATIVE_COMPETITION_RULES=NOT_TESTED");
    puts("REAL_JSON_OG_CUPS=NOT_TESTED");
    puts("REAL_JSON_ALL_COMPETITIONS=NOT_IMPLEMENTED");
    if(failures || !files || !clubs || !leagues) {
        puts("REAL_JSON_DOMESTIC_70_SEASONS=FAIL");return 1;
    }
    puts("REAL_JSON_DOMESTIC_70_SEASONS=PASS");return 0;
}
