#!/usr/bin/env bash
# Single-session Windows OG log audit. No game simulation, no save writes.
set -uo pipefail
cd "$(dirname "$0")/../.."
if [[ -z "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
  echo 'OG_RUNTIME=NOT_TESTED reason=NO_DIAGNOSTIC_SESSION'
  exit 2
fi
out="${SWOS_DIAGNOSTIC_DIR}"
if command -v cygpath >/dev/null 2>&1; then out="$(cygpath -u "$out")"; fi
log="$out/SWOS.log"
if [[ ! -s "$log" ]]; then
  echo 'OG_RUNTIME=NOT_TESTED reason=NO_CURRENT_SESSION_LOG'
  exit 2
fi
# Parser version is read from the checked-out source, not guessed from an old log.
bash sorgenti/tests/audit_windows_runtime_log.sh "$log" > "$out/SWOS_RUNTIME_AUDIT.log" 2>&1
result=$?
cat "$out/SWOS_RUNTIME_AUDIT.log"
exit "$result"
