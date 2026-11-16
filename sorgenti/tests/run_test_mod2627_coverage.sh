#!/usr/bin/env bash
# SWOS Expanded World 2026/27: official v4.1.2 inventory vs live source.
# Only data/identity claims are PASS. No pre-generated results or simulation.
set -euo pipefail
cd "$(dirname "$0")/../.."
manifest="docs/reference/mod2627_4_1_2_competitions.tsv"
countries="docs/reference/mod2627_4_1_2_countries.tsv"
supercups="docs/reference/mod2627_4_1_2_supercups.tsv"
leagues="data_json/worlds/2627/competitions/leagues.json"
world="sorgenti/src/world_data.c"
career="sorgenti/src/career_init.c"
for file in "$manifest" "$countries" "$supercups" "$leagues" "$world" "$career"; do
  if [[ ! -s "$file" ]]; then
    echo "MOD2627_COVERAGE=FAIL missing=$file"
    exit 1
  fi
done

# Names are a frozen snapshot of the original 4.1.2 release, not generated
# dynamically from the current Expanded source: a future missing cup will fail.
awk -F '\t' '
  /^#/ {next}
  NF!=3 {bad=1;next}
  {key=$1 SUBSEP $2 SUBSEP $3;if(seen[key]++)bad=1;counts[$1]++;rows++;
   if($1=="LEAGUE"||$1=="CUP"||$1=="LEAGUE_CUP")country[$2]=1}
  END {
    if(rows!=264||counts["LEAGUE"]!=134||counts["CUP"]!=87||
       counts["LEAGUE_CUP"]!=14||counts["STATE"]!=13||
       counts["CUPD"]!=1||counts["UEFA"]!=3||
       counts["CONTINENTAL"]!=12)bad=1;
    for(i in country)nc++;
    if(nc!=91)bad=1;
    print "MOD2627_REFERENCE countries=" nc " competitions=" rows       " leagues=" counts["LEAGUE"] " cups=" counts["CUP"]+counts["LEAGUE_CUP"]       " states=" counts["STATE"] " continentals=" counts["UEFA"]+counts["CONTINENTAL"];
    exit bad?1:0;
  }' "$manifest" || { echo 'MOD2627_REFERENCE=FAIL';exit 1; }
awk -F '\t' '
  /^#/ {next}
  NF!=2 {bad=1;next}
  {n++}
  END {print "MOD2627_SUPERCUPS=REFERENCE_ONLY count=" n;exit n==10&&!bad?0:1}
' "$supercups" || { echo "MOD2627_SUPERCUPS=FAIL";exit 1; }

