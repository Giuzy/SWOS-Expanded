/* SWOS Expanded: inspect a REAL Windows gameplay SWOS.log.
 * Read-only. An isolated C stress suite or a database inventory cannot
 * certify an OG career: this parser uses native resident-world markers.
 * Exit 0 only for a complete integration (cups and persistence included);
 * exit 1 for observed regressions; exit 2 for partial/missing evidence.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define LINE_CAP 8192
#define MAX_CUP_COVERAGE 256u
static long field(const char *,const char *,long);
typedef struct {
    int seen;
    long cup_definitions,cup_resident,cup_matches,cup_unsupported,cup_faults;
    long scorer_players,scorer_positive,scorer_goals,standings_goals;
} FeatureSnapshot;
typedef struct {
    long country,slot,compid,year,expected,selected;
    long real,ties,byes,padded,native_bracket,native_ties,native_pairs;
    char state[24],reason[64],rule[48],round_structure[40];
} CupCoverageRecord;
static void feature_read(FeatureSnapshot *out,const char *line)
{
    out->seen=1;
    out->cup_definitions=field(line,"cup-definitions",-1);
    out->cup_resident=field(line,"cup-resident",-1);
    out->cup_matches=field(line,"cup-matches",-1);
    out->cup_unsupported=field(line,"cup-unsupported",-1);
    out->cup_faults=field(line,"cup-faults",-1);
    out->scorer_players=field(line,"scorer-players",-1);
    out->scorer_positive=field(line,"scorer-positive",-1);
    out->scorer_goals=field(line,"scorer-league-goals",-1);
    out->standings_goals=field(line,"standings-goals",-1);
}
static long field(const char *s,const char *key,long fallback)
{
    char needle[80];
    const char *p;
    if(snprintf(needle,sizeof needle,"%s=",key)<0)return fallback;
    p=strstr(s,needle);
    if(!p)return fallback;
    p+=strlen(needle);
    if(*p=='-' || (*p>='0'&&*p<='9'))return strtol(p,NULL,10);
    return fallback;
}
static int tag(const char *s,const char *prefix)
{
    return strncmp(s,prefix,strlen(prefix))==0;
}
static void remember(char *out,size_t size,const char *what,
                     long row,const char *line)
{
    if(*out)return;
    snprintf(out,size,"%s line=%ld %.350s",what,row,line);
    size_t n=strlen(out);
    while(n&&(out[n-1]=='\r'||out[n-1]=='\n'))out[--n]=0;
}
int main(int argc,char **argv)
{
    const char *filename=argc>1?argv[1]:"SWOS.log";
    const char *expected=argc>2?argv[2]:"";
    FILE *in=fopen(filename,"rb");
    char line[LINE_CAP],first[500]="",build[80]="";
    long rows=0,start_ready=0,start_audit=0,ticks=0;
    long leagues=0,played=0,completed=0,matches=0,countries=0,invalid=0;
    long cup_defs=0,cup_countries=0,cup_resident=0,yearend=0;
    long views=0,view_missing=0,binds=0,career_saves=0,career_loads=0;
    long title_save_ok=0,title_save_fail=0;
    long f12_main=0,f12_cups=0,f10_data=0;
    long picker_backs=0,picker_pops=0,picker_unmatched=0;
    long native_faults=0,regressions=0,log_year=-1,log_phase=-1;
    long native_league_unsupported=0,unsupported_country=-1;
    long unsupported_division=-1,unsupported_expected=-1,unsupported_actual=-1;
    long prev_year=-1,prev_phase=-1;
    long season_closes=0,season_opens=0,season_failures=0;
    long og_stress_started=0,og_stress_finished=0,og_stress_date_exact=0;
    long og_stress_armed=0,stress_steps=0,stress_closes=0,stress_opens=0;
    long last_step_number=0,og_career_sessions=0;
    unsigned start_year=0,start_month=0,start_day=0;
    long last_op_year=-1,last_op_phase=-1,last_op_country=-1,last_op_division=-1;
    int pending_native_step=0;
    FeatureSnapshot feature_before={0},feature_after={0},feature_start={0};
    FeatureSnapshot offer_before={0},offer_after={0};
    FeatureSnapshot save_after={0},restore_after={0};
    long save_checkpoint_count=0,restore_checkpoint_count=0;
    long external_starts=0,external_restores=0,external_national=0;
    long external_qualifiers=0,external_finalists=0,external_uefa_clubs=0;
    long external_conti_season=0,external_worldcup_seasons=0;
    static const char *const external_names[]={
        "CHAMPIONS_LEAGUE","EUROPA_LEAGUE","CONFERENCE_LEAGUE",
        "EUROPEAN_CHAMPIONS_CUP","CUP_WINNERS_CUP","UEFA_CUP",
        "COPA_LIBERTADORES","OTHER_CONTINENTAL_CLUB_CUPS",
        "WORLD_CUP","EUROPEAN_CHAMPIONSHIPS","COPA_AMERICA",
        "AFRICAN_NATIONS_CUP","NORTH_AMERICAN_CUP","ASIAN_CUP",
        "OCEANIA_CUP","WORLD_CUP_QUALIFIERS","EURO_QUALIFIERS"
    };
    long external_seen[sizeof(external_names)/sizeof(external_names[0])]={0};
    long external_defined[sizeof(external_names)/sizeof(external_names[0])]={0};
    long external_activity[sizeof(external_names)/sizeof(external_names[0])]={0};
    long offer_native_views=0,offer_view_failures=0;
    long scorer_resident_views=0,scorer_local_views=0,scorer_resident_players=0;
    long viewer_back=0,picker_back_events=0;
    long cup_native_bye_verified=0,cup_native_bye_rejected=0;
    CupCoverageRecord cup_coverage[MAX_CUP_COVERAGE];
    unsigned cup_coverage_count=0;
    long cup_coverage_year=-1;
    int have_audit=0,after_start=0;
    if(!in) {
        puts("RUNTIME_LOG=NOT_TESTED reason=SWOS.log_missing");
        puts("WINDOWS_OG_INTEGRATION=INCOMPLETE reason=NO_LOG");
        puts("OVERALL=INCOMPLETE");
        return 2;
    }
    while(fgets(line,sizeof line,in)) {
        ++rows;
        if(tag(line,"[build] SWOS Expanded ")) {
            char value[80];
            if(sscanf(line,"[build] SWOS Expanded %79s",value)==1)
                snprintf(build,sizeof build,"%s",value);
        }
        /* A new career resets the resident OG world before any START TICK.
         * The old parser reset only on START READY (too late), creating the
         * observed false OG_DATE_REWIND across two NEW CAREER operations. */
        if(tag(line,"[og-career] RESET")) {
            ++og_career_sessions;
            start_ready=start_audit=ticks=0;
            leagues=played=completed=matches=countries=invalid=0;
            cup_defs=cup_countries=cup_resident=yearend=0;
            views=view_missing=binds=career_saves=career_loads=0;
            title_save_ok=title_save_fail=0;
            f12_main=f12_cups=f10_data=0;
            picker_backs=picker_pops=picker_unmatched=0;
            native_faults=regressions=0;log_year=log_phase=-1;
            prev_year=prev_phase=-1;
            season_closes=season_opens=season_failures=0;
            og_stress_started=og_stress_finished=og_stress_date_exact=0;
            og_stress_armed=stress_steps=stress_closes=stress_opens=0;
            last_step_number=0;
            start_year=start_month=start_day=0;
            have_audit=after_start=0;
            feature_before.seen=feature_after.seen=feature_start.seen=0;
            offer_before.seen=offer_after.seen=0;
            save_after.seen=restore_after.seen=0;
            save_checkpoint_count=restore_checkpoint_count=0;
            external_starts=external_restores=external_national=0;
            external_qualifiers=external_finalists=external_uefa_clubs=0;
            external_conti_season=external_worldcup_seasons=0;
            for(size_t k=0;k<sizeof(external_names)/sizeof(external_names[0]);++k)
                external_seen[k]=external_defined[k]=external_activity[k]=0;
            offer_native_views=offer_view_failures=0;
            scorer_resident_views=scorer_local_views=scorer_resident_players=0;
            viewer_back=picker_back_events=0;
            cup_native_bye_verified=cup_native_bye_rejected=0;
            cup_coverage_count=0;cup_coverage_year=-1;
            last_op_year=last_op_phase=last_op_country=last_op_division=-1;
            pending_native_step=0;
            first[0]=0;
        }
        if(tag(line,"[og-native-league-preflight] UNSUPPORTED ")) {
            ++native_league_unsupported;
            unsupported_country=field(line,"country",-1);
            unsupported_division=field(line,"division",-1);
            unsupported_expected=field(line,"expected",-1);
            unsupported_actual=field(line,"actual",-1);
            /* An unsupported league is not a simulated fixture or a
             * crash; report INCOMPLETE, never count it as resident. */
        }
        if(tag(line,"[og-world] START READY:")) {
            unsigned cy=0,cm=0,cd=0;
            if(sscanf(line,"[og-world] START READY: date=%4u-%2u-%2u",
                      &cy,&cm,&cd)!=3 || cm!=7u || cd!=1u) {
                ++regressions;
                remember(first,sizeof first,"START_DATE_NOT_JULY_1",rows,line);
            } else {
                start_year=cy; start_month=cm; start_day=cd;
            }
            ++start_ready;
            after_start=1;
            prev_year=-1;
            prev_phase=-1;
            picker_unmatched=0;
        }
        if(tag(line,"[og-world-audit] ")) {
            const int is_start=strstr(line,"stage=START")!=NULL;
            const int is_tick=strstr(line,"stage=TICK")!=NULL;
            const long new_year=field(line,"year",-1);
            const long new_phase=field(line,"phase",-1);
            const long new_leagues=field(line,"resident-leagues",-1);
            const long new_invalid=field(line,"invalid",-1);
            have_audit=1;
            if(is_start)++start_audit;
            if(is_tick && after_start) {
                ++ticks;
                if(prev_year>=0 &&
                   (new_year<prev_year ||
                    (new_year==prev_year&&new_phase<prev_phase))) {
                    ++regressions;
                    remember(first,sizeof first,"OG_DATE_REWIND",rows,line);
                }
                prev_year=new_year;
                prev_phase=new_phase;
            }
            if(new_leagues<=0 || new_invalid<0 || new_invalid>0 ||
               new_year<0 || new_phase<0) {
                ++regressions;
                remember(first,sizeof first,"OG_INVALID_RESIDENT_STATE",rows,line);
            }
            log_year=new_year;
            log_phase=new_phase;
            leagues=new_leagues;
            played=field(line,"played-leagues",0);
            completed=field(line,"completed-leagues",0);
            matches=field(line,"native-matches",0);
            invalid=new_invalid;
            countries=field(line,"countries",0);
            cup_defs=field(line,"cup-definitions",0);
            cup_countries=field(line,"cup-countries",0);
            cup_resident=field(line,"cup-resident",0);
            yearend=field(line,"yearend-callbacks",0);
        }
        /* Genuine OG completion and restart, emitted for each year by
         * Expanded_OGWorld_AdvanceToDate. Do NOT count old LWE suites. */
        if(tag(line,"[og-season-transition] CLOSE")) {
            ++season_closes;
            if(og_stress_started && !og_stress_finished)++stress_closes;
            if(field(line,"native-matches",0)<=0 ||
               field(line,"resident-leagues",0)<=0 ||
               field(line,"played-leagues",0)<=0 ||
               field(line,"invalid",1)!=0) {
                ++season_failures;
                remember(first,sizeof first,"OG_SEASON_CLOSE_EMPTY",rows,line);
            }
        }
        if(tag(line,"[og-season-transition] OPEN")) {
            ++season_opens;
            if(og_stress_started && !og_stress_finished)++stress_opens;
            if(field(line,"resident-leagues",0)<=0 ||
               field(line,"invalid",1)!=0) {
                ++season_failures;
                remember(first,sizeof first,"OG_SEASON_OPEN_INVALID",rows,line);
            }
        }
        if(tag(line,"[og-season-transition] FAIL")) {
            ++season_failures;
            remember(first,sizeof first,"OG_SEASON_TRANSITION_FAILED",rows,line);
        }
        if(tag(line,"[og-stress] ARMED years=70"))++og_stress_armed;
        if(tag(line,"[og-stress] START years=70")) {
            ++og_stress_started;
            stress_steps=stress_closes=stress_opens=0;
            last_step_number=0;
            feature_after.seen=0; /* BEFORE checkpoint is logged before START */
        }
        if(tag(line,"[og-stress] STEP")) {
            const long num=field(line,"number",-1);
            ++stress_steps;
            if(num!=stress_steps || num>70) {
                ++season_failures;
                remember(first,sizeof first,"OG_STRESS_STEP_ORDER",rows,line);
            }
            last_step_number=num;
        }
        if(tag(line,"[og-stress] FAILED") ||
           tag(line,"[og-stress] ARM FAILED")) {
            ++season_failures;
            remember(first,sizeof first,"OG_STRESS_FAILED",rows,line);
        }
        if(tag(line,"[og-cup-bye] NATIVE DRAW VERIFIED"))++cup_native_bye_verified;
        if(tag(line,"[og-cup-bye] REJECT"))++cup_native_bye_rejected;
        if(tag(line,"[og-cup-coverage] ")) {
            const long year=field(line,"year",-1);
            if(year>cup_coverage_year) {
                cup_coverage_year=year;
                cup_coverage_count=0;
            }
            if(year==cup_coverage_year &&
               cup_coverage_count<MAX_CUP_COVERAGE) {
                CupCoverageRecord *c=&cup_coverage[cup_coverage_count++];
                memset(c,0,sizeof(*c));
                c->country=field(line,"country",-1);
                c->slot=field(line,"slot",-1);
                c->compid=field(line,"compId",-1);
                c->year=year;
                c->expected=field(line,"expected",-1);
                c->selected=field(line,"selected",-1);
                c->real=c->ties=c->byes=c->padded=-1;
                c->native_bracket=c->native_ties=c->native_pairs=-1;
                const char *p=strstr(line,"state=");
                if(p)sscanf(p+6,"%23s",c->state);
                p=strstr(line,"reason=");
                if(p)sscanf(p+7,"%63s",c->reason);
            }
        }
        if(tag(line,"[og-cup-rule] ")) {
            const long year=field(line,"year",-1);
            const long country=field(line,"country",-1);
            const long slot=field(line,"slot",-1);
            for(unsigned i=0;i<cup_coverage_count;++i) {
                CupCoverageRecord *c=&cup_coverage[i];
                if(c->year!=year || c->country!=country || c->slot!=slot)
                    continue;
                c->real=field(line,"real-entrants",-1);
                c->ties=field(line,"first-round-ties",-1);
                c->byes=field(line,"byes",-1);
                c->padded=field(line,"padded-slots",-1);
                c->native_bracket=field(line,"native-bracket",-1);
                c->native_ties=field(line,"native-round1-ties",-1);
                c->native_pairs=field(line,"native-round1-pairs",-1);
                const char *p=strstr(line,"round-structure=");
                if(p)sscanf(p+16,"%39s",c->round_structure);
                p=strstr(line,"classification=");
                if(p)sscanf(p+15,"%47s",c->rule);
                break;
            }
        }
        if(tag(line,"[og-external-audit] ")) {
            if(strstr(line,"stage=START "))++external_starts;
            if(strstr(line,"stage=RESTORE_AFTER "))++external_restores;
            external_national=field(line,"national-unique",-1);
            external_qualifiers=field(line,"qualifier-definitions",-1);
            external_finalists=field(line,"final-definitions",-1);
            external_uefa_clubs=field(line,"uefa-clubs",-1);
            external_conti_season=field(line,"conti-season",-1);
            external_worldcup_seasons=field(line,"world-cup-season-counter",-1);
        }
        if(tag(line,"[og-external-competition] ") &&
           strstr(line,"stage=START ")) {
            for(size_t k=0;k<sizeof(external_names)/sizeof(external_names[0]);++k) {
                char key[96];
                snprintf(key,sizeof key," name=%s ",external_names[k]);
                if(!strstr(line,key))continue;
                ++external_seen[k];
                external_defined[k]=field(line,"defined",-1);
                external_activity[k]=field(line,"activity",-1);
                break;
            }
        }
        if(tag(line,"[og-scorers-view] ")) {
            if(strstr(line,"source=RESIDENT_OG")) {
                ++scorer_resident_views;
                scorer_resident_players=field(line,"players-with-league-goals",-1);
            } else if(strstr(line,"source=LOCAL_UNVERIFIED"))
                ++scorer_local_views;
        }
        if(tag(line,"[og-world-back] VIEWER EXIT"))++viewer_back;
        if(tag(line,"[og-world-back] PAD B queued"))++picker_back_events;
        if(tag(line,"[og-feature-audit] ")) {
            if(strstr(line,"stage=START "))feature_read(&feature_start,line);
            if(strstr(line,"stage=STRESS_BEFORE "))feature_read(&feature_before,line);
            if(strstr(line,"stage=STRESS_AFTER "))feature_read(&feature_after,line);
            if(strstr(line,"stage=OFFER_BEFORE "))feature_read(&offer_before,line);
            if(strstr(line,"stage=OFFER_AFTER "))feature_read(&offer_after,line);
            if(strstr(line,"stage=SAVE_AFTER ")) {
                feature_read(&save_after,line);
                ++save_checkpoint_count;
            }
            if(strstr(line,"stage=RESTORE_AFTER ")) {
                feature_read(&restore_after,line);
                ++restore_checkpoint_count;
            }
        }
        if(tag(line,"[og-world-step] BEGIN")) {
            pending_native_step=1;
            last_op_year=field(line,"year",-1);
            last_op_phase=field(line,"phase",-1);
            last_op_country=field(line,"country",-1);
            last_op_division=field(line,"division",-1);
        }
        if(tag(line,"[og-world-step] END")) {
            if(pending_native_step &&
               last_op_year==field(line,"year",-2) &&
               last_op_phase==field(line,"phase",-2) &&
               last_op_country==field(line,"country",-2) &&
               last_op_division==field(line,"division",-2))
                pending_native_step=0;
            else {
                ++season_failures;
                remember(first,sizeof first,"OG_WORLD_STEP_MISMATCH",rows,line);
            }
        }
        if(tag(line,"[og-stress] FINISHED years=70")) {
            ++og_stress_finished;
            if(field(line,"exact",0)==1)++og_stress_date_exact;
            else {
                ++season_failures;
                remember(first,sizeof first,"OG_70_YEAR_DATE_MISMATCH",rows,line);
            }
        }
        if(tag(line,"[og-offer-view] SELECT")) {
            ++offer_native_views;
            if(field(line,"ok",0)!=1) {
                ++offer_view_failures;
                remember(first,sizeof first,"OG_OFFER_VIEW_MUTATION",rows,line);
            }
        }
        if(tag(line,"[og-world-view] READ ONLY"))++views;
        if(tag(line,"[og-world-view] MISSING/INVALID"))++view_missing;
        if(tag(line,"[og-bind] "))++binds;
        if(tag(line,"[autosave] saved "))++career_saves;
        if(tag(line,"[autosave] restored "))++career_loads;
        if(tag(line,"[autosave-title] ")) {
            if(strstr(line,"status=COMPATIBLE_ON_DISK"))++title_save_ok;
            else if(strstr(line,"status=WRITE_FAILED") ||
                    strstr(line,"status=CHECK_FAILED"))++title_save_fail;
        }
        if(tag(line,"[f12-snapshot] ")) {
            if(strstr(line,"page=F12_OG_MAIN"))++f12_main;
            if(strstr(line,"page=F12_OG_CUPS"))++f12_cups;
            if(strstr(line,"page=F10_OG_DATA"))++f10_data;
        }
        if(tag(line,"[og-world-picker] BACK")) {
            if(strstr(line,"root=0")){++picker_backs;++picker_unmatched;}
        }
        if(tag(line,"[og-world-picker] POP")) {
            ++picker_pops;
            if(picker_unmatched>0)--picker_unmatched;
            else {
                ++regressions;
                remember(first,sizeof first,"UNEXPECTED_PICKER_POP",rows,line);
            }
        }
        /* A legitimate [og-world-fixture] FINISHED is NOT a failure. */
        if(tag(line,"[og-cup-resident] FAILED") ||
           tag(line,"[og-cup-resident] ERROR") ||
           tag(line,"[og-cup-resident] LOOP LIMIT") ||
           tag(line,"[og-world-fixture] INVALID") ||
           tag(line,"[og-world-fixture] LOOP LIMIT") ||
           tag(line,"[og-world-fixture] CATCHUP LOOP LIMIT") ||
           tag(line,"[og-world] WORLD TICK FAILED") ||
           tag(line,"[og-world] LEAGUE ERROR") ||
           tag(line,"[og-world] FORWARD TIME FAILED") ||
           tag(line,"[og-world] START FAILED") ||
           tag(line,"[calendar] FAIL ")) {
            ++native_faults;
            remember(first,sizeof first,"NATIVE_OG_RUNTIME",rows,line);
        }
    }
    fclose(in);
    if(og_stress_started &&
       (og_stress_started!=og_stress_finished ||
        stress_closes<og_stress_finished*70 ||
        stress_opens<og_stress_finished*70 ||
        stress_steps<og_stress_finished*70)) {
        ++season_failures;
        remember(first,sizeof first,"OG_70_YEAR_TEST_INCOMPLETE",
                 rows,"70 OG year transition closes/opens absent or unfinished");
    }
    if(offer_before.seen && offer_after.seen &&
       (offer_before.cup_resident!=offer_after.cup_resident ||
        offer_before.scorer_goals!=offer_after.scorer_goals ||
        offer_before.standings_goals!=offer_after.standings_goals)) {
        ++offer_view_failures;
        remember(first,sizeof first,"OG_OFFER_FEATURES_MUTATED",
                 rows,"Offer viewer changed resident OG cups/scorer totals");
    }
    if(picker_unmatched) {
        ++regressions;
        remember(first,sizeof first,"PICKER_BACK_NOT_POPPED",
                 rows,"Nested BACK did not return exactly one selector level");
    }
    const int correct_build=build[0] && expected[0] &&
                            strcmp(build,expected)==0;
    printf("RUNTIME_LOG=READ lines=%ld build=%s expected=%s scope=%s\n",
           rows,build[0]?build:"UNKNOWN",expected[0]?expected:"UNSPECIFIED",
           correct_build?"CURRENT_BUILD":"OTHER_BUILD");
    printf("OG_START=%s ready=%ld audits=%ld date=%04u-%02u-%02u\n",
           start_ready&&start_audit&&have_audit&&start_month==7u&&start_day==1u?
           "PASS":"NOT_TESTED",start_ready,start_audit,
           start_year,start_month,start_day);
    printf("OG_RESIDENT_LEAGUES=%s contexts=%ld countries=%ld played=%ld "
           "completed=%ld native-matches=%ld invalid=%ld year=%ld phase=%ld\n",
           have_audit&&leagues>0&&invalid==0?"PASS":
           have_audit?"FAIL":"NOT_TESTED",
           leagues,countries,played,completed,matches,invalid,log_year,log_phase);
    printf("OG_FORWARD_TIME=%s post-start-ticks=%ld\n",
           ticks>0&&!native_faults&&!regressions&&!season_failures?
           "OBSERVED_NO_CRASH":ticks?"FAIL":"NOT_TESTED",ticks);
    printf("OG_SEASON_TRANSITIONS=%s closed=%ld opened=%ld failed=%ld\n",
           season_closes && season_closes==season_opens &&
           !season_failures?"OBSERVED":season_failures?"FAIL":"NOT_TESTED",
           season_closes,season_opens,season_failures);
    printf("OG_70_NATIVE_SEASONS=%s armed=%ld started=%ld finished=%ld "
           "closed=%ld opened=%ld steps=%ld exact-dates=%ld\n",
           og_stress_finished>0 && og_stress_finished==og_stress_started &&
           og_stress_date_exact==og_stress_finished &&
           stress_closes==og_stress_finished*70 &&
           stress_opens==og_stress_finished*70 &&
           stress_steps==og_stress_finished*70 &&
           last_step_number==70 &&
           !season_failures && !native_faults && !regressions ?
             "PASS_TRANSITIONS_ONLY" :
           og_stress_started && season_failures?"FAIL":
           "NOT_TESTED",
           og_stress_armed,og_stress_started,og_stress_finished,
           stress_closes,stress_opens,stress_steps,og_stress_date_exact);
    printf("OG_LAST_WORLD_STEP=%s year=%ld phase=%ld country=%ld division=%ld\n",
           pending_native_step?"NO_END_RECORDED":"NONE_PENDING",
           last_op_year,last_op_phase,last_op_country,last_op_division);
    printf("OG_CUPS_BEFORE_AFTER=%s before=%ld after=%ld definitions=%ld\n",
           feature_before.seen && feature_after.seen ?
             (feature_before.cup_resident==0 && feature_after.cup_resident==0?
             "NOT_IMPLEMENTED":"RESIDENT_CONTEXTS_PRESENT") :
           feature_before.seen?"AFTER_MISSING":"NOT_TESTED",
           feature_before.seen?feature_before.cup_resident:-1,
           feature_after.seen?feature_after.cup_resident:-1,
           feature_start.seen?feature_start.cup_definitions:-1);
    printf("OG_CUP_NATIVE_MATCHES=%s resident-before=%ld matches-before=%ld "
           "resident-after=%ld matches-after=%ld unsupported-after=%ld "
           "faults-after=%ld\n",
           feature_before.seen && feature_after.seen ?
             (feature_after.cup_resident>0 &&
              feature_after.cup_matches>0 && feature_after.cup_faults==0?
               "OBSERVED_NATIVE_RESULTS":"INCOMPLETE_OR_NO_RESULTS") :
           feature_after.seen?"NO_BEFORE":"NOT_TESTED",
           feature_before.seen?feature_before.cup_resident:-1,
           feature_before.seen?feature_before.cup_matches:-1,
           feature_after.seen?feature_after.cup_resident:-1,
           feature_after.seen?feature_after.cup_matches:-1,
           feature_after.seen?feature_after.cup_unsupported:-1,
           feature_after.seen?feature_after.cup_faults:-1);
    printf("OG_SCORERS_BEFORE_AFTER=%s players-before=%ld "
           "goals-before=%ld standings-before=%ld "
           "players-after=%ld goals-after=%ld standings-after=%ld\n",
           feature_before.seen && feature_after.seen?
              (feature_after.standings_goals>0 && feature_after.scorer_goals==0?
               "PLAYER_COUNTERS_EMPTY":"NATIVE_COUNTERS_OBSERVED") :
           feature_before.seen?"AFTER_MISSING":"NOT_TESTED",
           feature_before.seen?feature_before.scorer_players:-1,
           feature_before.seen?feature_before.scorer_goals:-1,
           feature_before.seen?feature_before.standings_goals:-1,
           feature_after.seen?feature_after.scorer_players:-1,
           feature_after.seen?feature_after.scorer_goals:-1,
           feature_after.seen?feature_after.standings_goals:-1);
    printf("OG_TOP_GOAL_SCORERS_VIEWER=%s resident-views=%ld "
           "unverified-local-views=%ld players-with-goals-last=%ld "
           "visual-contents=NOT_TESTED\n",
           scorer_resident_views?"RESIDENT_DATA_READ_OBSERVED":
           scorer_local_views?"LOCAL_SOURCE_UNVERIFIED":"NOT_TESTED",
           scorer_resident_views,scorer_local_views,scorer_resident_players);
    puts("OG_RETIREMENTS=NOT_TESTED (manager/player lifecycle separate)");
    printf("OG_RESIDENT_SAVE_CHECKPOINTS=OBSERVED_IN_RAM saved=%ld "
           "restored=%ld player-records-before=%ld player-records-after=%ld "
           "cup-contexts-before=%ld cup-contexts-after=%ld "
           "scorer-goals-before=%ld scorer-goals-after=%ld\n",
           save_checkpoint_count,restore_checkpoint_count,
           save_after.seen?save_after.scorer_players:-1L,
           restore_after.seen?restore_after.scorer_players:-1L,
           save_after.seen?save_after.cup_resident:-1L,
           restore_after.seen?restore_after.cup_resident:-1L,
           save_after.seen?save_after.scorer_goals:-1L,
           restore_after.seen?restore_after.scorer_goals:-1L);
    printf("OG_RESIDENT_SAVE_RELOAD=%s "
           "year-restored-user-observation=SEPARATE\n",
           !restore_checkpoint_count?"NOT_TESTED":
           restore_after.scorer_players==0 && restore_after.cup_resident==0?
               "NATIVE_CONTEXTS_ABSENT_AFTER_LOAD":
           "COUNTERS_SEEN_BUT_FULL_ID_FIXTURES_RNG_NOT_PROVED");
    printf("OG_TITLE_AUTOSAVE=%s verified=%ld failures=%ld "
           "scope=SCHEMA_KIND_WORLD_ONLY cross-process-load=SEPARATE\n",
           title_save_fail?"FAIL":title_save_ok?"ON_DISK_PREFLIGHT_OBSERVED":
           "NOT_TESTED",title_save_ok,title_save_fail);
    puts("OG_70_SAVE_RELOAD=NOT_PROVED_BY_70_YEAR_TRANSITIONS");
    printf("OG_INTERNATIONAL_NATIONAL=%s national-definitions=%ld "
           "qualifier-definitions=%ld final-definitions=%ld "
           "uefa-clubs=%ld conti-season=%ld worldcup-season-counter=%ld "
           "start-checkpoints=%ld restore-checkpoints=%ld "
           "fixture-continuity=NOT_TESTED\n",
           external_starts||external_restores?
           "OG_DEFINITIONS_AND_SUBSYSTEMS_OBSERVED_NOT_RESIDENT":
           "NOT_TESTED",
           external_national,external_qualifiers,external_finalists,
           external_uefa_clubs,external_conti_season,
           external_worldcup_seasons,external_starts,external_restores);
    for(size_t k=0;k<sizeof(external_names)/sizeof(external_names[0]);++k) {
        printf("OG_EXTERNAL_COMPETITION name=%s result=%s "
               "defined=%ld native-activity=%ld "
               "fixture-proven=NO save-reload-proven=NO\n",
               external_names[k],
               !external_seen[k]?"NOT_TESTED":
               !external_defined[k]?"NOT_DEFINED_IN_WORLD":
               external_activity[k]>0?
                 "COUNTER_ACTIVITY_ONLY_DATED_FIXTURES_NOT_VERIFIED":
                 "DEFINITION_ONLY_NO_RESIDENT_FIXTURE_EVIDENCE",
               external_seen[k]?external_defined[k]:-1,
               external_seen[k]?external_activity[k]:-1);
    }
    printf("OG_OFFER_NATIVE_VIEW=%s calls=%ld failures=%ld\n",
           offer_view_failures?"FAIL":offer_native_views?"READ_ONLY_OBSERVED":"NOT_TESTED",
           offer_native_views,offer_view_failures);
    printf("OG_OFFER_CUPS_SCORERS_BEFORE_AFTER=%s "
           "cup-before=%ld cup-after=%ld goals-before=%ld goals-after=%ld\n",
           offer_before.seen&&offer_after.seen?
             (offer_before.cup_resident==offer_after.cup_resident &&
              offer_before.scorer_goals==offer_after.scorer_goals &&
              offer_before.standings_goals==offer_after.standings_goals?
               "UNCHANGED":"CHANGED"):"NOT_TESTED",
           offer_before.seen?offer_before.cup_resident:-1,
           offer_after.seen?offer_after.cup_resident:-1,
           offer_before.seen?offer_before.scorer_goals:-1,
           offer_after.seen?offer_after.scorer_goals:-1);
    printf("OG_VIEW=OBSERVED_READ_ONLY views=%ld missing=%ld\n",
           views,view_missing);
    printf("OG_MANAGER_BIND=%s events=%ld\n",binds?"OBSERVED":"NOT_TESTED",binds);
    printf("OG_VIEWER_BACK=%s viewer-level-exits=%ld pad-queue-events=%ld\n",
           viewer_back?"LOGGED_NOT_VISUALLY_PROVED":"NOT_TESTED",
           viewer_back,picker_back_events);
    printf("OG_PICKER_BACK=%s nested=%ld popped=%ld\n",
           regressions?"CHECK_LOG":picker_backs?
           picker_backs==picker_pops?"PASS":"FAIL":"NOT_TESTED",
           picker_backs,picker_pops);
    printf("OG_CUP_DEFINITIONS=%ld countries=%ld\n",cup_defs,cup_countries);
    printf("OG_CUP_START_COVERAGE=%s definitions=%ld resident=%ld "
           "unsupported=%ld faults=%ld scope=DOMESTIC_TYPE1_ONLY\n",
           !feature_start.seen?"NOT_TESTED":
           feature_start.cup_definitions>0 &&
           feature_start.cup_definitions==feature_start.cup_resident &&
           feature_start.cup_unsupported==0 &&
           feature_start.cup_faults==0?
           "ALL_SCOPED_CUPS_RESIDENT_AT_START":
           "PARTIAL_OR_INVALID",
           feature_start.seen?feature_start.cup_definitions:-1,
           feature_start.seen?feature_start.cup_resident:-1,
           feature_start.seen?feature_start.cup_unsupported:-1,
           feature_start.seen?feature_start.cup_faults:-1);
    printf("OG_CUP_NATIVE_BYE_EVENTS=OBSERVED verified=%ld rejected=%ld\n",
           cup_native_bye_verified,cup_native_bye_rejected);
    {
        unsigned resident=0,unresident=0,selector_mismatch=0;
        unsigned validatedNativeByes=0u;
        unsigned knownByeRule=0,otherRule=0,matchedRound=0;
        for(unsigned i=0;i<cup_coverage_count;++i) {
            const CupCoverageRecord *c=&cup_coverage[i];
            const int nativeByeOk=!strcmp(c->state,"RESIDENT") &&
                    !strcmp(c->reason,"NATIVE_BYES_VERIFIED");
            const int ok=!strcmp(c->state,"RESIDENT") &&
                         (!strcmp(c->reason,"OK") || nativeByeOk);
            if(ok) {
                ++resident;
                if(nativeByeOk)++validatedNativeByes;
            } else {
                if(!strcmp(c->rule,"OG_BYE_FORMAT_CANDIDATE")) {
                    ++knownByeRule;
                    if(!strcmp(c->round_structure,"OG_ROUND1_MATCHES_RULE"))
                        ++matchedRound;
                } else ++otherRule;
                ++unresident;
                if(c->selected!=c->expected)++selector_mismatch;
            }
        }
        const int coverage_complete=cup_coverage_count>0 &&
            (long)cup_coverage_count==cup_defs && !unresident;
        printf("OG_CUP_COVERAGE=%s scope=DOMESTIC_TYPE1_SLOTS_2_3 "
               "international-cups=NOT_AUDITED "
               "year=%ld definitions=%ld audited=%u "
               "resident=%u native-byes-verified=%u divergent=%u "
               "unverified-selector-mismatches=%u\n",
               coverage_complete?"ALL_SCOPED_DOMESTIC_TYPE1_RESIDENT":
               cup_coverage_count?"PARTIAL_WITH_EXPLICIT_REASONS":
               "NOT_TESTED",
               cup_coverage_year,cup_defs,cup_coverage_count,resident,
               validatedNativeByes,unresident,selector_mismatch);
        printf("OG_CUP_FORMAT_RULES=%s native-bye-candidates=%u "
               "native-round-matched=%u other-divergences=%u "
               "first-round-results=NOT_PROVEN_BY_FORMAT\n",
               cup_coverage_count?"RULES_OBSERVED_NOT_ALL_VERIFIED":
               "NOT_TESTED",knownByeRule,matchedRound,otherRule);
        unsigned incompleteCountries=0u,reportedCountries=0u;
        for(unsigned country=0u;country<256u;++country) {
            unsigned defined=0u,available=0u,verifiedByes=0u;
            for(unsigned i=0;i<cup_coverage_count;++i) {
                const CupCoverageRecord *c=&cup_coverage[i];
                if(c->country!=(long)country)continue;
                ++defined;
                if(!strcmp(c->state,"RESIDENT") &&
                   (!strcmp(c->reason,"OK") ||
                    !strcmp(c->reason,"NATIVE_BYES_VERIFIED"))) {
                    ++available;
                    if(!strcmp(c->reason,"NATIVE_BYES_VERIFIED"))
                        ++verifiedByes;
                }
            }
            if(!defined)continue;
            ++reportedCountries;
            if(defined!=available)++incompleteCountries;
            printf("OG_CUP_COUNTRY year=%ld country=%u definitions=%u "
                   "resident=%u native-byes=%u unavailable=%u state=%s\n",
                   cup_coverage_year,country,defined,available,
                   verifiedByes,defined-available,
                   defined==available?"ALL_RESIDENT":"INCOMPLETE");
        }
        printf("OG_CUP_COUNTRIES=%s expected=%ld audited=%u "
               "incomplete=%u scope=DOMESTIC_TYPE1_ONLY\n",
               !reportedCountries?"NOT_TESTED":
               incompleteCountries || (long)reportedCountries!=cup_countries ||
               (long)cup_coverage_count!=cup_defs ?
               "PARTIAL":"ALL_SCOPED_COUNTRIES_RESIDENT",
               cup_countries,reportedCountries,incompleteCountries);
        for(unsigned i=0;i<cup_coverage_count;++i) {
            const CupCoverageRecord *c=&cup_coverage[i];
            if(!strcmp(c->state,"RESIDENT") &&
               (!strcmp(c->reason,"OK") ||
                !strcmp(c->reason,"NATIVE_BYES_VERIFIED")))continue;
            printf("OG_CUP_DIVERGENCE year=%ld country=%ld slot=%ld "
                   "compId=%ld expected=%ld selected=%ld "
                   "state=%s reason=%s real-entrants=%ld "
                   "first-round-ties=%ld byes=%ld padded=%ld "
                   "native-bracket=%ld native-ties=%ld native-pairs=%ld "
                   "rule=%s round=%s\n",
                   c->year,c->country,c->slot,c->compid,c->expected,
                   c->selected,c->state,c->reason,c->real,c->ties,
                   c->byes,c->padded,c->native_bracket,c->native_ties,
                   c->native_pairs,c->rule[0]?c->rule:"UNINSTRUMENTED",
                   c->round_structure[0]?c->round_structure:"UNINSTRUMENTED");
        }
    }
    printf("OG_CUP_RESIDENT=%s contexts=%ld yearend-callbacks=%ld\n",
           have_audit&&cup_defs>0&&cup_resident==0?
           "MISSING_IMPLEMENTATION":cup_resident>0?
           "OBSERVED_NOT_FULL_PROOF":"NOT_TESTED",cup_resident,yearend);
    printf("OG_SAVE_RELOAD=%s saved=%ld restored=%ld\n",
           career_saves||career_loads?"PARTIAL_ONLY":"NOT_TESTED",
           career_saves,career_loads);
    puts("OG_FULL_RESIDENT_SAVE=NOT_TESTED");
    printf("F12_OG_CAPTURE=main:%ld cups:%ld data:%ld\n",
           f12_main,f12_cups,f10_data);
    printf("OG_NATIVE_LEAGUE_FORMAT=%s unsupported=%ld "
           "last-country=%ld last-division=%ld expected=%ld actual=%ld "
           "scope=REAL_WINDOWS_RUNTIME_ONLY\n",
           native_league_unsupported?"PARTIAL":"NO_UNSUPPORTED_OBSERVED",
           native_league_unsupported,unsupported_country,unsupported_division,
           unsupported_expected,unsupported_actual);
    printf("OG_NATIVE_FAULTS=%ld CHECK_FAILURES=%ld SESSIONS=%ld\n",native_faults,regressions,og_career_sessions);
    if(first[0])printf("FIRST_RUNTIME_FAILURE: %s\n",first);
    if(!correct_build) {
        puts("WINDOWS_OG_INTEGRATION=INCOMPLETE reason=BUILD_MISMATCH");
        puts("OVERALL=INCOMPLETE");
        return 2;
    }
    if(native_faults||regressions||season_failures||offer_view_failures||
       title_save_fail) {
        puts("WINDOWS_OG_INTEGRATION=FAIL");
        puts("OVERALL=FAIL");
        return 1;
    }
    if(!start_ready||!start_audit||!have_audit||leagues<=0) {
        puts("WINDOWS_OG_INTEGRATION=INCOMPLETE reason=NO_OG_START_EVIDENCE");
        puts("OVERALL=INCOMPLETE");
        return 2;
    }
    /* Do not misreport a career with skipped native league formats as
     * a complete world even if the remaining OG competitions advance. */
    if(native_league_unsupported) {
        puts("WINDOWS_OG_INTEGRATION=INCOMPLETE reason=NATIVE_LEAGUE_FORMAT_UNSUPPORTED");
        puts("OVERALL=INCOMPLETE");
        return 2;
    }
    /* Deliberately no green light for the full world until dated, real OG
     * cups and save/reload of their complete states have been proved. */
    puts("WINDOWS_OG_INTEGRATION=INCOMPLETE reason=CUPS_AND_SAVE_PENDING");
    puts("OVERALL=INCOMPLETE");
    return 2;
}
