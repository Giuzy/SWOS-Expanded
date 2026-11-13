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
    unsigned accepted=0,not_league=0,singletons=0;
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
        unsigned scheduled=0;
        if(!ExpandedWorldLive_Seal()) {
            fprintf(stderr,"[real-json] unable to seal %s\n",path);
            ok=0;
        }
        /* Count native SCHEDULED divisions, not every league tag.
         * Singleton groups are real source data but have no fixtures:
         * print each one, don't fabricate a 70-year league.
         * Any unscheduled group with >=2 teams remains a hard failure. */
        for(unsigned d=0;ok && d<n;++d) {
            uint8_t country=0,division=0,clubsIn=0,active=0;
            if(!ExpandedWorldLive_DivisionStatus(d,&country,&division,
                                                  &clubsIn,&active)) {
                ok=0;break;
            }
            if(active) {++scheduled;continue;}
            fprintf(stderr,"[real-json] UNSCHEDULED division file=%s "
                           "country=%u div=%u clubs=%u reason=%s\n",
                    path,(unsigned)country,(unsigned)division,
                    (unsigned)clubsIn,
                    clubsIn==1u?"SINGLETON_NO_FIXTURE":"UNSUPPORTED_NATIVE");
            if(clubsIn!=1u) ok=0;
            else {++not_league;++singletons;}
        }
        if(ok && (!scheduled ||
           !ExpandedWorldLive_AdvanceWeeks(3679u) ||
           ExpandedWorldLive_ArchivedLeagues()!=(uint32_t)(scheduled*70u))) {
            fprintf(stderr,"[real-json] incomplete 70 seasons: %s "
                    "scheduled=%u total_groups=%u archived=%lu\n",
                    path,scheduled,n,
                    (unsigned long)ExpandedWorldLive_ArchivedLeagues());
            ok=0;
        } else {
            *clubs+=accepted-singletons;
            *leagues+=scheduled;
            *club_seasons+=(unsigned long long)(accepted-singletons)*70u;
        }
    }
    if(ok) {
        *out_of_league+=not_league;
        printf("[real-json] %s imported=%u nonleague=%u "
               "scheduled_leagues=%lu seasons=%u\n",
               path,accepted,not_league,
               (unsigned long)ExpandedWorldLive_NativeScheduleLeagues(),
               accepted?70u:0u);
    }
    ExpandedWorldLive_End();
    return ok;
}

/* Unlike the per-file checks above, this stage loads EVERY domestic file
 * into ONE persistent Living World. That catches collisions, global limits,
 * omitted divisions and rollover bugs hidden by restarting for each file.
 * OG knockout cups/national pools remain out of scope: never fake them. */
