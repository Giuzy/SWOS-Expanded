/* Portable Windows runtime log inspector, no awk/Python dependency.
 * Build with the SAME MinGW C compiler that builds SWOS Expanded.
 * A headless PASS or an F12 display is not proof of complete OG integration.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define LINE_CAP 8192
static long field(const char *line,const char *name,long fallback) {
    char pattern[80];
    const char *p;
    snprintf(pattern,sizeof pattern,"%s=",name);
    p=strstr(line,pattern);
    if(!p)return fallback;
    p+=strlen(pattern);
    return strtol(p,NULL,10);
}
static void first_error(char *buffer,size_t cap,const char *kind,
                        long row,const char *line) {
    if(buffer[0])return;
    snprintf(buffer,cap,"%s line=%ld %.450s",kind,row,line);
    size_t n=strlen(buffer);
    while(n && (buffer[n-1]=='\n'||buffer[n-1]=='\r'))buffer[--n]=0;
}
static const char *status(int state) {
    return state==2?"FAIL":state==1?"PASS":"NOT_TESTED";
}
int main(int argc,char **argv) {
    FILE *in;
    char line[LINE_CAP],first[600]="",first_clock[600]="";
    long log_count=0,compares=0,failures=0;
    long imports=0,world_total=0,southern_checks=0,cup_registered=0;
    long native_results=0,replacements=0,snapshots=0,clock_events=0;
    long snapshots_main=0,snapshots_cups=0,snapshots_f10=0;
    long cup_callbacks=0,max_world_week=0,format_events=0;
    long julian_jumps=0,julian_jump_errors=0;
    int import_status=0,stand_status=0,handoff_status=0,south_status=0,clock_status=0;
    int has_legacy_clock=0,clock_overdue=0,cup_status=0,build_matched=0;
    unsigned civil_prev=0u;
    unsigned civil_events=0u,civil_advances=0u,civil_rewinds=0u;
    const char *expected=argc>2?argv[2]:"0.13.39_01";
    const char *filename=argc>1?argv[1]:"SWOS.log";
    in=fopen(filename,"rb");
    if(!in) {
        puts("RUNTIME_LOG=NOT_TESTED reason=SWOS.log_missing");
        puts("WINDOWS_INTEGRATION=INCOMPLETE reason=NO_LOG");
        puts("OVERALL=INCOMPLETE");
        return 2;
    }
    while(fgets(line,sizeof line,in)) {
        ++log_count;
        if(strstr(line,"[build] SWOS Expanded ")==line &&
           strstr(line,expected))build_matched=1;
        if(strstr(line,"[world-import-end]")==line) {
            ++imports;
            long total=field(line,"rows",-1);
            long accepted=field(line,"accepted",-1);
            long excluded=field(line,"excluded",-1);
            long errors=field(line,"failures",-1);
            world_total=total;
            if(total<0||accepted<0||excluded<0||
               total!=accepted+excluded||errors!=0||
               !strstr(line,"status=COMPLETE")) {
                import_status=2;
                first_error(first,sizeof first,"WORLD_IMPORT",log_count,line);
            } else if(import_status==0)import_status=1;
        }
        if(strstr(line,"[join-standings] FAIL")==line) {
            ++compares;++failures;stand_status=2;
            first_error(first,sizeof first,"OG_LWE_STANDINGS",log_count,line);
        } else if(strstr(line,"[join-standings] PASS")==line) {
            ++compares;if(!stand_status)stand_status=1;
        }
        if(strstr(line,"[midseason] FAIL EARLY_HANDOFF")==line) {
            handoff_status=2;
            first_error(first,sizeof first,"EARLY_HANDOFF",log_count,line);
        }
        if(strstr(line,"[clock-audit] event=JOIN_FINISH result=REACHED")==line &&
           handoff_status==0)handoff_status=1;
        if(strstr(line,"[midseason-southern] FAIL")==line) {
            south_status=2;
            first_error(first,sizeof first,"SOUTHERN_HANDOFF",log_count,line);
        } else if(strstr(line,"[midseason-southern] PASS")==line) {
            ++southern_checks;if(south_status==0)south_status=1;
        }
        /* Career civil clock monotonicity; native "next OG event" is
         * intentionally NOT a second civil date. Reset at career creation
         * and when a different .CAR snapshot is loaded. */
        if(strstr(line,"[clock-trace] RESET new-career")==line ||
           strstr(line,"[living-world] restored .CAR snapshot")==line)
            civil_prev=0u;
        if(strstr(line,"[clock-trace]")==line) {
            const char *clock=strstr(line,"civil=");
            unsigned cy=0u,cm=0u,cd=0u;
            if(clock && sscanf(clock,"civil=%4u-%2u-%2u",&cy,&cm,&cd)==3 &&
               cy>=1900u && cy<=2200u && cm>=1u && cm<=12u &&
               cd>=1u && cd<=31u) {
                const unsigned key=cy*10000u+cm*100u+cd;
                ++civil_events;
                if(civil_prev && key<civil_prev) {
                    ++civil_rewinds;
                    first_error(first_clock,sizeof first_clock,
                                "CIVIL_DATE_REWIND",log_count,line);
                    first_error(first,sizeof first,
                                "CIVIL_DATE_REWIND",log_count,line);
                }
                if(civil_prev && key>civil_prev) ++civil_advances;
                civil_prev=key;
            }
        }
        if(strstr(line,"[clock-trace-first-fail]")==line ||
           strstr(line,"[clock-date-audit] FAIL")==line) {
            clock_status=2;
            first_error(first_clock,sizeof first_clock,"CLOCK",log_count,line);
            first_error(first,sizeof first,"CLOCK",log_count,line);
        } else if(strstr(line,"[clock-trace]")==line) {
            ++clock_events;
            if(strstr(line,"state=POST_HANDOFF")&&clock_status==0)
                clock_status=1;
        }
        if(strstr(line,"[calendar-five-year]")==line) {
            long week=field(line,"absolute-week",0);
            if(week>max_world_week)max_world_week=week;
            ++julian_jumps;
            if(strstr(line,"FAIL")) {
                ++julian_jump_errors;
                first_error(first,sizeof first,"FIVE_YEAR_DATE",log_count,line);
            }
        }
        if(strstr(line,"[f12-snapshot]")==line) {
            ++snapshots;
            if(strstr(line,"page=F12_MAIN"))++snapshots_main;
            if(strstr(line,"page=F12_CUPS"))++snapshots_cups;
            if(strstr(line,"page=F10_IMPORT"))++snapshots_f10;
        }
        if(strstr(line,"[competition-format-runtime]")==line)++format_events;
        if(strstr(line,"[midseason-southern] INSPECT")==line) {
            long week=field(line,"week",0);
            if(week>max_world_week)max_world_week=week;
        }
        if(strstr(line,"[living-world] OG league result:")==line)++native_results;
        if(strstr(line,"[living-world] OG final replaced world score")==line)
            ++replacements;
        if(strstr(line,"[world-cup-load-audit]")==line) {
            long registered=field(line,"registered",0);
            if(registered>cup_registered)cup_registered=registered;
        }
        if(strstr(line,"[og-cups-audit]")==line)
            cup_callbacks+=field(line,"callbacks",0);
        if(strstr(line,"[living-world] wait=")==line) {
            long week=field(line,"wait",0);
            if(week>max_world_week)max_world_week=week;
        }
        if(strstr(line,"source=F12_CALENDAR_SAMPLE"))has_legacy_clock=1;
        if(strstr(line,"OG_NEXT_EVENT_OVERDUE"))clock_overdue=1;
        if(strstr(line,"[og-cups-skip]")==line) {
            long y=field(line,"finished",0);
            if(y*52>max_world_week)max_world_week=y*52;
        }
    }
    fclose(in);
    if(clock_overdue && clock_status!=2)clock_status=2;
    if(cup_registered>0 && cup_callbacks==0 && max_world_week>=52) {
        cup_status=2;
        first_error(first,sizeof first,"OG_CUPS_NOT_ADVANCED",
                    log_count,"registered cup definitions but no observed OG callbacks");
    } else if(cup_callbacks>0)cup_status=1; /* callbacks only, not all cup formats */
    printf("RUNTIME_LOG=READ lines=%ld expected=%s scope=%s\n",
           log_count,expected,build_matched?"CURRENT_BUILD":"OTHER_BUILD");
    printf("RUNTIME_WORLD_IMPORT=%s attempts=%ld rows=%ld\n",
           status(import_status),imports,world_total);
    printf("RUNTIME_OG_LWE_STANDINGS=%s comparisons=%ld failures=%ld\n",
           status(stand_status),compares,failures);
    printf("RUNTIME_OG_HANDOFF=%s (separate from table comparison)\n",
           status(handoff_status));
    printf("RUNTIME_OG_LWE_CLOCK=%s (upcoming fixture is NOT the civil date)\n",
           status(clock_status));
    printf("RUNTIME_CLOCK_TRACE=%s events=%ld legacy-sample=%d\n",
           clock_status==2?"FAIL":clock_status==1?"OBSERVED_NOT_PROOF":"NOT_TESTED",
           clock_events,has_legacy_clock);
    printf("RUNTIME_SOUTHERN_HANDOFF=%s strict-verified=%ld\n",
           status(south_status),southern_checks);
    printf("RUNTIME_FIVE_YEAR_ANNIVERSARY=%s checks=%ld failures=%ld\n",
           julian_jump_errors?"FAIL":julian_jumps?"DATE_ONLY_NOT_INTEGRATION":"NOT_TESTED",
           julian_jumps,julian_jump_errors);
    printf("RUNTIME_CIVIL_SEQUENCE=%s observations=%u advances=%u rewinds=%u\n",
           civil_rewinds?"FAIL":civil_advances?"OBSERVED_MONOTONIC_NOT_PROOF":
           "NOT_TESTED",civil_events,civil_advances,civil_rewinds);
    puts("RUNTIME_30_YEAR_CALENDAR_SAVE_LOAD=NOT_TESTED_IN_WINDOWS");
    printf("RUNTIME_OG_CUPS=%s registered=%ld callbacks=%ld\n",
           cup_status==1?"OBSERVED_CALLBACKS_NOT_ALL_CUPS_PROVEN":
           status(cup_status),cup_registered,cup_callbacks);
    printf("F12_LOG_SNAPSHOTS=%ld main=%ld cups=%ld f10=%ld\n",
           snapshots,snapshots_main,snapshots_cups,snapshots_f10);
    printf("RUNTIME_NATIVE_RESULTS=%ld WORLD_SCORE_REPLACEMENTS=%ld\n",
           native_results,replacements);
    printf("RUNTIME_FORMATS_OBSERVED=%ld (not a functionality test)\n",
           format_events);
    if(first_clock[0])printf("FIRST_CLOCK_FAILURE: %s\n",first_clock);
    if(first[0])printf("FIRST_RUNTIME_FAILURE: %s\n",first);
    if(!build_matched) {
        puts("WINDOWS_INTEGRATION=INCOMPLETE reason=WRONG_BUILD");
        puts("OVERALL=INCOMPLETE");
        return 2;
    }
    if(import_status==2 || stand_status==2 || handoff_status==2 || south_status==2 ||
       clock_status==2 || cup_status==2 || julian_jump_errors ||
       civil_rewinds) {
        puts("WINDOWS_INTEGRATION=FAIL");
        puts("OVERALL=FAIL");
        return 1;
    }
    puts("WINDOWS_INTEGRATION=INCOMPLETE reason=CALENDAR_LEAGUES_CUPS_NOT_ALL_VERIFIED");
    puts("OVERALL=INCOMPLETE");
    return 2;
}
