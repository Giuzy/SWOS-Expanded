#!/usr/bin/env bash
# Cup/scorer evidence from the actual Windows run only, never static proof.
set -uo pipefail
cd "$(dirname "$0")/../.."
if [[ -z "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
  echo 'OG_CUPS_SCORERS=NOT_TESTED reason=NO_SESSION'
  exit 2
fi
out="${SWOS_DIAGNOSTIC_DIR}"
if command -v cygpath >/dev/null 2>&1; then out="$(cygpath -u "$out")"; fi
log="$out/SWOS.log"
if [[ ! -s "$log" ]]; then
  echo 'OG_CUPS_SCORERS=NOT_TESTED reason=NO_CURRENT_SESSION_LOG'
  exit 2
fi
if grep -Eq '\[og-cup-resident\] (ERROR|FAILED)' "$log"; then
  echo 'OG_CUPS_SCORERS=FAIL reason=NATIVE_CUP_FAULT'
  exit 1
fi
if ! grep -Fq '[og-feature-audit] stage=START ' "$log"; then
  echo 'OG_CUPS_SCORERS=NOT_TESTED reason=NO_NATIVE_OG_START'
  exit 2
fi
grep -F '[og-feature-audit] stage=START ' "$log" | tail -1
grep -F '[og-intl-seed] ' "$log" || :
echo 'OG_CUPS_SCORERS=PARTIAL_NATIVE_START_OBSERVED'
echo 'OG_INTERNATIONAL_FIXTURES=NOT_VERIFIED_BY_SEED_LOG'
echo 'OG_CUP_PLAYER_GOALS_SAVE_RELOAD=NOT_VERIFIED'
exit 2