static int test_all_domestic_files(int argc,char **argv) {
    unsigned accepted=0,excluded=0,files=0,scheduled=0,singletons=0;
    unsigned scheduled_clubs=0;
    unsigned long long expected_club_seasons=0;
    int ok=ExpandedWorldLive_Begin(1996);
    if (!ok) {
        puts("REAL_JSON_GLOBAL_70_SEASONS=FAIL");
        return 0;
    }
    for (int a=1;a<argc;++a) {
        if (non_domestic_catalog(argv[a])) continue;
        long length=0,count=0;
        char *json=read_catalog(argv[a],&length);
        if (!json || !mjson_get_long(json,(int)length,"$.count",&count) ||
            count<0 || count>100000L) {
            fprintf(stderr,"[global-json] invalid catalogue: %s\n",argv[a]);
            free(json);ok=0;break;
        }
        ++files;
        for (unsigned i=0;i<(unsigned)count;++i) {
            long country=0,ordinal=0,division=0;
            char name[96];
            if (!team_number(json,(int)length,i,"country_number",&country) ||
                !team_number(json,(int)length,i,"team_ordinal",&ordinal) ||
                !team_number(json,(int)length,i,"league",&division) ||
                !team_name(json,(int)length,i,name,sizeof name)) {
                fprintf(stderr,"[global-json] malformed %s record=%u\n",argv[a],i);
                ok=0;break;
            }
            if (division<0 || division>=16) { ++excluded;continue; }
            if (country<0 || country>255 || ordinal<0 || ordinal>255) {
                fprintf(stderr,"[global-json] invalid club ID: %s record=%u\n",argv[a],i);
                ok=0;break;
            }
            uint16_t id=(uint16_t)(((uint16_t)country<<8)|(uint8_t)ordinal);
            if (!ExpandedWorldLive_AddClub((uint8_t)country,(uint8_t)division,
                                            id,200,name,3)) {
                fprintf(stderr,"[global-json] duplicate/overflow/unsupported "
                               "club=%04x division=%ld file=%s\n",
                        (unsigned)id,division,argv[a]);
                ok=0;break;
            }
            ++accepted;
        }
        free(json);
        if(!ok)break;
    }
    if (ok && (accepted==0 || files==0 ||
               ExpandedWorldLive_Clubs()!=accepted ||
               !ExpandedWorldLive_Seal())) {
        fprintf(stderr,"[global-json] failed world seal or club inventory\n");
        ok=0;
    }
    if (ok) {
        const uint32_t groups=ExpandedWorldLive_Leagues();
        for(uint32_t i=0;i<groups;++i) {
            uint8_t country=0,division=0,count=0,active=0;
            if(!ExpandedWorldLive_DivisionStatus(i,&country,&division,
                                                 &count,&active)) {
                ok=0;break;
            }
            if(active) {
                ++scheduled;
                scheduled_clubs+=(unsigned)count;
            } else if(count==1u) {
                ++singletons;
                fprintf(stderr,"[global-json] UNSCHEDULED SINGLETON "
                               "country=%u div=%u\n",
                        (unsigned)country,(unsigned)division);
            } else {
                fprintf(stderr,"[global-json] UNSCHEDULED MULTICLUB "
                               "country=%u div=%u clubs=%u\n",
                        (unsigned)country,(unsigned)division,(unsigned)count);
                ok=0;
            }
        }
        if(scheduled_clubs+singletons!=accepted || !scheduled) {
            fprintf(stderr,"[global-json] coverage mismatch: imported=%u "
                           "scheduled=%u singletons=%u\n",
                    accepted,scheduled_clubs,singletons);
            ok=0;
        }
    }
    if(ok) {
        expected_club_seasons=(unsigned long long)scheduled_clubs*70u;
        if(!ExpandedWorldLive_AdvanceWeeks(3679u) ||
           ExpandedWorldLive_ArchivedLeagues()!=(uint32_t)(scheduled*70u) ||
           !ExpandedWorldLive_ArchivedResults()) {
            fprintf(stderr,"[global-json] 70-year advance incomplete: "
                           "archived=%lu expected=%u\n",
                    (unsigned long)ExpandedWorldLive_ArchivedLeagues(),
                    scheduled*70u);
            ok=0;
        }
    }
    printf("GLOBAL_JSON_FILES=%u DOMESTIC_CLUBS_IMPORTED=%u "
           "DOMESTIC_CLUBS_SIMULATED=%u "
           "UNSCHEDULED_SINGLETON_CLUBS=%u OUTSIDE_LEAGUE_RECORDS=%u "
           "GLOBAL_LEAGUES=%u GLOBAL_CLUB_SEASONS=%llu\n",
           files,accepted,scheduled_clubs,singletons,excluded,
           scheduled,expected_club_seasons);
    printf("GLOBAL_DOMESTIC_CLUB_COVERAGE=%s\n",
           ok && accepted==scheduled_clubs+singletons?"PASS":"FAIL");
    printf("REAL_JSON_GLOBAL_70_SEASONS=%s\n",ok?"PASS":"FAIL");
    puts("GLOBAL_CUPS_OG_OFFSCREEN=NOT_TESTED");
    ExpandedWorldLive_End();
    return ok;
}

/* Full real 1996/97 data audit. A team-file squad has sixteen roster
 * records. EU* TMD entries are duplicated cup entrants, while POOLPLYR
 * stores distinct available player entries, not clubs. This does NOT
 * simulate OG national or continental match calendars. */
