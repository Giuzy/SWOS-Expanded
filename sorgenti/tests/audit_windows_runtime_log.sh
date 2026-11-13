#!/usr/bin/env bash
# Audit actual Windows gameplay, NOT the isolated C simulation.
set -euo pipefail
log="${1:-SWOS.log}"
expected="${2:-0.13.38_04}"
if [[ ! -f "$log" ]]; then
    printf 'RUNTIME_LOG=NOT_TESTED reason=SWOS.log_missing\n'
    printf 'RUNTIME_OG_LWE_STANDINGS=NOT_TESTED\nRUNTIME_OG_LWE_CLOCK=NOT_TESTED\n'
    printf 'RUNTIME_SOUTHERN_HANDOFF=NOT_TESTED\nRUNTIME_OG_CUPS=NOT_TESTED\nRUNTIME_CLOCK_TRACE=NOT_TESTED\n'
    exit 0
fi
awk -v expected="$expected" '
function field(line,name,  a,n,i,prefix) {
    n=split(line,a,/ +/); prefix=name "=";
    for(i=1;i<=n;i++)if(index(a[i],prefix)==1)return substr(a[i],length(prefix)+1);
    return "";
}
function first_problem(kind,line) {
    if(first=="")first=kind " line=" NR " " line;
}
BEGIN { import="NOT_TESTED"; standings="NOT_TESTED"; southern="NOT_TESTED";
        clock="NOT_TESTED"; clock_trace="NOT_TESTED"; cups="NOT_TESTED"; stage="";
        cup_callbacks=0; cup_registered=0;
        handoffs=0; failures=0; comparisons=0; matched_version=0;
        win_build="unknown"; first=""; delta=""; date="";
        listed_weeks=0; max_world_week=0; log_lines=0;
}
{
    log_lines++;
    if(index($0,"[build] SWOS Expanded ")==1) {
        win_build=$4;
        if(win_build==expected)matched_version=1;
    }
    if(index($0,"[world-import-end]")==1) {
        observed=field($0,"rows")+0; accounted=(field($0,"accepted")+0)+(field($0,"excluded")+0);
        import=(field($0,"status")=="COMPLETE" && observed==accounted && (field($0,"failures")+0)==0)?"PASS":"FAIL";
        if(import=="FAIL") first_problem("WORLD_IMPORT",$0);
    }
    if(index($0,"[world-cup-load-audit]")==1)
        cup_registered=field($0,"registered")+0;
    if(index($0,"[join-standings] FAIL")==1) {
        standings="FAIL"; failures++; comparisons++;
        first_problem("OG_LWE_STANDINGS",$0);
    } else if(index($0,"[join-standings] PASS")==1) {
        comparisons++;
        if(standings!="FAIL")standings="PASS";
    }
    if(index($0,"[midseason-southern] FAIL")==1) {
        southern="FAIL";first_problem("SOUTHERN_HANDOFF",$0);
    } else if(index($0,"[midseason-southern] PASS")==1) {
        if(southern!="FAIL")southern="PASS";
        handoffs++;
    }
    if(index($0,"[clock-trace-first-fail]")==1) {
        clock_trace="FAIL";first_problem("CLOCK_TRACE",$0);
        clock_trace_first=$0;
    } else if(index($0,"[clock-trace]")==1) {
        clock_events++;
        if(index($0,"state=POST_HANDOFF")>0 &&
           clock_trace!="FAIL")clock_trace="OBSERVED_NEAR_NOT_PROOF";
    }
    if(index($0,"[clock-audit]")>0 && index($0,"source=F12_CALENDAR_SAMPLE")>0) {
        if(index($0,"days-delta=")>0) {
            delta=field($0,"days-delta")+0;
            date=field($0,"OG");
            if(delta < -31 || delta > 31) {
                clock="FAIL";first_problem("OG_LWE_CLOCK",$0);
            } else if(clock!="FAIL")clock="SAMPLE_NEAR_NOT_PROOF";
        }
    }
    if(index($0,"[living-world] wait=")==1) {
        w=field($0,"wait")+0;
        if(w>max_world_week)max_world_week=w;
    }
    if(index($0,"[og-cups-audit]")==1) {
        cup_callbacks+=field($0,"callbacks")+0;
        if((field($0,"callbacks")+0)>0) cups="OBSERVED_CALLBACKS_NOT_FULL_PROOF";
    }
    if(index($0,"[og-cups-skip]")==1) {
        listed_weeks=field($0,"finished")+0;
        stage=field($0,"reason");
    }
    if(index($0,"[living-world] OG league result:")==1)native_game_updates++;
    if(index($0,"[living-world] OG final replaced world score")==1)world_score_replacements++;
    if(index($0,"[cup-conflict]")==1)native_cup_conflicts++;
}
END {
    if(cup_registered>0 && cup_callbacks==0 && (listed_weeks>0 || max_world_week>=52)) {
        cups="FAIL";
        first_problem("OG_CUPS","no witnessed cup callbacks; max-world-week=" max_world_week " native-skipped-seasons=" listed_weeks " last-skip=" stage);
    }
    printf "RUNTIME_LOG=READ lines=%d build=%s expected=%s scope=%s\n",log_lines,win_build,expected,matched_version?"CURRENT_BUILD":"OTHER_BUILD_NOT_CERTIFIED";
    printf "RUNTIME_WORLD_IMPORT=%s\n",import;
    printf "RUNTIME_OG_LWE_STANDINGS=%s comparisons=%d failures=%d\n",standings,comparisons,failures;
    printf "RUNTIME_OG_LWE_CLOCK=%s last-delta-days=%s last-og-date=%s\n",clock,delta,date;
    printf "RUNTIME_CLOCK_TRACE=%s events=%d\n",clock_trace,clock_events;
    if(clock_trace_first!="")printf "FIRST_CLOCK_TRACE_FAILURE: %s\n",clock_trace_first;
    printf "RUNTIME_SOUTHERN_HANDOFF=%s verified=%d\n",southern,handoffs;
    printf "RUNTIME_OG_CUPS=%s registered=%d callbacks=%d skipped_seasons=%d max-world-week=%d\n",cups,cup_registered,cup_callbacks,listed_weeks,max_world_week;
    printf "RUNTIME_NATIVE_RESULTS=%d WORLD_SCORE_REPLACEMENTS=%d\n",native_game_updates,world_score_replacements;
    printf "RUNTIME_CUP_CONFLICTS=%d (intentional test rejections are separate)\n",native_cup_conflicts;
    if(first!="") printf "FIRST_RUNTIME_FAILURE: %s\n",first;
    if(!matched_version) {
        print "RUNTIME_VERDICT=NOT_TESTED_FOR_CURRENT_BUILD (log from another version)";
        exit 0;
    }
    if(import=="FAIL" || standings=="FAIL" || southern=="FAIL" || clock=="FAIL" || clock_trace=="FAIL" || cups=="FAIL")exit 1;
}
' "$log"
