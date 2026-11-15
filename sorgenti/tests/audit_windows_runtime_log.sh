#!/usr/bin/env bash
# Compatibility wrapper. The OG C parser is the single log audit.
set -euo pipefail
log="SWOS.log"
if (( $# >= 1 )); then log="$1"; fi
cd "$(dirname "$0")/../.."
expected=""
if (( $# >= 2 )); then expected="$2"; fi
if [[ -z "$expected" ]]; then
  expected="$(sed -n 's/^#define SWOS_EXPANDED_VERSION "\([^"]*\)".*/\1/p' sorgenti/include/build_info.h)"
fi
dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT
gcc -std=c11 -O2 -Wall -Wextra -Werror \
    sorgenti/tests/audit_windows_runtime_log.c -o "$dir/og_runtime_audit"
"$dir/og_runtime_audit" "$log" "$expected"