static unsigned player_names_in_catalog(const char *js) {
    unsigned n=0;
    const char *p=js;
    while((p=strstr(p,"\"player_name\""))!=NULL) {
        ++n;
        p+=13;
    }
    return n;
}
static int test_entire_catalogue(int argc,char **argv) {
    unsigned files=0,teams=0,cupDuplicates=0,pool=0;
    unsigned roster=0,cupRoster=0,errors=0,unique=0,seenAgain=0;
    unsigned char seen[65536]={0};
    for(int a=1;a<argc;++a) {
        long length=0,count=0;
        char type[16];
        char *js=read_catalog(argv[a],&length);
        if(!js || !mjson_get_long(js,(int)length,"$.count",&count) ||
           mjson_get_string(js,(int)length,"$.type",type,sizeof type)<=0 ||
           count<0 || count>100000) {
            fprintf(stderr,"[db-audit] invalid catalogue %s\n",argv[a]);
            ++errors;free(js);continue;
        }
        ++files;
        int isPool=strcmp(type,"pool")==0, isTmd=strcmp(type,"tmd")==0;
        if(!isPool && !isTmd && strcmp(type,"team")!=0) {
            fprintf(stderr,"[db-audit] unexpected catalogue %s (%s)\n",
                    argv[a],type);
            ++errors;free(js);continue;
        }
        unsigned players=player_names_in_catalog(js);
        unsigned expected=(unsigned)count*(isPool?1u:16u);
        if(players!=expected) {
            fprintf(stderr,"[db-audit] players mismatch %s actual=%u expected=%u\n",
                    argv[a],players,expected);
            ++errors;
        }
        if(isPool) {
            pool+=(unsigned)count;
            for(unsigned i=0;i<(unsigned)count;++i) {
                char path[92],name[96];
                long id=0;
                snprintf(path,sizeof path,"$.players[%u].team_id",i);
                int okId=mjson_get_long(js,(int)length,path,&id);
                snprintf(path,sizeof path,"$.players[%u].player.player_name",i);
                int okName=mjson_get_string(js,(int)length,path,name,sizeof name)>0;
                if(!okId || !okName || id<0 || id>65535)
                    ++errors;
            }
        } else {
            if(isTmd) {cupDuplicates+=(unsigned)count;cupRoster+=players;}
            else {teams+=(unsigned)count;roster+=players;}
            for(unsigned i=0;i<(unsigned)count;++i) {
                long country=0,ordinal=0,division=0;
                char name[96];
                if(!team_number(js,(int)length,i,"country_number",&country) ||
                   !team_number(js,(int)length,i,"team_ordinal",&ordinal) ||
                   !team_number(js,(int)length,i,"league",&division) ||
                   !team_name(js,(int)length,i,name,sizeof name) ||
                   country<0 || country>255 || ordinal<0 || ordinal>255 ||
                   division<0 || division>255) {
                    fprintf(stderr,"[db-audit] invalid team %s[%u]\n",argv[a],i);
                    ++errors;continue;
                }
                unsigned id=((unsigned)country<<8)|(unsigned)ordinal;
                if(seen[id])++seenAgain;
                else {seen[id]=1;++unique;}
            }
        }
        free(js);
    }
    int pass=files==76u && teams==1620u && cupDuplicates==80u &&
             pool==900u && roster==25920u && cupRoster==1280u &&
             errors==0;
    printf("DATABASE_ALL_FILES=%u DATABASE_TEAMS=%u "
           "DATABASE_ROSTER_SLOTS=%u DATABASE_POOL_PLAYERS=%u "
           "DATABASE_TOTAL_PLAYER_SLOTS=%u DATABASE_TMD_DUPLICATES=%u "
           "DATABASE_UNIQUE_TEAM_IDS=%u DATABASE_REUSED_IDS=%u "
           "DATABASE_INVALID=%u\n",
           files,teams,roster,pool,roster+pool,cupDuplicates,unique,
           seenAgain,errors);
    printf("DATABASE_ALL_RECORDS_AUDIT=%s\n",pass?"PASS":"FAIL");
    puts("OG_NATIONAL_FIXTURES=NOT_TESTED");
    puts("OG_CONTINENTAL_CUP_FIXTURES=NOT_TESTED");
    return pass;
}

int main(int argc,char **argv) {
    unsigned files=0,clubs=0,leagues=0,excluded=0,failures=0;
    unsigned other_catalogs=0,other_records=0;
    unsigned long long club_seasons=0;
    if(argc<2) {
        puts("REAL_JSON_DOMESTIC_70_SEASONS=FAIL");
        return 1;
    }
    if(!test_entire_catalogue(argc,argv)) return 1;
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
    puts("REAL_JSON_SINGLETON_DIVISIONS=REPORTED_NOT_SIMULATED");
    puts("REAL_JSON_NATIVE_COMPETITION_RULES=NOT_TESTED");
    puts("REAL_JSON_OG_CUPS=NOT_TESTED");
    puts("REAL_JSON_ALL_COMPETITIONS=NOT_IMPLEMENTED");
    if(failures || !files || !clubs || !leagues) {
        puts("REAL_JSON_DOMESTIC_70_SEASONS=FAIL");return 1;
    }
    puts("REAL_JSON_DOMESTIC_70_SEASONS=PASS");
    fflush(stdout); /* ensure stdout markers are not interleaved with stderr */
    return test_all_domestic_files(argc,argv)?0:1;
}
