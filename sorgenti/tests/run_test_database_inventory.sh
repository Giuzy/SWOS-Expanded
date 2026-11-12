#!/usr/bin/env bash
# Inventory of the installed real 1996/97 data files. This is NOT the
# standalone synthetic Living World fixture stress, nor an OG career run.
set -euo pipefail
cd "$(dirname "$0")/../.."
shopt -s nullglob
native=(data/TEAM.[0-9][0-9][0-9])
json=(data_json/teams/*.json)
if (( ${#native[@]}==0 || ${#json[@]}==0 )); then
    echo "DB_INVENTORY=FAIL (missing TEAM.* files or JSON reference catalogue)"
    exit 1
fi
native_bytes=0
for file in "${native[@]}"; do
    if [[ ! -s "$file" ]]; then
        echo "DB_INVENTORY=FAIL (empty native file $file)"
        exit 1
    fi
    size=$(wc -c <"$file")
    native_bytes=$((native_bytes+size))
done
json_clubs=0
for file in "${json[@]}"; do
    count=$(awk -F: '/^[[:space:]]*"count"[[:space:]]*:/ { gsub(/[^0-9]/,"",$2); print $2; exit }' "$file")
    if [[ ! "$count" =~ ^[0-9]+$ ]]; then
        echo "DB_INVENTORY=FAIL (count missing from $file)"
        exit 1
    fi
    json_clubs=$((json_clubs+count))
done
echo "REAL_NATIVE_TEAM_FILES=${#native[@]}"
echo "REAL_NATIVE_TEAM_BYTES=$native_bytes"
echo "REAL_REFERENCE_JSON_FILES=${#json[@]}"
echo "REAL_REFERENCE_JSON_CLUB_RECORDS=$json_clubs"
echo "DB_INVENTORY=PASS"
echo "[NOTE] JSON count is a reference inventory. REAL imported clubs/leagues"
echo "       appear in SWOS.log / F12 after loading a career on Windows."
echo "       This inventory does NOT prove that all countries are simulated."
