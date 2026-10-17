#!/bin/sh
# Regenerate shims/icuuc/icuuc.def from a Proton icu.dll.
# Usage: tools/gen-icuuc.sh <proton>/files/lib/wine/x86_64-windows/icu.dll
set -eu
[ $# -eq 1 ] || { echo "usage: $0 <path to Proton's icu.dll>" >&2; exit 1; }
out="$(dirname "$0")/../shims/icuuc/icuuc.def"
{
    echo "LIBRARY icuuc.dll"
    echo "EXPORTS"
    objdump -p "$1" \
        | awk '/\[Ordinal\/Name Pointer\] Table/{f=1;next} f && /^$/{exit} f && /\[/{print $NF}' \
        | grep -E '^[A-Za-z_][A-Za-z0-9_]*$' | sort -u | sed 's/.*/  & = icu.&/'
} > "$out"
echo "$(grep -c ' = icu\.' "$out") forwarders written to $out"