# Compare every individual country ID and every division count, including
# the 14 Italian groups. No Python, third-party parser or new build tool.
awk -F '\t' '
  FNR==NR {
    if($0~/^#/)next;
    if(NF!=4||$1!~/^[0-9]+$/)bad=1;
    if($1 in expect)bad=1;
    expect[$1]=$3+0;wantCup[$1]=$4+0;need++;
    next;
  }
  /"team_number"[[:space:]]*:/ {
    line=$0;sub(/^.*"team_number"[[:space:]]*:[[:space:]]*/,"",line);
    sub(/[^0-9].*$/,"",line);id=line+0
  }
  /"divisions_count"[[:space:]]*:/ {
    line=$0;sub(/^.*"divisions_count"[[:space:]]*:[[:space:]]*/,"",line);
    sub(/[^0-9].*$/,"",line);n=line+0;
    if(!(id in expect)||seen[id]++||expect[id]!=n){
      print "MOD2627_MISSING_OR_CHANGED_COUNTRY=" id " divisions=" n " expected=" expect[id];
      bad=1;
    }
    total+=n;found++;
  }
  END {
    for(id in expect)if(!(id in seen)){
      print "MOD2627_MISSING_COUNTRY_ID=" id;bad=1;
    }
    print "MOD2627_LEAGUES_BY_ID countries=" found " divisions=" total       " expected_countries=" need;
    if(bad||found!=91||need!=91||total!=134)exit 1;
  }' "$countries" "$leagues" || { echo 'MOD2627_DIVISIONS=FAIL';exit 1; }

# Confirm OG world has all 12 named non-UEFA competitions and three
# league-phase UEFA definitions (not proof of playable resident fixtures).
while IFS=$'\t' read -r kind region name; do
  name="${name%$'\r'}"
  [[ "$kind" == "CONTINENTAL" ]] || continue
  if ! grep -Fq -- "\"$name\"" "$world"; then
    echo "MOD2627_MISSING_CONTINENTAL=$name";exit 1
  fi
done < "$manifest"
for name in CHAMPIONS_LEAGUE EUROPA_LEAGUE CONFERENCE_LEAGUE; do
  grep -Fq "\"${name}\"" "$career" || { echo "MOD2627_MISSING_UEFA=$name";exit 1; }
done
grep -Fq 'kWorld2627ContiCups[12]' "$world" || { echo 'MOD2627_OTHER_CUPS=FAIL';exit 1; }
grep -Fq 'kWorld2627UefaClubs' "$world" || { echo 'MOD2627_UEFA_CLUBS=FAIL';exit 1; }
grep -Eq 'kWorld2627States46[[:space:]]*=[[:space:]]*\{[^}]*,[[:space:]]*13,' "$world" || { echo 'MOD2627_BRAZIL_STATES=FAIL';exit 1; }
grep -Fq '"COPPA ITALIA SERIE D"' "$world" || { echo 'MOD2627_COPPA_SERIE_D=FAIL';exit 1; }
echo 'MOD2627_DEFINITIONS=PASS leagues=134 domestic_cups=101 extra=15 states=13 serie_d=1'
echo 'MOD2627_INTERNATIONAL_FIXTURES=NOT_VERIFIED definitions_do_not_equal_native_residency'
echo 'MOD2627_SUPERCUP_FIXTURES=NOT_VERIFIED reference=10'
echo 'MOD2627_OFFICIAL_COUNT_DIFFERENCE=OPEN marketed=260 technical=264'

# Only use the current session if available. Never promote old logs to a
# PASS for a newly built EXE. All 87 cup-bearing countries must agree.
log="${MOD2627_RUNTIME_LOG:-}"
if [[ -z "$log" && -n "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
    if command -v cygpath >/dev/null 2>&1; then
      log="$(cygpath -u "$SWOS_DIAGNOSTIC_DIR")/SWOS.log"
    else
      log="$SWOS_DIAGNOSTIC_DIR/SWOS.log"
    fi
fi
if [[ -n "$log" && -s "$log" ]] && grep -Fq '[og-cup-start] year=2026 ' "$log"; then
  awk -F '\t' '
    FNR==NR {if($0~/^#/)next;expect[$1]=$4+0;next}
    /\[og-cup-country\] year=2026 / {
      c=-1;d=-1;r=-1;f=-1;
      n=split($0,a," ");
      for(i=1;i<=n;i++){
        split(a[i],b,"=");
        if(b[1]=="country")c=b[2]+0;
        if(b[1]=="definitions")d=b[2]+0;
        if(b[1]=="resident")r=b[2]+0;
        if(b[1]=="failed")f=b[2]+0;
      }
      if(c<0||!(c in expect)||expect[c]!=d||r!=d||f!=0){
        print "MOD2627_CUP_COUNTRY_FAIL id=" c " expected=" expect[c] " got=" d " resident=" r;
        bad=1;
      }
      if(c>=0)seen[c]=1;
    }
    END {
      for(c in expect)if(expect[c]>0 && !(c in seen)){bad=1;print "MOD2627_CUP_COUNTRY_MISSING=" c;}
      print "MOD2627_RUNTIME_DOMESTIC_COUNTRIES=" length(seen) " status=" (bad?"FAIL":"OBSERVED");
      if(bad)exit 1;
    }' "$countries" "$log" || { echo "MOD2627_RUNTIME_DOMESTIC=FAIL";exit 1; }
  echo 'MOD2627_RUNTIME_DOMESTIC=OBSERVED_NATIVE_IDENTITY_NOT_ALL_FIXTURE_RULES'
else
  echo 'MOD2627_RUNTIME_DOMESTIC=NOT_TESTED no_2026_START_log_in_this_session'
fi
echo 'MOD2627_COVERAGE=PASS_STATIC_ONLY'
