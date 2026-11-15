#!/usr/bin/env bash
# Read the EXISTING Windows compiler log, never compile game here.
# Keep all warnings visible; no -Wno suppression and no fake PASS.
set -uo pipefail
cd "$(dirname "$0")/../.."
if [[ -n "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
    log="$(cygpath -u "$SWOS_DIAGNOSTIC_DIR")/COMPILAZIONE_0140.log"
else
    log="COMPILAZIONE_0140.log"
fi
if [[ ! -f "$log" ]]; then
    echo "COMPILER_WARNING_AUDIT=NOT_TESTED reason=CURRENT_BUILD_LOG_MISSING"
    exit 2
fi
grep 'warning:' "$log" || true
total=$(grep -c 'warning:' "$log" || true)
packed=$(grep -c 'warning:.*-Waddress-of-packed-member' "$log" || true)
indent=$(grep -c 'warning:.*-Wmisleading-indentation' "$log" || true)
braces=$(grep -c 'warning:.*-Wmissing-braces' "$log" || true)
other=$((total-packed-indent-braces))
echo "COMPILER_WARNINGS=OBSERVED total=$total packed-alignment=$packed misleading-indentation=$indent missing-braces=$braces other=$other"
if ((packed>0));then
 echo "WARNING_REVIEW=PACKED_POINTERS possible-unaligned-access priority=SAVE_AND_COMPETITION_STRUCTS result=REVIEW_REQUIRED"
fi
if ((indent>0));then
 echo "WARNING_REVIEW=MISLEADING_INDENTATION possible-control-flow-ambiguity result=REVIEW_REQUIRED"
fi
if ((braces>0));then
 echo "WARNING_REVIEW=MISSING_BRACES initializer-review result=REVIEW_REQUIRED"
fi
echo "WARNING_POLICY=NO_SUPPRESSION NO_AUTOMATIC_FIX NO_CLAIM_OF_ZERO_WARNINGS"
