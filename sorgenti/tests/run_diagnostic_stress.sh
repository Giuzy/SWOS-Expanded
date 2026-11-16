#!/usr/bin/env bash
# This is an observation of the CURRENT Windows game's 70-year run,
# never a headless LWE stress test or a second world simulation.
set -uo pipefail
cd "$(dirname "$0")/../.."
if [[ -z "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
  echo 'OG_70=NOT_TESTED reason=NO_SESSION'
  exit 2
fi
out="${SWOS_DIAGNOSTIC_DIR}"
if command -v cygpath >/dev/null 2>&1; then out="$(cygpath -u "$out")"; fi
log="$out/SWOS.log"
if [[ ! -s "$log" ]]; then
  echo 'OG_70=NOT_TESTED reason=NO_CURRENT_SESSION_LOG'
  exit 2
fi
if grep -Fq '[og-stress] FAILED' "$log"; then
  echo 'OG_70=FAIL reason=OG_STRESS_FAILED'
  exit 1
fi
if grep -Fq '[og-stress] FINISHED years=70 ' "$log"; then
  echo 'OG_70=OBSERVED_YEAR_ADVANCE_ONLY note=NO_SAVE_OR_MANAGER_PROOF'
  exit 0
fi
echo 'OG_70=NOT_TESTED reason=F12_STRESS_NOT_FINISHED_IN_THIS_SESSION'
exit 2
